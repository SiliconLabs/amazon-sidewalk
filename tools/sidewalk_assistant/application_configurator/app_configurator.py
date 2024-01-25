import argparse
import xml.etree.ElementTree as ET
import os
import sys
import json
import time
from jinja2 import Environment, FileSystemLoader

parser = argparse.ArgumentParser(add_help=True)
parser.add_argument('-in', '--input', help='Input directory', required=True)
parser.add_argument('-out', '--output', help='Output directory', required=True)
args = parser.parse_args()

############################################
# Constants and file names / extensions
############################################
# file name
INPUT_FILE_NAME = 'sidewalk.asconf'
PROPERTIES_FILE_NAME = 'sl_sidewalk_properties.h'
C_PROJECT_FILE_NAME = '.cproject'
SUBGHZ_CONFIG_FILE_NAME = "app_subghz_config.c"
COMMON_CONFIG_FILE_NAME = "sl_sidewalk_common_config.h"

# fle path
SCRIPT_PATH = os.path.dirname(os.path.realpath(__file__))
PROJECT_ROOT_PATH = os.path.join(args.input, '../../')
SIDEWALK_EXTENSION_PATH = os.path.join(SCRIPT_PATH, '../../../')
SUBGHZ_CONFIG_PATH = os.path.join(SIDEWALK_EXTENSION_PATH, 'component/ble_subghz/radio/subghz/rail/', SUBGHZ_CONFIG_FILE_NAME)
COMMON_CONFIG_PATH = os.path.join(PROJECT_ROOT_PATH, 'config/', COMMON_CONFIG_FILE_NAME)

# extension
TEMPLATE_EXT = '.jinja'
RESTRICT_EXT = '.restriction'

# others
C_DEFINE_STR = '#define '

############################################
# Classes
############################################
# restrictions
class Link_Restriction:

    def __init__(self) -> None:
        self.name = "linkType"
        self.arguments = {}
        self.arguments['id'] = 0
        self.arguments['enabled'] = []
        self.arguments['default'] = ""

    def toJSON(self):
        return json.dumps([self], default=lambda o: o.__dict__, sort_keys=False, indent=4)

class Region_Restriction:
    def __init__(self) -> None:
        self.name ="region"
        self.arguments = {}
        self.arguments['id'] = 0
        self.arguments['enabled'] = []
        self.arguments['maxOutputPower'] = []

    def toJSON(self):
        return json.dumps([self], default=lambda o: o.__dict__, sort_keys=False, indent=4)

############################################
# global variables
############################################
env = Environment(loader=FileSystemLoader("templates/"))
template = env.get_template(PROPERTIES_FILE_NAME + TEMPLATE_EXT)
restricted_link = Link_Restriction()
restricted_region = Region_Restriction()
macros = []

############################################
# @brief add property to the properties
# dictionary
############################################
def add_property(pKey, pVal):
    if pVal == None:
        return
    if pVal['value'] == None:
        # silently return for null properties
        return

    # find in dictionary
    if pVal['type'] == "SYMBOL":
        pass
    elif pVal['type'] == "INT":
        pass
    elif pVal['type'] == "STR":
        # concatenate " " to the string
        pVal['value'] = "\"" + pVal['value'] + "\""
    else:
        raise ValueError("Invalid property type")

    # add to properties
    macros.append({"key": pKey, "value":pVal['value']})

############################################
# @brief render the jinja template based on
# the script context
############################################

def render_template(template, output_file, context):
    with open(output_file, 'w') as f:
        f.write(template.render(context))

############################################
# @brief parser functions for project configuration
############################################
def c_preprocessor_macro_parser(key):
    try:
        with open(os.path.join(PROJECT_ROOT_PATH, C_PROJECT_FILE_NAME), 'r') as f:
            try:
                tree = ET.parse(f)
            except ET.ParseError:
                return None
            for node in tree.iter('listOptionValue'):
                if node.attrib['value'].startswith(key):
                    return node.attrib['value'].split('=')[1]
        return None
    except FileNotFoundError:
        return

def c_header_parser(file, key):
    with open(file, 'r') as f:
        for line in f:
        # for line starting with
            if line.startswith(C_DEFINE_STR + key):
                # -1 is the last element in the line
                return line.split()[-1].strip('\n')

############################################
# @brief from the json input file, add new
# properties
############################################
def generate_properties(f):
    # link properties
    propLinkType = find_in_json_file('linkTypes', file=f)
    add_property("SL_SIDEWALK_CONFIGURATOR_PROPERTIES_LINK_TYPE", propLinkType)

    proptxRxWakeUp = find_in_json_file('txRxWakeUp', file=f)
    add_property("SL_SIDEWALK_CONFIGURATOR_PROPERTIES_TX_RX_WAKEUP", proptxRxWakeUp)

    # BLE properties
    propDevicename = find_in_json_file('bleSettings', 'deviceName', file=f)
    # Set a default value if the property is not set
    if propDevicename is not None:
        if propDevicename['value'] == None or propDevicename['value'] == "":
            propDevicename['value'] = "SL_SIDEWALK"
            propDevicename['type'] = "STR"

    add_property("SL_SIDEWALK_CONFIGURATOR_PROPERTIES_BLE_DEVICE_NAME", propDevicename)

    # FSK properties
    # power profile
    propFskPowerProfile = find_in_json_file('fskSettings', 'deviceProfile', file=f)
    add_property("SL_SIDEWALK_CONFIGURATOR_PROPERTIES_FSK_POWER_PROFILE", propFskPowerProfile)
    # output power
    propFskOutputPower = find_in_json_file('fskSettings', 'outputPower', file=f)
    add_property("SL_SIDEWALK_CONFIGURATOR_PROPERTIES_FSK_OUTPUT_POWER", propFskOutputPower)

    # CSS properties
    # power profile
    propCSSPowerProfile = find_in_json_file('cssSettings', 'deviceProfile', file=f)
    add_property("SL_SIDEWALK_CONFIGURATOR_PROPERTIES_CSS_POWER_PROFILE", propCSSPowerProfile)
    # output power
    propCSSOutputPower = find_in_json_file('cssSettings', 'outputPower', file=f)
    add_property("SL_SIDEWALK_CONFIGURATOR_PROPERTIES_CSS_OUTPUT_POWER", propCSSOutputPower)


############################################
# @brief parse project configuration and
# create restrictions for the configurator
############################################
def parse_restrictions_cproject():
    # linkType
    if c_preprocessor_macro_parser('SL_BLE_SUPPORTED'):
        restricted_link.arguments['enabled'].append("SL_SIDEWALK_LINK_BLE")
    if c_preprocessor_macro_parser('SL_FSK_SUPPORTED'):
         restricted_link.arguments['enabled'].append("SL_SIDEWALK_LINK_FSK")
    if c_preprocessor_macro_parser('SL_CSS_SUPPORTED'):
         restricted_link.arguments['enabled'].append("SL_SIDEWALK_LINK_CSS")

    # defaultLinkType
    defaultLink = c_header_parser(COMMON_CONFIG_PATH , 'SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE')
    if defaultLink:
        restricted_link.arguments['default'] = defaultLink

def parse_restrictions_header():
    # region
    restricted_region.arguments['enabled'].append("RADIO_REGION_NA")

    # output power
    outputPower = {}
    outputPower['NA'] = c_header_parser(SUBGHZ_CONFIG_PATH , 'RADIO_MAX_TX_POWER_NA')

    if outputPower['NA']:
        restricted_region.arguments['maxOutputPower'].append(outputPower['NA'])

def retreive_project_config():
    count = 0
    # WAIT FOR PROJET CONFIGURATION TO BE DONE
    while len(restricted_link.arguments['enabled']) == 0 :
        if count >= 10:
            # File not found
            # Silently return (no restriction will be generated)
            return
        parse_restrictions_cproject()
        count += 1
        time.sleep(1)
    parse_restrictions_header()

############################################
# @brief generate the restrictions
############################################
def generate_restrictions():

    if (os.path.exists(os.path.join(PROJECT_ROOT_PATH, 'config/sidewalk/link_sidewalk.restriction')) and
        os.path.exists(os.path.join(PROJECT_ROOT_PATH, 'config/sidewalk/region_sidewalk.restriction'))):
        return

    retreive_project_config()

    with open(os.path.join(PROJECT_ROOT_PATH, 'config/sidewalk/link_sidewalk.restriction'), 'w') as f:

        f.write(restricted_link.toJSON())
    with open(os.path.join(PROJECT_ROOT_PATH, 'config/sidewalk/region_sidewalk.restriction'), 'w') as f:
        f.write(restricted_region.toJSON())


############################################
# @brief find the key in the json file
############################################
def find_in_json_file(*keys, file):
    with open(file, 'r') as f:
        data = json.load(f)
        for key in keys:
            if key not in data:
                return None
            data = data[key]
        return data


############################################
# @brief post processing
############################################
def post_processing():
    if not os.path.exists(args.output + '/app_config'):
        os.makedirs(args.output + '/app_config')

    # write app_config/sl_sidewalk_properties.h
    context = {
        "properties": macros,
    }
    render_template(template, os.path.join(args.output, PROPERTIES_FILE_NAME), context)

############################################
# @brief main function
############################################
def main():
    generate_restrictions()
    generate_properties(os.path.join(args.input, INPUT_FILE_NAME))

# program entry point
if __name__ == "__main__":
    main()
    post_processing()
