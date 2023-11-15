#include <iostream>
#include <string>
#include <list>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <map>
// #include "rsa_keygen.cpp"

// #pragma once 
#include "rsa_keygen.cpp"


class User {
private:
    std::string username, keyfilefullpath;
    std::string keyfile, keyfileparentpath, publickeyfilecontent, privatekeyfilecontent;
    std::string password;
    std::stringstream ciphertext;
    std::ifstream adminkeyfile, userkeyfile;
    std::filesystem::path keyfilepath();
    // std::string username.substr(0,username.find("_keyfile"))


public:
    User(const std::string& keyfile_name) : keyfile(keyfile_name) {
        // Mock logic to extract username from keyfile
        // keyfile = keyfile;
        keyfilefullpath = keyfile_name;
        std::filesystem::path keyfilepath(keyfile);
        // std::cout << ciphertext.str() << std::endl;

        if (keyfilepath.has_filename()){
            keyfile = keyfilepath.filename();
        }   

        if (keyfilepath.has_parent_path()){
            keyfileparentpath = keyfilepath.parent_path();
        }     

        publickeyfilecontent = getContent(keyfile_name);

        
        // std::cout << "keyfile: " << keyfile << " keypath:" << keyfileparentpath << std::endl;
        username = keyfile.substr(0,keyfile.find("_keyfile.pem"));
        // std::cout << "this is the username " << username << std::endl;

    }

    std::string getContent(std::string keyfile){
        std::ifstream userkeyfile(keyfile);
        // while (userkeyfile.get(ciphertext)) {
        //     // ciphertext += line; 
        // };
        std::stringstream content;
        content << userkeyfile.rdbuf();
        userkeyfile.close();
        return content.str();
    }
    
    bool isAdmin() {
        // if the user is using admin_keyfile
        // return keyfile == "admin_keyfile.pem";  // Simplified condition for demonstration
        if (authenticate() and username == "admin"){
            return true;
        }else{
            return false;
        }
    }



    bool authenticate(){
        //Mock logic to authenticate user
        //encryption username with public key file 
        //encryption start
        std::string publickeypath = "../public_keys/" + username + "_keyfile.pem";
        RSA* rsa_pub = read_public_key(publickeypath); //#Todo, change the path of public key

        if (rsa_pub == nullptr) {
            std::cerr << "Reading public key failed." << std::endl;
            return false;
        }
        // std::cout << "\nthis is the data before encryption: " << username << std::endl;
        std::vector<unsigned char> encrypted_data = encrypt_data(username, rsa_pub);

        RSA_free(rsa_pub);  // Free the RSA structure after use

        // std::cout <<  "\n this is the data after encryption: " << encrypted_data.data() << std::endl;
        //encryption complete

        //decryp the encrypted_data with private key provided by the user
        //decryption start
        std::string privatekeyfilepath = keyfilefullpath;//private key
        RSA* rsa_priv = read_private_key("../" + privatekeyfilepath); //#Todo, change the path of pprivatekey
        //check the size of the keyfile to handle over buffer flow

        if (rsa_priv == nullptr) {
            std::cerr << "Reading private key failed." << std::endl;
            return false ;
        }
        std::string decrypted_data = decrypt_data(encrypted_data, rsa_priv);
        // std::cout << decrypted_data << std::endl;
        RSA_free(rsa_priv);  // Free the RSA structure after use
        //decryption completed
        if (decrypted_data == username)return true;


        return false;
    }
    

    std::string getUsername() const {
        return username;
    }
};