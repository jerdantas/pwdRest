//
// Created by dantas on 8/15/26.
//

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <cstdlib>
#include <filesystem>
#include "PasswordStore.h"

using json = nlohmann::json;

int main() {
    // Setup Database Path
    const char* homeDir = std::getenv("HOME");
    if (!homeDir) {
        std::cerr << "HOME environment variable not set." << std::endl;
        return 1;
    }

    std::filesystem::path dir(std::string(homeDir) + "/.local/share/pwd");
    std::filesystem::create_directories(dir);

    PasswordStore store((dir / "passwd.db").string());
    httplib::Server svr;

    // GET /sites
    svr.Get("/sites", [&](const httplib::Request&, httplib::Response& res) {
        try {
            auto sites = store.listSites();
            json j = sites;
            res.set_content(j.dump(), "application/json");
        } catch (const std::exception& e) {
            res.set_content(std::string("Error listing sites: ") + e.what(),  "text/plain");
            res.status = 500;
            return;
        }
    });

    // GET /password/:site
    svr.Get(R"(/password/(.*))", [&](const httplib::Request& req, httplib::Response& res) {
        std::string site = req.matches[1];
        std::string user, pass;
        try {
            if (store.get(site, user, pass)) {
                json j = {{"userId", user}, {"password", pass}};
                res.set_content(j.dump(), "application/json");
            } else {
                res.status = 404;
            }
        } catch (const std::exception& e) {
            res.set_content(std::string("Error getting password for site: ") + e.what(),  "text/plain");
            res.status = 500;
            return;
        }
    });

    // POST /password
    svr.Post("/password", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            store.set(j["name"], j["userId"], j["password"]);
            res.status = 200;
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(e.what(), "text/plain");
        }
    });

    // DELETE /password/:site
    svr.Delete(R"(/password/(.*))", [&](const httplib::Request& req, httplib::Response& res) {
        std::string site = req.matches[1];
        try {
            if (store.del(site)) {
                res.status = 200;
            } else {
                res.status = 404;
            }
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(std::string("Error deleting password for site: ") + e.what(),  "text/plain");
            return;
        }
    });

    std::cout << "Database initialized at " << (dir / "passwd.db").string() << std::endl;
    std::cout << "Starting server on http://localhost:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);

    return 0;
}