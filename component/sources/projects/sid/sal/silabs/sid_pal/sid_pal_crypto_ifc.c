/***************************************************************************//**
 * @file
 * @brief sid_pal_crypto_ifc.c
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 * Your use of this software is governed by the terms of
 * Silicon Labs Master Software License Agreement (MSLA)available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 * This software contains Third Party Software licensed by Silicon Labs from
 * Amazon.com Services LLC and its affiliates and is governed by the sections
 * of the MSLA applicable to Third Party Software and the additional terms set
 * forth in amazon_sidewalk_license.txt.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *  claim that you wrote the original software. If you use this software
 *  in a product, an acknowledgment in the product documentation would be
 *  appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *  misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "psa/crypto.h"
#include "sid_pal_crypto_ifc.h"
#include "sl_malloc.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
static bool hal_init_done;
static bool secure_vault_enabled = false;

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static sid_error_t efr32_crypto_init(void)
{
  psa_status_t ret;

  ret = psa_crypto_init();
  if (ret != PSA_SUCCESS) {
    return SID_ERROR_GENERIC;
  }

  return SID_ERROR_NONE;
}

static sid_error_t efr32_crypto_deinit(void)
{
  mbedtls_psa_crypto_free();
  return SID_ERROR_NONE;
}

static sid_error_t efr32_crypto_rand(uint8_t *rand, size_t size)
{
  psa_status_t ret;

  ret = psa_generate_random(rand, size);
  if (ret != PSA_SUCCESS) {
    return SID_ERROR_GENERIC;
  }

  return SID_ERROR_NONE;
}

static sid_error_t efr32_crypto_hash(sid_pal_hash_params_t *params)
{
  psa_status_t ret;
  psa_algorithm_t hash_algo;

  if (params->data_size == 0) {
    return SID_ERROR_PARAM_OUT_OF_RANGE;
  }

  switch (params->algo) {
    case SID_PAL_HASH_SHA256:
      hash_algo = PSA_ALG_SHA_256;
      break;

    case SID_PAL_HASH_SHA512:
      hash_algo = PSA_ALG_SHA_512;
      break;

    default:
      return SID_ERROR_NOSUPPORT;
  }

  ret = psa_hash_compute(hash_algo,
                         params->data,
                         params->data_size,
                         params->digest,
                         params->digest_size,
                         &(params->digest_size));

  if (ret != PSA_SUCCESS || params->digest_size != PSA_HASH_LENGTH(hash_algo)) {
    return SID_ERROR_GENERIC;
  }

  return SID_ERROR_NONE;
}

static sid_error_t efr32_crypto_hmac(sid_pal_hmac_params_t *params)
{
  psa_status_t ret;
  psa_key_handle_t key_id;
  psa_key_attributes_t key_attr;
  psa_algorithm_t hash_algo;
  psa_mac_operation_t mac_op;

  if (params->data_size == 0) {
    return SID_ERROR_PARAM_OUT_OF_RANGE;
  }

  switch (params->algo) {
    case SID_PAL_HASH_SHA256:
      hash_algo = PSA_ALG_SHA_256;
      break;

    case SID_PAL_HASH_SHA512:
      hash_algo = PSA_ALG_SHA_512;
      break;

    default:
      return SID_ERROR_NOSUPPORT;
  }

  key_attr = psa_key_attributes_init();
  psa_set_key_type(&key_attr, PSA_KEY_TYPE_HMAC);
  psa_set_key_bits(&key_attr, params->key_size << 3);
  psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_SIGN_HASH);
  psa_set_key_algorithm(&key_attr, PSA_ALG_HMAC(hash_algo));

  ret = psa_import_key(&key_attr, params->key, params->key_size, &key_id);
  if (ret != PSA_SUCCESS) {
    return SID_ERROR_GENERIC;
  }

  mac_op = psa_mac_operation_init();
  ret = psa_mac_sign_setup(&mac_op, key_id, PSA_ALG_HMAC(hash_algo));
  if (ret != PSA_SUCCESS) {
    psa_destroy_key(key_id);
    return SID_ERROR_GENERIC;
  }

  ret = psa_mac_update(&mac_op, params->data, params->data_size);
  if (ret != PSA_SUCCESS) {
    psa_destroy_key(key_id);
    return SID_ERROR_GENERIC;
  }

  ret = psa_mac_sign_finish(&mac_op,
                            params->digest,
                            params->digest_size,
                            &(params->digest_size));

  if (ret != PSA_SUCCESS
      || params->digest_size != PSA_MAC_LENGTH(PSA_KEY_TYPE_HMAC,
                                               params->key_size << 3,
                                               PSA_ALG_HMAC(hash_algo))) {
    psa_destroy_key(key_id);
    return SID_ERROR_GENERIC;
  }

  ret = psa_destroy_key(key_id);
  if (ret != PSA_SUCCESS) {
    return SID_ERROR_GENERIC;
  }

  return SID_ERROR_NONE;
}

static sid_error_t efr32_crypto_aes_crypt(sid_pal_aes_params_t *params)
{
  psa_status_t ret;
  psa_key_handle_t key_id;
  psa_key_attributes_t key_attr;
  psa_algorithm_t aes_algo;

  if (params->in_size == 0) {
    return SID_ERROR_PARAM_OUT_OF_RANGE;
  }

  key_attr = psa_key_attributes_init();
  psa_set_key_type(&key_attr, PSA_KEY_TYPE_AES);

  switch (params->algo) {
    case SID_PAL_AES_CMAC_128:
      psa_set_key_bits(&key_attr, params->key_size);
      aes_algo = PSA_ALG_CMAC;
      break;

    case SID_PAL_AES_CTR_128:
      if (params->in_size > params->out_size) {
        return SID_ERROR_PARAM_OUT_OF_RANGE;
      }
      psa_set_key_bits(&key_attr, params->key_size);
      aes_algo = PSA_ALG_CTR;
      break;

    default:
      return SID_ERROR_NOSUPPORT;
  }
  psa_set_key_algorithm(&key_attr, aes_algo);

  switch (params->mode) {
    case SID_PAL_CRYPTO_ENCRYPT: {
      psa_cipher_operation_t cipher_op;
      psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_ENCRYPT);

      ret = psa_import_key(&key_attr, params->key,
                           params->key_size >> 3, &key_id);
      if (ret != PSA_SUCCESS) {
        return SID_ERROR_GENERIC;
      }

      cipher_op = psa_cipher_operation_init();
      ret = psa_cipher_encrypt_setup(&cipher_op, key_id, aes_algo);
      if (ret != PSA_SUCCESS) {
        psa_destroy_key(key_id);
        return SID_ERROR_GENERIC;
      }

      ret = psa_cipher_set_iv(&cipher_op, params->iv, params->iv_size);
      if (ret != PSA_SUCCESS) {
        psa_destroy_key(key_id);
        return SID_ERROR_GENERIC;
      }

      ret = psa_cipher_update(&cipher_op,
                              params->in,
                              params->in_size,
                              params->out,
                              params->in_size,
                              &(params->out_size));
      if (ret != PSA_SUCCESS) {
        psa_destroy_key(key_id);
        return SID_ERROR_GENERIC;
      }
      if (params->out_size != params->in_size) {
        psa_destroy_key(key_id);
        return SID_ERROR_GENERIC;
      }

      ret = psa_cipher_finish(&cipher_op,
                              params->out - params->out_size,
                              params->out_size - params->out_size,
                              &(params->out_size));
      if (ret != PSA_SUCCESS) {
        psa_destroy_key(key_id);
        return SID_ERROR_GENERIC;
      }
      // The out_size will be 0 after running psa_cipher_finish()
      params->out_size = params->in_size;
      break;
    }
    case SID_PAL_CRYPTO_DECRYPT: {
      psa_cipher_operation_t cipher_op;
      psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_DECRYPT);

      ret = psa_import_key(&key_attr, params->key,
                           params->key_size >> 3, &key_id);
      if (ret != PSA_SUCCESS) {
        return SID_ERROR_GENERIC;
      }

      cipher_op = psa_cipher_operation_init();
      ret = psa_cipher_decrypt_setup(&cipher_op, key_id, aes_algo);
      if (ret != PSA_SUCCESS) {
        psa_destroy_key(key_id);
        return SID_ERROR_GENERIC;
      }

      ret = psa_cipher_set_iv(&cipher_op, params->iv, params->iv_size);
      if (ret != PSA_SUCCESS) {
        psa_destroy_key(key_id);
        return SID_ERROR_GENERIC;
      }

      ret = psa_cipher_update(&cipher_op,
                              params->in,
                              params->in_size,
                              params->out,
                              params->in_size,
                              &(params->out_size));
      if (ret != PSA_SUCCESS) {
        psa_destroy_key(key_id);
        return SID_ERROR_GENERIC;
      }
      if (params->out_size != params->in_size) {
        psa_destroy_key(key_id);
        return SID_ERROR_GENERIC;
      }

      ret = psa_cipher_finish(&cipher_op,
                              params->out - params->out_size,
                              params->out_size - params->out_size,
                              &(params->out_size));
      if (ret != PSA_SUCCESS) {
        psa_destroy_key(key_id);
        return SID_ERROR_GENERIC;
      }
      // The out_size will be 0 after running psa_cipher_finish()
      params->out_size = params->in_size;
      break;
    }
    case SID_PAL_CRYPTO_MAC_CALCULATE: {
      psa_mac_operation_t mac_op;
      psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_SIGN_HASH);

      ret = psa_import_key(&key_attr, params->key,
                           params->key_size >> 3, &key_id);
      if (ret != PSA_SUCCESS) {
        return SID_ERROR_GENERIC;
      }

      mac_op = psa_mac_operation_init();
      ret = psa_mac_sign_setup(&mac_op, key_id, aes_algo);
      if (ret != PSA_SUCCESS) {
        psa_destroy_key(key_id);
        return SID_ERROR_GENERIC;
      }

      ret = psa_mac_update(&mac_op, params->in, params->in_size);
      if (ret != PSA_SUCCESS) {
        psa_destroy_key(key_id);
        return SID_ERROR_GENERIC;
      }

      ret = psa_mac_sign_finish(&mac_op,
                                params->out,
                                params->out_size,
                                &(params->out_size));

      if (ret != PSA_SUCCESS
          || params->out_size != PSA_MAC_LENGTH(PSA_KEY_TYPE_AES,
                                                params->key_size,
                                                aes_algo)) {
        psa_destroy_key(key_id);
        return SID_ERROR_GENERIC;
      }
      break;
    }
    default:
      return SID_ERROR_INVALID_ARGS;
  }

  ret = psa_destroy_key(key_id);
  if (ret != PSA_SUCCESS) {
    return SID_ERROR_GENERIC;
  }

  return SID_ERROR_NONE;
}

static sid_error_t efr32_crypto_aead_crypt(sid_pal_aead_params_t *params)
{
  uint8_t *cipher_mac = NULL;
  psa_status_t ret;
  sid_error_t sid_ret = SID_ERROR_NONE;
  psa_key_handle_t key_id;
  psa_key_attributes_t key_attr;
  psa_algorithm_t aead_algo;

  if (params->aad_size == 0 || params->in_size == 0 || params->mac == NULL) {
    return SID_ERROR_PARAM_OUT_OF_RANGE;
  }

  key_attr = psa_key_attributes_init();
  psa_set_key_type(&key_attr, PSA_KEY_TYPE_AES);

  switch (params->algo) {
    case SID_PAL_AEAD_GCM_128:
      psa_set_key_bits(&key_attr, params->key_size);
      aead_algo = PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, params->mac_size);
      break;

    case SID_PAL_AEAD_CCM_128:
    case SID_PAL_AEAD_CCM_STAR_128:
      psa_set_key_bits(&key_attr, params->key_size);
      aead_algo = PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, params->mac_size);
      break;

    default:
      return SID_ERROR_NOSUPPORT;
  }
  psa_set_key_algorithm(&key_attr, aead_algo);

  switch (params->mode) {
    case SID_PAL_CRYPTO_ENCRYPT:
      psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_ENCRYPT);

      // Create a buffer for ciphertext and mac
      cipher_mac = (uint8_t *)sl_malloc((params->out_size + params->mac_size));
      if (cipher_mac == NULL) {
        sid_ret = SID_ERROR_NULL_POINTER;
        goto exit;
      }

      ret = psa_import_key(&key_attr, params->key,
                           params->key_size >> 3, &key_id);
      if (ret != PSA_SUCCESS) {
        sid_ret = SID_ERROR_GENERIC;
        goto exit;
      }

      ret = psa_aead_encrypt(key_id,
                             aead_algo,
                             params->iv,
                             params->iv_size,
                             params->aad,
                             params->aad_size,
                             params->in,
                             params->in_size,
                             cipher_mac,
                             params->out_size + params->mac_size,
                             &(params->out_size));

      if (ret != PSA_SUCCESS
          || params->out_size != (params->in_size + params->mac_size)) {
        params->out_size = params->in_size;
        sid_ret = SID_ERROR_GENERIC;
        goto clnup;
      }

      // Copy output to buffers if output size is correct
      params->out_size = params->in_size;
      memcpy(params->out, cipher_mac, params->out_size);
      memcpy(params->mac, cipher_mac + params->out_size, params->mac_size);
      break;

    case SID_PAL_CRYPTO_DECRYPT:
      psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_DECRYPT);

      // Create a buffer to combine ciphertext and mac
      cipher_mac = (uint8_t *)sl_malloc((params->in_size + params->mac_size));
      if (cipher_mac == NULL) {
        sid_ret = SID_ERROR_NULL_POINTER;
        goto exit;
      }

      memcpy(cipher_mac, params->in, params->in_size);
      memcpy(cipher_mac + params->in_size, params->mac, params->mac_size);

      ret = psa_import_key(&key_attr, params->key,
                           params->key_size >> 3, &key_id);
      if (ret != PSA_SUCCESS) {
        sid_ret = SID_ERROR_GENERIC;
        goto exit;
      }

      ret = psa_aead_decrypt(key_id,
                             aead_algo,
                             params->iv,
                             params->iv_size,
                             params->aad,
                             params->aad_size,
                             cipher_mac,
                             params->in_size + params->mac_size,
                             params->out,
                             params->out_size,
                             &(params->out_size));

      if (ret != PSA_SUCCESS || params->out_size != params->in_size) {
        sid_ret = SID_ERROR_GENERIC;
        goto clnup;
      }
      break;

    default:
      return SID_ERROR_INVALID_ARGS;
  }

  clnup:
  ret = psa_destroy_key(key_id);
  if (ret != PSA_SUCCESS && sid_ret == SID_ERROR_NONE) {
    sid_ret = SID_ERROR_GENERIC;
  }

  exit:
  if (cipher_mac) {
    sl_free(cipher_mac);
  }

  return sid_ret;
}

static sid_error_t efr32_crypto_ecc_dsa(sid_pal_dsa_params_t *params)
{
  psa_status_t ret;
  psa_key_handle_t key_id;
  psa_key_attributes_t key_attr;

  if (params->in_size == 0) {
    return SID_ERROR_PARAM_OUT_OF_RANGE;
  }

  key_attr = psa_key_attributes_init();

  switch (params->algo) {
    case SID_PAL_EDDSA_ED25519:
      psa_set_key_bits(&key_attr, 255);
      psa_set_key_algorithm(&key_attr, PSA_ALG_PURE_EDDSA);
      if (params->mode == SID_PAL_CRYPTO_SIGN) {
        psa_set_key_type(&key_attr,
                         PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_TWISTED_EDWARDS));
        psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_SIGN_MESSAGE);
      } else {
        psa_set_key_type(&key_attr,
                         PSA_KEY_TYPE_ECC_PUBLIC_KEY(PSA_ECC_FAMILY_TWISTED_EDWARDS));
        psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_VERIFY_MESSAGE);
      }
      break;

    case SID_PAL_ECDSA_SECP256R1:
      psa_set_key_bits(&key_attr, 256);   // Independent of private or public key
      psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
      if (params->mode == SID_PAL_CRYPTO_SIGN) {
        psa_set_key_type(&key_attr,
                         PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
        psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_SIGN_MESSAGE);
      } else {
        psa_set_key_type(&key_attr,
                         PSA_KEY_TYPE_ECC_PUBLIC_KEY(PSA_ECC_FAMILY_SECP_R1));
        psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_VERIFY_MESSAGE);
      }
      break;

    default:
      return SID_ERROR_NOSUPPORT;
  }

  switch (params->mode) {
    case SID_PAL_CRYPTO_SIGN:
      if (secure_vault_enabled) {
        key_id = *((psa_key_handle_t *)params->key);
      } else {
        ret = psa_import_key(&key_attr, params->key, params->key_size, &key_id);
        if (ret != PSA_SUCCESS) {
          return SID_ERROR_GENERIC;
        }
      }
      if (params->algo == SID_PAL_EDDSA_ED25519) {
        ret = psa_sign_message(key_id,
                               PSA_ALG_PURE_EDDSA,
                               params->in,
                               params->in_size,
                               params->signature,
                               params->sig_size,
                               &(params->sig_size));
        if (ret != PSA_SUCCESS && params->sig_size != 32) {
          if (!secure_vault_enabled) {
            psa_destroy_key(key_id);
          }
          return SID_ERROR_GENERIC;
        }
        break;
      }

      ret = psa_sign_message(key_id,
                             PSA_ALG_ECDSA(PSA_ALG_SHA_256),
                             params->in,
                             params->in_size,
                             params->signature,
                             params->sig_size,
                             &(params->sig_size));
      if (ret != PSA_SUCCESS && params->sig_size != 64) {
        if (!secure_vault_enabled) {
          psa_destroy_key(key_id);
        }
        return SID_ERROR_GENERIC;
      }
      break;

    case SID_PAL_CRYPTO_VERIFY: {
      if (params->algo == SID_PAL_EDDSA_ED25519) {
        ret = psa_import_key(&key_attr, params->key, params->key_size,
                             &key_id);
        if (ret != PSA_SUCCESS) {
          return SID_ERROR_GENERIC;
        }

        ret = psa_verify_message(key_id,
                                 PSA_ALG_PURE_EDDSA,
                                 params->in,
                                 params->in_size,
                                 params->signature,
                                 params->sig_size);

        if (ret != PSA_SUCCESS) {
          psa_destroy_key(key_id);
          return SID_ERROR_GENERIC;
        }
        break;
      }

      // Public key in uncompressed format
      uint8_t buf_tmp[65];
      buf_tmp[0] = 0x04;
      memcpy(buf_tmp + 1, params->key, params->key_size);
      ret = psa_import_key(&key_attr, buf_tmp, params->key_size + 1, &key_id);
      if (ret != PSA_SUCCESS) {
        return SID_ERROR_GENERIC;
      }

      ret = psa_verify_message(key_id,
                               PSA_ALG_ECDSA(PSA_ALG_SHA_256),
                               params->in,
                               params->in_size,
                               params->signature,
                               params->sig_size);
      if (ret != PSA_SUCCESS) {
        psa_destroy_key(key_id);
        return SID_ERROR_GENERIC;
      }
      break;
    }
    default:
      return SID_ERROR_INVALID_ARGS;
  }

  if ((params->mode != SID_PAL_CRYPTO_SIGN && secure_vault_enabled) || !secure_vault_enabled) {
    ret = psa_destroy_key(key_id);
    if (ret != PSA_SUCCESS) {
      return SID_ERROR_GENERIC;
    }
  }

  return SID_ERROR_NONE;
}

static sid_error_t efr32_crypto_ecc_ecdh(sid_pal_ecdh_params_t *params)
{
  psa_status_t ret;
  psa_key_handle_t key_id;
  psa_key_attributes_t key_attr;

  key_attr = psa_key_attributes_init();
  psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_DERIVE);
  psa_set_key_algorithm(&key_attr, PSA_ALG_ECDH);

  switch (params->algo) {
    case SID_PAL_ECDH_SECP256R1: {
      uint8_t buf_tmp[65];
      psa_set_key_bits(&key_attr, 256);   // Independent of private or public key
      psa_set_key_type(&key_attr,
                       PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
      ret = psa_import_key(&key_attr, params->prk, params->prk_size, &key_id);
      if (ret != PSA_SUCCESS) {
        return SID_ERROR_GENERIC;
      }

      // Public key in uncompressed format
      buf_tmp[0] = 0x04;
      memcpy(buf_tmp + 1, params->puk, params->puk_size);
      ret = psa_raw_key_agreement(PSA_ALG_ECDH,
                                  key_id,
                                  buf_tmp,
                                  params->puk_size + 1,
                                  params->shared_secret,
                                  params->shared_secret_sz,
                                  &(params->shared_secret_sz));
      break;
    }
    case SID_PAL_ECDH_CURVE25519:
      psa_set_key_bits(&key_attr, 255);
      psa_set_key_type(&key_attr,
                       PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_MONTGOMERY));
      ret = psa_import_key(&key_attr, params->prk, params->prk_size, &key_id);
      if (ret != PSA_SUCCESS) {
        return SID_ERROR_GENERIC;
      }

      ret = psa_raw_key_agreement(PSA_ALG_ECDH,
                                  key_id,
                                  params->puk,
                                  params->puk_size,
                                  params->shared_secret,
                                  params->shared_secret_sz,
                                  &(params->shared_secret_sz));
      break;

    default:
      return SID_ERROR_NOSUPPORT;
  }

  if (ret != PSA_SUCCESS || params->shared_secret_sz != 32) {
    psa_destroy_key(key_id);
    return SID_ERROR_GENERIC;
  }

  ret = psa_destroy_key(key_id);
  if (ret != PSA_SUCCESS) {
    return SID_ERROR_GENERIC;
  }

  return SID_ERROR_NONE;
}

static sid_error_t efr32_crypto_ecc_key_gen(sid_pal_ecc_key_gen_params_t *params)
{
  psa_status_t ret;
  sid_error_t sid_ret = SID_ERROR_NONE;
  psa_key_handle_t key_id;
  psa_key_attributes_t key_attr;
  size_t prk_size;

  key_attr = psa_key_attributes_init();

  switch (params->algo) {
    case SID_PAL_EDDSA_ED25519:
      psa_set_key_type(&key_attr,
                       PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_TWISTED_EDWARDS));
      psa_set_key_bits(&key_attr, 255);
      psa_set_key_usage_flags(&key_attr,
                              PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_SIGN_MESSAGE);
      psa_set_key_algorithm(&key_attr, PSA_ALG_PURE_EDDSA);
      break;

    case SID_PAL_ECDSA_SECP256R1:
      psa_set_key_type(&key_attr,
                       PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
      psa_set_key_bits(&key_attr, 256);
      psa_set_key_usage_flags(&key_attr,
                              PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_SIGN_HASH);
      psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA_ANY);
      break;

    case SID_PAL_ECDH_SECP256R1:
      psa_set_key_type(&key_attr,
                       PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
      psa_set_key_bits(&key_attr, 256);
      psa_set_key_usage_flags(&key_attr,
                              PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_DERIVE);
      psa_set_key_algorithm(&key_attr, PSA_ALG_ECDH);
      break;

    case SID_PAL_ECDH_CURVE25519:
      psa_set_key_type(&key_attr,
                       PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_MONTGOMERY));
      psa_set_key_bits(&key_attr, 255);
      psa_set_key_usage_flags(&key_attr,
                              PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_DERIVE);
      psa_set_key_algorithm(&key_attr, PSA_ALG_ECDH);
      break;

    default:
      return SID_ERROR_NOSUPPORT;
  }

  ret = psa_generate_key(&key_attr, &key_id);
  if (ret != PSA_SUCCESS) {
    return SID_ERROR_GENERIC;
  }

  ret = psa_export_key(key_id, params->prk, params->prk_size, &prk_size);
  if (ret != PSA_SUCCESS || params->prk_size != prk_size) {
    sid_ret = SID_ERROR_GENERIC;
    goto clnup;
  }

  size_t puk_size;
  if (params->puk_size == 64) {
    uint8_t buf_tmp[65];
    ret = psa_export_public_key(key_id,
                                buf_tmp,
                                params->puk_size + 1,
                                &puk_size);
    memcpy(params->puk, buf_tmp + 1, params->puk_size);
    puk_size = 64;
  } else {
    ret = psa_export_public_key(key_id,
                                params->puk,
                                params->puk_size,
                                &puk_size);
  }

  if (ret != PSA_SUCCESS || params->puk_size != puk_size) {
    sid_ret = SID_ERROR_GENERIC;
    goto clnup;
  }

  clnup:
  ret = psa_destroy_key(key_id);
  if (ret != PSA_SUCCESS) {
    sid_ret = SID_ERROR_GENERIC;
  }

  return sid_ret;
}
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
// extern this function and call it from the application
// preferably before sid_pal_crypto_init()
void silabs_crypto_enable_sv(void)
{
  secure_vault_enabled = true;
}

sid_error_t sid_pal_crypto_init()
{
  if (hal_init_done) {
    return SID_ERROR_NONE;
  }

  sid_error_t ret = efr32_crypto_init();
  if (ret != SID_ERROR_NONE) {
    return ret;
  }

  hal_init_done = true;

  uint32_t seed;
  ret = sid_pal_crypto_rand((uint8_t*)&seed, sizeof(seed));
  if (ret == SID_ERROR_NONE) {
    srand(seed);
  } else {
    hal_init_done = false;
  }

  return ret;
}

sid_error_t sid_pal_crypto_deinit(void)
{
  efr32_crypto_deinit();
  hal_init_done = false;

  return SID_ERROR_NONE;
}

sid_error_t sid_pal_crypto_rand(uint8_t *rand, size_t size)
{
  if (!hal_init_done) {
    return SID_ERROR_UNINITIALIZED;
  }

  if (!rand || !size) {
    return SID_ERROR_NULL_POINTER;
  }

  return efr32_crypto_rand(rand, size);
}

sid_error_t sid_pal_crypto_hash(sid_pal_hash_params_t *params)
{
  if (!hal_init_done) {
    return SID_ERROR_UNINITIALIZED;
  }

  if (!params || !params->data || !params->digest) {
    return SID_ERROR_NULL_POINTER;
  }

  return efr32_crypto_hash(params);
}

sid_error_t sid_pal_crypto_hmac(sid_pal_hmac_params_t *params)
{
  if (!hal_init_done) {
    return SID_ERROR_UNINITIALIZED;
  }

  if (!params || !params->key || !params->data || !params->digest) {
    return SID_ERROR_NULL_POINTER;
  }

  return efr32_crypto_hmac(params);
}

sid_error_t sid_pal_crypto_aes_crypt(sid_pal_aes_params_t *params)
{
  if (!hal_init_done) {
    return SID_ERROR_UNINITIALIZED;
  }

  if (!params || !params->key || !params->in || !params->out) {
    return SID_ERROR_NULL_POINTER;
  }

  return efr32_crypto_aes_crypt(params);
}

sid_error_t sid_pal_crypto_aead_crypt(sid_pal_aead_params_t *params)
{
  if (!hal_init_done) {
    return SID_ERROR_UNINITIALIZED;
  }

  if (!params || !params->key || !params->in || !params->out) {
    return SID_ERROR_NULL_POINTER;
  }

  return efr32_crypto_aead_crypt(params);
}

sid_error_t sid_pal_crypto_ecc_dsa(sid_pal_dsa_params_t *params)
{
  if (!hal_init_done) {
    return SID_ERROR_UNINITIALIZED;
  }

  if (!params || !params->key || !params->in || !params->signature) {
    return SID_ERROR_NULL_POINTER;
  }

  return efr32_crypto_ecc_dsa(params);
}

sid_error_t sid_pal_crypto_ecc_ecdh(sid_pal_ecdh_params_t *params)
{
  if (!hal_init_done) {
    return SID_ERROR_UNINITIALIZED;
  }

  if (!params || !params->prk || !params->puk || !params->shared_secret) {
    return SID_ERROR_NULL_POINTER;
  }

  return efr32_crypto_ecc_ecdh(params);
}

sid_error_t sid_pal_crypto_ecc_key_gen(sid_pal_ecc_key_gen_params_t *params)
{
  if (!hal_init_done) {
    return SID_ERROR_UNINITIALIZED;
  }

  if (!params || !params->prk || !params->puk) {
    return SID_ERROR_NULL_POINTER;
  }

  return efr32_crypto_ecc_key_gen(params);
}
