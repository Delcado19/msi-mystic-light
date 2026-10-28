#include "ServiceProtocol.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLocalSocket>
#include <QElapsedTimer>
#include <QtEndian>
#include <stdexcept>
#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#include <sddl.h>
#include <shlobj.h>
#endif
namespace StudioService {
QString pipeName() { return "LightingStudio.Service.v1"; }
#ifdef Q_OS_WIN
namespace {
QString tokenSid(HANDLE token) {
    DWORD size=0; GetTokenInformation(token,TokenUser,nullptr,0,&size);
    if(!size || size>65536)return {};
    QByteArray buffer(int(size),0);
    if(!GetTokenInformation(token,TokenUser,buffer.data(),size,&size))return {};
    LPWSTR sid=nullptr;
    if(!ConvertSidToStringSidW(reinterpret_cast<TOKEN_USER*>(buffer.data())->User.Sid,&sid))return {};
    const auto result=QString::fromWCharArray(sid);LocalFree(sid);return result;
}
QString processSid(HANDLE process) {
    HANDLE token=nullptr;if(!OpenProcessToken(process,TOKEN_QUERY,&token))return {};
    const auto result=tokenSid(token);CloseHandle(token);return result;
}
}
QString installedHost() {
    SC_HANDLE scm=OpenSCManagerW(nullptr,nullptr,SC_MANAGER_CONNECT);
    SC_HANDLE service=scm?OpenServiceW(scm,L"LightingStudio",SERVICE_QUERY_STATUS):nullptr;
    const bool installed=service!=nullptr;
    if(service)CloseServiceHandle(service);if(scm)CloseServiceHandle(scm);
    if(!installed)return {};
    PWSTR folder=nullptr;if(FAILED(SHGetKnownFolderPath(FOLDERID_ProgramFiles,0,nullptr,&folder)))return {};
    const auto path=QString::fromWCharArray(folder)+"/Lighting Studio/lighting_service_host.exe";CoTaskMemFree(folder);return path;
}
QString currentUserSid() { return processSid(GetCurrentProcess()); }
bool isSystem() { return currentUserSid()=="S-1-5-18"; }
bool authorizedClient(qintptr descriptor,const QString& owner) {
    const auto pipe=reinterpret_cast<HANDLE>(descriptor);
    // Deny network logons even if they authenticate as the configured owner.
    if(owner.isEmpty() || !ImpersonateNamedPipeClient(pipe))return false;
    HANDLE token=nullptr;bool allowed=false;
    if(OpenThreadToken(GetCurrentThread(),TOKEN_QUERY,TRUE,&token)) {
        BYTE network[SECURITY_MAX_SID_SIZE];DWORD size=sizeof(network);BOOL member=TRUE;
        if(CreateWellKnownSid(WinNetworkSid,nullptr,network,&size) && CheckTokenMembership(token,network,&member) && !member)
            allowed=tokenSid(token)==owner;
        CloseHandle(token);
    }
    // Continuing as a client after a failed revert would cross the trust boundary.
    if(!RevertToSelf())TerminateProcess(GetCurrentProcess(),ERROR_ACCESS_DENIED);
    return allowed;
}
bool trustedServer(qintptr descriptor,const QString& executable,bool simulation) {
    ULONG pid=0;if(!GetNamedPipeServerProcessId(reinterpret_cast<HANDLE>(descriptor),&pid))return false;
    HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);if(!process)return false;
    wchar_t path[32768];DWORD size=32768;
    bool identity=false;
    if(simulation)identity=processSid(process)==currentUserSid();
    else {
        // SCM service status is readable without opening a SYSTEM process token.
        SC_HANDLE scm=OpenSCManagerW(nullptr,nullptr,SC_MANAGER_CONNECT);
        SC_HANDLE service=scm?OpenServiceW(scm,L"LightingStudio",SERVICE_QUERY_STATUS):nullptr;
        SERVICE_STATUS_PROCESS status{};DWORD written=0;
        identity=service && QueryServiceStatusEx(service,SC_STATUS_PROCESS_INFO,reinterpret_cast<LPBYTE>(&status),sizeof(status),&written) && status.dwProcessId==pid;
        if(service)CloseServiceHandle(service);if(scm)CloseServiceHandle(scm);
    }
    const bool allowed=identity && QueryFullProcessImageNameW(process,0,path,&size) &&
        QDir::cleanPath(QString::fromWCharArray(path,int(size))).compare(QDir::cleanPath(QFileInfo(executable).absoluteFilePath()),Qt::CaseInsensitive)==0;
    CloseHandle(process);return allowed;
}
#else
QString installedHost() { return {}; }
QString currentUserSid() { return {}; }
bool isSystem() { return false; }
bool authorizedClient(qintptr,const QString&) { return false; }
bool trustedServer(qintptr,const QString&,bool) { return false; }
#endif
QByteArray encode(const QJsonObject& object) {
    auto bytes=QJsonDocument(object).toJson(QJsonDocument::Compact);
    if(bytes.isEmpty() || bytes.size()>maximumMessage)throw std::invalid_argument("Service message exceeds 32 MiB");
    QByteArray result(4,0);qToBigEndian<quint32>(quint32(bytes.size()),result.data());result+=bytes;return result;
}
QJsonObject decode(const QByteArray& bytes) {
    if(bytes.isEmpty() || bytes.size()>maximumMessage)throw std::invalid_argument("Invalid service message size");
    QJsonParseError error;const auto json=QJsonDocument::fromJson(bytes,&error);
    if(error.error!=QJsonParseError::NoError || !json.isObject())throw std::invalid_argument("Invalid service JSON");
    return json.object();
}
QJsonObject request(const QJsonObject& command,const QString& executable,bool simulation,const QString& name) {
    QLocalSocket socket;socket.setReadBufferSize(maximumMessage+4);
    socket.connectToServer(name.isEmpty()?pipeName():name);
    if(!socket.waitForConnected(1500))throw std::runtime_error(("Lighting service unavailable: "+socket.errorString()).toStdString());
    if(!trustedServer(socket.socketDescriptor(),executable,simulation))throw std::runtime_error("Lighting service identity rejected");
    const auto bytes=encode(command);if(socket.write(bytes)!=bytes.size())throw std::runtime_error("Service request write failed");
    QElapsedTimer deadline;deadline.start();
    while(socket.bytesToWrite()>0)if(deadline.elapsed()>15000 || !socket.waitForBytesWritten(1500))throw std::runtime_error("Service request timed out");
    QByteArray reply;quint32 size=0;
    while(true) {
        reply+=socket.readAll();
        if(!size && reply.size()>=4) {
            size=qFromBigEndian<quint32>(reply.constData());
            if(!size || size>maximumMessage)throw std::runtime_error("Service reply exceeds bounds");
        }
        if(size && reply.size()>=size+4)break;
        if(deadline.elapsed()>15000 || socket.state()==QLocalSocket::UnconnectedState)throw std::runtime_error("Lighting service did not reply");
        socket.waitForReadyRead(250);
    }
    if(reply.size()!=size+4)throw std::runtime_error("Unexpected trailing service data");
    const auto result=decode(reply.mid(4));
    if(!result.value("ok").toBool())throw std::runtime_error(result.value("error").toString("Service request rejected").toStdString());
    return result;
}
}
