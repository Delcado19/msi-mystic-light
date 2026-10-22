// SPDX-License-Identifier: GPL-2.0-or-later
// OpenRGB SDK protocol v0 adapter, scoped to the two verified ENE XPG modules.
// The separate RAM-only helper owns PawnIO; other devices never use this socket.
#pragma once
#include "device/IDriver.h"
#include "device/Channel.h"
#include <QTcpSocket>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>
#include <QProcess>
#include <QThread>
#include <QDataStream>
#include <QElapsedTimer>
#include <QDebug>
#include <memory>
#include <optional>

// Use a separate configuration: the RAM helper must not take ownership of
// the motherboard/GPU/keyboards already driven by Lighting Studio.
inline bool startRamControllerHelper() {
    QString executable=QStandardPaths::findExecutable("OpenRGB.exe");
    if(executable.isEmpty()){
        const QStringList candidates={QDir::homePath()+"/Documents/Software_Projects/OpenRGB/OpenRGB/release/OpenRGB.exe","C:/Program Files/OpenRGB/OpenRGB.exe"};
        for(const auto& path:candidates)if(QFile::exists(path)){executable=path;break;}
    }
    if(executable.isEmpty())return false;
    const QString folder=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/ram-controller";
    if(!QDir().mkpath(folder))return false;QFile source(":/studio/ram-controller.json");if(!source.open(QIODevice::ReadOnly))return false;
    QSaveFile target(folder+"/OpenRGB.json");const auto bytes=source.readAll();if(!target.open(QIODevice::WriteOnly) || target.write(bytes)!=bytes.size() || !target.commit())return false;
    const auto quote=[](QString value){value.replace("'","''");return "'"+value+"'";};
    const QString arguments="--server --server-host 127.0.0.1 --server-port 6745 --noautoconnect --config \""+folder+"\" --loglevel 3";
    const QString command="Start-Process -FilePath "+quote(executable)+" -ArgumentList "+quote(arguments)+" -WorkingDirectory "+quote(QFileInfo(executable).absolutePath())+" -Verb RunAs -WindowStyle Hidden";
    return QProcess::startDetached("powershell.exe",{"-NoProfile","-Command",command});
}

struct RamSdkDescriptor {QString name,version,location;int ledCount=0,directIndex=-1;QByteArray directMode;};
inline std::optional<RamSdkDescriptor> parseRamSdkDescriptor(const QByteArray& data) {
    QDataStream in(data);in.setByteOrder(QDataStream::LittleEndian);RamSdkDescriptor result;
    const auto text=[&]() {quint16 size=0;in>>size;if(size==0 || size>4096){in.setStatus(QDataStream::ReadCorruptData);return QString();}QByteArray bytes(size,0);if(in.readRawData(bytes.data(),size)!=size || bytes.back()!=0){in.setStatus(QDataStream::ReadCorruptData);return QString();}bytes.chop(1);return QString::fromUtf8(bytes);};
    quint32 size=0,type=0;in>>size>>type;if(size!=quint32(data.size()) || type!=1)return {};
    result.name=text();text();result.version=text();text();result.location=text();
    quint16 modes=0;quint32 active=0;in>>modes>>active;if(modes>64)return {};
    for(int i=0;i<modes;++i){const auto start=in.device()->pos();const auto name=text();quint32 field=0;for(int j=0;j<9;++j)in>>field;quint16 colors=0;in>>colors;if(colors>256)return {};in.skipRawData(colors*4);if(name=="Direct"){result.directIndex=i;result.directMode=data.mid(start,in.device()->pos()-start);}}
    quint16 zones=0;in>>zones;if(zones>32)return {};
    for(int i=0;i<zones;++i){text();quint32 field=0;for(int j=0;j<4;++j)in>>field;quint16 matrix=0;in>>matrix;in.skipRawData(matrix);}
    quint16 leds=0;in>>leds;result.ledCount=leds;if(leds>32)return {};
    for(int i=0;i<leds;++i){text();quint32 value=0;in>>value;}
    quint16 colors=0;in>>colors;if(colors!=leds || in.skipRawData(colors*4)!=colors*4)return {};
    if(in.status()!=QDataStream::Ok || result.name!="ENE DRAM" || !result.version.startsWith("AUDA0-E6K5") || leds!=8 || result.directIndex<0 || !(result.location.endsWith("0x71") || result.location.endsWith("0x73")))return {};
    return result;
}
inline QByteArray ramSdkColorPayload(const std::vector<MsiColor>& colors) {
    QByteArray body;QDataStream out(&body,QIODevice::WriteOnly);out.setByteOrder(QDataStream::LittleEndian);out<<quint32(6+colors.size()*4)<<quint16(colors.size());for(const auto& c:colors)out<<quint32(c.r|(quint32(c.g)<<8)|(quint32(c.b)<<16));return body;
}
class OpenRgbRamDriver : public IDriver {
public:
    struct Device {quint32 id;RamSdkDescriptor info;};
    bool open() {
        socket_=std::make_unique<QTcpSocket>();socket_->connectToHost("127.0.0.1",6745);if(!socket_->waitForConnected(500)){close();return false;}
        QByteArray zero(4,0);if(!request(0,40,zero))return false;const auto countData=request(0,0,{});if(!countData || countData->size()!=4)return false;
        QDataStream countStream(*countData);countStream.setByteOrder(QDataStream::LittleEndian);quint32 count=0;countStream>>count;if(count>16)return false;
        for(quint32 id=0;id<count;++id){const auto bytes=request(id,1,zero);if(!bytes)continue;const auto info=parseRamSdkDescriptor(*bytes);if(info){const auto name=info->location.endsWith("0x71")?"XpgRam1":"XpgRam2";devices_[name]={id,*info};qInfo()<<"RAM:"<<name<<info->ledCount<<"LEDs"<<info->location;}}
        return !devices_.empty();
    }
    std::vector<Channel> channels()const {
        std::vector<Channel> channels;
        for(const auto& [name,device]:devices_){Channel channel(name,device.info.ledCount);auto c=Component::makeGenericPlaceholder(device.info.ledCount);c.brand="ADATA XPG";c.type="RAM";c.productName="XPG Lancer Blade RGB DDR5-6000 / 24 GB";c.displayName=c.productName+" / "+name;c.width=140;c.height=40;for(int i=0;i<c.ledCount;++i){c.ledCoordinates[i]={i*18,0};c.ledNames[i]="Light bar LED "+std::to_string(i+1);}c.layoutDefaults=c.ledCoordinates;channel.addComponent(c);channels.push_back(std::move(channel));}return channels;
    }
    void moveToThread(QThread* thread){if(socket_)socket_->moveToThread(thread);}
    void close(){if(socket_){socket_->abort();socket_.reset();}}
    bool supportsMode(const std::string& name,ChannelMode mode)const override{return devices_.count(name) && mode==ChannelMode::Canvas;}
    bool setChannelMode(const std::string& name,ChannelMode mode)override {
        if(!supportsMode(name,mode))return false;const auto& device=devices_.at(name);QByteArray body;QDataStream out(&body,QIODevice::WriteOnly);out.setByteOrder(QDataStream::LittleEndian);out<<quint32(8+device.info.directMode.size())<<quint32(device.info.directIndex);body+=device.info.directMode;return send(device.id,1101,body);
    }
    ChannelMode channelMode(const std::string& name)const override{return devices_.count(name)?ChannelMode::Canvas:ChannelMode::Native;}
    bool setNativeMode(const std::string&,const NativeModeSettings&)override{return false;}
    bool pushCanvasColors(const std::string& name,const std::vector<MsiColor>& colors)override {
        if(!devices_.count(name) || int(colors.size())!=devices_.at(name).info.ledCount)return false;
        // Bound backlog: never queue obsolete color frames behind a stalled helper.
        if(!socket_ || socket_->state()!=QAbstractSocket::ConnectedState || socket_->bytesToWrite()>4096)return false;
        if(socket_->bytesAvailable())socket_->readAll();return send(devices_.at(name).id,1050,ramSdkColorPayload(colors));
    }
private:
    bool send(quint32 device,quint32 kind,const QByteArray& payload) {
        if(!socket_ || socket_->state()!=QAbstractSocket::ConnectedState)return false;
        QByteArray packet("ORGB",4);QDataStream out(&packet,QIODevice::Append);out.setByteOrder(QDataStream::LittleEndian);out<<device<<kind<<quint32(payload.size());packet+=payload;
        if(socket_->write(packet)!=packet.size())return false;socket_->flush();return socket_->bytesToWrite()==0 || socket_->waitForBytesWritten(100);
    }
    std::optional<QByteArray> request(quint32 device,quint32 kind,const QByteArray& payload) {
        if(!send(device,kind,payload))return {};QElapsedTimer timeout;timeout.start();
        const auto read=[&](int size)->QByteArray{while(socket_->bytesAvailable()<size){const int remaining=1000-int(timeout.elapsed());if(remaining<=0 || !socket_->waitForReadyRead(remaining))return {};}return socket_->read(size);};
        for(int frame=0;frame<8;++frame){const auto header=read(16);if(header.size()!=16 || header.left(4)!="ORGB")return {};QDataStream in(header.mid(4));in.setByteOrder(QDataStream::LittleEndian);quint32 id=0,packetKind=0,size=0;in>>id>>packetKind>>size;if(size>1024*1024)return {};const auto body=read(size);if(body.size()!=size)return {};if(packetKind==kind && id==device)return body;}return {};
    }
    std::unique_ptr<QTcpSocket> socket_;std::map<std::string,Device> devices_;
};
