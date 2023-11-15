# SFU785

# 1. Installing OpenSSL:
On Ubuntu:
```
sudo apt-get update
sudo apt-get install libssl-dev
```
On CentOS/RHEL:
```
sudo yum install openssl-devel
```
On Fedora:
```
sudo dnf install openssl-devel
```
On macOS:
```
brew install openssl
```
# 2. Compiling Your Program:
```
g++ -std=c++17 -o my_program my_program.cpp -lssl -lcrypto
```
or in vscode, add this into settings.json
```
"code-runner.executorMap": {
        "cpp": "cd $dir && g++ -std=c++17 $fileName -o $fileNameWithoutExt -lssl -lcrypto && $dir$fileNameWithoutExt"
    },
```# Bibifig1
