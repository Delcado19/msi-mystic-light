#pragma once
#include <QObject>
#include <QJsonObject>
#include <functional>
class QLocalSocket;
class QLocalServer;
class QWinEventNotifier;
// Length-bounded single-request connections, authenticated before JSON parsing.
class ServiceServer : public QObject {
public:
    using Reply=std::function<void(const QJsonObject&)>;
    // The handler answers through Reply, immediately or later from the server's thread; never twice.
    using Handler=std::function<void(const QJsonObject&,const Reply&)>;
    ServiceServer(QString owner,Handler handler,QObject* parent=nullptr);
    ~ServiceServer() override;
    bool listen(const QString& name);
    QString errorString() const { return error_; }
private:
    void serve(QLocalSocket* socket);
    QString owner_,error_;
    Handler handler_;
    int clients_=0;
#ifdef Q_OS_WIN
    bool createInstance(bool first);
    void connected();
    void rearm();
    QString path_;
    void* pending_=nullptr;   // Pipe instance waiting for the next client (HANDLE).
    void* event_=nullptr;     // Overlapped connect event (HANDLE).
    void* overlapped_=nullptr;
    QWinEventNotifier* notifier_=nullptr;
    bool immediate_=false;    // ConnectNamedPipe reported ERROR_PIPE_CONNECTED.
#else
    QLocalServer* server_=nullptr;
#endif
};
