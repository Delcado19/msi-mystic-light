#pragma once
#include <QLocalServer>
#include <QJsonObject>
#include <functional>
// Length-bounded single-request connections, authenticated before JSON parsing.
class ServiceServer : public QLocalServer {
public:
    ServiceServer(QString owner,std::function<QJsonObject(const QJsonObject&)> handler,QObject* parent=nullptr);
private:
    QString owner_;
    std::function<QJsonObject(const QJsonObject&)> handler_;
    int clients_=0;
};
