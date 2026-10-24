// SPDX-License-Identifier: GPL-2.0-or-later
// Protocol/ABI derived from OpenRGB GigabyteRGBFusion2BlackwellGPUController
// and i2c_smbus_nvapi. Restricted to the detected RTX 5080 AERO OC (4179:1458).
#pragma once
#include "modules/IDriverModule.h"

#include <QLibrary>
#include <QDebug>
#include <array>
#include <chrono>

inline std::array<uint8_t,64> aeroColorPacket(RgbColor color) {
    std::array<uint8_t,64> packet{};
    packet[0]=0x16;packet[1]=1;packet[3]=3;packet[4]=10;
    packet[5]=color.r;packet[6]=color.g;packet[7]=color.b;
    return packet; // Direct color, zone 0; never send the 0x13 flash-save command.
}
class AeroBackend {
public:
    static constexpr const char* kChannel="GigabyteAero5080";
    bool open(bool probe=true) {
        if(!library_.load())return false;
        auto query=reinterpret_cast<void*(*)(uint32_t)>(library_.resolve("nvapi_QueryInterface"));if(!query)return false;
        auto initialize=reinterpret_cast<int(*)()>(query(0x0150E828));
        auto enumerate=reinterpret_cast<int(*)(void**,int*)>(query(0xE5AC921F));
        auto pci=reinterpret_cast<int(*)(void*,uint32_t*,uint32_t*,uint32_t*,uint32_t*)>(query(0x2DDFB66E));
        write_=reinterpret_cast<Transfer>(query(0x283AC65A));read_=reinterpret_cast<Transfer>(query(0x4D7B0709));
        if(!initialize || !enumerate || !pci || !write_ || !read_ || initialize()!=0)return false;
        std::array<void*,64> handles{};int count=0;if(enumerate(handles.data(),&count)!=0 || count<0 || count>64)return false;
        int matching=0;
        for(int i=0;i<count;++i) { uint32_t device=0,subsystem=0,revision=0,extended=0;
            if(pci(handles[i],&device,&subsystem,&revision,&extended)==0 && device==0x2C0210DE && subsystem==0x41791458) ++matching;
        }
        if(matching!=1) return false; // Never pick the first of multiple identical GPU endpoints.
        for(int i=0;i<count;++i){uint32_t device=0,subsystem=0,revision=0,extended=0;
            if(pci(handles[i],&device,&subsystem,&revision,&extended)!=0 || device!=0x2C0210DE || subsystem!=0x41791458)continue;
            handle_=handles[i];
            if(!probe){ready_=true;return true;} // PCI discovery does not issue the lighting handshake.
            std::array<uint8_t,64> probe{};probe[0]=0x10;probe[1]=1;
            std::array<uint8_t,4> signature{};
            if(transfer(write_,probe.data(),probe.size()) && transfer(read_,signature.data(),signature.size()) && validSignature(signature)){
                ready_=true;qInfo()<<"GPU: RTX 5080 AERO Blackwell lighting controller verified";return true;
            }
            handle_=nullptr;
        }
        qWarning()<<"GPU: RTX 5080 AERO lighting handshake unavailable";return false;
    }
    static bool validSignature(const std::array<uint8_t,4>& bytes){return bytes[0]==1 && (bytes[1]==1 || bytes[1]==2) && bytes[2]==1;}
    bool pushColors(const QVector<RgbColor>& colors){
        if(!ready_ || colors.size()!=1)return false;
        const auto now=std::chrono::steady_clock::now();if(attempted_ && now-lastSend_<std::chrono::milliseconds(33))return true;
        attempted_=true;lastSend_=now;
        auto packet=aeroColorPacket(colors.front());if(!transfer(write_,packet.data(),packet.size())){if(!failed_)qWarning()<<"GPU: lighting write failed";failed_=true;return false;}
        if(!sent_)qInfo()<<"GPU: first Canvas color frame accepted";sent_=true;failed_=false;return true;
    }
private:
    // NVAPI V3 natural x64 layout; only RGB port 1 / address 0x75 is accessed.
    struct I2cInfo {uint32_t version=0,displayMask=0;uint8_t ddc=0,address=0xEA;uint8_t* reg=nullptr;uint32_t regSize=0;uint8_t* data=nullptr;uint32_t size=0,speed=0xFFFF,speedKhz=0;uint8_t port=1;uint32_t portSet=1;};
    static_assert(sizeof(I2cInfo)==64,"NVAPI V3 requires the Windows x64 ABI");
    using Transfer=int(*)(void*,I2cInfo*,uint32_t*);
    bool transfer(Transfer operation,uint8_t* data,uint32_t size){I2cInfo info;info.version=sizeof(I2cInfo)|(3u<<16);info.data=data;info.size=size;uint32_t unknown=0;return operation(handle_,&info,&unknown)==0 && info.size==size;}
    QLibrary library_{"nvapi64"};void* handle_=nullptr;Transfer write_=nullptr,read_=nullptr;bool ready_=false,attempted_=false,failed_=false,sent_=false;
    std::chrono::steady_clock::time_point lastSend_;
};
