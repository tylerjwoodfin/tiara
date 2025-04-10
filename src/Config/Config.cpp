#include "Config.h"
#include "../helpers/consts.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstdlib>

using namespace std;

// Helper function to expand environment variables in a path
string expand_env_vars(const string& path) {
    string result = path;
    size_t pos = 0;
    
    while ((pos = result.find("$", pos)) != string::npos) {
        size_t end = result.find("/", pos);
        if (end == string::npos) {
            end = result.length();
        }
        
        string env_var = result.substr(pos + 1, end - pos - 1);
        const char* env_val = getenv(env_var.c_str());
        
        if (env_val) {
            result.replace(pos, end - pos, env_val);
            pos += strlen(env_val);
        } else {
            pos = end;
        }
    }
    
    return result;
}

void Config::read_config() {
    ifstream config_file(CONFIG_FILE);
    if (!config_file.is_open()) {
        // If config file doesn't exist, create it with default values
        data_file_location = string(getenv("HOME")) + "/.local/share/tiara/data";
        prefix = false;
        write_config();
        return;
    }

    string line;
    while (getline(config_file, line)) {
        if (line.empty() || line[0] == '#') continue;  // Skip empty lines and comments
        
        size_t pos = line.find('=');
        if (pos == string::npos) continue;
        
        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);
        
        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        if (key == "data_file_location") {
            data_file_location = expand_env_vars(value);
        } else if (key == "prefix") {
            prefix = (value == "true");
        }
        // Add other config options here as needed
    }
    
    // If data_file_location is not set in config, use default
    if (data_file_location.empty()) {
        data_file_location = string(getenv("HOME")) + "/.local/share/tiara/data";
    }
}

void Config::write_config() {
    // Create config directory if it doesn't exist
    filesystem::path config_path(CONFIG_FILE);
    filesystem::create_directories(config_path.parent_path());
    
    ofstream config_file(CONFIG_FILE);
    if (!config_file.is_open()) {
        cerr << "Error: Could not open config file for writing: " << CONFIG_FILE << endl;
        return;
    }
    
    config_file << "# Tiara Configuration File" << endl;
    config_file << "# Data file location" << endl;
    config_file << "data_file_location=$HOME/syncthing/md/tiara/data" << endl;
    config_file << "# Enable prefix for cards" << endl;
    config_file << "prefix=" << (prefix ? "true" : "false") << endl;
    // Add other config options here as needed
} 