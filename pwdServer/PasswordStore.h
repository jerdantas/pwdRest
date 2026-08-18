//
// Created by dantas on 8/18/26.
//

#ifndef PASSWORDSTORE_H
#define PASSWORDSTORE_H

// #include <sqlcipher/sqlite3.h>  // On some systems
#include <sqlite3.h>
#include <string>
#include <vector>
#include <stdexcept>

class PasswordStore {
public:
    explicit PasswordStore(const std::string& dbPath = "passwd.db");
    ~PasswordStore();

    // Non-copyable
    PasswordStore(const PasswordStore&) = delete;
    PasswordStore& operator=(const PasswordStore&) = delete;

    // CRUD
    bool get(const std::string& site, std::string& user, std::string& pass) const;
    void set(const std::string& site, const std::string& user, const std::string& pass) const;
    bool del(const std::string& site) const;

    std::vector<std::string> listSites() const;

private:
    sqlite3* db{nullptr};

    void openDb(const std::string& path);
    void initSchema() const;
    static void finalize(sqlite3_stmt* stmt) noexcept;
};

#endif // PASSWORDSTORE_H
