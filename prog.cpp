#include <iostream>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <cassert>
#include <random>
#include <limits>
#include <string>
#include <cctype>

class EncryptionService {
public:
    static std::string encode(const std::string& data, const std::string& key) {
        return "coded_" + key + "" + std::to_string(data.length()) + "" + data;
    }

    static std::string decode(const std::string& data) {
        if (data.substr(0, 6) == "coded_") {
            size_t pos = data.find_last_of("_");
            return data.substr(pos + 1);
        }
        return data;
    }

    static std::string createSalt() {
        static const char* charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        std::string salt;
        for (int i = 0; i < 16; ++i) {
            salt += charset[rand() % (sizeof(charset) - 1)];
        }
        return salt;
    }
};

class PasswordGenerator {
public:
    static std::string generateStrongPassword(size_t length = 12) {
        static const std::string chars =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "!@#$%^&*()";
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, chars.size() - 1);

        std::string password;
        for (size_t i = 0; i < length; ++i) {
            password += chars[distribution(generator)];
        }

        return password;
    }
};

class PasswordKeeper {
private:
    std::unordered_map<std::string, std::pair<std::string, std::string>> credentials;
    std::string filepath;

    bool isValidPassword(const std::string& pass) {
        bool hasUpperCase = false;
        for (char c : pass) {
            if (std::isupper(c)) {
                hasUpperCase = true;
                break;
            }
        }
        return pass.length() >= 8 && hasUpperCase;
    }

    void loadCredentials() {
        std::ifstream file(filepath);
        std::string user, codedPassword, key;
        while (file >> user >> key >> codedPassword) {
            credentials[user] = { codedPassword, key };
        }
    }

    void storeCredentials() {
        std::ofstream file(filepath, std::ios::trunc); // Open file in overwrite mode
        for (const auto& pair : credentials) {
            file << pair.first << " " << pair.second.second << " " << pair.second.first << "\n";
        }
    }

public:
    PasswordKeeper(const std::string& path) : filepath(path) {
        loadCredentials();
    }

    ~PasswordKeeper() {
        storeCredentials();
    }

    bool registerUser(const std::string& user, const std::string& pass) {
        if (credentials.find(user) == credentials.end()) {
            if (isValidPassword(pass)) {
                std::string key = EncryptionService::createSalt();
                credentials[user] = { EncryptionService::encode(pass, key), key };
                return true;
            }
            else {
                std::cout << "Invalid password. Password must have a minimum of 8 characters and at least one uppercase letter.\n";
            }
        }
        return false;
    }

    bool registerUserWithRandomPassword(const std::string& user) {
        std::string password = PasswordGenerator::generateStrongPassword();
        std::cout << "Generated Password: " << password << "\n";
        return registerUser(user, password);
    }

    bool validateUser(const std::string& user, const std::string& pass) {
        auto it = credentials.find(user);
        if (it != credentials.end() && EncryptionService::encode(pass, it->second.second) == it->second.first) {
            std::cout << "Login successful. Welcome, " << user << "!\n";
            int choice;
            do {
                std::cout << "1. Add Password\n2. Retrieve Password\n3. Exit\nChoose an option: ";
                std::cin >> choice;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear input buffer

                switch (choice) {
                case 1:
                    addPassword(user);
                    break;
                case 2:
                    retrievePassword(user);
                    break;
                case 3:
                    std::cout << "Exiting...\n";
                    break;
                default:
                    std::cout << "Invalid option. Please try again.\n";
                }
            } while (choice != 3);
            return true;
        }
        else {
            std::cout << "Login failed. Incorrect username or password.\n";
            return false;
        }
    }

    void addPassword(const std::string& user) {
        std::string website, password;
        std::cout << "Enter website: ";
        std::cin >> website;
        std::cout << "Enter password: ";
        std::cin >> password;
        // You can further encrypt the password before storing if needed
        credentials[user].first = password;
        std::cout << "Password added successfully for " << website << ".\n";
    }

    void retrievePassword(const std::string& user) {
        std::string website;
        std::cout << "Enter website: ";
        std::cin >> website;
        auto it = credentials.find(user);
        if (it != credentials.end()) {
            std::cout << "Password for " << website << ": " << it->second.first << "\n";
        }
        else {
            std::cout << "No password found for " << website << ".\n";
        }
    }
};

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    PasswordKeeper manager("passwords.txt");
    std::string username, password;

    std::cout << "Welcome to the Secure Password Manager\n";
    std::cout << "1. Register\n2. Login\nChoose an option: ";

    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear input buffer

    switch (choice) {
    case 1: // Register
        std::cout << "Register a new user\n";
        std::cout << "Enter username: ";
        std::getline(std::cin, username);
        std::cout << "Do you want to generate a strong password? (Y/N): ";
        char genChoice;
        std::cin >> genChoice;
        if (genChoice == 'Y' || genChoice == 'y') {
            if (manager.registerUserWithRandomPassword(username)) {
                std::cout << "Registration successful.\n";
            }
            else {
                std::cout << "User already exists.\n";
            }
        }
        else {
            std::
                cout << "Enter password: ";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Flush input buffer
            std::getline(std::cin, password);
            if (manager.registerUser(username, password)) {
                std::cout << "Registration successful.\n";
            }
            else {
                std::cout << "User already exists.\n";
            }
        }
        break;
    case 2: // Login
        std::cout << "Login\n";
        std::cout << "Enter username: ";
        std::getline(std::cin, username);
        std::cout << "Enter password: ";
        std::getline(std::cin, password);
        manager.validateUser(username, password);
        break;
    default:
        std::cout << "Invalid option.\n";
    }

    return 0;
}