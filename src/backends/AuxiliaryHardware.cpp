// SPDX-License-Identifier: GPL-2.0-or-later
// Restores the original Lighting Studio qualified NVAPI path behind the module transport contract.
#include "AuxiliaryHardware.h"
#include "AeroBackend.h"
#include "PawnIoRam.h"
#include <stdexcept>

namespace {
class AuxiliaryTransport final : public IHidDevice {
public:
    explicit AuxiliaryTransport(const DeviceIdentity& identity) {
        if(identity.transport=="nvapi" && identity.path=="nvapi://10de:2c02/1458:4179") {
            gpu_=std::make_unique<AeroBackend>(); ready_=gpu_->open();
        } else if(identity.transport=="pawnio" && identity.vendorId==0x1022 && identity.productId==0x790b) {
            int address=0;
            if(identity.path=="pawnio://amd-piix4/0/0x71")address=0x71;
            if(identity.path=="pawnio://amd-piix4/0/0x73")address=0x73;
            if(address) {ram_=std::make_unique<PawnIoRam>();ready_=ram_->open(address);
                if(!ready_)throw std::runtime_error(ram_->error().toStdString());}
        }
    }
    bool open(std::uint16_t,std::uint16_t) override { return false; }
    bool isOpen() const override { return ready_; }
    void close() override { ready_=false;ram_.reset();gpu_.reset(); }
    bool sendFeatureReport(const std::uint8_t* bytes,std::size_t count) override {
        const std::size_t expected=ram_ ? 24 : 3;
        if(!ready_ || !bytes || count!=expected) return false;
        QVector<RgbColor> colors;
        for(std::size_t i=0;i<count;i+=3) colors.append({bytes[i],bytes[i+1],bytes[i+2]});
        if(ram_) {
            if(!ram_->pushColors(colors))throw std::runtime_error(ram_->error().toStdString());
            return true;
        }
        return gpu_ && gpu_->pushColors(colors);
    }
private:
    bool ready_=false;
    std::unique_ptr<AeroBackend> gpu_;
    std::unique_ptr<PawnIoRam> ram_;
};
}
QVector<ProfileCandidate> scanAuxiliaryHardware(const QVector<DeviceProfile>& profiles,bool probeRam)
{
    QVector<DeviceIdentity> devices;
    bool wantGpu=false,wantRam=false;
    for(const auto& profile:profiles) { wantGpu|=profile.match.value("transport")=="nvapi";wantRam|=profile.match.value("transport")=="pawnio"; }
    AeroBackend gpu;
    if(wantGpu && gpu.open(false)) { DeviceIdentity device; device.transport="nvapi"; device.vendorId=0x10de; device.productId=0x2c02;
        device.path="nvapi://10de:2c02/1458:4179"; devices.append(device); }
    if(wantRam)for(int address:scanPawnIoRam(probeRam)) {
        DeviceIdentity device;device.transport="pawnio";device.vendorId=0x1022;device.productId=0x790b;
        device.path=QString("pawnio://amd-piix4/0/0x%1").arg(address,2,16,QLatin1Char('0'));devices.append(device);
    }
    QVector<ProfileCandidate> result;
    for(const auto& device:devices) for(const auto& profile:profiles) if(profile.matches(device)) result.append({device,profile.id});
    return result;
}
std::unique_ptr<IHidDevice> openAuxiliaryTransport(const DeviceIdentity& identity)
{
    auto transport=std::make_unique<AuxiliaryTransport>(identity);
    if(!transport->isOpen()) throw std::runtime_error("Auxiliary lighting endpoint unavailable or identity handshake rejected");
    return transport;
}
