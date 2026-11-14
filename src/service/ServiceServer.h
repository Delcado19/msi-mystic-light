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
    ServiceServer(QString owner,std::function<QJsonObject(const QJsonObject&)> handler,QObject* parent=nullptr);
    ~ServiceServer() override;
    bool listen(const QString& name);
    QString errorString() const { return error_; }
private:
    void serve(QLocalSocket* socket);
    QString owner_,error_;
    std::function<QJsonObject(const QJsonObject&)> handler_;
    int clients_=0;
#ifdef Q_OS_WIN
    bool createInstance(bool first);
    void connected();
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
