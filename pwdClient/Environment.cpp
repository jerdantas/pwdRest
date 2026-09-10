//
// Created by dantas on 9/9/26.
//

#include <fstream>
#include <unordered_map>
#include <algorithm>

#include "Environment.h"

// class Environment {

// Trims leading and trailing whitespace
std::string Environment::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// Strips matching single or double surrounding quotes
std::string Environment::strip_quotes(const std::string& str) {
    if (str.length() >= 2) {
        char first = str.front();
        char last = str.back();
        if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
            return str.substr(1, str.length() - 2);
        }
    }
    return str;
}

void Environment::parse_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);

        // Skip empty lines and comment lines
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Strip optional 'export ' prefix common in shell files
        if (line.rfind("export ", 0) == 0) {
            line = trim(line.substr(7));
        }

        // Split key and value on the first '=' delimiter
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = trim(line.substr(0, pos));
            std::string val = trim(line.substr(pos + 1));
            val = strip_quotes(val);

            if (!key.empty()) {
                env_vars[key] = val;
            }
        }
    }
}

std::string Environment::get_key(const std::string& key) const {
    auto it = env_vars.find(key);
    return (it != env_vars.end()) ? it->second : "";
}

Environment::Environment(const std::string& path) {
    parse_file(path);
}

std::string Environment::userid() const {
    return get_key("userid");
}

std::string Environment::password() const {
    return get_key("password");
}

std::string Environment::server() const {
    return get_key("server");
}
