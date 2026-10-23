#include "HardwareOutput.h"
#include "startup/ConflictDialog.h"
#include "hid/HidApiDevice.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <stdexcept>

namespace {
class RecordingTransport : public IHidDevice {
public:
    explicit RecordingTransport(std::shared_ptr<QVector<QByteArray>> reports) : reports_(std::move(reports)) {}
    bool open(std::uint16_t,std::uint16_t) override { return false; }
    void close() override { open_=false; }
    bool isOpen() const override { return open_; }
    bool writeOutputReport(const std::uint8_t* bytes,std::size_t count) override { return sendFeatureReport(bytes,count); }
    bool sendFeatureReport(const std::uint8_t* bytes,std::size_t count) override {
        if(!open_) return false;
        reports_->append(QByteArray(reinterpret_cast<const char*>(bytes),qsizetype(count))); return true;
    }
private:
    bool open_=true;
    std::shared_ptr<QVector<QByteArray>> reports_;
};
bool sameEndpoint(const DeviceIdentity& a,const DeviceIdentity& b)
{
    return a.path==b.path && a.vendorId==b.vendorId && a.productId==b.productId &&
        a.usagePage==b.usagePage && a.usage==b.usage && a.interfaceNumber==b.interfaceNumber;
}
class PairedTransport final : public IHidDevice {
public:
    HidApiDevice control, leds;
    bool open(std::uint16_t,std::uint16_t) override { return false; }
    void close() override { control.close(); leds.close(); }
    bool isOpen() const override { return control.isOpen() && leds.isOpen(); }
    bool sendFeatureReport(const std::uint8_t* bytes,std::size_t count) override { return isOpen() && control.sendFeatureReport(bytes,count); }
    bool writeOutputReport(const std::uint8_t* bytes,std::size_t count) override { return isOpen() && leds.writeOutputReport(bytes,count); }
};
}
DeviceIdentity vulcanControlIdentity(const DeviceIdentity& led,const std::vector<HidDescriptor>& devices)
{
    DeviceIdentity result; int controls=0, ledEndpoints=0; bool selectedLed=false;
    for(const auto& device:devices) if(device.vendorId==led.vendorId && device.productId==led.productId) {
        if(device.interfaceNumber==3 && device.usagePage==1) { ++ledEndpoints; if(QString::fromUtf8(device.path.c_str())==led.path) selectedLed=true; }
        if(device.interfaceNumber==1 && device.usagePage==11) { result=deviceIdentity(device,{}); ++controls; }
    }
    // Never pair interfaces by enumeration order: multiple identical keyboards need explicit pairing support.
    if(controls!=1 || ledEndpoints!=1 || !selectedLed) throw std::runtime_error("Cannot uniquely pair Vulcan control and LED interfaces");
    return result;
}
namespace {
bool present(const OutputProof& proof)
{
    const auto devices=HidApiDevice::enumerate();
    if(!proof.controlIdentity.path.isEmpty()) {
        try { if(!sameEndpoint(vulcanControlIdentity(proof.identity,devices),proof.controlIdentity)) return false; }
        catch(const std::exception&) { return false; }
    }
    const auto candidates=findProfileCandidates(devices,readBoardIdentity(),{proof.profile});
    for(const auto& candidate:candidates) if(sameEndpoint(candidate.device,proof.identity)) return true;
    return false;
}
void loadModules(ModuleRegistry& modules,const QString& directory)
{
    const auto errors=modules.loadDirectory(directory);
    if(!errors.isEmpty()) throw std::runtime_error(errors.join("\n").toStdString());
}
}
QVector<QByteArray> previewOutputReports(const ModuleRegistry& modules,const OutputProof& proof)
{
    auto reports=std::make_shared<QVector<QByteArray>>(); OutputSession session;
    if(!session.connect(modules,proof.identity,proof.profile,std::make_unique<RecordingTransport>(reports)) ||
        !session.stage(proof.initial) || !session.flush()) throw std::runtime_error(session.error().toStdString());
    return *reports;
}
bool confirmOutputReports(QWidget* parent,const QVector<OutputProof>& proofs)
{
    QDialog dialog(parent); dialog.setObjectName("outputConfirmation"); dialog.setWindowTitle("Review hardware output"); dialog.resize(840,640);
    auto* layout=new QVBoxLayout(&dialog);
    auto* note=new QLabel("Review exact initial reports below. Software output may subsequently vary RGB payloads "
        "on the listed channels as effects are edited or preview time changes. MSI 0x50 reports include all 18 controller zone records; "
        "unassigned zones are not read back or preserved. Stop closes the connection and leaves the last device state.");
    note->setWordWrap(true); layout->addWidget(note);
    QString text;
    for(const auto& proof:proofs) {
        text+=QString("Endpoint: %1\nProfile: %2\nVID:PID: %3:%4; interface %5; usage %6/%7\n")
            .arg(proof.identity.path,proof.profile.name).arg(proof.identity.vendorId,4,16,QChar('0'))
            .arg(proof.identity.productId,4,16,QChar('0')).arg(proof.identity.interfaceNumber)
            .arg(proof.identity.usagePage).arg(proof.identity.usage);
        if(!proof.controlIdentity.path.isEmpty()) text+="Control endpoint (feature reports): "+proof.controlIdentity.path+"\nLED endpoint (output reports): "+proof.identity.path+"\n";
        if(proof.profile.driverId=="lighting.peripherals") {
            const auto protocol=proof.profile.channels[0].address.value("protocol").toString();
            text+=protocol=="vulcan" ? "Transfer order: control feature report, then seven LED output reports.\n" : protocol=="apex3" ? "Transfers: HID output reports (brightness, then colors).\n" : "Transfers: HID feature reports.\n";
        }
        for(const auto& channel:proof.initial.channels)
            text+=QString("Channel %1: %2, %3 LEDs\n").arg(channel.channelId,channel.software ? "Software" : "Native "+channel.native.id).arg(channel.colors.size());
        for(int index=0;index<proof.reports.size();++index)
            text+=QString("Report %1 (%2 bytes):\n%3\n").arg(index+1).arg(proof.reports[index].size()).arg(QString::fromLatin1(proof.reports[index].toHex(' ')));
        text+="\n";
    }
    auto* reports=new QPlainTextEdit(text); reports->setObjectName("outputReports"); reports->setReadOnly(true); layout->addWidget(reports);
    auto* consent=new QCheckBox("I approve these reports and continuous RGB updates on the listed software channels.");
    consent->setObjectName("approveOutput"); layout->addWidget(consent);
    auto* buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel); layout->addWidget(buttons);
    buttons->button(QDialogButtonBox::Ok)->setEnabled(false); buttons->button(QDialogButtonBox::Ok)->setText("Start output");
    buttons->button(QDialogButtonBox::Cancel)->setDefault(true);
    QObject::connect(consent,&QCheckBox::toggled,buttons->button(QDialogButtonBox::Ok),&QPushButton::setEnabled);
    QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);
    QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    return dialog.exec()==QDialog::Accepted;
}
QVector<OutputConnection> authorizeHardwareOutput(QWidget* parent,const QVector<DeviceProfile>& profiles,
    const QVector<OutputFrame>& frames,const QString& moduleDirectory)
{
    SystemConflictProcesses processes; ConflictDialog conflicts(processes,parent);
    if(conflicts.exec()!=QDialog::Accepted) return {};
    const auto devices=HidApiDevice::enumerate();
    const auto candidates=findProfileCandidates(devices,readBoardIdentity(),profiles);
    ModuleRegistry modules; loadModules(modules,moduleDirectory);
    QVector<OutputProof> proofs;
    for(const auto& frame:frames) {
        OutputProof proof; proof.initial=frame; proof.profile=assignmentProfile(profiles,frame.profileId);
        bool found=false;
        for(const auto& candidate:candidates) if(candidate.profileId==frame.profileId && candidate.device.path==frame.endpoint) {
            proof.identity=candidate.device; found=true; break;
        }
        if(!found) throw std::runtime_error("A configured endpoint is absent or no longer matches its profile");
        if(proof.profile.driverId=="lighting.peripherals" && proof.profile.channels[0].address.value("protocol")=="vulcan")
            proof.controlIdentity=vulcanControlIdentity(proof.identity,devices);
        proof.reports=previewOutputReports(modules,proof); proofs.append(proof);
    }
    if(!confirmOutputReports(parent,proofs)) return {};
    QVector<OutputConnection> result;
    for(const auto& proof:proofs) {
        result.append({proof.initial,[proof,moduleDirectory] {
            ModuleRegistry modules; loadModules(modules,moduleDirectory);
            // Recheck both serialization and live descriptors after the review, before opening hardware.
            if(previewOutputReports(modules,proof)!=proof.reports) throw std::runtime_error("Driver reports changed since confirmation");
            if(!present(proof)) throw std::runtime_error("Confirmed endpoint disappeared or changed identity");
            std::unique_ptr<IHidDevice> transport;
            if(!proof.controlIdentity.path.isEmpty()) {
                auto pair=std::make_unique<PairedTransport>();
                if(!pair->control.openPath(proof.controlIdentity.path.toUtf8().toStdString()) || !pair->leds.openPath(proof.identity.path.toUtf8().toStdString()))
                    throw std::runtime_error("Opening confirmed Vulcan interfaces failed");
                transport=std::move(pair);
            } else {
                auto single=std::make_unique<HidApiDevice>();
                if(!single->openPath(proof.identity.path.toUtf8().toStdString())) throw std::runtime_error("Opening confirmed HID path failed");
                transport=std::move(single);
            }
            auto session=std::make_unique<OutputSession>();
            if(!session->connect(modules,proof.identity,proof.profile,std::move(transport))) throw std::runtime_error(session->error().toStdString());
            return session;
        },[proof] { return present(proof); }});
    }
    return result;
}
