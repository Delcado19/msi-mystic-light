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
// Writes <id>.json (characters unsafe in file names become '_'); an existing file holding the same ID
// is overwritten in place. Returns the path. Throws std::runtime_error("Cannot write <path>: <reason>").
QString writeComponentFile(const QString& folder, const ComponentDefinition& definition);
// Removes the file holding this ID; returns false when none does.
bool removeComponentFile(const QString& folder, const QString& id);
// Splits the old array file into one file per component and renames it to
// custom-components.migrated.json. Returns the number migrated; 0 when the old file does not exist.
int migrateLibraryFile(const QString& legacyJson, const QString& folder);
