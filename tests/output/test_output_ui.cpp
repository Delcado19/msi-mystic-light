#include "output/HardwareOutput.h"
#include "protocol/MsiProtocol.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QDir>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTimer>
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
    void reviewReports();
    void previewAndEditStop();
    void cancelAndChangedReview();
    void unplugAndRestart();
};
void OutputUiTest::reviewReports()
{
    LightingDocument document({},nullptr,{profile()}); configure(document);
    OutputProof proof; proof.identity=identity(); proof.profile=profile(); proof.initial=prepareOutput(document,document.render())[0];
    ModuleRegistry modules; QString error; QVERIFY(modules.load(TEST_MODULE_PATH,&error));
    proof.reports=previewOutputReports(modules,proof);
    QCOMPARE(proof.reports.size(),2); QCOMPARE(proof.reports[0].size(),290); QCOMPARE(proof.reports[1].size(),727);
    bool sawReports=false, disabled=false;
    QTimer::singleShot(0,[&] {
        auto* dialog=QApplication::activeModalWidget();
        auto* buttons=dialog->findChild<QDialogButtonBox*>(); disabled=!buttons->button(QDialogButtonBox::Ok)->isEnabled();
        const auto text=dialog->findChild<QPlainTextEdit*>("outputReports")->toPlainText();
        sawReports=text.contains(identity().path) && text.contains(QString::fromLatin1(proof.reports[1].toHex(' ')));
        dialog->findChild<QCheckBox*>("approveOutput")->setChecked(true);
        QVERIFY(dialog->grab().save(QString(TEST_ARTIFACT_DIR)+"/output-confirmation.png"));
        buttons->button(QDialogButtonBox::Ok)->click();
    });
    QVERIFY(confirmOutputReports(nullptr,{proof})); QVERIFY(disabled); QVERIFY(sawReports);
    QTimer::singleShot(0,[] { QApplication::activeModalWidget()->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Cancel)->click(); });
    QVERIFY(!confirmOutputReports(nullptr,{proof}));
}
void OutputUiTest::previewAndEditStop()
{
    LightingWindow window({},[] { return QVector<ProfileCandidate>{}; },nullptr,{profile()}); configure(window.document());
    auto record=std::make_shared<Record>(); int authorizations=0;
    OutputPanel panel(window,[&](QWidget*,const QVector<OutputFrame>& frames) {
        ++authorizations; return QVector<OutputConnection>{connection(frames[0],record)};
    });
    auto* start=panel.findChild<QPushButton*>("startOutput"); auto* stop=panel.findChild<QPushButton*>("stopOutput");
    start->click(); QTRY_COMPARE(record->writes(),2); QTRY_VERIFY(state(panel,"Running"));
    QCOMPARE(authorizations,1);
    window.findChild<QDoubleSpinBox*>("previewTime")->setValue(1);
    QTRY_COMPARE(record->writes(),3);
    const auto revision=window.document().revision();
    window.document().edit("Blue",[](LightingState& data) { data.layers[0].color=Qt::blue; });
    QVERIFY(window.document().revision()>revision); QTRY_VERIFY(state(panel,"Stopped")); QCOMPARE(record->writes(),3);
    {
        const std::lock_guard<std::mutex> lock(record->mutex);
        const auto red=BuildReport0x51(MsiGen1AdvArea::JARGB1,{{1,2,3}});
        QVERIFY(record->reports.back()==std::vector<std::uint8_t>(red.begin(),red.end())); QCOMPARE(record->closes,1);
    }
    QVERIFY(start->isEnabled()); QVERIFY(!stop->isEnabled());
    window.document().history()->undo(); QCOMPARE(record->writes(),3); // Undo never resumes confirmed output.
    window.document().history()->setClean();
}
void OutputUiTest::cancelAndChangedReview()
{
    LightingWindow window({},[] { return QVector<ProfileCandidate>{}; },nullptr,{profile()}); configure(window.document());
    OutputPanel cancelled(window,[](QWidget*,const QVector<OutputFrame>&) { return QVector<OutputConnection>{}; });
    cancelled.findChild<QPushButton*>("startOutput")->click();
    QVERIFY(cancelled.findChild<QLabel*>("outputStatus")->text().contains("cancelled"));
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
    LightingWindow window({},[] { return QVector<ProfileCandidate>{}; },nullptr,{profile()}); configure(window.document());
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
