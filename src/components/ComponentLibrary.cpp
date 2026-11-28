#include "components/ComponentLibrary.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSaveFile>
#include <stdexcept>

namespace {
constexpr qint64 fileLimit = 32 * 1024 * 1024; // the old single-file library (migration only)
// One component file: the same 1 MiB loadComponentDefinition() reads, so a saved part never vanishes
// (CodeRabbit PR #10).
constexpr qint64 componentLimit = 1024 * 1024;

// id -> file path of every readable component file, in name order.
QMap<QString, QString> filesById(const QString& folder)
{
    QMap<QString, QString> result;
    for (const auto& entry : QDir(folder).entryInfoList({"*.json"}, QDir::Files, QDir::Name)) {
        try {
            const auto id = loadComponentDefinition(entry.absoluteFilePath()).id;
            if (!result.contains(id)) result.insert(id, entry.absoluteFilePath());
        } catch (const std::exception&) {}
    }
    return result;
}

QString fileBase(const ComponentDefinition& definition)
{
    auto name = definition.name.trimmed().left(80);
    name.replace(QRegularExpression(R"([<>:"/\\|?*\x00-\x1F])"), "_");
    if (name.isEmpty()) name = "Component";
    // The tag ends the name, so Windows' rules for trailing dots, spaces and device names never apply.
    return name + " - " + QString::fromLatin1(QCryptographicHash::hash(definition.id.toUtf8(), QCryptographicHash::Sha256).toHex().left(8));
}

bool hasBase(const QString& fileName, const QString& base)
{
    return fileName == base + ".json" || QRegularExpression("^" + QRegularExpression::escape(base) + R"(-\d+\.json$)").match(fileName).hasMatch();
}

QString freePath(const QString& folder, const QString& base)
{
    auto path = QDir(folder).filePath(base + ".json");
    for (int n = 2; QFileInfo::exists(path); ++n) path = QDir(folder).filePath(base + "-" + QString::number(n) + ".json");
    return path;
}
}

LibraryRead readComponentFolder(const QString& folder, const ComponentCatalog& builtIn)
{
    LibraryRead read;
    if (folder.isEmpty() || !QFileInfo(folder).isDir()) { read.issues.append({folder, "folder not found"}); return read; }
    for (const auto& entry : QDir(folder).entryInfoList({"*.json"}, QDir::Files, QDir::Name)) {
        try {
            if (entry.size() > componentLimit) throw std::invalid_argument("file exceeds 1 MiB");
            const auto definition = loadComponentDefinition(entry.absoluteFilePath());
            if (builtIn.contains(definition.id)) throw std::invalid_argument("ID \"" + definition.id.toStdString() + "\" belongs to a built-in component");
            if (read.components.contains(definition.id)) throw std::invalid_argument("ID \"" + definition.id.toStdString() + "\" is already used by another file");
            read.components.insert(definition.id, definition);
        } catch (const std::exception& error) {
            read.issues.append({entry.fileName(), QString::fromUtf8(error.what())});
        }
    }
    return read;
}

QString writeComponentFile(const QString& folder, const ComponentDefinition& definition)
{
    const auto bytes = QJsonDocument(definition.toJson()).toJson(QJsonDocument::Indented);
    const auto base = fileBase(definition);
    auto path = filesById(folder).value(definition.id);
    if (path.isEmpty()) path = freePath(folder, base);
    if (bytes.size() > componentLimit) throw std::runtime_error(("Cannot write " + QDir::toNativeSeparators(path) + ": the component is larger than 1 MiB").toStdString());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        throw std::runtime_error(("Cannot write " + QDir::toNativeSeparators(path) + ": " + file.errorString()).toStdString());
    // Written in place first, renamed second: a failed rename only keeps the old name.
    if (!hasBase(QFileInfo(path).fileName(), base)) {
        const auto target = freePath(folder, base);
        if (QFile::rename(path, target)) path = target;
    }
    return path;
}

int renameComponentFiles(const QString& folder, const ComponentCatalog& builtIn)
{
    QHash<QString, int> holders; // ID -> readable files holding it
    QVector<QPair<QString, ComponentDefinition>> found;
    for (const auto& entry : QDir(folder).entryInfoList({"*.json"}, QDir::Files, QDir::Name)) {
        try {
            auto definition = loadComponentDefinition(entry.absoluteFilePath());
            ++holders[definition.id];
            found.append({entry.absoluteFilePath(), definition});
        } catch (const std::exception&) {}
    }
    int renamed = 0;
    for (const auto& [path, definition] : found) {
        // The library skips these; a new name could change which duplicate wins its ID.
        if (holders.value(definition.id) > 1 || builtIn.contains(definition.id)) continue;
        const auto base = fileBase(definition);
        if (!hasBase(QFileInfo(path).fileName(), base) && QFile::rename(path, freePath(folder, base))) ++renamed;
    }
    return renamed;
}

QString componentFilePath(const QString& folder, const QString& id)
{
    return filesById(folder).value(id);
}

bool removeComponentFile(const QString& folder, const QString& id)
{
    const auto path = filesById(folder).value(id);
    return !path.isEmpty() && QFile::remove(path);
}

int migrateLibraryFile(const QString& legacyJson, const QString& folder, const ComponentCatalog& builtIn, QStringList* kept)
{
    QFile file(legacyJson);
    if (!file.exists()) return 0;
    if (!file.open(QIODevice::ReadOnly)) throw std::runtime_error(("Cannot read " + legacyJson + ": " + file.errorString()).toStdString());
    const auto bytes = file.read(fileLimit + 1);
    file.close();
    const auto json = QJsonDocument::fromJson(bytes);
    if (bytes.size() > fileLimit || !json.isArray()) throw std::invalid_argument("Invalid custom component library");
    if (!QDir().mkpath(folder)) throw std::runtime_error(("Cannot create " + folder).toStdString());
    const auto withoutVerification = [](const ComponentDefinition& definition) { auto json = definition.toJson(); json.remove("verification"); return json; };
    int count = 0;
    for (const auto& item : json.array()) {
        auto definition = ComponentDefinition::fromJson(item.toObject());
        if (!componentFilePath(folder, definition.id).isEmpty()) continue; // the folder's copy may be newer
        if (builtIn.contains(definition.id)) {
            // Former code-only parts (generic fans/strips, M25, NV5, Glacier) are built-in files now.
            if (withoutVerification(builtIn.value(definition.id)) == withoutVerification(definition)) continue;
            auto id = definition.id + ".own";
            for (int n = 2; builtIn.contains(id) || !componentFilePath(folder, id).isEmpty(); ++n) id = definition.id + ".own" + QString::number(n);
            definition.id = id;
            if (kept) kept->append(definition.name);
        }
        writeComponentFile(folder, definition); ++count;
    }
    // Keep the old file as a backup under a name nothing reads any more.
    const auto backup = QFileInfo(legacyJson).absoluteDir().filePath("custom-components.migrated.json");
    QFile::remove(backup);
    if (!QFile::rename(legacyJson, backup)) throw std::runtime_error(("Cannot rename " + legacyJson).toStdString());
    return count;
}
