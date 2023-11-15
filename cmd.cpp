#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <map>
#include <regex>  

// used for keypair generation
#include "rsa_keygen.cpp"

namespace fs = std::filesystem;
int role = 1;

class FileServer {
public:
    FileServer() : current_path(fs::current_path() / "filesystem") {
        fs::create_directory(current_path);
        fs::current_path(current_path);
    }


    void run(fs::path root, fs::path publicKeysPath, fs::path privateKeysPath, int typeOfUser, std::string username) {
        rootPathToFileServer = root;
        publicKeysFolder = publicKeysPath;
        privateKeysFolder = privateKeysPath;
        userType = typeOfUser;
        userRoot = current_path;
        setWorkingDirectory(userRoot);
        if (userType == 1) {
            userRoot = current_path / username;
            userRoot = userRoot.lexically_normal();
            changeDirectory(userRoot);
            setWorkingDirectory(userRoot);
        }
        std::string command;
        while (true) {
            std::cout << ">> ";
            std::getline(std::cin, command);
            if (command == "exit") break;
            executeCommand(command, username, typeOfUser);
        }
    }

    // to show the directory in the filesystem
    void setWorkingDirectory(const fs::path& path) {
        working_directory = path;
    }

private:
    fs::path working_directory;
    fs::path current_path;
    fs::path rootPathToFileServer;
    fs::path userRoot;  // users should not be able to cd a level above userroot
    fs::path publicKeysFolder;
    fs::path privateKeysFolder;
    int userType; // if userType is 0 then admin, if userType is 1 then it is a user.


    void executeCommand(const std::string& command, const std::string& username, const bool& isUser) {
        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;

        if (cmd == "cd") {
            std::string dir;
            iss >> dir;
            changeDirectory(dir);
        } else if (cmd == "pwd") {
            printWorkingDirectory();
        } else if (cmd == "ls") {
            listDirectory();
        } else if (cmd == "mkdir") {
            if (isUser){
                std::string dir;
                iss >> dir;
                makeDirectory(dir);
            }else{
                std::cout << "Sorry, Admin is not allowed to create directories" << std::endl;
            }
            
        } else if (cmd == "mkfile") {
            if (isUser){
                std::string filename, contents;
                iss >> filename;
                // check file content
                if (iss.eof()) {
                    std::cout << "Error: No content provided." << std::endl;
                    return;
                }
                std::getline(iss, contents);  // Get the rest of the line
                createFile(filename, contents.substr(1), username);  // Skip leading space
            }else{
                std::cout << "Sorry, Admin is not allowed to create files" << std::endl;
            }
            
        } else if (cmd == "cat") {
            std::string filename;
            iss >> filename;
            readFile(filename, username, isUser);
        } else if (cmd == "share") {
            if (isUser){
                std::string filename, username;
                iss >> filename >> username;
                shareFile(filename, username);
            }else{
                std::cout << "Sorry, Admin is not allowed to share" << std::endl;
            }
            
        } else if (cmd == "adduser" && userType == 0) {
            if (isUser){
                std::cout << "Unknown command" << std::endl;
            }else{
                std::string username;
                iss >> username;
                createUser(username);
            }
            
        }else {
            std::cout << "Unknown command" << std::endl;
        }
    }

    void changeDirectory(const std::string& dir) {
        fs::path new_path = current_path / dir;
        new_path = new_path.lexically_normal();  // Resolve . and ..

        // Ensure the new path does not escape userRoot
        if (new_path.string().find(userRoot.string()) != 0 || !fs::exists(new_path) || !fs::is_directory(new_path)) {
            std::cout << "Directory does not exist" << std::endl;
            return;
        }

        if (fs::exists(new_path) && fs::is_directory(new_path)) {
            current_path = new_path;
            working_directory = fs::canonical(new_path);  // Update working_directory, obtain the canonical form of the path
            fs::current_path(new_path);
        } else {
            std::cout << "Directory does not exist" << std::endl;
        }
    }

    void printWorkingDirectory() {
        fs::path relative_path = fs::relative(working_directory, userRoot);
        // std::cout << "workingdir" << working_directory << "root" << userRoot << std::endl;//testing ###
        std::cout << "/" << relative_path.string() << std::endl;
    }

    void listDirectory() {
        std::cout << "d -> ." << std::endl;
        // check if the directory is root dir
        if (working_directory != userRoot) {
            std::cout << "d -> .." << std::endl;
        }
        for (const auto& entry : fs::directory_iterator(current_path)) {
            std::cout << (entry.is_directory() ? "d" : "f") << " -> " << entry.path().filename().string() << std::endl;
        }
    }

    void makeDirectory(const std::string& dir) {
        fs::path new_dir = current_path / dir;
        new_dir = new_dir.lexically_normal();  // Resolve . and ..
        if (new_dir.string().find(userRoot.string()) != 0) {
            std::cout << "Directory path invalid" << std::endl;
            return;
        }
        // deny mkdir in root and share directory
        std::string share_path = userRoot.string() + "/shared";
        fs::path current_dir = new_dir.parent_path();  // get the directory path of parent
        if (current_dir.string() == userRoot.string() || current_dir.string() == share_path){
            std::cout << "Forbidden" << std::endl;
            return;
        }
        if (!fs::create_directory(new_dir)) {
            std::cout << "Directory already exists" << std::endl;
        }
    }

    void createFile(const std::string& filename, const std::string& contents, const std::string& username) {
        fs::path file_path = current_path / filename;
        file_path = file_path.lexically_normal();  // Resolve . and ..
        fs::path current_path_lex = file_path.parent_path();  // get the directory path of the file
        // Ensure the path does not escape userRoot
        if (file_path.string().find(userRoot.string()) != 0 || fs::is_directory(file_path)) {
            std::cout << "Please create file in exist path" << std::endl;
            return;
        }
        // deny mkfile in root and share directory
        std::string share_path = userRoot.string() + "/shared";
        if (current_path_lex.string() == userRoot.string() || current_path_lex.string() == share_path){
            std::cout << "Forbidden" << std::endl;
            return;
        }
        std::ofstream file(file_path, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open file for writing: " << file_path << std::endl;
            return;
        }
        // encrypt the string
        std::string pb = publicKeysFolder;
        std::string publickeyfilepath = pb + "/" + username + "_keyfile.pem";
        RSA* rsa_pub = read_public_key(publickeyfilepath); //#Todo, change the path of public key
        if (rsa_pub == nullptr) {
            std::cerr << "Reading public key failed." << std::endl;
            return ;
        }
        std::vector<unsigned char> encrypted_data = encrypt_data(contents, rsa_pub);
        RSA_free(rsa_pub);  // Free the RSA structure after use
        file.write(reinterpret_cast<const char*>(encrypted_data.data()), encrypted_data.size());
    }

    void readFile(const std::string& filename, const std::string& username, const bool& isUser) {
        fs::path file_path = current_path / filename;
        file_path = file_path.lexically_normal();  // Resolve . and ..
        if (fs::exists(file_path) && fs::is_regular_file(file_path)) {
            std::ifstream file(file_path, std::ios::binary | std::ios::ate);
            if (!file) {
                std::cerr << "Failed to open file for reading: " << file_path << std::endl;
                return ;
            }
            // Ensure the path does not escape userRoot
            if (file_path.string().find(userRoot.string()) != 0 || fs::is_directory(file_path)) {
                std::cout << "File does not exist" << std::endl;
                return;
            }
            // decrypt
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);
            
            std::vector<unsigned char> encrypted_data(size);
            if (!file.read(reinterpret_cast<char*>(encrypted_data.data()), size)) {
                std::cerr << "Failed to read file: " << file_path << std::endl;
                return;
            }

            std::string pv = privateKeysFolder;
            std::string privatekeyfilepath;
            // admin and user are using different logic to read keyfiles
            // user
            if (isUser){
                // if reading from shared directory
                if (file_path.string().find("shared") != std::string::npos && fs::is_symlink(file_path)) {
                    fs::path actualFilePath = fs::read_symlink(file_path);
                    // get original owner username from path
                    // path looks like user/c/filesystem/user1/personal/test.txt
                    size_t posFileSystem = actualFilePath.string().find("filesystem");
                    size_t posFirstSlash = actualFilePath.string().find("/", posFileSystem);
                    size_t posSecondSlash = actualFilePath.string().find("/", posFirstSlash + 1);
                    std::string usr = actualFilePath.string().substr(posFirstSlash + 1, posSecondSlash - posFirstSlash - 1);
                    privatekeyfilepath = pv  + "/" + usr + "_keyfile.pem";
                } else {
                privatekeyfilepath = pv  + "/" + username + "_keyfile.pem";
                }
            } else {
                std::string path_username = getUserName(file_path);
                privatekeyfilepath = pv  + "/" + path_username + "_keyfile.pem";
            }
            
            RSA* rsa_priv = read_private_key(privatekeyfilepath); 
            if (rsa_priv == nullptr) {
                std::cerr << "Reading private key failed." << std::endl;
                return;
            }
            std::string decrypted_data = decrypt_data(encrypted_data, rsa_priv);
            std::cout << decrypted_data << std::endl;
            RSA_free(rsa_priv);  // Free the RSA structure after use


            // std::string line;
            // while (std::getline(file, line)) {
            //     std::cout << line << std::endl;
            // }
        } else {
            std::cout << filename << " doesn't exist" << std::endl;
        }
    }


    void createUser(const std::string& username) {
        // Regex to match a username consisting only of letters and numbers
        std::regex username_regex("^[a-zA-Z0-9]+$");
        if (!std::regex_match(username, username_regex)) {
            std::cout << "Invalid username. Username should only contain letters and numbers." << std::endl;
            return;
        }
        // check if user exists. If not then:
        // create keyfile
        // create directory for username under fileserver
        fs::current_path(rootPathToFileServer);
        // check if username already has a directory
        if (fs::exists(username)) {
            std::cout << "User " << username << " already exists "<< std::endl;
        } else {
            std::cout << rootPathToFileServer << " current path" << std::endl;
            fs::create_directory(username);
            fs::path new_path = fs::current_path() / username;
            new_path = new_path.lexically_normal();  
            std::cout << new_path << " New Path is this" << std::endl;
            fs::current_path(new_path);
            fs::create_directory("personal");
            fs::create_directory("shared");
            // userRoot = new_path;
            fs::current_path(rootPathToFileServer);

            std::string pb = publicKeysFolder;
            std::string publickeyfilepath = pb + "/" +username + "_keyfile.pem";
            std::string pv = privateKeysFolder;
            std::string privatekeyfilepath = pv + "/" +username + "_keyfile.pem";

            RSA* rsa = generate_key_pair();
            if (rsa == nullptr) {
                std::cerr << "RSA key generation failed." << std::endl;
            }
            save_key_pair(rsa, publickeyfilepath, privatekeyfilepath);
            RSA_free(rsa);
            // std::ofstream keyfile(keyfilepath, std::ios::app);
            // keyfile << username << "\n"; 
            // keyfile.close();
            std::cout << publickeyfilepath << " is updated! " << std::endl; 
            std::cout << privatekeyfilepath << " is updated! " << std::endl; 
        }

        // create private and shared directory for this user
    }


    void shareFile(const std::string& filename, const std::string& username) {
        // TODO need to manage user directories and permissions.
        std::cout << "Sharing " << filename << " with " << username << std::endl;
        // Check if file doesn't exist
        fs::path file_path = current_path / filename;
        if (!fs::exists(file_path)) {
            std::cout << "File " << filename << " doesn't exist" << std::endl;
            return;
        }
        if (fs::is_symlink(file_path)) {
            std::cout << "Cannot share a file that has been shared with you " << std::endl;
            return;
        }
        // Check if username doesnt exist
        fs::path userDirPath = rootPathToFileServer / username;
        if (!fs::exists(userDirPath)) {
            std::cout << "User " << username << " doesn't exist" << std::endl;
            return;
        }
        // add symlink
        fs::path targetUserShareDir = userDirPath / "shared";
        if (fs::exists(targetUserShareDir)) {
            fs::path link = targetUserShareDir / filename;
            std::cout << "target: " << file_path <<  std::endl;
            std::cout << "link: " << link << std::endl;
            fs::create_symlink(file_path, link);
        }
    }

    // for admin to get username when entering a user's directory
    std::string getUserName(const fs::path& currentPath) {
        // Check if the admin is in the filesystem root or a user's folder
        std::string root_str = userRoot.string();
        fs::path relative_path = fs::relative(currentPath, root_str);
        std::string relative_path_str = relative_path.string();        

        // Split the relative path string into components
        std::istringstream ss(relative_path_str);
        std::string token;
        std::getline(ss, token, '/');  // Get the first component of the relative path

        // Check if the first component is a valid user name
        if (!token.empty()) {
            return token;  // Return the user name
        }

        return "";  // Return an empty string if the admin is not in a user's folder or the user name is invalid
    }

};


// int main() {
//     FileServer server;
//     server.run();
//     return 0;
// }
