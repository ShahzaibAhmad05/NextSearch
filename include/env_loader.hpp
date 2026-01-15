#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace cord19 {

// Simple .env file parser
// Reads KEY=VALUE pairs from a file and returns them as a map
inline std::unordered_map<std::string, std::string> load_env_file(const std::string& filepath) {
    std::unordered_map<std::string, std::string> env_vars;
    std::ifstream file(filepath);
    
    if (!file.is_open()) {
        return env_vars; // Return empty map if file doesn't exist
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        
        // Find the '=' separator
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n") + 1);
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);
        
        // Remove quotes if present
        if (value.size() >= 2 && 
            ((value.front() == '"' && value.back() == '"') ||
             (value.front() == '\'' && value.back() == '\''))) {
            value = value.substr(1, value.size() - 2);
        }
        
        env_vars[key] = value;
    }
    
    return env_vars;
}

} // namespace cord19
