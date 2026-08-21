//
// Created by dantas on 8/15/26.
//


#include "PasswordClient.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <utility>

using json = nlohmann::json;

PasswordClient::PasswordClient(std::string  serverUrl) : baseUrl(std::move(serverUrl)) {}

bool PasswordClient::login() const {
    const char* homeDir = std::getenv("HOME");
    if (!homeDir) return false;

    std::string envPath = std::string(homeDir) + "/.local/pwd-client/.env";
    std::ifstream file(envPath);
    if (!file.is_open()) return false;

    std::string ownerId, ownerPwd;
    std::getline(file, ownerId);
    std::getline(file, ownerPwd);

    return login(ownerId, ownerPwd);
}

bool PasswordClient::login(const std::string& ownerId, const std::string& ownerPwd) const {
    httplib::Client cli(baseUrl);
    json j = {{"ownerId", ownerId}, {"ownerPwd", ownerPwd}};
    auto res = cli.Post("/login", j.dump(), "application/json");
    // std::cout << "Login response status: " << res->status << std::endl;
    if (res && res->status == 200) {
        auto resp_j = json::parse(res->body);
        jwtToken = resp_j["token"];
        return true;
    }
    return false;
}

bool PasswordClient::signup(const std::string& ownerId, const std::string& ownerName, const std::string& password) const {
    httplib::Client cli(baseUrl);
    json j = {
        {"ownerId", ownerId},
        {"ownerName", ownerName},
        {"ownerPwd", password}
    };
    auto res = cli.Post("/signup", j.dump(), "application/json");
    // std::cout << "Signup response status: " << res->status << std::endl;
    return res && res->status == 201;
}

httplib::Headers PasswordClient::getHeaders() const {
    return {{"Authorization", "Bearer " + jwtToken}};
}

bool PasswordClient::get(const std::string& name, std::string& userId, std::string& password) const {
    httplib::Client cli(baseUrl);
    auto res = cli.Get("/password/" + name, getHeaders());
    // std::cout << "Get password response status: " << res->status << std::endl;
    if (res && res->status == 401) {
        if (login()) {
            res = cli.Get("/password/" + name, getHeaders());
            // std::cout << "Get password response status after login: " << res->status << std::endl;
        }
    }
    if (res && res->status == 200) {
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
    httplib::Client cli(baseUrl);
    json j = {{"name", name}, {"userId", userId}, {"password", password}};
    auto res = cli.Post("/password", getHeaders(), j.dump(), "application/json");
    // std::cout << "Set password response status: " << res->status << std::endl;
    if (res && res->status == 401) {
        if (login()) {
            res = cli.Post("/password", getHeaders(), j.dump(), "application/json");
            // std::cout << "Set password response status after login: " << res->status << std::endl;
        }
    }
    if (!res || res->status != 200) {
        throw std::runtime_error("Failed to save password to server: " + (res ? res->body : "Network error"));
    }
}

bool PasswordClient::del(const std::string& name) const {
    httplib::Client cli(baseUrl);
    auto res = cli.Delete("/password/" + name, getHeaders());
    // std::cout << "Delete password response status: " << res->status << std::endl;
    if (res && res->status == 401) {
        if (login()) {
            res = cli.Delete("/password/" + name, getHeaders());
            // std::cout << "Delete password response status after login: " << res->status << std::endl;
        }
    }
    if (!res) {
        return false;
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
    httplib::Client cli(baseUrl);
    auto res = cli.Get("/sites", getHeaders());
    // std::cout << "List sites response status: " << res->status << std::endl;
    if (res && res->status == 401) {
        if (login()) {
            res = cli.Get("/sites", getHeaders());
            // std::cout << "List sites response status after login: " << res->status << res->body << std::endl;
        }
    }
    std::vector<std::string> sites;
    if (res && res->status == 200) {
        auto j = json::parse(res->body);
        for (const auto& item : j) {
            sites.push_back(item.get<std::string>());
        }
    }
    if (res && res->status != 200) {
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