cmake_minimum_required(VERSION 3.14)

project(pwdRest VERSION 0.2 LANGUAGES CXX)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(Qt6_NO_XKB 1)
set(Qt6_NO_VULKAN 1)

set(CMAKE_PREFIX_PATH "/home/dantas/Qt/6.10.1/gcc_64/lib/cmake")

find_package(Qt6Widgets REQUIRED)
find_package(SQLite3 REQUIRED)

include(FetchContent)
# Fetch cpp-httplib for REST API
FetchContent_Declare(
httplib
GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
GIT_TAG        v0.14.1
)
FetchContent_MakeAvailable(httplib)

# Fetch nlohmann_json for parsing/formatting JSON
FetchContent_Declare(
json
GIT_REPOSITORY https://github.com/nlohmann/json.git
GIT_TAG        v3.11.2
)
FetchContent_MakeAvailable(json)

# Fetch jwt-cpp for JWT generation and validation
FetchContent_Declare(
jwt_cpp
GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
GIT_TAG        v0.7.0
)
FetchContent_MakeAvailable(jwt_cpp)

find_package(OpenSSL REQUIRED)

set(CLIENT_SOURCES
        pwdClient/client_main.cpp
        pwdClient/PasswordClient.cpp
        pwdClient/MainWindow.cpp
        pwdClient/NewEntryDialog.cpp
        pwdClient/LoginDialog.cpp
        pwdClient/resources.qrc
)

set(CLIENT_HEADERS
        pwdClient/MainWindow.h
        pwdClient/NewEntryDialog.h
        pwdClient/PasswordClient.h
        pwdClient/LoginDialog.h
)

set (UIS
        pwdClient/MainWindow.ui
        pwdClient/NewEntryDialog.ui
        pwdClient/LoginDialog.ui
        pwdClient/Environment.cpp
        pwdClient/Environment.h
)

set(SERVER_SOURCES
    pwdServer/server_main.cpp
    pwdServer/PasswordStore.cpp
)

set(SERVER_HEADERS
    pwdServer/PasswordStore.h
)

# Client Target
add_executable(pwd-client ${CLIENT_SOURCES} ${CLIENT_HEADERS} ${UIS})
qt6_add_resources(GENERATED_RESOURCES pwdClient/resources.qrc)
target_sources(pwd-client PRIVATE ${GENERATED_RESOURCES})
target_link_libraries(pwd-client PRIVATE Qt6::Widgets httplib::httplib nlohmann_json::nlohmann_json OpenSSL::SSL OpenSSL::Crypto)
target_compile_definitions(pwd-client PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

# Server Target
add_executable(pwd-server ${SERVER_SOURCES} ${SERVER_HEADERS})
target_include_directories(pwd-server PRIVATE ${SQLite3_INCLUDE_DIRS})
target_link_libraries(pwd-server PRIVATE ${SQLite3_LIBRARIES} httplib::httplib nlohmann_json::nlohmann_json jwt-cpp::jwt-cpp OpenSSL::SSL OpenSSL::Crypto)

# --- Installation & Packaging Configuration ---
include(GNUInstallDirs)

# Install executable
install(TARGETS pwd-client
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# CPack configuration for Debian/Ubuntu (.deb)
set(CPACK_PACKAGE_NAME "pwd-client")
set(CPACK_PACKAGE_VENDOR "pwdRest")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Desktop client for pwdRest password manager")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "DEB")

# Package maintainer details required for Debian packaging
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Your Name <your.email@example.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6widgets6, libssl3")

include(CPack)