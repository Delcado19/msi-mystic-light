#include "ServiceServer.h"
#include "ServiceProtocol.h"
#include <QLocalSocket>
#include <QTimer>
#include <QtEndian>
#include <memory>
#include <stdexcept>
ServiceServer::ServiceServer(QString owner,std::function<QJsonObject(const QJsonObject&)> handler,QObject* parent)
    :QLocalServer(parent),owner_(std::move(owner)),handler_(std::move(handler))
{
    // Windows ACLs alone cannot express our installation-owner policy through Qt.
    // Authentication below rejects network logons and every other user before parsing.
    setSocketOptions(QLocalServer::WorldAccessOption);setMaxPendingConnections(4);
    connect(this,&QLocalServer::newConnection,this,[this] {
        while(hasPendingConnections()) {
            auto* socket=nextPendingConnection();
            if(clients_>=4){socket->abort();socket->deleteLater();continue;}++clients_;
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
    });
}
