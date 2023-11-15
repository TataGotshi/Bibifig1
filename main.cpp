#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>

#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>
// #include <openssl/evp.h>
// #include <openssl/aes.h>
// #include <openssl/rand.h>

// #include "encryption.cpp"


namespace fs = std::filesystem;

// using namespace std;
std::string directory;
#include "User.cpp"
#include "cmd.cpp"
//assign key length
const int AES_KEY_LENGTH = 256;
const int RSA_KEY_BITS = 2048;


fs::path binaryPath;
fs::path publicKeysPath;
fs::path privateKeysPath;

// void generateKey(const std::string& keyFile, const std::string& username, const std::string& password) {
//     // ... (unchanged)

//     std::cout << "Key file created successfully: " << keyFile << std::endl;

//     // Clean up
//     fclose(keyFilePtr);
//     RSA_free(rsa);
// }


// File System Abstraction:You might have classes for Directory and File to abstract file system operations.
// class Directory {
// //...
// };

// class File {
// //...
// };



//create keyfile at current directory
//I am not sure shall we put the keyfile in the filesystem or outside of the file system.
//we need to encrypt this part
void createKeyfile(std::string directory, std::string filename, std::string text){
    std::string keyfilepath = directory + "/"+filename+"keyfile.pem";

    std::ofstream keyfile(keyfilepath);
    keyfile << text << std::endl; 
    // keyfile << "password\n";
    keyfile.close();

    std::cout << keyfilepath + " is created! " << std::endl; 

};

void createAdminKeyfile(std::string public_directory, std::string private_directory, std::string username){
    std::string publickeyfilepath = public_directory + "/admin_keyfile.pem";
    std::string privatekeyfilepath = private_directory + "/admin_keyfile.pem";

    RSA* rsa = generate_key_pair();
    if (rsa == nullptr) {
        std::cerr << "RSA key generation failed." << std::endl;
    }
    save_key_pair(rsa, publickeyfilepath, privatekeyfilepath);
    RSA_free(rsa);

    // std::ofstream keyfile(keyfilepath, std::ios::app);
    // keyfile << username << "\n"; 
    // keyfile.close();

    std::cout << public_directory + "/admin_keyfile.pem" + " is updated! " << std::endl; 
    std::cout << private_directory + "/admin_keyfile.pem" + " is updated! " << std::endl; 

};


//shell functions
//get current path
void getPath(){
    fs::path currentPath = fs::current_path();
    std::cout << "Current working directory: " << currentPath << std::endl;
}
//list directory
void listDirectory(const std::string& path = directory) {
    for (const auto& entry : fs::directory_iterator(path)) {
        std::cout << entry.path().filename() << std::endl;
    }
}
//change directory
void changeDirectory(const std::string& path) {
    if (fs::exists(path) && fs::is_directory(path)) {
        fs::current_path(path);
        std::cout << "Changed to directory: " << path << std::endl;
    } else {
        std::cerr << "Directory not found: " << path << std::endl;
    }
}
//create directory
void createDirectory(const std::string& path) {
    if (fs::create_directory(path)) {
        std::cout << "Created directory: " << path << std::endl;
    } else {
        std::cerr << "Failed to create directory: " << path << std::endl;
    }
}
//create file
void createFile(const std::string& path) {
    std::ofstream file(path);
    if (file.is_open()) {
        std::cout << "Created file: " << path << std::endl;
    } else {
        std::cerr << "Failed to create file: " << path << std::endl;
    }
}


//create new user and generate new keyfile

// void adduser(std::string newusername){
//     // std::cout << "new user added" << std::endl;
//     //update admin keyfile


//     int key = 4;
//     std::string ciphertext;
//     std::string text_decrypted;

//     ciphertext = encrypt(newusername,key);
//     createKeyfile(directory, newusername + "_",ciphertext);

//     text_decrypted = decrypt(ciphertext,key);
// };




//get the current direcotry from main argument
std::string getPath(std::string directory){

    directory.erase(directory.find_last_of('/')+1);
    return directory;
};


// Main Function: Entry point to program:
int main(int argc, char* argv[]) {
    directory = getPath(argv[0]);
    binaryPath = fs::current_path();
    // if public keys directory does not exist then create one
    std::string publicKeys = "public_keys";
    fs::path pathToKeys = binaryPath / publicKeys;
    std::cerr << pathToKeys << "PathToKeys is this " << std::endl;
    publicKeysPath = pathToKeys;
    if (!fs::exists(pathToKeys)) {
        fs::create_directory("public_keys");
    }
    // if private keys directory does not exist then create one
    std::string privateKeys = "private_keys";
    pathToKeys = binaryPath / privateKeys;
    std::cerr << pathToKeys << "PathToKeys is this " << std::endl;
    privateKeysPath = pathToKeys;
    if (!fs::exists(pathToKeys)) {
        fs::create_directory("private_keys");
    }


    //if the user is not input any argument for key file
    if (argc != 2) {
        std::cerr << "need keyfile " << std::endl;
        std::cerr << "we will create a keyfile for you" << std::endl;
        // createKeyfile(directory);//create key file and exit the application
        createAdminKeyfile(publicKeysPath, privateKeysPath, "admin");//create admin key file 

        return 1;
    }

    // std::cout << "public key path: " << publicKeysPath << std::endl;
    // std::cout << "private key path: " << privateKeysPath << std::endl;
    
    // if (argv[1].length() < RSA_KEY_BITS) {
    // // Copy data into buffer
    // strncpy(buffer, input.c_str(), sizeof(buffer) - 1);
    // buffer[sizeof(buffer) - 1] = '\0'; // Ensure null-termination

    User user(argv[1]);
    FileServer server;

    if (user.isAdmin()){
        std::cout << "Logged in as " << user.getUsername() << std::endl;
        std::cerr << "Wellcome Administrator!" << std::endl;
        // shell(user);
        server.run(fs::current_path(), publicKeysPath, privateKeysPath, 0, user.getUsername());
    }else if(user.authenticate()){
        std::cout << "Logged in as " << user.getUsername() << std::endl;
        // shell(user);
        server.run(fs::current_path(), publicKeysPath, privateKeysPath, 1, user.getUsername());
    }else{
        std::cerr << "Invalid keyfile" << std::endl;
        // return 1;
    }

    // } else {
    //     std::cout << "invalid keyfile size" << std::endl;
    // }




    return 0;
}





