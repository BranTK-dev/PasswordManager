cmake_minimum_required(VERSION 3.16)
project(PasswordManager VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt does its own signal/slot codegen, these turn that on
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

# Every .cpp file lives in src/, headers in include/
file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/*.h")

add_executable(PasswordManager
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(PasswordManager PRIVATE include)

target_link_libraries(PasswordManager PRIVATE
    Qt6::Widgets
    Qt6::Sql
)
