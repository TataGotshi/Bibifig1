#ifndef SHARED_CODE_CPP
#define SHARED_CODE_CPP


#include "rsa_keygen.h"
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#include <string>
#include <vector>

RSA* generate_key_pair() {
    RSA *r = RSA_new();
    BIGNUM *e = BN_new();

    if (e == NULL || r == NULL) {
        ERR_print_errors_fp(stderr);
        RSA_free(r);
        BN_free(e);
        return nullptr;
    }

    BN_set_word(e, RSA_F4);  // e is the public exponent, usually 65537

    if (RSA_generate_key_ex(r, 2048, e, NULL) != 1) {
        ERR_print_errors_fp(stderr);
        RSA_free(r);
        BN_free(e);
        return nullptr;
    }

    BN_free(e);  // Free the BIGNUM now that it's no longer needed
    return r;
}

void save_key_pair(RSA* r, const std::string& public_key_filepath, const std::string& private_key_filepath) {
    BIO *bio = BIO_new_file(public_key_filepath.c_str(), "w+");
    PEM_write_bio_RSAPublicKey(bio, r);
    BIO_free(bio);

    bio = BIO_new_file(private_key_filepath.c_str(), "w+");
    PEM_write_bio_RSAPrivateKey(bio, r, NULL, NULL, 0, NULL, NULL);
    BIO_free(bio);

}

// read keyfiles
RSA* read_public_key(const std::string& public_key_filepath) {
    BIO *bio = BIO_new_file(public_key_filepath.c_str(), "r");
    RSA *rsa_pub = PEM_read_bio_RSAPublicKey(bio, NULL, NULL, NULL);
    BIO_free(bio);
    return rsa_pub;
}

RSA* read_private_key(const std::string& private_key_filepath) {
    BIO *bio = BIO_new_file(private_key_filepath.c_str(), "r");
    if (!bio) {
        std::cerr << "Error opening private key file" << std::endl;
        return nullptr;
    }

    RSA *rsa_priv = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
    if (RSA_size(rsa_priv) != 256){
        std::cerr << "invalid keyfile length" << std::endl;
        return nullptr;
    }
    if (!rsa_priv) {
        std::cerr << "Error reading private key" << std::endl;
        ERR_print_errors_fp(stderr); // Print OpenSSL errors to stderr
        return nullptr;
    }
    BIO_free(bio);
    return rsa_priv;
}

// encrypt and decrypt
std::vector<unsigned char> encrypt_data(const std::string& data, RSA* rsa_pub) {
    std::vector<unsigned char> encrypted(RSA_size(rsa_pub));
    int encLen = RSA_public_encrypt(data.size(), (unsigned char*)data.c_str(), encrypted.data(), rsa_pub, RSA_PKCS1_OAEP_PADDING);
    if (encLen == -1) {
        ERR_print_errors_fp(stderr);
        encrypted.clear();
    }
    return encrypted;
}

std::string decrypt_data(const std::vector<unsigned char>& encrypted_data, RSA* rsa_priv) {
    std::vector<unsigned char> decrypted(RSA_size(rsa_priv));
    int decLen = RSA_private_decrypt(encrypted_data.size(), encrypted_data.data(), decrypted.data(), rsa_priv, RSA_PKCS1_OAEP_PADDING);
    if (decLen == -1) {
        ERR_print_errors_fp(stderr);
        return "";
    }
    return std::string(decrypted.begin(), decrypted.begin() + decLen);
}


#endif