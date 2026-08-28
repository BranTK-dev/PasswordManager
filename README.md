# Password Manager

A desktop password manager built with C++ and Qt6. It stores logins,
website credentials, and secure notes behind a single master
password, with everything encrypted before it touches disk.

The goal isn't just a working app, it's a project that actually
exercises real software engineering: OOP design, GUI architecture,
database integration, and application security, all in working code
rather than theory.

## Features

- Master password login
- Dashboard view of all saved credentials
- Add / edit / delete credentials
- Search and filter
- Categories
- Built-in password generator
- Copy username/password to clipboard
- Show/hide password toggle
- Favorites
- Secure notes
- Auto-save

## Credential Fields

Website/Application, URL, Username, Email, Password, Category, Notes,
Date Created, Date Modified, Favorite.

## Architecture

- **MainWindow** - main UI, dashboard
- **LoginDialog** - master password entry screen
- **Credential** - data model for a single stored entry
- **PasswordManager** - core logic tying UI to data
- **DatabaseManager** - SQLite read/write layer
- **EncryptionManager** - encrypt/decrypt credential data
- **SettingsManager** - app settings and preferences

## Storage

SQLite database for credentials. Passwords are encrypted before
they're stored. The master password is required to unlock the app.

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
include/     Header files
src/         Source files
resources/   Icons, stylesheets, etc.
database/    SQLite database file (created at runtime, not committed)
screenshots/ App screenshots for docs
docs/        Additional documentation
```

## Development Roadmap

- [x] **Phase 1: Project Setup & Basic UI** - folder structure, CMake
  build, blank MainWindow
- [x] **Phase 2: Login Screen** - LoginDialog UI and login to dashboard
  flow (master password verification comes in Phase 7)
- [x] **Phase 3: Core Data Model** - Credential class with all fields
- [x] **Phase 4: CRUD Operations** - add/edit/delete dialogs, wired to
  an in-memory list
- [x] **Phase 5: SQLite Integration** - DatabaseManager, persistent
  storage
- [x] **Phase 6: Search & Filter** - search bar, category filter,
  favorites toggle
- [ ] **Phase 7: Encryption** - EncryptionManager, master password
  actually gates decryption
- [ ] **Phase 8: Extras & Polish** - password generator, clipboard
  copy, show/hide toggle, settings, about screen, auto-save
- [ ] **Phase 9: Testing & Cleanup** - edge case testing, code
  cleanup, README/screenshots for the repo

## Future Improvements

Cloud sync, browser extension, dark mode, password strength analysis,
backups, biometric login.

## Skills Demonstrated

Modern C++, Qt Widgets, OOP, SQLite, security fundamentals, Git,
GitHub, software architecture.
