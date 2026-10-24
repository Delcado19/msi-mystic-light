// SPDX-License-Identifier: GPL-2.0-or-later
// ENE register protocol follows OpenRGB ENESMBusInterface/ENESMBusController.
#pragma once
#include "modules/IDriverModule.h"
#include <QByteArray>
#include <array>
#include <functional>

// The caller holds the shared SMBus lock across register selection and data access.
class EneRamProtocol {
public:
    using Input = std::array<quint64,9>;
    using Output = std::array<quint64,5>;
    using Transfer = std::function<bool(const Input&,Output&)>;
    EneRamProtocol(int address, Transfer transfer):address_(address),transfer_(std::move(transfer)) {}
    bool verify() {
        if(address_!=0x71 && address_!=0x73) return false;
        QByteArray name;
        for(int i=0;i<16;++i) { int value=read(0x1000+i); if(value<0)return false; name.append(char(value)); }
        const auto zero=name.indexOf('\0'); if(zero>=0)name.truncate(zero);
        return name=="AUDA0-E6K5-0101" && read(0x1c02)==8;
    }
    bool pushColors(const QVector<RgbColor>& colors) {
        if((address_!=0x71 && address_!=0x73) || colors.size()!=8) return false;
        // ENE SetDirect requires a volatile Apply; setting 0x8020 alone leaves the old effect active.
        if(!write(0x8020,1) || !write(0x80a0,1)) return false;
        for(int led=0;led<8;++led) {
            if(!select(0x8100+3*led))return false;
            // ENE V2 uses R,B,G; three-byte blocks match the reference SMBus path.
            const auto& c=colors[led]; Input in{};Output out{};
            in[0]=address_;in[2]=3;in[3]=5;
            in[4]=3u | (quint64(c.r)<<8) | (quint64(c.b)<<16) | (quint64(c.g)<<24);
            if(!transfer_(in,out))return false;
        }
        return true; // Volatile direct buffer only; never write address-remap, SPD or flash registers.
    }
    bool verifyDirectColors(const QVector<RgbColor>& colors, QString* error) {
        if (colors.size()!=8 || (address_!=0x71 && address_!=0x73)) return false;
        const auto check=[&](int reg,int expected) {
            const int actual=read(reg);
            if(actual==expected)return true;
            if(error)*error=QString("RAM register 0x%1: expected 0x%2, read %3")
                .arg(reg,4,16,QLatin1Char('0')).arg(expected,2,16,QLatin1Char('0'))
                .arg(actual<0 ? QString("failed") : QString("0x%1").arg(actual,2,16,QLatin1Char('0')));
            return false;
        };
        // Read back the first live frame; successful SMBus writes alone do not prove direct mode took effect.
        if(!check(0x8020,1))return false;
        for(int led=0;led<8;++led) {
            const auto& c=colors[led];
            if(!check(0x8100+3*led,c.r) || !check(0x8101+3*led,c.b) || !check(0x8102+3*led,c.g))return false;
        }
        return true;
    }
private:
    bool select(int reg) {
        Input in{};Output out{};in[0]=address_;in[3]=3;
        in[4]=((reg&255)<<8)|((reg>>8)&255);
        return transfer_(in,out);
    }
    int read(int reg) {
        if(!select(reg))return -1;
        Input in{};Output out{};in[0]=address_;in[1]=1;in[2]=0x81;in[3]=2;
        return transfer_(in,out) && out[0]<=255 ? int(out[0]) : -1;
    }
    bool write(int reg,int value) {
        if(!select(reg))return false;
        Input in{};Output out{};in[0]=address_;in[2]=1;in[3]=2;in[4]=value;
        return transfer_(in,out);
    }
    int address_; Transfer transfer_;
};
