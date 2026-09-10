//
// Created by dantas on 8/18/26.
//

#include "PasswordStore.h"
#include <stdexcept>

PasswordStore::PasswordStore(const std::string& dbPath) {
    openDb(dbPath);
    initSchema();
}

PasswordStore::~PasswordStore() {
    if (db) sqlite3_close(db);
}

void PasswordStore::openDb(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        std::string err = sqlite3_errmsg(db);
        throw std::runtime_error("Can't open database: " + err);
    }
}

void PasswordStore::initSchema() const {
    const char* sql = R"SQL(
        CREATE TABLE IF NOT EXISTS "PassWd" (
            "id"        INTEGER NOT NULL,
            "OwnerId"   VARCHAR(48) NOT NULL,
            "Site"      VARCHAR(40) NOT NULL,
            "UserId"    VARCHAR(48),
            "Passwd"    VARCHAR(16),
            PRIMARY KEY("id" AUTOINCREMENT),
            FOREIGN KEY("OwnerId") REFERENCES "Owners"("OwnerId"),
            UNIQUE("OwnerId", "Site")
        );
        CREATE TABLE IF NOT EXISTS "Owners" (
            "OwnerId"    VARCHAR(48) NOT NULL UNIQUE,
            "OwnerName"  VARCHAR(48) NOT NULL,
            "OwnerPwd"  VARCHAR(128) NOT NULL,
            PRIMARY KEY("OwnerId")
        );
    )SQL";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string err = errMsg ? errMsg : "unknown";
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to initialize schema: " + err);
    }
}

bool PasswordStore::get(const std::string& ownerId, const std::string& site, std::string& user, std::string& pass) const {
    const char* sql = "SELECT UserId, Passwd FROM PassWd WHERE OwnerId = ?1 AND Site = ?2;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("DB prepare failed (get)");

    sqlite3_bind_text(stmt, 1, ownerId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, site.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char* u = sqlite3_column_text(stmt, 0);
        const unsigned char* p = sqlite3_column_text(stmt, 1);
        user = u ? reinterpret_cast<const char*>(u) : "";
        pass = p ? reinterpret_cast<const char*>(p) : "";
        found = true;
    } else if (rc != SQLITE_DONE) {
        finalize(stmt);
        throw std::runtime_error("DB step failed (get)");
    }

    finalize(stmt);
    return found;
}

std::vector<PwData> PasswordStore::getList(const std::string& ownerId, const std::string& site) const {
    const char* sql = "SELECT Site, UserId, Passwd FROM PassWd WHERE OwnerId = ?1 AND Site LIKE ?2 COLLATE NOCASE;";
    std::string site_patter = "%" + site + "%";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("DB prepare failed (get)");

    sqlite3_bind_text(stmt, 1, ownerId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, site_patter.c_str(), -1, SQLITE_TRANSIENT);

    std::vector<PwData> result;

    while (true) {
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            const std::string storedSite = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            const std::string user  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            const std::string pass  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            result.push_back({storedSite, user, pass});
        } else if (rc == SQLITE_DONE) {
            finalize(stmt);
            break;
        } else {
            finalize(stmt);
            throw std::runtime_error("DB step failed (list)");
        }
    }

    return result;
}

void PasswordStore::set(const std::string& ownerId, const std::string& site, const std::string& user, const std::string& pass) const {
    const char* sql = R"SQL(
        INSERT INTO PassWd (OwnerId, Site, UserId, Passwd)
        VALUES (?1, ?2, ?3, ?4)
        ON CONFLICT(OwnerId, Site) DO UPDATE SET
            UserId = excluded.UserId,
            Passwd = excluded.Passwd;
    )SQL";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("DB prepare failed (set)");

    sqlite3_bind_text(stmt, 1, ownerId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, site.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, pass.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        finalize(stmt);
        throw std::runtime_error("DB step failed (set)");
    }
    finalize(stmt);
}

bool PasswordStore::del(const std::string& ownerId, const std::string& site) const {
    const char* sql = "DELETE FROM PassWd WHERE OwnerId = ?1 AND Site = ?2;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("DB prepare failed (del)");

    sqlite3_bind_text(stmt, 1, ownerId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, site.c_str(), -1, SQLITE_TRANSIENT);

    bool deleted = false;
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        deleted = sqlite3_changes(db) > 0;
    } else {
        finalize(stmt);
        throw std::runtime_error("DB step failed (del)");
    }
    finalize(stmt);
    return deleted;
}

std::vector<std::string> PasswordStore::listSites(const std::string& ownerId) const {
    const char* sql = "SELECT Site FROM PassWd WHERE OwnerId = ?1 ORDER BY Site ASC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("DB prepare failed (list)");

    sqlite3_bind_text(stmt, 1, ownerId.c_str(), -1, SQLITE_TRANSIENT);

    std::vector<std::string> sites;
    while (true) {
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            if (const unsigned char* s = sqlite3_column_text(stmt, 0)) sites.emplace_back(reinterpret_cast<const char*>(s));
        } else if (rc == SQLITE_DONE) {
            break;
        } else {
            finalize(stmt);
            throw std::runtime_error("DB step failed (list)");
        }
    }
    finalize(stmt);
    return sites;
}

bool PasswordStore::createOwner(const std::string& ownerId, const std::string& ownerName, const std::string& ownerPwd) const {
    const char* sql = "INSERT INTO Owners (OwnerId, OwnerName, OwnerPwd) VALUES (?1, ?2, ?3);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, ownerId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, ownerName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, ownerPwd.c_str(), -1, SQLITE_TRANSIENT); // Note: passwords should be hashed in production

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    finalize(stmt);
    return success;
}

int PasswordStore::validateOwner(const std::string& ownerId, const std::string& ownerPwd) const {
    const char* sql = "SELECT OwnerPwd FROM Owners WHERE OwnerId = ?1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return 500;

    sqlite3_bind_text(stmt, 1, ownerId.c_str(), -1, SQLITE_TRANSIENT);

    int returnCode = 404; // Owner not found

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* p = sqlite3_column_text(stmt, 0);
        if (p && ownerPwd == reinterpret_cast<const char*>(p)) {
            returnCode = 200; // login OK
        }
        else {
            returnCode = 401; // Invalid password
        }
    }
    finalize(stmt);
    return returnCode;
}

void PasswordStore::finalize(sqlite3_stmt* stmt) noexcept {
    if (stmt) sqlite3_finalize(stmt);
}
