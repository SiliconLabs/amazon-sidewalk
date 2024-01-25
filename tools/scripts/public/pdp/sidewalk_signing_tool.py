# Copyright 2019-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
#
# AMAZON PROPRIETARY/CONFIDENTIAL
#
# You may not use this file except in compliance with the terms and
# conditions set forth in the accompanying LICENSE.TXT file.
#
# THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
# DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.

import base64
import argparse
import sys
import json
import binascii
import logging
import time
from os import path
from enum import IntEnum, auto
from collections import defaultdict
from packaging import version

from yubihsm import YubiHsm
from yubihsm.exceptions import YubiHsmConnectionError, YubiHsmAuthenticationError, YubiHsmDeviceError
from yubihsm.objects import AsymmetricKey, ObjectInfo
from yubihsm.defs import ALGORITHM, CAPABILITY, OBJECT, ERROR
from yubihsm.backends.http import HttpBackend

from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.asymmetric import ed25519
from cryptography.hazmat.primitives.asymmetric.utils import encode_dss_signature, decode_dss_signature
from cryptography.exceptions import InvalidSignature

import ssl
from urllib3.poolmanager import PoolManager
from requests.adapters import HTTPAdapter

__version__ = "1.1.5"

SMSN_LEN = 32
SERIAL_LEN = 4
ED25519_PUBK_LEN = 32
P256R1_PUBK_LEN = 64
ED25519_SIG_LEN = 64
P256R1_SIG_LEN = 64
SIDEWALK_AUTH_KEY_INDEX  = 5
SIDEWALK_AUTH_KEY_INDEX_PREPROD = 6
CHAIN_DEPTH_CTL_LABEL = "SW_CTL_CHAIN_DEPTH"
HSM_INFO_LABEL = "HSM_INFO"

class MissingCertificateObject(Exception):
    pass

class NAMESPACE(IntEnum):
    """Each namespace can store 16 objects
    Namespace Control has auth keys and flags
    Namespace for cert has 4 certificates for EDDSA, EDDSA-test, ECDSA, ECDSA-test,
    4 objects in each
    """
    CONTROL = 0x0
    CERT_START = 0x10
    AMAZON = 0x10
    SIDEWALK = 0x20
    MANU = 0x30
    MANU_LEGACY = 0x20
    CERT_END = 0xf0

class CURVE(IntEnum):
    ED25519 = 0x0
    P256R1 = 0x1

class STAGE(IntEnum):
    PROD = 0x0
    TEST = 0x1
    PREPROD = 0x02

class ELEMENT(IntEnum):
    PRIV = 0x0
    PUBK = 0x1
    SIGNATURE = 0x2
    SERIAL = 0x3

class CA_TYPE(IntEnum):
    AMZN = auto()
    SIDEWALK = auto()
    MANU = auto()
    PROD = auto()
    DAK = auto()
    DEVICE = auto()
    MODEL = auto()

class HSM_VERSION(IntEnum):
    LEGACY = 0
    LONG_CHAIN = 1
    PREPROD = 2

class SidewalkCert:
    def __init__(self, type, curve, serial, pubk, signature):
        if curve == CURVE.ED25519:
            if len(pubk) != ED25519_PUBK_LEN or len(signature) != ED25519_SIG_LEN:
                raise ValueError("Invalid length of public key(%d) or signature(%d) of ed25516" % (len(pubk), len(signature)))
        if curve == CURVE.P256R1:
            if len(pubk) != P256R1_PUBK_LEN or len(signature) != P256R1_SIG_LEN:
                raise ValueError("Invalid length of public key(%d) or signature(%d) of p256r1" % (len(pubk), len(signature)))
        if type != CA_TYPE.DEVICE and len(serial) != SERIAL_LEN:
            raise ValueError("Invalid lengh of serial: %d" % len(serial))

        self.type = type
        self.curve = curve
        self.serial = serial
        self.signature = signature
        self.pubk = pubk

    def verify(self, cert_to_verify):
        assert self.curve == cert_to_verify.curve

        if self.curve == CURVE.ED25519:
            verify_with_sig_ed25519(self.pubk, cert_to_verify.signature,
                                    cert_to_verify.pubk + cert_to_verify.serial)

        elif self.curve == CURVE.P256R1:
            verify_with_sig_p256r1(self.pubk, cert_to_verify.signature,
                                   cert_to_verify.pubk + cert_to_verify.serial)


class SidewalkCertChain(list):
    def validate(self):
        for i in range(len(self)):
            issuer = self[0] if i == 0 else self[i-1]
            cert = self[i]
            logging.info("%s is going to verify %s" % (issuer.type, cert.type))
            try:
                issuer.verify(cert)
            except InvalidSignature:
                logging.error("%s cannot verify %s in the chain of %s" % (issuer.type, cert.type, cert.curve))
                raise

    def get_raw(self):
        raw_chain = bytes()
        for cert in reversed(self):
            raw_chain += cert.serial
            raw_chain += cert.pubk
            raw_chain += cert.signature
        return raw_chain

    @classmethod
    def from_raw(cls, data, curve):
        def split_bytes(data, len):
            return (data[:len], data[len:])

        obj = cls()
        for ca in reversed([ca for ca in CA_TYPE if ca >= CA_TYPE.AMZN and ca <= CA_TYPE.DEVICE]):
            serial_len = SMSN_LEN if ca == CA_TYPE.DEVICE else SERIAL_LEN
            (serial, data) = split_bytes(data, serial_len)
            pubk_len = ED25519_PUBK_LEN if curve == CURVE.ED25519 else P256R1_PUBK_LEN
            (pubk, data) = split_bytes(data, pubk_len)
            sig_len = ED25519_SIG_LEN if curve == CURVE.ED25519 else P256R1_SIG_LEN
            (sig, data) = split_bytes(data, sig_len)
            obj.append(SidewalkCert(ca, curve, serial, pubk, sig))
        if len(data):
            raise ValueError("Chain is longer then expected")
        obj.reverse()
        return obj




class SidewalkCertsOnHsm:
    def __init__(self, session, signer_tag, cached_objects=None):
        # Construct basic map of HSM
        class Object:
            def __init__(self, instance):
                self._instance = instance
                self._info = None
                self._content = None

            @property
            def id(self):
                return self._instance.id

            @property
            def type(self):
                return self._instance.object_type

            @property
            def info(self):
                if self._info is None:
                    logging.debug("Getting info for object 0x%x on HSM" % self.id)
                    self._info = self._instance.get_info()
                return self._info

            @property
            def content(self):
                if self._content is None:
                    logging.debug("Getting content for object 0x%x on HSM" % self.id)
                    self._content = self._instance.get()
                return self._content

            @property
            def namespace(self):
                return self._instance.id // 0x10 * 0x10

            @property
            def tag(self):
                return "0x%04x-%s" % (self.id, self.type)

        logging.info("Listing objects on HSM")
        self.objects = [Object(o) for o in session.list_objects()]

        if cached_objects:
            for object in self.objects:
                tag = object.tag
                try:
                    object._info = ObjectInfo(*cached_objects[tag]['info'])
                    object._content = bytes.fromhex(cached_objects[tag]['content'])
                except KeyError as e:
                    pass

        hsm_info_obj = self.search_for(
            lambda o : o.id < 0x10 and o.info.label == HSM_INFO_LABEL)
        if hsm_info_obj:
            hsm_info = json.loads(hsm_info_obj.content)
            logging.info("Found HSM INFO: %s" % hsm_info)
            if version.parse(__version__) < version.parse(hsm_info['toolreq']):
                raise Exception("The HSM needs signing tools newer than %s to work" % hsm_info['toolreq'])
            self.legacy_chain = not hsm_info['longchain']
        else:
            chain_depth_control = self.search_for(
                lambda o : o.id < 0x10 and o.info.label == CHAIN_DEPTH_CTL_LABEL)

            if chain_depth_control is None:
                self.legacy_chain = True
            else:
                chain_depth = int.from_bytes(chain_depth_control.content, 'little')
                if chain_depth != 5:
                    # Sidewalk currently only uses 5-depth chain
                    raise Exception("Unsupported chain depth " + str(chain_depth) + ' found in HSM')
                self.legacy_chain = False

        signer = self.search_for(
            lambda o : o.type == OBJECT.ASYMMETRIC_KEY and \
                o.info.label.startswith(signer_tag))

        if signer is None:
            raise Exception("Can't find the Signer with " + signer_tag)

        logging.info("Found signer at namespace=0x%x", signer.namespace)

        if self.legacy_chain:
            self._namespace_def = {
                CA_TYPE.AMZN  : NAMESPACE.AMAZON,
                CA_TYPE.MANU  : NAMESPACE.MANU_LEGACY,
                CA_TYPE.MODEL : signer.namespace
            }
        else:
            prod_tag = signer_tag.replace('_DAK', '_PROD')
            if signer_tag == prod_tag:
                raise ValueError("The tag for the DAK doesn't contain the mark 'DAK'")

            # To reduce number of searchs, just check the object for ELEMENT.PUBK
            serial_index_offset = signer.id - signer.namespace - ELEMENT.PRIV + ELEMENT.PUBK

            for namespace in range(NAMESPACE.CERT_START, NAMESPACE.CERT_END, 0x10):
                id = namespace + serial_index_offset
                logging.info("Searching in id 0x%x for %s" % (id, prod_tag))
                prod_pubk = self.search_for_id_and_type(id, OBJECT.OPAQUE)
                if prod_pubk and prod_pubk.info.label.startswith(prod_tag):
                    break
                prod_pubk = None

            if prod_pubk is None:
                raise Exception("Can't find corresponding product cert for " + signer_tag)

            logging.info("Found prod ca at namespace=0x%x", prod_pubk.namespace)

            self._namespace_def = {
                CA_TYPE.AMZN     : NAMESPACE.AMAZON,
                CA_TYPE.SIDEWALK : NAMESPACE.SIDEWALK,
                CA_TYPE.MANU     : NAMESPACE.MANU,
                CA_TYPE.PROD     : prod_pubk.namespace,
                CA_TYPE.DAK      : signer.namespace
            }

    def search_for(self, func):
        for object in self.objects:
            if func(object):
                return object
        return None

    def search_for_id_and_type(self, id, objectype):
        return self.search_for(lambda o : o.id == id and o.type == objectype)

    def construct_id_from(self, namespace, curve, stage, element):
        index = namespace
        index += curve << 3
        # PRODUCTION use the top half of a curve's space while the others use the bottom half
        index += 4 if stage != STAGE.PROD else 0
        index += element
        return index

    def sign(self, curve, stage, data):
        signer_type = CA_TYPE.DAK if not self.legacy_chain else CA_TYPE.MODEL
        signer_namespace = self._namespace_def[signer_type]
        signer_id = self.construct_id_from(signer_namespace, curve, stage, ELEMENT.PRIV)
        signer_object = self.search_for_id_and_type(signer_id, OBJECT.ASYMMETRIC_KEY)
        if signer_object is None:
            raise Exception("Can't find the signer")

        if curve == CURVE.ED25519:
            logging.debug("Signing eddsa with 0x%x on HSM" % signer_object.id)
            return signer_object._instance.sign_eddsa(data)
        elif curve == CURVE.P256R1:
            hasher = hashes.Hash(hashes.SHA256())
            hasher.update(data)
            digest = hasher.finalize()
            logging.debug("Signing ecdsa with 0x%x on HSM" % signer_object.id)
            sig_in_der = signer_object._instance.sign_ecdsa(data)
            # YubiHSM's ecdsa signature is encoded in DER
            (r,s) = decode_dss_signature(sig_in_der)
            return r.to_bytes(32, byteorder='big') + s.to_bytes(32, byteorder='big')

    def get_certificate(self, type, stage, curve):
        namespace = self._namespace_def[type]
        cert_data = {}
        for e in ELEMENT:
            if e == ELEMENT.PRIV:
                continue
            id = self.construct_id_from(namespace, curve, stage ,e)

            logging.debug("Getting cert obj on 0x%x for %s" % (id, e))
            object = self.search_for_id_and_type(id, OBJECT.OPAQUE)

            if object is None:
                raise MissingCertificateObject("missing certificate object for %s at 0x%x" % (e, id) )
            cert_data[e] = object.content

        return SidewalkCert(type=type,
                            curve=curve,
                            serial=cert_data[ELEMENT.SERIAL],
                            signature=cert_data[ELEMENT.SIGNATURE],
                            pubk=cert_data[ELEMENT.PUBK])

    def get_certificate_chain(self, stage, curve):
        chain = SidewalkCertChain()
        for cert_type, namespace in sorted(self._namespace_def.items()):
            logging.debug("Getting cert for " + str(cert_type))
            chain.append(self.get_certificate(cert_type, stage, curve))
        return chain

    def dump(self):
        t = {}
        for object in self.objects:
            tag = object.tag
            t[tag] = {
                'id' : object.id,
                'type' : object.type,
                'info' : object.info
            }
            if object.type == OBJECT.OPAQUE:
                t[tag]['content'] = str(binascii.hexlify(object.content), 'ascii')
        return(json.dumps(t, indent=4))


def generate_smsn(stage, device_type, apid, dsn):
    hasher = hashes.Hash(hashes.SHA256())
    hasher.update((device_type + '-' + stage2str(stage) + dsn + apid).encode('ascii'))
    return hasher.finalize()


def arg_parser_builder():
    def base64str(val):
        try:
            return base64.standard_b64decode(val)
        except binascii.Error:
            raise argparse.ArgumentTypeError('unable to decode')

    def hexstr(val):
        try:
            return bytes.fromhex(val)
        except binascii.Error:
            raise argparse.ArgumentTypeError('unable to decode')

    def filepath(val):
        try:
            with open(val) as tempfile:
                return val
        except:
            raise argparse.ArgumentTypeError('unable to open file')

    def lenth_of_data(val):
        length = int(val)
        if length <= 0:
            raise argparse.ArgumentTypeError('must be greater than 0')
        return length

    parser = argparse.ArgumentParser(add_help=True)


    parser.add_argument('-V', '--version',
                        action='store_true',
                        help="Print version and exit")

    parser.add_argument('-p', '--product',
                        required=True,
                        help="Label of the product defined in the HSM")

    parser.add_argument('--pin',
                        required=True,
                        help="Pin for the HSM signing domain")

    parser.add_argument('--pin_slot',
                        type=int,
                        help="Key slot that PIN will be authenticated with")

    parser.add_argument('--eddsa_csr',
                        type=base64str,
                        help="Ed25519 Certificate Signing Request")

    parser.add_argument('--ecdsa_csr',
                        type=base64str,
                        help="P256R1 Certifiate Signing Request")

    parser.add_argument('-c', '--connector',
                        required=True,
                        help="URL of the yubihsm-connector")

    parser.add_argument('--ca_cert',
                        type=filepath,
                        help="CA of the yubihsm-connector for HTTPS")

    parser.add_argument('--client_cert',
                        type=filepath,
                        help="Certificate the client presents for authentication for HTTPS")

    parser.add_argument('--client_key',
                        type=filepath,
                        help="Key the client presents for authentication for HTTPS")

    parser.add_argument('-s', '--smsn_len',
                        type=lenth_of_data,
                        default=SMSN_LEN,
                        help="Length of the Sidewalk Manufacturing Serial Number \
                              or device certificate subject")

    parser.add_argument('-g', '--generate_smsn',
                        action='store_true',
                        help="Generate a Sidewalk Manufacturing Serial Number")

    parser.add_argument('-d', '--device_type',
                        help="Device Type of the product")

    parser.add_argument('-D', '--dsn',
                        help="Unique serial number for the device")

    parser.add_argument('-a', '--apid',
                        help="The Advertised Product ID")

    parser.add_argument('--outform',
                        choices=['json', 'flat'],
                        default='json',
                        help="The output format of the data")

    parser.add_argument('--control_log_dir',
                        type=str,
                        help='The path to save auto-generated control log')

    parser.add_argument('--control_log_ver',
                        type=str,
                        default='4-0-1',
                        help='The version in which the control log to be generated')

    parser.add_argument('-o',
                        type=argparse.FileType('w'),
                        default='-',
                        help="Output the signing data to a file")

    parser.add_argument('--ed25519_private_key',
                        type=hexstr,
                        help="The ED25519 private key")

    parser.add_argument('--p256r1_private_key',
                        type=hexstr,
                        help="The P256R1 private key")

    parser.add_argument('--ed25519_private_key_file',
                        type=argparse.FileType('rb'),
                        help="The ED25519 private key")

    parser.add_argument('--p256r1_private_key_file',
                        type=argparse.FileType('rb'),
                        help="The P256R1 private key")

    parser.add_argument('--device_profile_json',
                        type=argparse.FileType("r"),
                        help="Json response of 'aws iotwireless get-device-profile ...' ")

    parser.add_argument('--cache_cert',
                        action='store_true',
                        help="Cache the certificates to reduce access to HSM")

    parser.add_argument('-v', '--verbose',
                        action='count',
                        default=0,
                        help="Verbose mode")

    parser.add_argument('--verify_cert',
                        type=int,
                        choices=[0, 1],
                        default=1,
                        help="Verify generated certificate with loaded certificate chain")

    parser.add_argument('--test_cert',
                        action='store_true',
                        help="Use test certificates in HSM")

    return parser


def load_private_key_from_pem(data, curve):
    if curve == CURVE.ED25519:
        private_key = serialization.load_pem_private_key(
            data=data,
            password=None
        )
        pubk = private_key.public_key().public_bytes(
            encoding=serialization.Encoding.Raw,
            format=serialization.PublicFormat.Raw
        )
        priv = private_key.private_bytes(
            encoding=serialization.Encoding.Raw,
            format=serialization.PrivateFormat.Raw,
            encryption_algorithm=serialization.NoEncryption())

    elif curve == CURVE.P256R1:
        private_key = serialization.load_pem_private_key(
            data=data,
            password=None
        )
        public_numbers = private_key.public_key().public_numbers()
        pubk = public_numbers.x.to_bytes(
            32, 'big') + public_numbers.y.to_bytes(32, 'big')
        private_number = private_key.private_numbers()
        priv = private_number.private_value.to_bytes(32, 'big')

    else:
        raise ValueError("unsupported curve")

    return (priv, pubk)


def verify_with_sig_ed25519(public_key, signature, data):
    pubk = ed25519.Ed25519PublicKey.from_public_bytes(public_key)
    pubk.verify(signature=signature, data=data)


def verify_with_sig_p256r1(public_key, signature, data):
    pubk = ec.EllipticCurvePublicNumbers(
        x=int.from_bytes(public_key[:32], 'big'),
        y=int.from_bytes(public_key[32:], 'big'),
        curve=ec.SECP256R1()).public_key()

    signature = encode_dss_signature(
        r=int.from_bytes(signature[:32], 'big'),
        s=int.from_bytes(signature[32:], 'big'))

    pubk.verify(
        signature=signature,
        data=data,
        signature_algorithm=ec.ECDSA(hashes.SHA256()))


def decode_csr(csr, smsn_len, curve, verify_sig=True):
    if curve == CURVE.ED25519:
        pubk_len = ED25519_PUBK_LEN
        sig_len = ED25519_SIG_LEN
    elif curve == CURVE.P256R1:
        pubk_len = P256R1_PUBK_LEN
        sig_len = P256R1_SIG_LEN

    if len(csr) == pubk_len + smsn_len:
        sig_len = 0

    if len(csr) != pubk_len + smsn_len + sig_len:
        raise ValueError("Invalid lenght of CSR for curve=%r, got %d"
                          % (curve, len(csr)))

    pubk = csr[0:pubk_len]
    csr = csr[pubk_len:]
    smsn = csr[:smsn_len]
    csr = csr[smsn_len:]
    sig = csr[:sig_len]

    # For on device cert gen, check if the CSRs are valid
    if verify_sig and len(sig):
        if curve == CURVE.ED25519:
            verify_with_sig_ed25519(pubk, sig, pubk + smsn)
        if curve == CURVE.P256R1:
            verify_with_sig_p256r1(pubk, sig, pubk + smsn)

    logging.info("CSR for %r: pubk=%s,smsn=%s,sig=%s" % (curve,
                                                         str(binascii.hexlify(pubk), 'ascii'),
                                                         str(binascii.hexlify(smsn), 'ascii'),
                                                         str(binascii.hexlify(sig), 'ascii')))
    return (pubk, smsn, sig)

def generate_cl_filename():
    return 'C_CONTROL_LOG_' + time.strftime("%Y%m%d%H%M%S", time.localtime()) + '.txt'

def bin2hexstr(binary):
    return str(binascii.hexlify(binary), 'ascii')

def bin2base64(binary):
    return str(base64.b64encode(binary), 'ascii')

def stage2str(stage):
    str = ''
    if stage == STAGE.PROD:
        str = 'PRODUCTION'
    elif stage == STAGE.TEST:
        str = 'TEST'
    elif stage == STAGE.PREPROD:
        str = 'PREPRODUCTION'
    return str

def output_formatter_json(output):
    d = defaultdict(dict)
    d['eD25519'] = bin2base64(output['ed25519_chain'])
    d['p256R1'] = bin2base64(output['p256r1_chain'])
    d['label'] = stage2str(output['stage']).lower()
    if output['smsn']:
        d['metadata']['smsn'] = bin2hexstr(output['smsn'])
    if output['apid']:
        d['metadata']['apid'] = output['apid']
    if output['ed25519_priv']:
        d['metadata']['devicePrivKeyEd25519'] = bin2hexstr(output['ed25519_priv'])
    if output['p256r1_priv']:
        d['metadata']['devicePrivKeyP256R1'] = bin2hexstr(output['p256r1_priv'])
    if output['application_server_public_key']:
        d['applicationServerPublicKey'] = bin2hexstr(output['application_server_public_key'])
    return json.dumps(d, indent=4) + "\n"

def output_formatter_flat(output):
    str = ''
    if output['smsn']:
        str += "smsn: " + bin2hexstr(output['smsn']) + "\n"
    if output['ed25519_priv']:
        str += "devicePrivKeyEd25519: " + bin2hexstr(output['ed25519_priv']) + "\n"
    if output['p256r1_priv']:
        str += "devicePrivKeyP256R1: " + bin2hexstr(output['p256r1_priv']) + "\n"
    str += "ED25519 Sidewalk Certificate Chain: " + bin2base64(output['ed25519_chain']) + "\n"
    str += "P256R1 Sidewalk Certificate Chain: " + bin2base64(output['p256r1_chain']) + "\n"
    str += "Label: " + stage2str(output['stage']).lower() + "\n"
    if output['application_server_public_key']:
        str += "Application Server Public Key:" + bin2hexstr(output['application_server_public_key']) + "\n"

    return str

def output_formatter_cl_4_0_1(output):
    j = {
        "controlLogs": [
            {
                "version": "4-0-1",
                "device": {
                    "serialNumber": bin2hexstr(output['smsn']),
                    "productIdentifier": {
                        "advertisedProductId": output['apid']
                    },
                    "sidewalkData": {
                        "sidewalkED25519CertificateChain": bin2base64(output['ed25519_chain']),
                        "sidewalkP256R1CertificateChain": bin2base64(output['p256r1_chain']),
                        "label": stage2str(output['stage']).lower()
                    }
                }
            }
        ]
    }
    return json.dumps(j, indent=2) + "\n"

def create_hsm_session(hsm, pin, pin_slot, stage):
    try_slots = [SIDEWALK_AUTH_KEY_INDEX, SIDEWALK_AUTH_KEY_INDEX_PREPROD]
    if stage == STAGE.PREPROD:
        try_slots = [SIDEWALK_AUTH_KEY_INDEX_PREPROD, SIDEWALK_AUTH_KEY_INDEX]
    if pin_slot:
        try_slots = [pin_slot]

    for slot in try_slots:
        try:
            logging.debug("Authenticating with key slot %d" % slot)
            return hsm.create_session_derived(slot, pin)
        except YubiHsmDeviceError as e:
            if e.code != ERROR.OBJECT_NOT_FOUND:
                raise e
    raise Exception("No key slot found for authentication")

def main():

    cl_output_formatters = {
        '4-0-1' : output_formatter_cl_4_0_1
    }

    args = arg_parser_builder().parse_args()

    if args.version:
        sys.exit("version: " + __version__)

    if args.verbose > 0:
        logging.basicConfig(level='DEBUG' if args.verbose >= 2 else 'INFO')

    # Tests on the arguments
    if args.generate_smsn:
        if args.smsn_len != SMSN_LEN:
            sys.exit("smsn_len not valid when generating smsn")
        if args.device_type is None or args.dsn is None or args.apid is None:
            sys.exit("needs device_type, dsn, and apid when generating smsn")

    if args.control_log_dir:
        if not path.isdir(args.control_log_dir):
            sys.exit("%s doesn't exist" % args.control_log_dir)
        if args.control_log_ver not in cl_output_formatters:
            sys.exit("Unsupported control log version %s" % args.control_log_ver)
        if args.apid is None:
            sys.exit("APID need to be availabe to generate control logs")

    if (args.client_cert is None) != (args.client_key is None):
        sys.exit("Both of clent cert and key should be provided")

    if args.ed25519_private_key is not None and args.ed25519_private_key_file is not None:
        sys.exit('Both ed25519_private_key_file and ed25519_private_key provided')

    if args.p256r1_private_key is not None and args.p256r1_private_key_file is not None:
        sys.exit('Both p256r1_private_key_file and p256r1_private_key provided')

    if (args.eddsa_csr is None
            and (not args.generate_smsn or args.ed25519_private_key_file is None)):
        sys.exit('Public key for eddsa should be provided --eddsa_csr or --ed25519_private_key_file')

    if (args.ecdsa_csr is None
            and (not args.generate_smsn or args.p256r1_private_key_file is None)):
        sys.exit('Public key for ecdsa should be provided --ecdsa_csr or --p256r1_private_key_file')

    if (args.ecdsa_csr and not args.eddsa_csr) or (args.eddsa_csr and not args.ecdsa_csr):
        sys.exit('Both of CSRs should be provided')

    # Prepare keys
    # Public key: from CSR or PEM (with -g, the user can just provide no CSR)
    # Private key: from argument or PEM

    ed25519_priv = None
    ed25519_pubk = None
    p256r1_priv = None
    p256r1_pubk = None
    application_server_public_key = None

    if args.device_profile_json is not None:
        device_profile = json.loads(args.device_profile_json.read())
        application_server_public_key = binascii.unhexlify(device_profile['Sidewalk']['ApplicationServerPublicKey'])

    if args.ed25519_private_key_file is not None:
        (ed25519_priv, ed25519_pubk) = load_private_key_from_pem(args.ed25519_private_key_file.read(), CURVE.ED25519)
        logging.debug("Loaded ed25519 priv: " + str(base64.standard_b64encode(ed25519_priv), 'ascii'))
        logging.debug("Loaded ed25519 pubk: " + str(base64.standard_b64encode(ed25519_pubk), 'ascii'))
    elif args.ed25519_private_key is not None:
        ed25519_priv = args.ed25519_private_key

    if args.eddsa_csr is not None:
        try:
            (eddsa_csr_pubk, eddsa_csr_smsn, eddsa_csr_sig) = decode_csr(args.eddsa_csr,
                                                                         (0 if args.generate_smsn else args.smsn_len),
                                                                         CURVE.ED25519)
        except InvalidSignature:
            sys.exit("Invalid eddsa_csr. Bad signature")
        if ed25519_pubk and ed25519_pubk != eddsa_csr_pubk:
            sys.exit("Public keys of ed25519 in CSR and PEM file do not match")
        ed25519_pubk = eddsa_csr_pubk

    if args.p256r1_private_key_file is not None:
        (p256r1_priv, p256r1_pubk) = load_private_key_from_pem(args.p256r1_private_key_file.read(), CURVE.P256R1)
        logging.debug("Loaded p256r1 priv: " + str(base64.standard_b64encode(p256r1_priv), 'ascii'))
        logging.debug("Loaded p256r1 pubk: " + str(base64.standard_b64encode(p256r1_pubk), 'ascii'))
    elif args.p256r1_private_key is not None:
        p256r1_priv = args.p256r1_private_key

    if args.ecdsa_csr is not None:
        try:
            (ecdsa_csr_pubk, ecdsa_csr_smsn, ecdsa_csr_sig) = decode_csr(args.ecdsa_csr,
                                                                         (0 if args.generate_smsn else args.smsn_len),
                                                                         CURVE.P256R1)
        except InvalidSignature:
            sys.exit("Invalid ecdsa_csr. Bad signature")
        if p256r1_pubk and p256r1_pubk != ecdsa_csr_pubk:
            sys.exit("Public keys of p256r1 in CSR and PEM file do not match")
        p256r1_pubk = ecdsa_csr_pubk

    # Check the public key and the private key, if provided
    if args.verify_cert:
        test_data = b'test data'
        if ed25519_priv is not None:
            private_key = ed25519.Ed25519PrivateKey.from_private_bytes(ed25519_priv)
            signature = private_key.sign(test_data)
            verify_with_sig_ed25519(ed25519_pubk, signature, test_data)

        if p256r1_priv is not None:
            private_key = ec.derive_private_key(int.from_bytes(p256r1_priv, 'big'), ec.SECP256R1())
            (r, s) = decode_dss_signature(private_key.sign(test_data, ec.ECDSA(hashes.SHA256())))
            signature = r.to_bytes(32, byteorder='big') + s.to_bytes(32, byteorder='big')
            verify_with_sig_p256r1(p256r1_pubk, signature, test_data)

    if args.product.startswith('RNET_'):
        stage = STAGE.PROD
    elif args.product.startswith('PREPROD_'):
        stage = STAGE.PREPROD
    elif args.product.startswith('TEST_') or args.test_cert:
        stage = STAGE.TEST
    else:
        sys.exit("Unable to determine the stage from the product tag %s." % args.product)

    # SMSN is from CSR (without -g) or generated
    if args.generate_smsn:
        smsn = generate_smsn(stage=stage,
                             device_type=args.device_type,
                             apid=args.apid,
                             dsn=args.dsn)
    else:
        smsn = eddsa_csr_smsn
        if eddsa_csr_smsn != ecdsa_csr_smsn:
            sys.exit("serials in both CSRs do not match")
        if (eddsa_csr_sig and not ecdsa_csr_sig) or (ecdsa_csr_sig and not eddsa_csr_sig):
            sys.exit("Only one of eddsa_csr and ecdsa_csr has the signature")

    logging.info("Generated/Grabbed SMSN=" + str(binascii.hexlify(smsn), 'ascii'))

    # Set up certs for TLS
    def init_wrapper(init_func, ca, client_cert, client_key):
        class Tls12HttpAdapter(HTTPAdapter):
            """"Use TLSv1.2"""

            def init_poolmanager(self, connections, maxsize, block=False):
                self.poolmanager = PoolManager(
                    num_pools=connections, maxsize=maxsize,
                    block=block, ssl_version=ssl.PROTOCOL_TLSv1_2)

        def init_with_certs(*args, **kwargs):
            init_func(*args, **kwargs)

            if ca is not None:
                args[0]._session.verify = ca
            if client_cert is not None:
                args[0]._session.cert = (client_cert, client_key)
            args[0]._session.mount('https://', Tls12HttpAdapter())
        return init_with_certs

    if args.ca_cert is not None or args.client_cert is not None:
        HttpBackend.__init__ = init_wrapper(HttpBackend.__init__,
                                            args.ca_cert, args.client_cert, args.client_key)

    # Signing and pull out the chain
    try:
        with YubiHsm.connect(args.connector) as hsm,\
            create_hsm_session(hsm, args.pin, args.pin_slot, stage) as session:

            import tempfile, os
            cache = None
            cache_file = path.join(
                tempfile.gettempdir(),
                "signing-tool-" + str(hsm.get_device_info().serial) + ".tmp")
            if args.cache_cert:
                logging.info("Loading cache at " + cache_file)
                try:
                    with open(cache_file, 'r') as f:
                        cache = json.loads(f.read())
                except (FileNotFoundError, json.decoder.JSONDecodeError):
                    pass
            else:
                try:
                    os.remove(cache_file)
                except FileNotFoundError:
                    pass

            hsmStore = SidewalkCertsOnHsm(session, args.product, cache)

            def generate_chain(curve, device_pubk):
                logging.info("Pulling the chain for %s from HSM", curve)
                try:
                    chain = hsmStore.get_certificate_chain(stage, curve)
                except MissingCertificateObject as e:
                    sys.exit("Uable to get the %s cert chain for %s: %s" % (stage, curve, e))

                logging.info("Signing the device cert for %s", curve)
                device_cert = SidewalkCert(type=CA_TYPE.DEVICE,
                                        curve=curve,
                                        serial=smsn,
                                        pubk=device_pubk,
                                        signature=hsmStore.sign(curve, stage, device_pubk+smsn))

                chain.append(device_cert)       # Make the complete chain
                if args.verify_cert > 0:
                    chain.validate()
                return chain.get_raw()

            # Collect what we have and generate the output
            output = {
                'ed25519_chain': generate_chain(CURVE.ED25519, ed25519_pubk),
                'p256r1_chain' : generate_chain(CURVE.P256R1, p256r1_pubk),
                'stage' : stage,
                'smsn' :  smsn,
                'apid' :  args.apid,
                'ed25519_priv' :  ed25519_priv,
                'p256r1_priv' :  p256r1_priv,
                'application_server_public_key' : application_server_public_key
            }

            if args.outform == 'json':
                args.o.write(output_formatter_json(output))
            else:
                args.o.write(output_formatter_flat(output))

            if args.control_log_dir:
                while True:
                    # Never overwrite an existing control log file
                    cl_path = path.join(args.control_log_dir, generate_cl_filename())
                    if not path.isfile(cl_path):
                        break
                    # Wait for a while so that the filename will change
                    time.sleep(1)
                with open(cl_path, 'w') as f:
                    f.write(cl_output_formatters[args.control_log_ver](output))
                    print(cl_path, file=sys.stderr)

            if args.cache_cert and cache is None:
                with open(cache_file, 'w') as f:
                    f.write(hsmStore.dump())

    except YubiHsmConnectionError as e:
        sys.exit("HSM connector error: %s" % e)
    except YubiHsmAuthenticationError as e:
        sys.exit("PIN of the HSM may be wrong")

if __name__ == "__main__":
    main()
