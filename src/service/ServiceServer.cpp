#include "ServiceServer.h"
#include "ServiceProtocol.h"
#include <QLocalSocket>
#include <QTimer>
#include <QtEndian>
#include <memory>
#include <stdexcept>
#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#include <sddl.h>
#include <QWinEventNotifier>
#else
#include <QLocalServer>
#endif
ServiceServer::ServiceServer(QString owner,std::function<QJsonObject(const QJsonObject&)> handler,QObject* parent)
    :QObject(parent),owner_(std::move(owner)),handler_(std::move(handler))
{
}
void ServiceServer::serve(QLocalSocket* socket)
{
    if(clients_>=4){socket->abort();socket->deleteLater();return;}++clients_;
    socket->setReadBufferSize(StudioService::maximumMessage+4);
    auto buffer=std::make_shared<QByteArray>();auto authenticated=std::make_shared<bool>(false);auto done=std::make_shared<bool>(false);
    auto* timer=new QTimer(socket);timer->setSingleShot(true);timer->setInterval(15000);
    connect(timer,&QTimer::timeout,socket,&QLocalSocket::abort);timer->start();
    connect(socket,&QLocalSocket::disconnected,this,[this,socket]{--clients_;socket->deleteLater();});
    connect(socket,&QLocalSocket::readyRead,this,[this,socket,buffer,authenticated,done,timer] {
        if(*done)return;
        if(!*authenticated) {
            if(!StudioService::authorizedClient(socket->socketDescriptor(),owner_)){socket->abort();return;}
            *authenticated=true;
        }
        *buffer+=socket->readAll();if(buffer->size()<4)return;
        const auto size=qFromBigEndian<quint32>(buffer->constData());
        if(!size || size>StudioService::maximumMessage || buffer->size()>size+4){socket->abort();return;}
        if(buffer->size()!=size+4)return;
        timer->stop();*done=true;
        QJsonObject reply;
        try{reply=handler_(StudioService::decode(buffer->mid(4)));}
        catch(const std::exception& e){reply={{"ok",false},{"error",QString::fromUtf8(e.what())}};}
        socket->write(StudioService::encode(reply));socket->flush();socket->disconnectFromServer();
    });
    if(socket->bytesAvailable())QMetaObject::invokeMethod(socket,"readyRead",Qt::QueuedConnection);
}
#ifdef Q_OS_WIN
// Own pipe instead of QLocalServer (audit 2026-11-14): Qt can only grant world access on Windows,
// so any local user could open the pipe and hold client slots before authentication. This DACL
// lets only SYSTEM, the server's own account (needed to add instances) and the configured owner
// open it at all; authorizedClient below still checks the impersonated token as a second layer.
// The owner keeps generic write, which includes FILE_CREATE_PIPE_INSTANCE: QLocalSocket opens with
// GENERIC_WRITE, and clients reject foreign instances through trustedServer's process check.
bool ServiceServer::createInstance(bool first)
{
    const auto self=StudioService::currentUserSid();
    const auto sddl=QString("D:P(A;;GA;;;SY)(A;;GA;;;%1)(A;;GRGW;;;%2)").arg(self,owner_);
    PSECURITY_DESCRIPTOR descriptor=nullptr;
    if(!ConvertStringSecurityDescriptorToSecurityDescriptorW(reinterpret_cast<LPCWSTR>(sddl.utf16()),SDDL_REVISION_1,&descriptor,nullptr)) {
        error_="Invalid service pipe security descriptor";return false;
    }
    SECURITY_ATTRIBUTES attributes{sizeof(attributes),descriptor,FALSE};
    // FIRST_PIPE_INSTANCE: listening fails if another process created the name first (pipe squatting).
    const DWORD open=PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED|(first?FILE_FLAG_FIRST_PIPE_INSTANCE:0);
    HANDLE pipe=CreateNamedPipeW(reinterpret_cast<LPCWSTR>(path_.utf16()),open,PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT|PIPE_REJECT_REMOTE_CLIENTS,
        PIPE_UNLIMITED_INSTANCES,65536,65536,0,&attributes);
    LocalFree(descriptor);
    if(pipe==INVALID_HANDLE_VALUE){error_=QString("Cannot create service pipe (Windows error %1)").arg(GetLastError());return false;}
    auto* overlapped=static_cast<OVERLAPPED*>(overlapped_);*overlapped={};overlapped->hEvent=event_;ResetEvent(event_);
    pending_=pipe;immediate_=false;
    if(ConnectNamedPipe(pipe,overlapped))return true;
    switch(GetLastError()) {
    case ERROR_IO_PENDING: return true;
    case ERROR_PIPE_CONNECTED: immediate_=true;SetEvent(event_);return true;   // Client won the race; the event would not fire.
    default: error_=QString("Cannot wait for service clients (Windows error %1)").arg(GetLastError());
        CloseHandle(pipe);pending_=nullptr;return false;
    }
}
void ServiceServer::connected()
{
    if(!pending_){ResetEvent(event_);return;}   // Creating the last instance failed; nothing is listening.
    HANDLE pipe=pending_;pending_=nullptr;DWORD ignored=0;
    const bool ok=immediate_ || GetOverlappedResult(pipe,static_cast<OVERLAPPED*>(overlapped_),&ignored,FALSE);
    // Keep one instance listening before serving, so the next client never sees a busy pipe.
    if(!createInstance(false))qWarning("%s",qPrintable(error_));
    if(!ok){CloseHandle(pipe);return;}   // ERROR_NO_DATA: the client came and went.
    auto* socket=new QLocalSocket(this);
    if(!socket->setSocketDescriptor(reinterpret_cast<qintptr>(pipe))){CloseHandle(pipe);delete socket;return;}
    serve(socket);
}
bool ServiceServer::listen(const QString& name)
{
    if(event_ || name.isEmpty() || name.contains('\\') || name.contains('/')){error_="Invalid service pipe name";return false;}
    path_="\\\\.\\pipe\\"+name;
    event_=CreateEventW(nullptr,TRUE,FALSE,nullptr);overlapped_=new OVERLAPPED{};
    if(!event_){error_="Cannot create service pipe event";return false;}
    if(!createInstance(true))return false;
    notifier_=new QWinEventNotifier(event_,this);
    connect(notifier_,&QWinEventNotifier::activated,this,[this]{connected();});
    return true;
}
ServiceServer::~ServiceServer()
{
    if(notifier_)notifier_->setEnabled(false);
    // The pending connect still references the OVERLAPPED and event; cancel before freeing them.
    if(pending_){CancelIoEx(pending_,static_cast<OVERLAPPED*>(overlapped_));DWORD ignored=0;GetOverlappedResult(pending_,static_cast<OVERLAPPED*>(overlapped_),&ignored,TRUE);CloseHandle(pending_);}
    delete static_cast<OVERLAPPED*>(overlapped_);
    delete notifier_;notifier_=nullptr;
    if(event_)CloseHandle(event_);
}
#else
// shortcut: non-Windows keeps QLocalServer and authorizedClient still rejects everyone; add a
// SO_PEERCRED owner check when the Linux service is built (owner decision 2026-11-14).
bool ServiceServer::listen(const QString& name)
{
    server_=new QLocalServer(this);server_->setSocketOptions(QLocalServer::WorldAccessOption);server_->setMaxPendingConnections(4);
    connect(server_,&QLocalServer::newConnection,this,[this]{while(server_->hasPendingConnections())serve(server_->nextPendingConnection());});
    if(server_->listen(name))return true;
    error_=server_->errorString();return false;
}
ServiceServer::~ServiceServer()=default;
#endif
