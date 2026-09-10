//
// Created by dantas on 8/15/26.
//


#include "PasswordClient.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <utility>
#include <iostream>

using json = nlohmann::json;

static std::string getHost(const std::string& url) {
    size_t pos = url.find("://");
    if (pos != std::string::npos) {
        size_t path_pos = url.find('/', pos + 3);
        if (path_pos != std::string::npos) return url.substr(0, path_pos);
    }
    return url;
}

static std::string buildPath(const std::string& url, const std::string& endpoint) {
    size_t pos = url.find("://");
    if (pos != std::string::npos) {
        size_t path_pos = url.find('/', pos + 3);
        if (path_pos != std::string::npos) return url.substr(path_pos) + endpoint;
    }
    return endpoint;
}

PasswordClient::PasswordClient()= default;

void PasswordClient::setBaseUrl(const std::string& serverUrl) {
    baseUrl = serverUrl;
}

void PasswordClient::setOwner(const std::string& owner, const std::string& password) {
    ownerId = owner;
    ownerPwd = password;
}

bool PasswordClient::login() const {
    httplib::Client cli(getHost(baseUrl));
    cli.enable_server_certificate_verification(false);
    json j = {{"ownerId", ownerId}, {"ownerPwd", ownerPwd}};
    auto res = cli.Post(buildPath(baseUrl, "/login"), j.dump(), "application/json");
    if (!res) {
        std::cerr << "Network error during login" << std::endl;
        return false;
    }
    // std::cout << "Login response status: " << res->status << std::endl;
    if (res->status == 200) {
        auto resp_j = json::parse(res->body);
        jwtToken = resp_j["token"];
        return true;
    }
    return false;
}

bool PasswordClient::signup(const std::string& ownerId, const std::string& ownerName, const std::string& password) const {
    httplib::Client cli(getHost(baseUrl));
    cli.enable_server_certificate_verification(false);
    json j = {
        {"ownerId", ownerId},
        {"ownerName", ownerName},
        {"ownerPwd", password}
    };
    auto res = cli.Post(buildPath(baseUrl, "/signup"), j.dump(), "application/json");
    if (!res) return false;
    // std::cout << "Signup response status: " << res->status << std::endl;
    return res->status == 201;
}

httplib::Headers PasswordClient::getHeaders() const {
    return {{"Authorization", "Bearer " + jwtToken}};
}

bool PasswordClient::get(const std::string& name, std::string& userId, std::string& password) const {
    httplib::Client cli(getHost(baseUrl));
    cli.enable_server_certificate_verification(false);
    auto res = cli.Get(buildPath(baseUrl, "/password/" + name), getHeaders());
    if (!res) return false;
    // std::cout << "Get password response status: " << res->status << std::endl;
    if (res->status == 401) {
        if (login()) {
            res = cli.Get(buildPath(baseUrl, "/password/" + name), getHeaders());
            if (!res) return false;
            // std::cout << "Get password response status after login: " << res->status << std::endl;
        }
    }
    if (res->status == 200) {
        auto j = json::parse(res->body);
        userId = j["userId"];
        password = j["password"];
        return true;
    }
    if (res &&res->status == 500) {
        throw std::runtime_error("Error getting password for site: " + res->body);
    }
    return false;
}

void PasswordClient::set(const std::string& name, const std::string& userId, const std::string& password) const {
    httplib::Client cli(getHost(baseUrl));
    cli.enable_server_certificate_verification(false);
    json j = {{"name", name}, {"userId", userId}, {"password", password}};
    auto res = cli.Post(buildPath(baseUrl, "/password"), getHeaders(), j.dump(), "application/json");
    if (!res) throw std::runtime_error("Network error during set password");
    // std::cout << "Set password response status: " << res->status << std::endl;
    if (res->status == 401) {
        if (login()) {
            res = cli.Post(buildPath(baseUrl, "/password"), getHeaders(), j.dump(), "application/json");
            if (!res) throw std::runtime_error("Network error during set password retry");
            // std::cout << "Set password response status after login: " << res->status << std::endl;
        }
    }
    if (res->status != 200) {
        throw std::runtime_error("Failed to save password to server: " + res->body);
    }
}

bool PasswordClient::del(const std::string& name) const {
    httplib::Client cli(getHost(baseUrl));
    cli.enable_server_certificate_verification(false);
    auto res = cli.Delete(buildPath(baseUrl, "/password/" + name), getHeaders());
    if (!res) return false;
    // std::cout << "Delete password response status: " << res->status << std::endl;
    if (res->status == 401) {
        if (login()) {
            res = cli.Delete(buildPath(baseUrl, "/password/" + name), getHeaders());
            if (!res) return false;
            // std::cout << "Delete password response status after login: " << res->status << std::endl;
        }
    }

    switch (res->status) {
        case 200:
            return true;

        case 404:
            return false;

            case 500:
                throw std::runtime_error("Error deleting password: " + res->body);

        default: ;
    }
    return false;
}

std::vector<std::string> PasswordClient::listSites() const {
    httplib::Client cli(getHost(baseUrl));
    cli.enable_server_certificate_verification(false);
    auto res = cli.Get(buildPath(baseUrl, "/sites"), getHeaders());
    if (!res) throw std::runtime_error("Network error listing sites");
    // std::cout << "List sites response status: " << res->status << std::endl;
    if (res->status == 401) {
        if (login()) {
            res = cli.Get(buildPath(baseUrl, "/sites"), getHeaders());
            if (!res) throw std::runtime_error("Network error listing sites retry");
            // std::cout << "List sites response status after login: " << res->status << res->body << std::endl;
        }
    }
    std::vector<std::string> sites;
    if (res->status == 200) {
        auto j = json::parse(res->body);
        for (const auto& item : j) {
            sites.push_back(item.get<std::string>());
        }
    }
    if (res->status != 200) {
        throw std::runtime_error("Error listing sites: " + res->body);
    }

    return sites;
}

QStringList PasswordClient::getSiteNames() const {
    QStringList list;
    for (const auto& site : listSites()) {
        list << QString::fromStdString(site);
    }
    return list;
}