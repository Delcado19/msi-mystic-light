#include "backends/AeroBackend.h"
#include "backends/EneRamProtocol.h"
#include "modules/ModuleRegistry.h"
#include "protocol/MsiProtocol.h"
#include "../hid/FakeHidDevice.h"
#include <QtTest>
#include <QFile>
#include <QJsonDocument>
#include <QCryptographicHash>
class AuxiliaryTest : public QObject {
    Q_OBJECT
private slots:
    void ramProtocol() {
        QVector<EneRamProtocol::Input> commands;
        int selected=-1, failAt=-1;
        const QByteArray firmware("AUDA0-E6K5-0101\0",16);
        auto transfer=[&](const auto& in,auto& out) {
            commands.append(in);
            if(commands.size()==failAt)return false;
            if(in[0]!=0x71 && in[0]!=0x73)return false;
            if(in[1]==0 && in[2]==0 && in[3]==3) selected=int(((in[4]&255)<<8)|(in[4]>>8));
            if(in[1]==1)out[0]=selected>=0x1000 && selected<0x1010 ? quint8(firmware[selected-0x1000]) : selected==0x1c02 ? 8 : 0;
            return true;
        };
        EneRamProtocol ram(0x71,transfer);QVERIFY(ram.verify());
        commands.clear();QVERIFY(ram.pushColors(QVector<RgbColor>(8,{11,22,33})));
        QCOMPARE(commands.size(),20);
        QCOMPARE(commands[0][4],quint64(0x2080));QCOMPARE(commands[1][2],quint64(1));QCOMPARE(commands[1][4],quint64(1));
        QCOMPARE(commands[2][4],quint64(0xa080)); // Apply direct mode before writing the live buffer.
        QCOMPARE(commands[3][2],quint64(1));QCOMPARE(commands[3][4],quint64(1)); // 0x01 applies, 0xAA saves to flash.
        for(int i=0;i<8;++i) {
            const auto& select=commands[4+2*i];const auto& data=commands[5+2*i];
            const int reg=0x8100+3*i;
            QCOMPARE(select[4],quint64(((reg&255)<<8)|(reg>>8)));
            QCOMPARE(data[2],quint64(3));QCOMPARE(data[3],quint64(5));
            QCOMPARE(data[4],quint64(3u|(11u<<8)|(33u<<16)|(22u<<24)));
        }
        commands.clear();QVERIFY(!ram.pushColors({}));QVERIFY(commands.isEmpty());
        for(int failure : {2,3,4,5,6,20}) {
            commands.clear(); failAt=failure;
            QVERIFY(!ram.pushColors(QVector<RgbColor>(8,{1,2,3})));QCOMPARE(commands.size(),failure);
        }
        commands.clear();failAt=2;QVERIFY(!ram.verify());QCOMPARE(commands.size(),2);
        commands.clear();EneRamProtocol unsafe(0x50,transfer);QVERIFY(!unsafe.verify());QVERIFY(!unsafe.pushColors(QVector<RgbColor>(8)));QVERIFY(commands.isEmpty());
        EneRamProtocol impostor(0x71,[](const auto&,auto& out){out[0]=0;return true;});QVERIFY(!impostor.verify());
        commands.clear();failAt=-1;EneRamProtocol second(0x73,transfer);QVERIFY(second.verify());
        commands.clear();QVERIFY(second.pushColors(QVector<RgbColor>(8,{1,2,3})));QCOMPARE(commands.first()[0],quint64(0x73));
        EneRamProtocol wrongCount(0x71,[&](const auto& in,auto& out){if(!transfer(in,out))return false;if(in[1] && selected==0x1c02)out[0]=10;return true;});
        QVERIFY(!wrongCount.verify());
        EneRamProtocol malformed(0x71,[](const auto&,auto& out){out[0]=256;return true;});QVERIFY(!malformed.verify());
    }
    void ramDirectReadback() {
        QMap<int,int> registers; int selected=-1;
        auto transfer=[&](const auto& in,auto& out) {
            if(in[3]==3)selected=int(((in[4]&255)<<8)|(in[4]>>8));
            else if(in[1]==1)out[0]=registers.value(selected,0);
            else if(in[3]==2)registers[selected]=int(in[4]);
            else if(in[3]==5)for(int i=0;i<3;++i)registers[selected+i]=int((in[4]>>(8*(i+1)))&255);
            return true;
        };
        EneRamProtocol ram(0x71,transfer); QString error;
        QVector<RgbColor> colors;for(int i=0;i<8;++i)colors.append({quint8(10+i),quint8(30+i),quint8(50+i)});
        QVERIFY(ram.pushColors(colors));QVERIFY(ram.verifyDirectColors(colors,&error));
        registers[0x8020]=0;QVERIFY(!ram.verifyDirectColors(colors,&error));QVERIFY(error.contains("0x8020"));
        registers[0x8020]=1;registers[0x8117]=0;
        QVERIFY(!ram.verifyDirectColors(colors,&error));QVERIFY(error.contains("0x8117"));
        EneRamProtocol failed(0x73,[](const auto&,auto&){return false;});
        QVERIFY(!failed.verifyDirectColors(colors,&error));QVERIFY(error.contains("failed"));
    }
    void ramGoldenFramesAndFailures() {
        // ENE V2 wire format, independently pinned: R,B,G at 0x8100..0x8117; no flash save.
        const QVector<RgbColor> colors{{0,0,0},{255,255,255},{255,0,0},{0,255,0},{0,0,255},{1,127,254},{128,2,253},{254,255,1}};
        const quint64 registers[]{0x0081,0x0381,0x0681,0x0981,0x0c81,0x0f81,0x1281,0x1581};
        const quint64 blocks[]{0x00000003,0xffffff03,0x0000ff03,0xff000003,0x00ff0003,0x7ffe0103,0x02fd8003,0xff01fe03};
        for(int address : {0x71,0x73}) {
            QVector<EneRamProtocol::Input> commands;int failAt=0;
            EneRamProtocol ram(address,[&](const auto& in,auto&){commands.append(in);return commands.size()!=failAt;});
            QVERIFY(ram.pushColors(colors));QCOMPARE(commands.size(),20);
            for(int i=0;i<8;++i) {
                const EneRamProtocol::Input select{quint64(address),0,0,3,registers[i],0,0,0,0};
                const EneRamProtocol::Input data{quint64(address),0,3,5,blocks[i],0,0,0,0};
                QCOMPARE(commands[4+2*i],select);QCOMPARE(commands[5+2*i],data);
            }
            for(int failure=1;failure<=20;++failure) {
                commands.clear();failAt=failure;QVERIFY(!ram.pushColors(colors));QCOMPARE(commands.size(),failure);
            }
            for(int count : {0,1,7,9,65535}) {
                commands.clear();QVERIFY(!ram.pushColors(QVector<RgbColor>(count)));QVERIFY(commands.isEmpty());
            }
        }
        for(int address : {-1,0,0x50,0x70,0x72,0x74,0x7f,0x80}) {
            int calls=0;EneRamProtocol unsafe(address,[&](const auto&,auto&){++calls;return true;});
            QVERIFY(!unsafe.verify());QVERIFY(!unsafe.pushColors(colors));QVERIFY(!unsafe.verifyDirectColors(colors,nullptr));QCOMPARE(calls,0);
        }
    }
    void signedRamModule() {
        QFile file(QString(PROFILE_DIR)+"/../third_party/pawnio/SmbusPIIX4.bin");QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.size(),qint64(42404));
        QCOMPARE(QCryptographicHash::hash(file.readAll(),QCryptographicHash::Sha256).toHex(),QByteArray("91f9b4b1c39e3d399ce48477a89d8f6bd3e58a2241064a4daec3a1513dff56e5"));
    }
    void packetsAndDescriptor() {
        const auto packet=aeroColorPacket({11,22,33}); QCOMPARE(packet[0],uint8_t(0x16));
        QCOMPARE(packet[5],uint8_t(11)); QCOMPARE(packet[6],uint8_t(22)); QCOMPARE(packet[7],uint8_t(33));
        QVERIFY(AeroBackend::validSignature({1,1,1,0})); QVERIFY(!AeroBackend::validSignature({0,1,1,0}));

    }
    void moduleAndIdentity() {
        ModuleRegistry modules;QString error; QVERIFY2(modules.load(AUX_MODULE_PATH,&error),qPrintable(error));
        for(const auto& name:QStringList{"gigabyte-aero-5080","xpg-ram-71","xpg-ram-73"}) {
            const auto profile=DeviceProfile::load(QString(PROFILE_DIR)+"/"+name+".json"); QVERIFY(modules.acceptsProfile(profile,&error));
            DeviceIdentity identity;identity.path=profile.match["endpoint"].toString();identity.transport=profile.match["transport"].toString();
            identity.vendorId=profile.match["vendorId"].toInt();identity.productId=profile.match["productId"].toInt();
            QVERIFY(profile.matches(identity)); auto impostor=identity;impostor.transport="hid";QVERIFY(!profile.matches(impostor));
            impostor=identity;impostor.path+="other";QVERIFY(!profile.matches(impostor));
            FakeHidDevice transport;QVERIFY(transport.open(1,1));
            auto driver=modules.create(identity,profile,transport,&error);QVERIFY2(driver,qPrintable(error)); QVERIFY(transport.sentReports.empty());
            const auto& channel=profile.channels[0]; QVERIFY(driver->driver().pushColors(channel.id,QVector<RgbColor>(channel.fixedLeds,{11,22,33})));
            QCOMPARE(transport.sentReports.size(),size_t(1)); QCOMPARE(transport.sentReports[0].size(),size_t(channel.fixedLeds*3));
            QVERIFY(!driver->driver().pushColors(channel.id,{}));
            auto invalid=profile;invalid.channels[0].fixedLeds=2;QVERIFY(!modules.acceptsProfile(invalid,&error));
        }
    }
    void shieldCanvasPreservesHeaders() {
        ModuleRegistry modules;QString error;QVERIFY(modules.load(MSI_MODULE_PATH,&error));
        const auto profile=DeviceProfile::load(QString(PROFILE_DIR)+"/msi-mpg-b850-edge-ti-wifi.json");
        DeviceIdentity identity;identity.path="recording";identity.vendorId=0xdb0;identity.productId=0x76;
        identity.boardManufacturer="Micro-Star International Co., Ltd.";identity.boardModel="MPG B850 EDGE TI WIFI (MS-7E62)";
        FakeHidDevice transport;QVERIFY(transport.open(1,1));auto instance=modules.create(identity,profile,transport,&error);QVERIFY(instance);
        auto& driver=instance->driver();QVERIFY(driver.pushColors("JARGB1",{{1,2,3}}));
        QVERIFY(driver.pushColors("IOShield",{{10,20,30},{30,40,50}}));
        const auto report=transport.sentReports.back();QCOMPARE(report.size(),size_t(290)); QCOMPARE(report[0],uint8_t(0x50));
        QCOMPARE(report[1],uint8_t(9)); // Peer ARGB remains Advanced.
        QCOMPARE(report[1+4*16],uint8_t(MsiLightingMode::Steady)); // IO COVER uses Steady.
        QCOMPARE(report[2+4*16],uint8_t(20)); QCOMPARE(report[3+4*16],uint8_t(30)); QCOMPARE(report[4+4*16],uint8_t(40));
        const auto before=transport.sentReports.size();QVERIFY(driver.pushColors("IOShield",{{20,30,40}}));QCOMPARE(transport.sentReports.size(),before);
        QVERIFY(driver.pushColors("IOShield",{{200,100,50}}));QCOMPARE(transport.sentReports.size(),before+1);
    }
};
QTEST_GUILESS_MAIN(AuxiliaryTest)
#include "test_auxiliary.moc"
