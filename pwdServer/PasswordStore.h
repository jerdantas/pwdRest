//
// Created by dantas on 8/18/26.
//

#ifndef PASSWORDSTORE_H
#define PASSWORDSTORE_H

#include <sqlite3.h>
#include <string>
#include <vector>

struct PwData {
    std::string site;
    std::string user;
    std::string pass;
};

class PasswordStore {
public:
    explicit PasswordStore(const std::string& dbPath = "passwd.db");
    ~PasswordStore();

    // Non-copyable
    PasswordStore(const PasswordStore&) = delete;
    PasswordStore& operator=(const PasswordStore&) = delete;

    // CRUD
    bool get(const std::string& ownerId, const std::string& site, std::string& user, std::string& pass) const;
    void set(const std::string& ownerId, const std::string& site, const std::string& user, const std::string& pass) const;

    [[nodiscard]] bool del(const std::string& ownerId, const std::string& site) const;
    [[nodiscard]] std::vector<std::string> listSites(const std::string& ownerId) const;
    [[nodiscard]] std::vector<PwData> getList(const std::string& ownerId, const std::string& site) const;

    // User Management
    [[nodiscard]] bool createOwner(const std::string& ownerId, const std::string& ownerName, const std::string& ownerPwd) const;
    [[nodiscard]] int validateOwner(const std::string& ownerId, const std::string& ownerPwd) const;

private:
    sqlite3* db{nullptr};

    void openDb(const std::string& path);
    void initSchema() const;
    static void finalize(sqlite3_stmt* stmt) noexcept;
};

#endif // PASSWORDSTORE_H
