#ifndef RSA_KEYGEN_H
#define RSA_KEYGEN_H

#include <openssl/rsa.h>
#include <string>
#include <vector>

RSA* generate_key_pair();
void save_key_pair(RSA* r, const std::string& public_key_filepath, const std::string& private_key_filepath);
RSA* read_public_key(const std::string& public_key_filepath);
RSA* read_private_key(const std::string& private_key_filepath);
std::vector<unsigned char> encrypt_data(const std::string& data, RSA* rsa_pub);
std::string decrypt_data(const std::vector<unsigned char>& encrypted_data, RSA* rsa_priv);

#endif // RSA_KEYGEN_H