#pragma once

#include "components/ComponentCatalog.h"

// User component library: one JSON file per component in a folder the user chooses
// (plan 2026-11-27, Task 3; replaces the single custom-components.json array).

struct LibraryIssue { QString file, reason; };
struct LibraryRead { ComponentCatalog components; QVector<LibraryIssue> issues; };

// Never throws for bad content: each unreadable file, built-in ID or duplicate ID is reported by file
// name and skipped (the first file in name order keeps a duplicated ID; built-in IDs always win).
// A missing folder gives one issue "folder not found".
LibraryRead readComponentFolder(const QString& folder, const ComponentCatalog& builtIn);
// Writes "<name> - <tag>.json": the part's name (characters Windows forbids become '_') and 8 hex digits
// of the ID's SHA-256, so equal names do not collide (owner request 2026-11-28; was "<id>.json"). Files
// are found by the ID inside, never by name. An existing file holding the same ID is overwritten and then
// renamed to match the name; a failed rename keeps the old file name. Returns the path.
// Throws std::runtime_error("Cannot write <path>: <reason>").
QString writeComponentFile(const QString& folder, const ComponentDefinition& definition);
// Gives every readable file whose name does not match its part the readable name above (files of earlier
// builds). Files the library skips (duplicate or built-in ID, unreadable) keep their names, so the file
// that wins a duplicate ID does not change. Returns the number of renamed files; failures are skipped.
int renameComponentFiles(const QString& folder, const ComponentCatalog& builtIn);
// The file holding this ID, or empty.
QString componentFilePath(const QString& folder, const QString& id);
// Removes the file holding this ID; returns false when none does.
bool removeComponentFile(const QString& folder, const QString& id);
// Splits the old array file into one file per component and renames it to
// custom-components.migrated.json. Returns the number of files written; 0 when the old file does not exist.
// Entries whose ID is now built in (former code-only parts, final review 2026-11-27): equal to the
// built-in apart from `verification` -> no file; different -> kept under "<id>.own" and named in `kept`.
// An ID that already has a file in the folder is left alone (the folder's copy may be newer).
int migrateLibraryFile(const QString& legacyJson, const QString& folder, const ComponentCatalog& builtIn = {}, QStringList* kept = nullptr);
