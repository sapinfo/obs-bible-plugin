# SQLite3 Amalgamation

Vendored copy of SQLite's official amalgamation source.

- **Version**: 3.46.0 (2024-05-23)
- **Source**: https://www.sqlite.org/2024/sqlite-amalgamation-3460000.zip
- **Files**:
  - `sqlite3.c` — the entire SQLite library in one C file
  - `sqlite3.h` — public API header
- **License**: Public Domain (see https://www.sqlite.org/copyright.html)

This plugin links SQLite statically into itself by compiling `sqlite3.c` as
part of the plugin target. No external SQLite package or system install is
required on any platform (macOS / Windows / Linux).

To upgrade, download a newer amalgamation zip and replace both files.
