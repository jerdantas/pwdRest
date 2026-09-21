# Developer Guide for pwdRest

This document provides project-specific information for developers working on `pwdRest`.

## 1. Build & Configuration

The project is a C++ application consisting of a REST server and a Qt-based desktop client.

### Dependencies
- **Qt 6.10.1** (specifically Widgets module)
- **SQLite 3**
- **OpenSSL**
- **CMake** (3.14+)
- **Fetched via CMake (FetchContent):**
    - `cpp-httplib` (v0.14.1)
    - `nlohmann/json` (v3.11.2)
    - `jwt-cpp` (v0.7.0)

### Build Instructions
Standard CMake build process is used. Convenience scripts are provided in the root directory:

- **Build Server:** `./build_server.sh`
- **Build Client:** `./build_client.sh`

Both scripts create a `build` directory and compile the respective targets.

### Server Configuration
- **JWT Secret:** Defined in `pwdServer/server_main.cpp` as `JWT_SECRET`.
- **Database Path:** The server creates and uses a SQLite database at `~/.local/share/pwd/rempasswd.db`. Ensure the directory exists or the application has permissions to create it.
- **Logging:** Use the `-p` command-line flag when starting the server to print incoming HTTP requests and their response statuses to stdout.

## 2. Testing Information

### Running Tests
The project uses standard C++ for testing core logic. To run the `PasswordStore` tests:

1. Add a test target to `CMakeLists.txt` (see example below).
2. Build the test target.
3. Execute the resulting binary.

### Example Test
A sample test for `PasswordStore` looks like this:

```cpp
#include <cassert>
#include "pwdServer/PasswordStore.h"

int main() {
    PasswordStore store("test.db");
    bool created = store.createOwner("user1", "User One", "pass123");
    assert(created);
    
    int loginStatus = store.validateOwner("user1", "pass123");
    assert(loginStatus == 200);
    
    store.set("user1", "site.com", "admin", "secret");
    std::string u, p;
    assert(store.get("user1", "site.com", u, p));
    assert(u == "admin");
    
    return 0;
}
```

### Guidelines for New Tests
- Use separate SQLite database files for tests to avoid corrupting development data.
- Verify return codes for `validateOwner`: `200` for success, `401` for invalid password, `404` for not found, `500` for DB error.
- Tests should clean up their temporary database files upon completion.

## 3. Additional Development Information

### Code Style
- **C++ Standard:** C++20.
- **Naming:** CamelCase for classes (`PasswordStore`), camelCase for methods (`listSites`), and snake_case for local variables or specific JSON fields where applicable.
- **Indentation:** Follow the existing 4-space indentation.

### Architecture Notes
- The server is stateless regarding sessions, relying on JWT for authentication.
- The `PasswordStore` class handles all SQLite interactions and ensures the schema is initialized on startup.
- The client uses `httplib` for communication and `Qt` for the UI.
