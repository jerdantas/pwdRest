//
// Created by dantas on 9/9/26.
//

#ifndef PWDREST_ENVIRONMENT_H
#define PWDREST_ENVIRONMENT_H

#include <qstring.h>
#include <string>
#include <unordered_map>

class Environment {
private:
    std::unordered_map<std::string, std::string> env_vars;

    // Trims leading and trailing whitespace
    static std::string trim(const std::string& str);
    // Strips matching single or double surrounding quotes
    static std::string strip_quotes(const std::string& str);
    void parse_file(const std::string& path);
    std::string get_key(const std::string& key) const;

public:
    explicit Environment(const std::string& path);

    std::string userid() const;
    std::string password() const;
    std::string server() const;
};

#endif //PWDREST_ENVIRONMENT_H
