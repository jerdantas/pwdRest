//
// Created by dantas on 8/15/26.
//

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <jwt-cpp/jwt.h>
#include <iostream>
#include <cstdlib>
#include <filesystem>
#include "PasswordStore.h"

using json = nlohmann::json;

const std::string JWT_SECRET = "super_secret_key_for_pwd_rest";

int main() {
    // Setup Database Path
    const char* homeDir = std::getenv("HOME");
    if (!homeDir) {
        std::cerr << "HOME environment variable not set." << std::endl;
        return 1;
    }

    std::filesystem::path dir(std::string(homeDir) + "/.local/share/pwd");
    std::filesystem::create_directories(dir);

    PasswordStore store((dir / "rempasswd.db").string());
    httplib::Server svr;

    auto getOwnerId = [](const httplib::Request& req) -> std::string {
        std::string auth_header = req.get_header_value("Authorization");
        if (auth_header.length() > 7) {
            std::string token = auth_header.substr(7);
            auto decoded = jwt::decode(token);
            return decoded.get_payload_claim("ownerId").as_string();
        }
        throw std::runtime_error("Invalid Authorization header");
    };

    svr.set_pre_routing_handler([&](const httplib::Request& req, httplib::Response& res) {
        if (req.path == "/signup" || req.path == "/login") {
            return httplib::Server::HandlerResponse::Unhandled;
        }

        if (req.has_header("Authorization")) {
            std::string auth_header = req.get_header_value("Authorization");
            if (auth_header.find("Bearer ") == 0) {
                std::string token = auth_header.substr(7);
                try {
                    auto decoded = jwt::decode(token);
                    auto verifier = jwt::verify()
                        .allow_algorithm(jwt::algorithm::hs256{JWT_SECRET})
                        .with_issuer("pwdRest");
                    verifier.verify(decoded);
                    return httplib::Server::HandlerResponse::Unhandled;
                } catch (const std::exception& e) {
                    // Token verification failed
                }
            }
        }

        res.status = 401;
        res.set_content("Unauthorized", "text/plain");
        return httplib::Server::HandlerResponse::Handled;
    });

    // POST /signup
    svr.Post("/signup", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string ownerId = j.at("ownerId");
            std::string ownerName = j.at("ownerName");
            std::string ownerPwd = j.at("ownerPwd");

            if (store.createOwner(ownerId, ownerName, ownerPwd)) {
                res.status = 201;
                res.set_content("Owner created successfully.", "text/plain");
            } else {
                res.status = 400;
                res.set_content("Failed to create owner (may already exist).", "text/plain");
            }
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(std::string("Error creating owner: ") + e.what(), "text/plain");
        }
    });

    // POST /login
    svr.Post("/login", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string ownerId = j.at("ownerId");
            std::string ownerPwd = j.at("ownerPwd");

            switch (store.validateOwner(ownerId, ownerPwd)) {

            case 200: {
                auto token = jwt::create()
                    .set_issuer("pwdRest")
                    .set_type("JWT")
                    .set_payload_claim("ownerId", jwt::claim(ownerId))
                    .set_expires_at(std::chrono::system_clock::now() + std::chrono::minutes(5))
                    .sign(jwt::algorithm::hs256{JWT_SECRET});

                json response = {{"token", token}};
                res.status = 200;
                res.set_content(response.dump(), "application/json");
                break;
            }

            case 401: {
                res.status = 401;
                res.set_content("Unauthorized", "text/plain");
                break;
            }

            case 404: {
                res.status = 404;
                res.set_content("User not found", "text/plain");
                break;
            }

            default:
                res.status = 500;
                res.set_content("Internal server error", "text/plain");
                break;
            }
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(std::string("Error during login: ") + e.what(), "text/plain");
        }
    });

    // GET /sites
    svr.Get("/sites", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            std::string ownerId = getOwnerId(req);
            auto sites = store.listSites(ownerId);
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
            std::string ownerId = getOwnerId(req);
            if (store.get(ownerId, site, user, pass)) {
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
            std::string ownerId = getOwnerId(req);
            store.set(ownerId, j["name"], j["userId"], j["password"]);
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
            std::string ownerId = getOwnerId(req);
            if (store.del(ownerId, site)) {
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