#pragma once
#include <QByteArray>
#include <QJsonObject>
#include <QString>
#include <QtGlobal>
class QLocalSocket;
namespace StudioService {
constexpr quint32 maximumMessage = 32 * 1024 * 1024;
constexpr int protocolVersion = 1;
QString pipeName();
QString installedHost();
QString currentUserSid();
bool isSystem();
// Authenticate both pipe ends; callers cannot supply identity in JSON.
bool authorizedClient(qintptr pipe, const QString& ownerSid);
bool trustedServer(qintptr pipe, const QString& executable, bool simulation = false);
QByteArray encode(const QJsonObject& object);
QJsonObject decode(const QByteArray& bytes);
QJsonObject request(const QJsonObject& command, const QString& executable, bool simulation = false, const QString& pipe = {});
}
