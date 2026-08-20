//
// Created by dantas on 8/15/26.
//

#ifndef PASSWORDCLIENT_H
#define PASSWORDCLIENT_H

#include <string>
#include <vector>
#include <QStringList>
#include <httplib.h>

class PasswordClient {
public:
    explicit PasswordClient(std::string  serverUrl);

    bool get(const std::string& name, std::string& userId, std::string& password) const;
    void set(const std::string& name, const std::string& userId, const std::string& password) const;
    bool del(const std::string& name) const;
    std::vector<std::string> listSites() const;
    QStringList getSiteNames() const;

private:
    std::string baseUrl;
    mutable std::string jwtToken;

    bool login() const;
    httplib::Headers getHeaders() const;
};

#endif // PASSWORDCLIENT_H