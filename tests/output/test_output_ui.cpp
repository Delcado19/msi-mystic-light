#include <QDialog>
#include "output/HardwareOutput.h"
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
    return {std::move(frame),[record] {
        ModuleRegistry modules; QString error;
        if(!modules.load(TEST_MODULE_PATH,&error)) throw std::runtime_error(error.toStdString());
        auto session=std::make_unique<OutputSession>();
        if(!session->connect(modules,identity(),profile(),std::make_unique<Transport>(record))) throw std::runtime_error(session->error().toStdString());
        return session;
    },std::move(present)};
}
bool state(OutputPanel& panel,const QString& expected)
{
    const auto* tree=panel.findChild<QTreeWidget*>("outputEndpoints");
    return tree->topLevelItemCount()==1 && tree->topLevelItem(0)->text(1)==expected;
}
}
class OutputUiTest : public QObject {
    Q_OBJECT
private slots:
    void initTestCase() { QVERIFY(QDir().mkpath(TEST_ARTIFACT_DIR)); }
    void applicationDevicePage();
    void scopeGuard();
    void pausedGradientPlayback();
    void automaticOutput();
    void reviewReports();
    void liveEditsAndConfigurationStop();
    void cancelAndChangedReview();
    void unplugAndRestart();
};
void OutputUiTest::applicationDevicePage()
{
    LightingWindow window({},[] { return QVector<ProfileCandidate>{}; },nullptr,{profile()}); configure(window.document()); window.findChild<QPushButton*>("playPreview")->setChecked(false);
    auto record=std::make_shared<Record>();
    auto* panel=new OutputPanel(window,[record](QWidget*,const QVector<OutputFrame>& frames) { return QVector<OutputConnection>{connection(frames[0],record)}; },&window);
    window.installOutputPanel(panel); QObject::connect(panel,&OutputPanel::outputStateChanged,&window,&LightingWindow::setOutputState);
    window.resize(1440,900); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window));
    window.findChild<QAction*>("reviewOutput")->trigger();
    QCOMPARE(window.findChild<QStackedWidget*>("applicationPages")->currentIndex(),0); QVERIFY(panel->isVisible());
    QCoreApplication::processEvents();
    QVERIFY(!window.findChild<QToolBar*>("previewToolbar")->isVisible());
    QVERIFY(!window.findChild<QDockWidget*>("hardwareOutputDock")); QVERIFY(!window.findChild<LightingCanvas*>("effectsCanvas")->isVisible());
    QVERIFY(window.grab().save(QString(TEST_ARTIFACT_DIR)+"/application-devices.png"));
    panel->findChild<QPushButton*>("startOutput")->click(); QTRY_VERIFY(state(*panel,"Running")); QTRY_COMPARE(record->writes(),2);
    window.findChild<QAction*>("workspaceLayout")->trigger();
    auto* layout=window.findChild<LightingCanvas*>("layoutCanvas"); QVERIFY(layout->isVisible());
    QCOMPARE(layout->grab().toImage().pixelColor(layout->viewportPoint({320,200}).toPoint()),QColor(1,2,3));
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
    QTRY_VERIFY(state(panel,"Running")); QTest::qWait(200);
    window.findChild<QPushButton*>("playPreview")->setChecked(false);
    bool firstChanged=false,secondChanged=false;
    {
        const std::lock_guard<std::mutex> lock(record->mutex);
        std::vector<std::vector<std::uint8_t>> frames;
        for(const auto& report:record->reports) if(report[0]==0x51) frames.push_back(report);
        QVERIFY(frames.size()>=4);
        firstChanged=frames[0]!=frames[frames.size()-2]; secondChanged=frames[1]!=frames.back();
    }
    QVERIFY(firstChanged); QVERIFY(secondChanged); panel.stopOutput(); window.document().history()->setClean();
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
    auto record=std::make_shared<Record>(); int authorizations=0;
    OutputPanel panel(window,[&](QWidget*,const QVector<OutputFrame>& frames) {
        ++authorizations; return QVector<OutputConnection>{connection(frames[0],record)};
    });
    QObject::connect(&panel,&OutputPanel::outputStateChanged,&window,&LightingWindow::setOutputState);
    auto* outputAction=window.findChild<QAction*>("reviewOutput"); QVERIFY(outputAction);
    auto* start=panel.findChild<QPushButton*>("startOutput"); auto* stop=panel.findChild<QPushButton*>("stopOutput");
    start->click(); QTRY_COMPARE(record->writes(),2); QTRY_VERIFY(state(panel,"Running"));
    QCOMPARE(authorizations,1); QTRY_COMPARE(outputAction->text(),QString("Output: Running"));
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
    QVERIFY(start->isEnabled()); QVERIFY(!stop->isEnabled()); QCOMPARE(outputAction->text(),QString("Output: Stopped"));
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
    auto present=std::make_shared<std::atomic_bool>(true); auto record=std::make_shared<Record>(); int reviews=0;
    OutputPanel panel(window,[&](QWidget*,const QVector<OutputFrame>& frames) {
        ++reviews; return QVector<OutputConnection>{connection(frames[0],record,[present] { return present->load(); })};
    });
    auto* start=panel.findChild<QPushButton*>("startOutput"); start->click(); QTRY_COMPARE(record->writes(),2);
    present->store(false); QTRY_VERIFY(state(panel,"Faulted")); QCOMPARE(reviews,1);
    QVERIFY(panel.findChild<QLabel*>("outputStatus")->text().contains("disappeared"));
    present->store(true); record=std::make_shared<Record>(); start->click(); QTRY_COMPARE(record->writes(),2);
    QCOMPARE(reviews,2); panel.findChild<QPushButton*>("stopOutput")->click(); QTRY_VERIFY(state(panel,"Stopped"));
    panel.resize(900,320); panel.show(); QVERIFY(QTest::qWaitForWindowExposed(&panel));
    QVERIFY(panel.grab().save(QString(TEST_ARTIFACT_DIR)+"/output-panel.png"));
    window.document().history()->setClean();
}
QTEST_MAIN(OutputUiTest)
#include "test_output_ui.moc"
