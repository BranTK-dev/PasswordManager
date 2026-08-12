# Password Manager

A desktop password manager built with C++ and Qt6. Stores credentials
and secure notes behind a master password, using SQLite for storage.

## Status

Phase 1 complete: project scaffolding, CMake build, and a basic
MainWindow. Login, CRUD, search, SQLite, and encryption come in later
phases.

## Requirements

- CMake 3.16+
- Qt6 (Widgets, Sql)
- A C++17 compiler

On Ubuntu/Debian:

```
sudo apt install cmake build-essential qt6-base-dev libqt6sql6-sqlite qt6-tools-dev
```

## Building

```
mkdir build && cd build
cmake ..
cmake --build .
```

## Running

```
./build/PasswordManager
```

## Project Structure

```
include/    Header files
src/        Source files
resources/  Icons, stylesheets, etc.
database/   SQLite database file (created at runtime, not committed)
screenshots/  App screenshots for docs
docs/       Additional documentation
```
