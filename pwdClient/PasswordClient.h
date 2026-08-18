//
// Created by dantas on 8/15/26.
//

#ifndef PASSWORDCLIENT_H
#define PASSWORDCLIENT_H

#include <string>
#include <vector>
#include <QStringList>

class PasswordClient {
public:
    explicit PasswordClient(const std::string& serverUrl);

    bool get(const std::string& name, std::string& userId, std::string& password);
    void set(const std::string& name, const std::string& userId, const std::string& password);
    bool del(const std::string& name);
    std::vector<std::string> listSites();
    QStringList getSiteNames();

private:
    std::string baseUrl;
};

#endif // PASSWORDCLIENT_H