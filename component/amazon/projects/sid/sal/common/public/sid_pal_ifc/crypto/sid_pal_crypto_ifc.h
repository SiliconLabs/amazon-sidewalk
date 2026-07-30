/*
 * Copyright 2020-2023 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_CRYPTO_IFC_H
#define SID_PAL_CRYPTO_IFC_H

/**
 * \addtogroup sid_ifc
 * @{
 */
/**
 * \addtogroup sid_pal_ifc
 * @{
 */
/**
 * \addtogroup sid_pal_crypto_ifc_support
 * @{
 */

#include <sid_error.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * @addtogroup sid_pal_crypto_ifc_support_types Type definitions
 * @ingroup sid_pal_crypto_ifc_support
 * @{
 *****************************************************************************/
/**
 * @brief Enumeration of hash algorithms supported by SID PAL.
 *
 * This enumeration defines the hash algorithms that are supported by the 
 * SID PAL (Platform Abstraction Layer) for cryptographic operations.
 */
typedef enum {
    SID_PAL_HASH_SHA256 = 1,        /*!< SHA-256 hash algorithm */
    SID_PAL_HASH_SHA512,            /*!< SHA-512 hash algorithm */
} sid_pal_hash_algo_t;

/**
 * @brief Enumeration of AES algorithms supported by SID PAL.
 *
 * This enumeration defines the AES algorithms that are supported by the 
 * SID PAL (Platform Abstraction Layer) for cryptographic operations.
 */
typedef enum {
    SID_PAL_AES_CMAC_128 = 1,       /*!< AES-CMAC-128 algorithm */
    SID_PAL_AES_CTR_128,            /*!< AES-CTR-128 algorithm */
} sid_pal_aes_algo_t;

/**
 * @brief Enumeration of AEAD algorithms supported by SID PAL.
 *
 * This enumeration defines the AEAD algorithms that are supported by the 
 * SID PAL (Platform Abstraction Layer) for cryptographic operations.
 */
typedef enum {
    SID_PAL_AEAD_GCM_128 = 1,       /*!< AEAD-GCM-128 algorithm */
    SID_PAL_AEAD_CCM_128,           /*!< AEAD-CCM-128 algorithm */
    SID_PAL_AEAD_CCM_STAR_128,      /*!< AEAD-CCM-STAR-128 algorithm */
} sid_pal_aead_algo_t;

/**
 * @brief Enumeration of ECC algorithms supported by SID PAL.
 *
 * This enumeration defines the ECC algorithms that are supported by the 
 * SID PAL (Platform Abstraction Layer) for cryptographic operations.
 */
typedef enum {
    SID_PAL_ECDH_CURVE25519 = 1,    /*!< ECDH-Curve25519 algorithm */
    SID_PAL_ECDH_SECP256R1,         /*!< ECDH-SECP256R1 algorithm */
    SID_PAL_EDDSA_ED25519,          /*!< EdDSA-Ed25519 algorithm */
    SID_PAL_ECDSA_SECP256R1,        /*!< ECDSA-SECP256R1 algorithm */
} sid_pal_ecc_algo_t;

/**
 * @brief Enumeration of AES modes supported by SID PAL.
 *
 * This enumeration defines the AES modes that are supported by the 
 * SID PAL (Platform Abstraction Layer) for cryptographic operations.
 */
typedef enum {
    SID_PAL_CRYPTO_ENCRYPT = 1,     /*!< AES encryption mode */
    SID_PAL_CRYPTO_DECRYPT,         /*!< AES decryption mode */
    SID_PAL_CRYPTO_MAC_CALCULATE,   /*!< AES MAC calculation mode */
} sid_pal_aes_mode_t;

/**
 * @brief Enumeration of DSA modes supported by SID PAL.
 *
 * This enumeration defines the DSA modes that are supported by the 
 * SID PAL (Platform Abstraction Layer) for cryptographic operations.
 */
typedef enum {
    SID_PAL_CRYPTO_SIGN = 1,        /*!< DSA signing mode */
    SID_PAL_CRYPTO_VERIFY,          /*!< DSA verification mode */
} sid_pal_dsa_mode_t;

/**
 * @brief Parameters for hash operations.
 *
 * This structure defines the parameters required for hash operations
 * supported by the SID PAL (Platform Abstraction Layer).
 */
typedef struct {
    sid_pal_hash_algo_t algo;       /*!< Hash algorithm to be used */
    uint8_t const *data;            /*!< Pointer to the input data */
    size_t data_size;               /*!< Size of the input data */
    uint8_t *digest;                /*!< Pointer to the output digest */
    size_t digest_size;             /*!< Size of the output digest */
} sid_pal_hash_params_t;

/**
 * @brief Parameters for HMAC operations.
 *
 * This structure defines the parameters required for HMAC operations
 * supported by the SID PAL (Platform Abstraction Layer).
 */
typedef struct {
    sid_pal_hash_algo_t algo;       /*!< HMAC algorithm to be used */
    uint8_t const *key;             /*!< Pointer to the key */
    size_t key_size;                /*!< Size of the key */
    uint8_t const *data;            /*!< Pointer to the input data */
    size_t data_size;               /*!< Size of the input data */
    uint8_t *digest;                /*!< Pointer to the output digest */
    size_t digest_size;             /*!< Size of the output digest */
} sid_pal_hmac_params_t;

/**
 * @brief Parameters for AES cryptographic operations.
 *
 * This structure defines the parameters required for AES cryptographic
 * operations supported by the SID PAL (Platform Abstraction Layer).
 */
typedef struct {
    sid_pal_aes_algo_t algo;        /*!< AES algorithm to be used */
    sid_pal_aes_mode_t mode;        /*!< AES mode of operation */
    uint8_t const *key;             /*!< Pointer to the key */
    size_t key_size;                /*!< Size of the key */
    uint8_t const *iv;              /*!< Pointer to the initialization vector */
    size_t iv_size;                 /*!< Size of the initialization vector */
    uint8_t const *in;              /*!< Pointer to the input data */
    size_t in_size;                 /*!< Size of the input data */
    uint8_t *out;                   /*!< Pointer to the output data */
    size_t out_size;                /*!< Size of the output data */
} sid_pal_aes_params_t;

/**
 * @brief Parameters for AEAD cryptographic operations.
 *
 * This structure defines the parameters required for AEAD cryptographic
 * operations supported by the SID PAL (Platform Abstraction Layer).
 */
typedef struct {
    sid_pal_aead_algo_t algo;       /*!< AEAD algorithm to be used */
    sid_pal_aes_mode_t mode;        /*!< AEAD mode of operation */
    uint8_t const *key;             /*!< Pointer to the key */
    size_t key_size;                /*!< Size of the key */
    uint8_t const *iv;              /*!< Pointer to the initialization vector */
    size_t iv_size;                 /*!< Size of the initialization vector */
    uint8_t const *aad;             /*!< Pointer to the additional authenticated data */
    size_t aad_size;                /*!< Size of the additional authenticated data */
    uint8_t const *in;              /*!< Pointer to the input data */
    size_t in_size;                 /*!< Size of the input data */
    uint8_t *out;                   /*!< Pointer to the output data */
    size_t out_size;                /*!< Size of the output data */
    uint8_t *mac;                   /*!< Pointer to the message authentication code */
    size_t mac_size;                /*!< Size of the message authentication code */
} sid_pal_aead_params_t;

/**
 * @brief Parameters for ECC DSA cryptographic operations.
 *
 * This structure defines the parameters required for ECC DSA cryptographic
 * operations supported by the SID PAL (Platform Abstraction Layer).
 */
typedef struct {
    sid_pal_ecc_algo_t algo;        /*!< ECC algorithm to be used */
    sid_pal_dsa_mode_t mode;        /*!< DSA mode of operation */
    uint8_t const *key;             /*!< Pointer to the key */
    size_t key_size;                /*!< Size of the key */
    uint8_t const *in;              /*!< Pointer to the input data */
    size_t in_size;                 /*!< Size of the input data */
    uint8_t *signature;             /*!< Pointer to the signature */
    size_t sig_size;                /*!< Size of the signature */
} sid_pal_dsa_params_t;

/**
 * @brief Parameters for ECDH cryptographic operations.
 *
 * This structure defines the parameters required for ECDH cryptographic
 * operations supported by the SID PAL (Platform Abstraction Layer).
 */
typedef struct {
    sid_pal_ecc_algo_t algo;        /*!< ECC algorithm to be used */
    uint8_t const *prk;             /*!< Pointer to the private key */
    size_t prk_size;                /*!< Size of the private key */
    uint8_t const *puk;             /*!< Pointer to the public key */
    size_t puk_size;                /*!< Size of the public key */
    uint8_t *shared_secret;         /*!< Pointer to the shared secret */
    size_t shared_secret_sz;        /*!< Size of the shared secret */
} sid_pal_ecdh_params_t;

/**
 * @brief Parameters for ECC key generation.
 *
 * This structure defines the parameters required for ECC key generation
 * supported by the SID PAL (Platform Abstraction Layer).
 */
typedef struct {
    sid_pal_ecc_algo_t algo;        /*!< ECC algorithm to be used */
    uint8_t *prk;                   /*!< Pointer to the private key */
    size_t prk_size;                /*!< Size of the private key */
    uint8_t *puk;                   /*!< Pointer to the public key */
    size_t puk_size;                /*!< Size of the public key */
} sid_pal_ecc_key_gen_params_t;

/** @} (end sid_pal_crypto_ifc_support_types) */

/**
 * @brief Initialize sid_pal crypto HAL.
 *
 * This function initializes the SID PAL crypto HAL and prepares it for cryptographic operations.
 *
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_init(void);

/**
 * @brief Deinitialize crypto HAL.
 *
 * This function deinitializes the SID PAL crypto HAL and releases any resources that were allocated.
 *
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_deinit(void);

/**
 * @brief Generate random number.
 *
 * This function generates a random number and stores it in the provided buffer.
 *
 * @param[out]  rand   Pointer to rand buffer.
 * @param[in]   size   Size of rand number
 *
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_rand(uint8_t *rand, size_t size);

/**
 * @brief Generate hash. SHA256 and SHA512 is now supported.
 *
 * This function generates a hash using the specified parameters.
 *
 * @param[in,out]  params  Pointer to the hash parameters.
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_hash(sid_pal_hash_params_t *params);

/**
 * @brief Generate HMAC. HMAC/SHA256 and HMAC/SHA512 is now supported.
 *
 * This function generates an HMAC using the specified parameters.
 *
 * @param[in,out]  params  Pointer to the hash parameters.
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_hmac(sid_pal_hmac_params_t* params);

/**
 * @brief Encrypt or decrypt using following AES algorithm. AES-CMAC AES-CTR are supported.
 *
 * This function encrypts or decrypts data using the specified AES algorithm.
 *
 * @param[in,out]  params  Pointer to AES parameters.
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_aes_crypt(sid_pal_aes_params_t *params);

/**
 * @brief Encrypt or decrypt using AEAD algorithm.
 *
 * This function encrypts or decrypts data using the specified AEAD algorithm.
 *
 * @param[in,out]  params  Pointer to the AEAD parameters.
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_aead_crypt(sid_pal_aead_params_t  *params);

/**
 * @brief Sign or verify elliptic curve digital signature
 *        using given algorithm.
 *
 * This function signs or verifies an elliptic curve digital signature using the specified parameters.
 *
 * @param[in,out]  params  Pointer to the ECC DSA parameters.
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_ecc_dsa(sid_pal_dsa_params_t *params);

/**
 * @brief Generate shared secret using private key and public key
 *
 * This function generates a shared secret using the specified private key and public key.
 *
 * @param[in,out]  params  Pointer to the ECDH parameters.
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_ecc_ecdh(sid_pal_ecdh_params_t *params);

/**
 * @brief Generate ECC key pair using  given algorithm.
 *
 * This function generates an ECC key pair using the specified algorithm.
 *
 * @param[in,out]  params  Generate ECC key pair using given algorithm.
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_ecc_key_gen(sid_pal_ecc_key_gen_params_t *params);

#ifdef __cplusplus
}
#endif

#endif /* SID_PAL_CRYPTO_IFC_H */

/** @} */ // end of sid_ifc group
/** @} */ // end of sid_pal_ifc group
/** @} */ // end of sid_pal_crypto_ifc_support group
