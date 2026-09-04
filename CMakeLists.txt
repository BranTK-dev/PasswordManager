cmake_minimum_required(VERSION 3.16)
project(PasswordManager VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt does its own signal/slot codegen, these turn that on
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)
find_package(OpenSSL REQUIRED)

# Every .cpp file lives in src/, headers in include/
file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/*.h")

# Embeds resources/app_icon.png into the binary (window icon, works on
# every platform). AUTORCC (enabled above) compiles this automatically.
set(APP_RESOURCES resources/resources.qrc)

# On Windows, also embed the .ico as the actual .exe icon (what shows
# up in Explorer/taskbar). Harmless no-op on Linux/Mac.
if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS resources/app_icon.rc)
endif()

add_executable(PasswordManager
    ${SOURCES}
    ${HEADERS}
    ${APP_RESOURCES}
    ${APP_ICON_RESOURCE_WINDOWS}
)

target_include_directories(PasswordManager PRIVATE include)

target_link_libraries(PasswordManager PRIVATE
    Qt6::Widgets
    Qt6::Sql
    OpenSSL::Crypto
)
