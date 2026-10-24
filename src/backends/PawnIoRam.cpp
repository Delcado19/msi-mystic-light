// SPDX-License-Identifier: GPL-2.0-or-later
#include "PawnIoRam.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QLibrary>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>
#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#endif

namespace {
QString libraryPath() {
    // Official PawnIO installs here; never load a driver DLL from PATH or the working directory.
    const auto programFiles=qEnvironmentVariable("ProgramW6432",qEnvironmentVariable("ProgramFiles","C:/Program Files"));
    return QDir(programFiles).filePath("PawnIO/PawnIOLib.dll");
}
QMutex cacheMutex;
QVector<int> verifiedAddresses;
QString discoveryError;
}
struct PawnIoRam::Impl {
    QLibrary library{libraryPath()};
    QString error;
    int address=0;
    bool ready=false, sent=false;
#ifdef Q_OS_WIN
    using Open=HRESULT(WINAPI*)(HANDLE*);
    using Load=HRESULT(WINAPI*)(HANDLE,const unsigned char*,SIZE_T);
    using Execute=HRESULT(WINAPI*)(HANDLE,const char*,const ULONG64*,SIZE_T,ULONG64*,SIZE_T,SIZE_T*);
    using Close=HRESULT(WINAPI*)(HANDLE);
    Execute execute=nullptr; Close close=nullptr; HANDLE handle=nullptr,mutex=nullptr;
    ~Impl() { if(handle && close)close(handle);if(mutex)CloseHandle(mutex); }
    bool call(const char* name,const quint64* in,size_t count,quint64* out,size_t capacity,size_t expected) {
        SIZE_T returned=0;
        const auto status=execute(handle,name,in,count,out,capacity,&returned);
        if(FAILED(status) || returned!=expected) {
            error=QString("PawnIO %1 failed (0x%2, returned %3/%4)").arg(name).arg(quint32(status),8,16,QLatin1Char('0')).arg(returned).arg(expected);
            return false;
        }
        return true;
    }
    bool locked(const std::function<bool()>& operation) {
        const auto wait=WaitForSingleObject(mutex,250);
        if(wait!=WAIT_OBJECT_0 && wait!=WAIT_ABANDONED) {error="SMBus is busy or its shared lock is unavailable";return false;}
        struct Release {HANDLE handle;~Release(){ReleaseMutex(handle);}} release{mutex};
        return operation();
    }
    bool onPrimary(const std::function<bool(EneRamProtocol&)>& operation) {
        return locked([&] {
            quint64 primary=0,previous=0;
            if(!call("ioctl_piix4_port_sel",&primary,1,&previous,1,1))return false;
            EneRamProtocol protocol(address,[&](const auto& in,auto& out) {
                // Simple reads return one cell; block/byte writes return none.
                const size_t count=in[1]==1 ? 1 : 0;
                return call("ioctl_smbus_xfer",in.data(),in.size(),out.data(),count,count);
            });
            const bool result=operation(protocol);
            const bool restored=call("ioctl_piix4_port_sel",&previous,1,&primary,1,1);
            return result && restored;
        });
    }
#endif
};
PawnIoRam::PawnIoRam():impl_(std::make_unique<Impl>()) {}
PawnIoRam::~PawnIoRam()=default;
QString PawnIoRam::error() const {return impl_->error;}
bool PawnIoRam::runtimeAvailable(QString* error) {
#ifdef Q_OS_WIN
    QLibrary library(libraryPath());
    using Version=HRESULT(WINAPI*)(ULONG*);
    const auto version=reinterpret_cast<Version>(library.resolve("pawnio_version"));
    ULONG value=0;
    if(version && SUCCEEDED(version(&value)) && (value>>16)==2) {if(error)error->clear();return true;}
#endif
    if(error)*error="RAM requires the official PawnIO 2.x installation from https://pawnio.eu";
    return false;
}
bool PawnIoRam::open(int address) {
    if(address!=0x71 && address!=0x73) {impl_->error="Unsupported RAM address";return false;}
    if(impl_->ready)return false;
    impl_=std::make_unique<Impl>(); // A failed open must not retain handles across retries.
    if(!runtimeAvailable(&impl_->error))return false;
#ifdef Q_OS_WIN
    auto& p=*impl_;p.address=address;
    const auto open=reinterpret_cast<Impl::Open>(p.library.resolve("pawnio_open"));
    const auto load=reinterpret_cast<Impl::Load>(p.library.resolve("pawnio_load"));
    p.execute=reinterpret_cast<Impl::Execute>(p.library.resolve("pawnio_execute"));
    p.close=reinterpret_cast<Impl::Close>(p.library.resolve("pawnio_close"));
    if(!open || !load || !p.execute || !p.close) {p.error="PawnIO library is missing required entry points";return false;}
    QFile module(QCoreApplication::applicationDirPath()+"/third_party/pawnio/SmbusPIIX4.bin");
    if(!module.open(QIODevice::ReadOnly) || module.size()!=42404) {p.error="Signed AMD SMBus module is missing";return false;}
    const auto blob=module.readAll();
    if(QCryptographicHash::hash(blob,QCryptographicHash::Sha256).toHex()!="91f9b4b1c39e3d399ce48477a89d8f6bd3e58a2241064a4daec3a1513dff56e5") {
        p.error="AMD SMBus module integrity check failed";return false;
    }
    p.mutex=CreateMutexW(nullptr,FALSE,L"Global\\Access_SMBUS.HTP.Method");
    if(!p.mutex) {p.error="Cannot open the shared SMBus lock";return false;}
    if(!p.locked([&] {
        const auto status=open(&p.handle);
        if(FAILED(status)) {p.error=QString("Cannot open PawnIO (0x%1); verify driver installation and access permissions").arg(quint32(status),8,16,QLatin1Char('0'));return false;}
        const auto loaded=load(p.handle,reinterpret_cast<const unsigned char*>(blob.constData()),blob.size());
        if(FAILED(loaded)) {p.error="Signed PawnIO AMD module rejected or chipset unsupported";return false;}
        std::array<quint64,3> identity{};
        if(!p.call("ioctl_identity",nullptr,0,identity.data(),3,3))return false;
        if((identity[2]&0xffffffff)!=0x790b1022) {p.error="Unsupported AMD SMBus controller";return false;}
        return true;
    }))return false;
    p.ready=p.onPrimary([](auto& protocol) {return protocol.verify();});
    if(!p.ready && p.error.isEmpty())p.error="RAM identity rejected: expected ENE AUDA0-E6K5-0101 with 8 LEDs";
    if(p.ready)qInfo().noquote()<<QString("RAM 0x%1: ENE identity verified via PawnIO").arg(address,2,16,QLatin1Char('0'));
    return p.ready;
#else
    return false;
#endif
}
bool PawnIoRam::pushColors(const QVector<RgbColor>& colors) {
#ifdef Q_OS_WIN
    if(!impl_->ready || colors.size()!=8)return false;
    const bool success=impl_->onPrimary([&](auto& protocol) {
        if(!protocol.pushColors(colors))return false;
        return impl_->sent || protocol.verifyDirectColors(colors,&impl_->error);
    });
    if(!success) {
        if(impl_->error.isEmpty())impl_->error="RAM direct frame transfer failed";
        qWarning().noquote()<<QString("RAM 0x%1: %2").arg(impl_->address,2,16,QLatin1Char('0')).arg(impl_->error);
        return false;
    }
    if(!impl_->sent) {
        QByteArray rgb; for(const auto& c:colors){rgb.append(char(c.r));rgb.append(char(c.g));rgb.append(char(c.b));}
        qInfo().noquote()<<QString("RAM 0x%1: first Canvas frame verified by register readback; RGB=%2")
            .arg(impl_->address,2,16,QLatin1Char('0')).arg(QString::fromLatin1(rgb.toHex()));
        impl_->sent=true;
    }
    return true;
#else
    Q_UNUSED(colors);return false;
#endif
}
QVector<int> scanPawnIoRam(bool activeProbe) {
    if(!activeProbe) {QMutexLocker lock(&cacheMutex);return verifiedAddresses;}
    QVector<int> result;QString error;
    if(!PawnIoRam::runtimeAvailable(&error))qWarning().noquote()<<error;
    else for(int address:{0x71,0x73}) {
        PawnIoRam ram;if(ram.open(address))result.append(address);
        else {if(error.isEmpty())error=ram.error();qWarning().noquote()<<ram.error();}
    }
    QMutexLocker lock(&cacheMutex);verifiedAddresses=result;discoveryError=error;return result;
}
QString pawnIoRamDiscoveryError() {QMutexLocker lock(&cacheMutex);return discoveryError;}
