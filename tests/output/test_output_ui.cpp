#include <QDialog>
#include <QPainter>
#include "output/HardwareOutput.h"
#include "editor/SnapGrid.h"
#include "protocol/MsiProtocol.h"
#include <QCheckBox>
#include <QAction>
#include <QStackedWidget>
#include <QToolBar>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QDir>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTimer>
#include <QTemporaryDir>
#include <QTreeWidget>
#include <QtTest>
#include <atomic>
#include <mutex>

namespace {
struct Record {
    std::mutex mutex;
    std::vector<std::vector<std::uint8_t>> reports;
    int closes=0;
    int writes() { const std::lock_guard<std::mutex> lock(mutex); return int(reports.size()); }
};
class Transport : public IHidDevice {
public:
    explicit Transport(std::shared_ptr<Record> record): record_(std::move(record)) {}
    bool open(std::uint16_t,std::uint16_t) override { return false; }
    bool isOpen() const override { return open_; }
    void close() override { open_=false; const std::lock_guard<std::mutex> lock(record_->mutex); ++record_->closes; }
    bool sendFeatureReport(const std::uint8_t* bytes,std::size_t count) override {
        const std::lock_guard<std::mutex> lock(record_->mutex); record_->reports.emplace_back(bytes,bytes+count); return open_;
    }
private:
    bool open_=true;
    std::shared_ptr<Record> record_;
};
DeviceIdentity identity()
{
    DeviceIdentity result; result.path="fake-hid-path"; result.vendorId=0xdb0; result.productId=0x76;
    result.boardManufacturer="Micro-Star International Co., Ltd."; result.boardModel="MPG B850 EDGE TI WIFI (MS-7E62)"; return result;
}
DeviceProfile profile() { return DeviceProfile::load(TEST_PROFILE_PATH); }
void configure(LightingDocument& document)
{
    ComponentDefinition component; component.id="test"; component.name="Test LED";
    component.positions={{0,0}}; component.mapping={0}; document.defineComponent(component);
    const auto placement=document.addComponent(component.id); document.addLayer();
    document.edit("Red",[](LightingState& state) { state.layers[0].color=QColor(1,2,3); });
    ChannelAssignment assignment; assignment.endpoint=identity().path; assignment.profileId=profile().id;
    assignment.channelId="JARGB1"; assignment.software=true; assignment.placements={placement}; document.assignChannel(assignment);
}
OutputConnection connection(OutputFrame frame,std::shared_ptr<Record> record,std::function<bool()> present={})
{
    const auto endpoint=frame.endpoint;
    return {std::move(frame),[record,endpoint] {
        ModuleRegistry modules; QString error;
        if(!modules.load(TEST_MODULE_PATH,&error)) throw std::runtime_error(error.toStdString());
        auto session=std::make_unique<OutputSession>();
        auto device=identity(); device.path=endpoint;
        if(!session->connect(modules,device,profile(),std::make_unique<Transport>(record))) throw std::runtime_error(session->error().toStdString());
        return session;
    },std::move(present)};
}
bool state(OutputPanel& panel,const QString& expected)
{
    const auto* tree=panel.findChild<QTreeWidget*>("outputEndpoints");
    for(int i=0;i<tree->topLevelItemCount();++i)
        if(tree->topLevelItem(i)->data(0,Qt::UserRole).toString()==identity().path) return tree->topLevelItem(i)->text(2)==expected;
    return false;
}
}
class OutputUiTest : public QObject {
    Q_OBJECT
    QTemporaryDir settingsDirectory_;
private slots:
    void initTestCase() {
        QVERIFY(QDir().mkpath(TEST_ARTIFACT_DIR));QVERIFY(settingsDirectory_.isValid());
        QCoreApplication::setOrganizationName("LightingStudioOutputTests");
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,settingsDirectory_.path());
    }
    void init() { SnapGrid{false,10}.save(); } // Output color assertions exclude the optional editing-grid overlay.
    void applicationDevicePage();
    void layoutDragOutput();
    void headerControls();
    void auxiliaryInventory();
    void independentConnections();
    void scopeGuard();
    void pausedGradientPlayback();
    void automaticOutput();
    void reviewReports();
    void liveEditsAndConfigurationStop();
    void cancelAndChangedReview();
    void unplugAndRestart();
};
void OutputUiTest::layoutDragOutput()
{
    LightingWindow window({},[] { return QVector<ProfileCandidate>{}; },nullptr,{profile()}); configure(window.document());
    window.document().edit("Sampling test image",[](auto& state) {
        auto& layer=state.layers[0]; layer.effect=EffectKind::Image; layer.extent={640,400};
        layer.image=QImage(640,400,QImage::Format_ARGB32_Premultiplied); layer.image.fill(Qt::red);
        QPainter painter(&layer.image); painter.fillRect(200,0,440,400,Qt::blue);
    });
    window.findChild<QPushButton*>("playPreview")->setChecked(false);
    auto record=std::make_shared<Record>();
    OutputPanel panel(window,[record](QWidget*,const QVector<OutputFrame>& frames) { return QVector<OutputConnection>{connection(frames[0],record)}; });
    panel.enableAutomaticOutput(); QTRY_VERIFY(state(panel,"Running")); QTRY_COMPARE(record->writes(),2);
    window.resize(1440,900); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto* canvas=window.findChild<LightingCanvas*>("layoutCanvas");
    window.findChild<QAction*>("workspaceLayout")->trigger();
    const auto id=window.document().state().placements[0].id;
    canvas->select(window.document().state().assignments[0].id);
    const auto before=window.document().state().placements[0].origin; const auto history=window.document().history()->index();
    const auto from=canvas->viewportPoint(before).toPoint(), to=canvas->viewportPoint({300,80}).toPoint();
    QTest::mousePress(canvas,Qt::LeftButton,Qt::NoModifier,from);
    QCOMPARE(canvas->selection(),id);
    QTest::mouseMove(canvas,to);
    QTRY_VERIFY(record->writes()>2);
    QCOMPARE(window.document().state().placements[0].origin,before);
    QCOMPARE(window.document().history()->index(),history);
    const auto placements=canvas->previewPlacements();
    const auto colors=prepareOutput(window.document(),window.previewFrame(),0,&placements)[0].channels[0].colors;
    QCOMPARE(colors[0].b,quint8(255)); QCOMPARE(colors[0].r,quint8(0));
    const auto received=[record](const auto& expected) {
        const std::lock_guard<std::mutex> lock(record->mutex);
        return !record->reports.empty() && record->reports.back()==std::vector<std::uint8_t>(expected.begin(),expected.end());
    };
    const auto blue=BuildReport0x51(MsiGen1AdvArea::JARGB1,{{0,0,255}});
    // The scheduler may still be sending a previous red frame; wait for the actual blue packet, not just any write.
    QTRY_VERIFY(received(blue));
    {
        const std::lock_guard<std::mutex> lock(record->mutex);
        const auto& last=record->reports.back();
        const auto expected=BuildReport0x51(MsiGen1AdvArea::JARGB1,{{0,0,255}});
        QCOMPARE(last,std::vector<std::uint8_t>(expected.begin(),expected.end()));
    }
    QTest::keyClick(canvas,Qt::Key_Escape);
    const auto red=BuildReport0x51(MsiGen1AdvArea::JARGB1,{{255,0,0}});
    QTRY_VERIFY(received(red));
    QCOMPARE(canvas->previewPlacements()[0].origin,before);
    QTest::mousePress(canvas,Qt::LeftButton,Qt::NoModifier,from); QTest::mouseMove(canvas,to);
    QTest::mouseRelease(canvas,Qt::LeftButton,Qt::NoModifier,to);
    QCOMPARE(window.document().history()->index(),history+1);
    const auto origin=window.document().state().placements[0].origin;
    QVERIFY(QLineF(origin,QPointF(300,80)).length()<2);
    QCOMPARE(canvas->previewPlacements()[0].origin,origin);
    panel.stopOutput(); window.document().history()->setClean();
}
void OutputUiTest::headerControls()
{
    const ProfileCandidate detected{identity(),profile().id};
    LightingWindow window({},[detected] { return QVector<ProfileCandidate>{detected}; },nullptr,{profile()}); configure(window.document());
    auto assignment=window.document().state().assignments[0]; assignment.id=QUuid::createUuid(); assignment.channelId="JARGB2";
    assignment.placements={window.document().addComponent("test")}; window.document().assignChannel(assignment);
    window.findChild<QPushButton*>("playPreview")->setChecked(false);
    auto record=std::make_shared<Record>();
    OutputPanel panel(window,[record](QWidget*,const QVector<OutputFrame>& frames) { return QVector<OutputConnection>{connection(frames[0],record)}; });
    panel.enableAutomaticOutput(); QTRY_VERIFY(state(panel,"Running"));
    auto* tree=panel.findChild<QTreeWidget*>("outputEndpoints");
    QCOMPARE(tree->topLevelItemCount(),1); QCOMPARE(tree->topLevelItem(0)->childCount(),0);
    QCOMPARE(tree->topLevelItem(0)->text(0),identity().path);
    QCOMPARE(tree->topLevelItem(0)->text(1),profile().id);
    QVERIFY(tree->maximumHeight()>100);
    auto* start=panel.findChild<QPushButton*>("startOutput"); auto* stop=panel.findChild<QPushButton*>("stopOutput");
    QVERIFY(!start->isEnabled()); QVERIFY(!start->property("primary").toBool());
    auto* inventory=window.findChild<QTreeWidget*>("layoutComponents");
    inventory->setCurrentItem(inventory->topLevelItem(1));
    inventory->setCurrentItem(inventory->topLevelItem(0)); stop->click();
    QCOMPARE(tree->topLevelItem(0)->text(2),QString("1 active / 1 paused"));
    QVERIFY(panel.findChild<QLabel*>("outputChannelDetails")->text().contains("JARGB1: Paused"));
    QCOMPARE(tree->currentItem()->data(0,Qt::UserRole+2).toString(),QString("JARGB1"));
    QVERIFY(start->isEnabled()); QVERIFY(!stop->isEnabled());
    QTest::qWait(80); const int pausedWrites=record->writes();
    window.document().edit("Peer keeps running",[](auto& state) { state.layers[0].color=Qt::blue; });
    QTRY_VERIFY(record->writes()>pausedWrites);
    {
        const std::lock_guard<std::mutex> lock(record->mutex);
        const auto expected=BuildReport0x51(MsiGen1AdvArea::JARGB2,{{0,0,255}});
        for(int i=pausedWrites;i<record->reports.size();++i)
            QCOMPARE(record->reports[i],std::vector<std::uint8_t>(expected.begin(),expected.end()));
    }
    start->click(); QTRY_COMPARE(tree->topLevelItem(0)->text(2),QString("Running"));
    QVERIFY(!start->isEnabled());
    inventory->setCurrentItem(inventory->topLevelItem(2));
    QVERIFY(!start->isEnabled()); QVERIFY(!stop->isEnabled());
    auto* member=inventory->topLevelItem(1)->child(0); QVERIFY(member); inventory->setCurrentItem(member);
    QCOMPARE(tree->currentItem()->data(0,Qt::UserRole+2).toString(),QString("JARGB2"));
    QVERIFY(!start->isEnabled()); QVERIFY(stop->isEnabled());
    panel.stopOutput(); window.document().history()->setClean();
}

void OutputUiTest::auxiliaryInventory()
{
    const QDir directory=QFileInfo(TEST_PROFILE_PATH).absoluteDir();
    QVector<DeviceProfile> profiles{profile()};
    ComponentCatalog components;
    for(const auto& id:QStringList{"gpu.aero5080","msi.ioshield","ram.xpg"}) {
        const auto component=loadComponentDefinition(directory.filePath("../components/"+id+".json")); components.insert(component.id,component);
    }
    QVector<ProfileCandidate> candidates{{identity(),profile().id}};
    for(const auto& file:QStringList{"gigabyte-aero-5080","xpg-ram-71","xpg-ram-73"}) {
        const auto item=DeviceProfile::load(directory.filePath(file+".json")); profiles.append(item);
        DeviceIdentity device;device.path=item.match["endpoint"].toString();device.transport=item.match["transport"].toString();
        device.vendorId=item.match["vendorId"].toInt();device.productId=item.match["productId"].toInt();candidates.append({device,item.id});
    }
    LightingWindow offline(components,[] { return QVector<ProfileCandidate>{}; },nullptr,profiles);
    OutputPanel panel(offline,{});
    auto* table=panel.findChild<QTreeWidget*>("outputEndpoints"); QCOMPARE(table->topLevelItemCount(),3);
    for(int i=0;i<3;++i) { QCOMPARE(table->topLevelItem(i)->childCount(),0); QCOMPARE(table->topLevelItem(i)->text(2),QString("Not detected")); }
    LightingWindow detected(components,[candidates] { return candidates; },nullptr,profiles);
    detected.document().edit("Older embedded catalog",[](auto& state) { state.components.clear(); });
    QTRY_COMPARE(detected.document().state().assignments.size(),4);
    QSet<QString> channels;
    for(const auto& assignment:detected.document().state().assignments) { QVERIFY(assignment.software);channels.insert(assignment.channelId); }
    QCOMPARE(channels,QSet<QString>({"IOShield","GigabyteAero5080","XpgRam1","XpgRam2"}));
    for (const auto& placement : detected.document().state().placements) if (placement.componentId == "ram.xpg") {
        const auto points = detected.document().componentPoints(placement);
        QCOMPARE(points.size(), 8);
        QVERIFY(qAbs(points.last().x() - points.first().x() - 126.0) < .001);
    }
    QTemporaryDir projects;
    const auto project = projects.filePath("ram.json");
    detected.document().edit("Legacy RAM geometry", [](auto& state) {
        for (int i = 0; i < 8; ++i) state.components["ram.xpg"].positions[i] = QPointF(i * 18, 0);
    });
    detected.document().save(project);
    LightingDocument restored(components, nullptr, profiles); restored.load(project);
    QCOMPARE(restored.catalog()["ram.xpg"].positions, components["ram.xpg"].positions);
    QCOMPARE(restored.state().placements.first().origin, detected.document().state().placements.first().origin);
    detected.document().edit("Custom RAM geometry", [](auto& state) { state.components["ram.xpg"].positions[1].setY(1); });
    detected.document().save(project); restored.load(project);
    QCOMPARE(restored.catalog()["ram.xpg"].positions, detected.document().catalog()["ram.xpg"].positions);
    detected.document().history()->setClean();offline.document().history()->setClean();
}

void OutputUiTest::independentConnections()
{
    LightingWindow window({},[] { return QVector<ProfileCandidate>{}; },nullptr,{profile()}); configure(window.document());
    auto second=window.document().state().assignments[0]; second.id=QUuid::createUuid(); second.endpoint="second-endpoint";
    second.placements={window.document().addComponent("test")}; window.document().assignChannel(second);
    window.findChild<QPushButton*>("playPreview")->setChecked(false);
    auto firstRecord=std::make_shared<Record>(), secondRecord=std::make_shared<Record>();
    auto present=std::make_shared<std::atomic_bool>(true); bool rejectResume=false;
    OutputPanel panel(window,[&](QWidget*,const QVector<OutputFrame>& frames) {
        QVector<OutputConnection> result;
        for(const auto& frame:frames) {
            if(frame.endpoint==identity().path) {
                if(rejectResume) throw std::runtime_error("Test reconnection rejected");
                result.append(connection(frame,firstRecord,[present] { return present->load(); }));
            } else result.append(connection(frame,secondRecord));
        }
        return result;
    });
    panel.enableAutomaticOutput(); QTRY_COMPARE(firstRecord->writes(),2); QTRY_COMPARE(secondRecord->writes(),2);
    present->store(false); QTRY_VERIFY(state(panel,"Faulted"));
    rejectResume=true; panel.findChild<QPushButton*>("startOutput")->click();
    QVERIFY(panel.findChild<QLabel*>("outputStatus")->text().contains("rejected"));
    const int before=secondRecord->writes();
    window.document().edit("Healthy peer stays connected",[](auto& state) { state.layers[0].color=Qt::blue; });
    QTRY_VERIFY(secondRecord->writes()>before);
    QCOMPARE(panel.findChild<QTreeWidget*>("outputEndpoints")->topLevelItem(1)->text(2),QString("Running"));
    panel.stopOutput(); window.document().history()->setClean();
}

void OutputUiTest::applicationDevicePage()
{
    LightingWindow window({},[] { return QVector<ProfileCandidate>{}; },nullptr,{profile()}); configure(window.document()); window.findChild<QPushButton*>("playPreview")->setChecked(false);
    auto record=std::make_shared<Record>();
    auto* panel=new OutputPanel(window,[record](QWidget*,const QVector<OutputFrame>& frames) { return QVector<OutputConnection>{connection(frames[0],record)}; },&window);
    window.installOutputPanel(panel); QObject::connect(panel,&OutputPanel::outputStateChanged,&window,&LightingWindow::setOutputState);
    window.resize(1440,900); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window));
    QVERIFY(!window.findChild<QAction*>("reviewOutput")->isEnabled());
    QVERIFY(window.findChild<QLabel*>("toolbarOutputStatus"));
    window.findChild<QAction*>("workspaceHardware")->trigger();
    QCOMPARE(window.findChild<QStackedWidget*>("applicationPages")->currentIndex(),0); QVERIFY(panel->isVisible());
    QCoreApplication::processEvents();
    QVERIFY(!window.findChild<QToolBar*>("previewToolbar")->isVisible());
    QVERIFY(!window.findChild<QDockWidget*>("hardwareOutputDock")); QVERIFY(!window.findChild<LightingCanvas*>("effectsCanvas")->isVisible());
    QVERIFY(window.grab().save(QString(TEST_ARTIFACT_DIR)+"/application-devices.png"));
    panel->findChild<QPushButton*>("startOutput")->click(); QTRY_VERIFY(state(*panel,"Running")); QTRY_COMPARE(record->writes(),2);
    window.findChild<QAction*>("workspaceLayout")->trigger();
    auto* layout=window.findChild<LightingCanvas*>("layoutCanvas"); QVERIFY(layout->isVisible());
    QCOMPARE(layout->grab().toImage().pixelColor(layout->viewportPoint({320,200}).toPoint()),QColor(1,2,3));
    window.findChild<QAction*>("snapToGrid")->setChecked(true);
    QCOMPARE(window.previewFrame().pixelColor(320,200),QColor(1,2,3)); // Grid is UI-only and must never reach hardware colors.
    window.findChild<QAction*>("workspaceEffects")->trigger(); QVERIFY(window.findChild<LightingCanvas*>("effectsCanvas")->isVisible());
    panel->stopOutput(); QTRY_VERIFY(state(*panel,"Stopped")); window.document().history()->setClean();
}
void OutputUiTest::automaticOutput()
{
    LightingWindow window({},[] { return QVector<ProfileCandidate>{}; },nullptr,{profile()}); configure(window.document());
    window.findChild<QPushButton*>("playPreview")->setChecked(false);
    auto record=std::make_shared<Record>(); int connections=0;
    OutputPanel panel(window,[&](QWidget*,const QVector<OutputFrame>& frames) { ++connections; return QVector<OutputConnection>{connection(frames[0],record)}; });
    panel.enableAutomaticOutput(); QTRY_VERIFY(state(panel,"Running")); QTRY_COMPARE(record->writes(),2);
    auto assignment=window.document().state().assignments[0]; assignment.native.brightness=3;
    assignment.software=false; window.document().assignChannel(assignment);
    QTRY_VERIFY(connections>=2); QTRY_VERIFY(state(panel,"Running"));
    auto offline=assignment; offline.id=QUuid::createUuid(); offline.endpoint="offline"; offline.placements.clear();
    window.document().assignChannel(offline); QTRY_VERIFY(connections>=3); QTRY_VERIFY(state(panel,"Running"));
    const auto before=record->writes(); window.document().edit("Live color",[](auto& state) { state.layers[0].color=Qt::green; });
    QTest::qWait(50); QCOMPARE(connections,3); // Missing saved endpoints do not cause a reconnect loop.
    window.document().history()->undo(); // Queue a configuration reconnect, then stop before it runs.
    panel.stopOutput(); const auto stopped=record->writes();
    window.document().history()->undo(); QTest::qWait(50); QCOMPARE(record->writes(),stopped); QVERIFY(stopped>=before);
    window.document().history()->setClean();
}

void OutputUiTest::pausedGradientPlayback()
{
    LightingWindow window({},[] { return QVector<ProfileCandidate>{}; },nullptr,{profile()}); configure(window.document());
    const auto secondPlacement=window.document().addComponent("test");
    window.document().edit("Travelling gradient",[&](auto& state) {
        auto& source=state.layers[0]; source.effect=EffectKind::LinearGradient;
        source.color=Qt::red; source.secondaryColor=Qt::blue; source.driftX=114; source.rotation=-30;
        auto second=state.assignments[0]; second.id=QUuid::createUuid(); second.channelId="JARGB2"; second.placements={secondPlacement};
        state.assignments.append(second);
    });
    window.findChild<QPushButton*>("playPreview")->setChecked(false);
    auto record=std::make_shared<Record>();
    OutputPanel panel(window,[record](QWidget*,const QVector<OutputFrame>& frames) { return QVector<OutputConnection>{connection(frames[0],record)}; });
    panel.findChild<QPushButton*>("startOutput")->click();
    QVERIFY(window.findChild<QPushButton*>("playPreview")->isChecked());
    QTRY_VERIFY(state(panel,"Running"));
    const auto bothMoving=[&] {
        const std::lock_guard<std::mutex> lock(record->mutex);
        QMap<int,std::vector<std::uint8_t>> first;
        QSet<int> changed;
        for(const auto& report:record->reports) if(report[0]==0x51) {
            const int area=(report[2]<<8)|report[3];
            if(!first.contains(area)) first.insert(area,report);
            else if(first[area]!=report) changed.insert(area);
        }
        return changed.contains(0x0400) && changed.contains(0x0401);
    };
    // Wait for observed changes, not a fixed delay that races Debug rendering and worker scheduling.
    QTRY_VERIFY_WITH_TIMEOUT(bothMoving(),5000);
    window.findChild<QPushButton*>("playPreview")->setChecked(false);
    panel.stopOutput(); window.document().history()->setClean();
}

void OutputUiTest::scopeGuard()
{
    LightingDocument document({},nullptr,{profile()}); configure(document);
    const auto approved=prepareOutput(document,document.render())[0]; auto candidate=approved;
    candidate.channels[0].colors[0]={200,100,50}; QVERIFY(sameOutputScope(approved,candidate));
    candidate.endpoint="other"; QVERIFY(!sameOutputScope(approved,candidate)); candidate=approved;
    candidate.profileId="other"; QVERIFY(!sameOutputScope(approved,candidate)); candidate=approved;
    candidate.channels[0].channelId="JARGB2"; QVERIFY(!sameOutputScope(approved,candidate)); candidate=approved;
    candidate.channels[0].colors.append(RgbColor{}); QVERIFY(!sameOutputScope(approved,candidate)); candidate=approved;
    candidate.channels.clear(); QVERIFY(!sameOutputScope(approved,candidate));
    auto native=approved; native.channels[0].software=false; native.channels[0].colors.clear();
    QVERIFY(!sameOutputScope(approved,native)); candidate=native;
    candidate.channels[0].native.brightness=1; QVERIFY(!sameOutputScope(native,candidate)); candidate=native;
    candidate.channels[0].native.colors[0]={1,2,3}; QVERIFY(!sameOutputScope(native,candidate));
}
void OutputUiTest::reviewReports()
{
    LightingDocument document({},nullptr,{profile()}); configure(document);
    OutputProof proof; proof.identity=identity(); proof.profile=profile(); proof.initial=prepareOutput(document,document.render())[0];
    ModuleRegistry modules; QString error; QVERIFY(modules.load(TEST_MODULE_PATH,&error));
    proof.reports=previewOutputReports(modules,proof);
    QCOMPARE(proof.reports.size(),2); QCOMPARE(proof.reports[0].size(),290); QCOMPARE(proof.reports[1].size(),727);
    QVERIFY(!proof.reports[1].isEmpty());

}
void OutputUiTest::liveEditsAndConfigurationStop()
{
    LightingWindow window({},[] { return QVector<ProfileCandidate>{}; },nullptr,{profile()}); configure(window.document()); window.findChild<QPushButton*>("playPreview")->setChecked(false);
    // Drain initial queued discovery before counting explicit frame submissions.
    // Otherwise its refresh can legitimately submit another frame after a slow first flush.
    QCoreApplication::processEvents();
    auto record=std::make_shared<Record>(); int authorizations=0;
    OutputPanel panel(window,[&](QWidget*,const QVector<OutputFrame>& frames) {
        ++authorizations; return QVector<OutputConnection>{connection(frames[0],record)};
    });
    QObject::connect(&panel,&OutputPanel::outputStateChanged,&window,&LightingWindow::setOutputState);
    auto* outputAction=window.findChild<QAction*>("reviewOutput"); QVERIFY(outputAction);
    auto* start=panel.findChild<QPushButton*>("startOutput"); auto* stop=panel.findChild<QPushButton*>("stopOutput");
    start->click(); QTRY_COMPARE(record->writes(),2); QTRY_VERIFY(state(panel,"Running"));
    QCOMPARE(authorizations,1); QTRY_COMPARE(outputAction->text(),QString("Output: 1 active"));
    window.findChild<QDoubleSpinBox*>("previewTime")->setValue(1);
    QTRY_COMPARE(record->writes(),3);
    window.document().edit("Blue",[](LightingState& data) { data.layers[0].color=Qt::blue; });
    QTRY_COMPARE(record->writes(),4); QVERIFY(state(panel,"Running")); QCOMPARE(authorizations,1);
    {
        const std::lock_guard<std::mutex> lock(record->mutex);
        const auto blue=BuildReport0x51(MsiGen1AdvArea::JARGB1,{{0,0,255}});
        QVERIFY(record->reports.back()==std::vector<std::uint8_t>(blue.begin(),blue.end()));
    }
    window.document().history()->undo(); QTRY_COMPARE(record->writes(),5); QVERIFY(state(panel,"Running"));
    auto changed=window.document().state().assignments[0]; changed.software=false;
    window.document().assignChannel(changed); QTRY_VERIFY(state(panel,"Running")); QTRY_VERIFY(record->writes()>5);
    panel.stopOutput(); const int stoppedWrites=record->writes();
    QVERIFY(start->isEnabled()); QVERIFY(!stop->isEnabled()); QCOMPARE(outputAction->text(),QString("Output: 0 active / 1 stopped"));
    window.document().history()->undo(); QTest::qWait(50); QCOMPARE(record->writes(),stoppedWrites); // Explicit Stop survives subsequent document changes.
    start->click(); QTRY_VERIFY(record->writes()>stoppedWrites); QVERIFY(authorizations>=3);
    QTemporaryDir dir; const auto file=dir.filePath("project.lighting.json"); window.document().save(file);
    window.loadProject(file); QTRY_VERIFY(state(panel,"Running"));
    window.document().history()->setClean();
}
void OutputUiTest::cancelAndChangedReview()
{
    LightingWindow window({},[] { return QVector<ProfileCandidate>{}; },nullptr,{profile()}); configure(window.document()); window.findChild<QPushButton*>("playPreview")->setChecked(false);
    OutputPanel cancelled(window,[](QWidget*,const QVector<OutputFrame>&) { return QVector<OutputConnection>{}; });
    cancelled.findChild<QPushButton*>("startOutput")->click();
    QVERIFY(cancelled.findChild<QLabel*>("outputStatus")->text().contains("Waiting"));
    auto called=std::make_shared<std::atomic_int>(0);
    OutputPanel stale(window,[&](QWidget*,const QVector<OutputFrame>& frames) {
        window.document().addLayer();
        OutputConnection request; request.initial=frames[0]; request.factory=[called] {
            ++*called; return std::make_unique<OutputSession>();
        };
        return QVector<OutputConnection>{request};
    });
    stale.findChild<QPushButton*>("startOutput")->click();
    QCOMPARE(called->load(),0); QVERIFY(stale.findChild<QLabel*>("outputStatus")->text().contains("changed"));
    window.document().history()->setClean();
}
void OutputUiTest::unplugAndRestart()
{
    LightingWindow window({},[] { return QVector<ProfileCandidate>{}; },nullptr,{profile()}); configure(window.document()); window.findChild<QPushButton*>("playPreview")->setChecked(false);
    // Finish queued startup discovery before asserting the initial connection report count.
    QCoreApplication::processEvents();
    auto present=std::make_shared<std::atomic_bool>(true); auto record=std::make_shared<Record>(); int reviews=0;
    OutputPanel panel(window,[&](QWidget*,const QVector<OutputFrame>& frames) {
        ++reviews; return QVector<OutputConnection>{connection(frames[0],record,[present] { return present->load(); })};
    });
    auto* start=panel.findChild<QPushButton*>("startOutput"); start->click(); QTRY_COMPARE(record->writes(),2);
    QSignalSpy summaries(&panel,&OutputPanel::outputStateChanged);
    present->store(false); QTRY_VERIFY(state(panel,"Faulted")); QCOMPARE(reviews,1);
    QVERIFY(!summaries.isEmpty());QVERIFY(summaries.last()[0].toString().contains("1 errors"));
    QVERIFY(panel.findChild<QLabel*>("outputChannelDetails")->text().contains("Faulted"));
    QVERIFY(panel.findChild<QLabel*>("outputStatus")->text().contains("disappeared"));
    present->store(true); record=std::make_shared<Record>(); start->click(); QTRY_COMPARE(record->writes(),2);
    QCOMPARE(reviews,2); panel.findChild<QPushButton*>("stopOutput")->click(); QTRY_VERIFY(state(panel,"Stopped"));
    panel.resize(900,320); panel.show(); QVERIFY(QTest::qWaitForWindowExposed(&panel));
    QVERIFY(panel.grab().save(QString(TEST_ARTIFACT_DIR)+"/output-panel.png"));
    window.document().history()->setClean();
}
QTEST_MAIN(OutputUiTest)
#include "test_output_ui.moc"
