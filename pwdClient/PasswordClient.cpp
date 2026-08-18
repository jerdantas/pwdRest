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

bool PasswordClient::get(const std::string& name, std::string& userId, std::string& password) const {
    httplib::Client cli(baseUrl);
    auto res = cli.Get("/pwserver/password/" + name);
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
    auto res = cli.Post("/pwserver/password", j.dump(), "application/json");
    if (!res || res->status != 200) {
        throw std::runtime_error("Failed to save password to server: " + res->body);
    }
}

bool PasswordClient::del(const std::string& name) const {
    httplib::Client cli(baseUrl);
    auto res = cli.Delete("/pwserver/password/" + name);
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
    auto res = cli.Get("/pwserver/sites");
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