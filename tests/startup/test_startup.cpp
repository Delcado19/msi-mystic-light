#include "startup/ConflictDialog.h"
#include <QtTest>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QFontDatabase>
#include <stdexcept>

class FakeProcesses final : public IConflictProcesses {
public:
    QVector<ConflictProcess> running{{11, "SignalRgb.exe", "C:/rgb/SignalRgb.exe", "Potential writer", 100, false},
        {22, "OpenRGB.exe", "C:/rgb/OpenRGB.exe", "Potential writer", 200, false}};
    QVector<quint32> graceful, forced;
    bool denied = false, failScan = false, restart = false;
    int exitScanDelay = 0;
    QMap<quint32,int> pendingExits;
    ConflictService adminService{"SignalRgb.Service",42,true,ConflictService::Running,true,"\"C:/rgb/SignalRgbService.exe\""};
    bool adminSupported=false,adminRestarts=false,adminNeverStops=false,clearOnAdminError=false,adminActive=false,failScanOnAdminRequest=false;
    QString adminQueryError,adminRequestError,adminPollError;
    int adminRequests=0,adminPendingPolls=2;
    QString querySignalRgbService(ConflictService& out) override {
        out=adminService;return adminSupported?adminQueryError:QString("Service query unavailable");
    }
    QString requestSignalRgbAdminStop(quint32 pid) override {
        if(adminActive)return "An administrator service-stop request is already active";
        ++adminRequests;
        if(pid!=adminService.pid)return "Service identity changed";
        if(clearOnAdminError)running.clear();
        if(failScanOnAdminRequest)failScan=true;
        adminActive=adminRequestError.isEmpty();
        return adminRequestError;
    }
    QString pollSignalRgbAdminStop(bool& pending) override {
        pending=adminPendingPolls-- > 0;
        if(!pending)adminActive=false;
        if(pending)return {}; // Match the native contract: a live controller has no exit result yet.
        if(!adminPollError.isEmpty() || adminNeverStops)return adminPollError;
        if(adminRestarts) {
            adminService.pid=142;running={{142,"SignalRgbService.exe",{},"service",0,false}};
        } else {adminService.state=ConflictService::Stopped;adminService.pid=0;running.clear();}
        return {};
    }
    QVector<ConflictProcess> scan() override {
        if (failScan) throw std::runtime_error("Access denied");
        for(auto it=pendingExits.begin();it!=pendingExits.end();) {
            if(it.value()-- > 0){++it;continue;}
            const auto pid=it.key();running.removeIf([=](const auto& process){return process.pid==pid;});it=pendingExits.erase(it);
        }
        return running;
    }
    QString stop(const ConflictProcess& process, bool force) override {
        if (denied) return "Access denied";
        if (!force) { graceful.append(process.pid); return {}; }
        forced.append(process.pid);
        if(exitScanDelay){pendingExits[process.pid]=exitScanDelay;return {};}
        for (int i = 0; i < running.size(); ++i) {
            if (running[i].pid == process.pid) {
                running.removeAt(i);
                if (restart) {
                    auto replacement = process; replacement.pid += 100; replacement.started += 1;
                    running.append(replacement);
                }
                break;
            }
        }
        return {};
    }
};

class FakeServices final : public IConflictServices {
public:
    QVector<ConflictService> entries{{"discovered.name", 42, true, ConflictService::Running, true}};
    QString error;
    int stops = 0;
    QString enumerate(QVector<ConflictService>& out) override { out = entries; return error; }
    QString query(const QString&, ConflictService& out) override { out = entries.first(); return error; }
    QString requestStop(const QString& name) override {
        if (name != "discovered.name") return "wrong service";
        ++stops; return error;
    }
};

class StartupTest : public QObject {
    Q_OBJECT
    QPushButton* button(ConflictDialog& dialog, const char* name) {
        return dialog.findChild<QPushButton*>(name);
    }
private slots:
    void initTestCase() {
#ifdef Q_OS_WIN
        // The offscreen plugin does not populate the native Windows font database.
        QFontDatabase::addApplicationFont(qEnvironmentVariable("WINDIR") + "/Fonts/segoeui.ttf");
        QApplication::setFont(QFont("Segoe UI", 9));
#endif
    }
    void exactNamesAndGuards() {
        QVERIFY(!conflictReason("C:/rgb/SignalRgb.EXE").isEmpty());
        QVERIFY(!conflictReason("LEDKeeper2.exe").isEmpty());
        QVERIFY(conflictReason("SomeMSIUpdater.exe").isEmpty());
        QVERIFY(conflictReason("signalrgblauncher.exe").isEmpty());
        QVERIFY(requiresManualStop("LightingService.exe"));
        QVERIFY(!conflictReason("SignalRgbService.exe").isEmpty());QVERIFY(!requiresManualStop("SignalRgbService.exe"));
        QVERIFY(conflictReason("SignalRgbServiceUpdater.exe").isEmpty());
        SystemConflictProcesses system;
        QVERIFY(!system.stop({}, true).isEmpty());
        ConflictProcess invalid{123, "SignalRgb.exe", {}, "", 0, false};
        QVERIFY(!system.stop(invalid, true).isEmpty());
        invalid.executable = "LEDKeeper2.exe";
        QVERIFY(system.stop(invalid, true).contains("manually"));
    }
    void servicePolicy() {
        ConflictProcess p{42, "SignalRgbService.exe", "C:/rgb/SignalRgbService.exe", "", 10, false};
        FakeServices scm;
        QVERIFY(stopSignalRgbService(p, false, scm).isEmpty()); QCOMPARE(scm.stops, 1);
        QVERIFY(!stopSignalRgbService(p, true, scm).isEmpty()); QCOMPARE(scm.stops, 1);
        scm.entries[0].state = ConflictService::StopPending;
        QVERIFY(stopSignalRgbService(p, true, scm).isEmpty());
        scm.entries[0].state = ConflictService::Stopped;
        QVERIFY(stopSignalRgbService(p, true, scm).isEmpty());
        scm.entries[0].state = ConflictService::Running;
        scm.entries[0].acceptsStop = false;
        QVERIFY(stopSignalRgbService(p, false, scm).contains("accept stop"));
        scm.entries[0].ownProcess = false;
        QVERIFY(stopSignalRgbService(p, false, scm).contains("exclusive"));
        scm.entries[0].ownProcess = true;
        scm.entries.append(scm.entries[0]);
        QVERIFY(stopSignalRgbService(p, false, scm).contains("exclusive"));
        scm.entries.removeLast(); scm.entries[0].pid++;
        QVERIFY(!stopSignalRgbService(p, true, scm).isEmpty());
        scm.error = "Access denied";
        QCOMPARE(stopSignalRgbService(p, false, scm), scm.error);
        QCOMPARE(scm.stops, 1);
    }
    void administratorServiceIdentity_data() {
        QTest::addColumn<QString>("command");QTest::addColumn<bool>("allowed");
        QTest::newRow("quoted")<<QString("\"C:/rgb/SignalRgbService.exe\"")<<true;
        QTest::newRow("spaces-and-arguments")<<QString("\"C:/Program Files/rgb/SignalRgbService.exe\" --service")<<true;
        QTest::newRow("unquoted")<<QString("C:\\rgb\\SIGNALRGBSERVICE.EXE --service")<<true;
        QTest::newRow("custom-install-other-drive")<<QString("\"D:/Applications/My RGB Suite/SignalRgbService.exe\" --service")<<true;
        QTest::newRow("observed-install")<<QString("\"C:/ProgramData/WhirlwindFX/SignalRgb/service/SignalRgbService.exe\"")<<true;
        QTest::newRow("empty")<<QString()<<false;
        QTest::newRow("relative")<<QString("SignalRgbService.exe")<<false;
        QTest::newRow("wrong-name")<<QString("C:/rgb/SignalRgbService.exe.bak")<<false;
        QTest::newRow("unclosed-quote")<<QString("\"C:/rgb/SignalRgbService.exe")<<false;
        QTest::newRow("quote-concatenation")<<QString("\"C:/rgb/SignalRgbService.exe\"suffix")<<false;
        QTest::newRow("unquoted-spaces")<<QString("C:/Program Files/rgb/SignalRgbService.exe")<<false;
        QTest::newRow("unexpanded-variable")<<QString("%ProgramData%/rgb/SignalRgbService.exe")<<false;
        QTest::newRow("unc")<<QString("\\\\server\\rgb\\SignalRgbService.exe")<<false;
        QTest::newRow("device-prefix")<<QString("\\\\?\\C:\\rgb\\SignalRgbService.exe")<<false;
        QTest::newRow("nt-prefix")<<QString("\\??\\C:\\rgb\\SignalRgbService.exe")<<false;
        QTest::newRow("drive-relative")<<QString("C:SignalRgbService.exe")<<false;
        QTest::newRow("invalid-drive-letter")<<QString::fromUtf8("Ä:/rgb/SignalRgbService.exe")<<false;
        QTest::newRow("embedded-null")<<(QString("C:/rgb/")+QChar::Null+"SignalRgbService.exe")<<false;
    }
    void administratorServiceIdentity() {
        QFETCH(QString,command);QFETCH(bool,allowed);
        ConflictService service{"SignalRgb.Service",42,true,ConflictService::Running,true,command};
        // No process path or creation time is available for this protected service.
        const QVector<ConflictProcess> processes{{42,"SignalRgbService.exe",{},"service",0,false}};
        QCOMPARE(signalRgbAdminStopError(service,processes).isEmpty(),allowed);
    }
    void administratorServiceGuardsAndErrors() {
        FakeProcesses fixture;auto service=fixture.adminService;
        const QVector<ConflictProcess> processes{{42,"SignalRgbService.exe",{},"service",0,false}};
        QVERIFY(signalRgbAdminStopError(service,processes).isEmpty());
        auto changed=service;changed.name="Other.Service";QVERIFY(!signalRgbAdminStopError(changed,processes).isEmpty());
        changed=service;changed.pid=0;QVERIFY(!signalRgbAdminStopError(changed,processes).isEmpty());
        changed=service;changed.pid=43;QVERIFY(!signalRgbAdminStopError(changed,processes).isEmpty());
        changed=service;changed.ownProcess=false;QVERIFY(!signalRgbAdminStopError(changed,processes).isEmpty());
        changed=service;changed.acceptsStop=false;QVERIFY(!signalRgbAdminStopError(changed,processes).isEmpty());
        for(auto state:{ConflictService::Other,ConflictService::StopPending,ConflictService::Stopped}) {
            changed=service;changed.state=state;QVERIFY(!signalRgbAdminStopError(changed,processes).isEmpty());
        }
        auto client=processes;client.append({52,"SignalRgb.exe",{},"client",0,false});
        QVERIFY(signalRgbAdminStopError(service,client).contains("Close the SignalRGB application"));
        QVERIFY(!signalRgbAdminStopError(service,{}).isEmpty());
        auto withPath=processes;withPath[0].path="C:/other/SignalRgbService.exe";
        QVERIFY(signalRgbAdminStopError(service,withPath).contains("differs"));
        withPath[0].path="c:/RGB/SignalRgbService.exe";QVERIFY(signalRgbAdminStopError(service,withPath).isEmpty());
        QVERIFY(signalRgbAdminStopExitError(0).isEmpty());QVERIFY(signalRgbAdminStopExitError(1062).isEmpty());
        for(quint32 code:{5u,1223u,1051u,1052u,1061u,1060u,259u,0xffffffffu})
            QVERIFY(signalRgbAdminStopExitError(code).contains(QString::number(code)));
        SystemConflictProcesses native;QVERIFY(!native.requestSignalRgbAdminStop(0).isEmpty());
    }
    void administratorStopLifecycle() {
        FakeProcesses processes;processes.adminSupported=true;
        processes.running={{42,"SignalRgbService.exe",{},"service",0,false}};
        ConflictDialog dialog(processes);dialog.show();auto* stop=button(dialog,"stopSignalRgbServiceAdmin");
        QVERIFY(stop->isEnabled());stop->click();stop->click();QCOMPARE(processes.adminRequests,1);
        QVERIFY(!stop->isEnabled());QVERIFY(!button(dialog,"ignore")->isEnabled());
        QTest::qWait(250);QVERIFY(dialog.isVisible());
        QTRY_COMPARE_WITH_TIMEOUT(dialog.result(),int(QDialog::Accepted),2000);
        QVERIFY(processes.graceful.isEmpty());QVERIFY(processes.forced.isEmpty());
    }
    void administratorStopErrorsStayVisible() {
        for(const auto code:{1223u,5u,1051u,1060u}) {
            FakeProcesses processes;processes.adminSupported=true;processes.running={{42,"SignalRgbService.exe",{},"service",0,false}};
            processes.adminRequestError=signalRgbAdminStopExitError(code);processes.clearOnAdminError=true;
            ConflictDialog dialog(processes);dialog.show();button(dialog,"stopSignalRgbServiceAdmin")->click();QTest::qWait(20);
            QVERIFY(dialog.isVisible());QVERIFY(dialog.findChild<QLabel*>("conflictStatus")->text().contains(QString::number(code)));
            QVERIFY(button(dialog,"ignore")->isEnabled());QCOMPARE(processes.adminRequests,1);button(dialog,"cancel")->click();
        }
        FakeProcesses processes;processes.adminSupported=true;processes.running={{42,"SignalRgbService.exe",{},"service",0,false}};
        processes.adminPollError=signalRgbAdminStopExitError(5);
        ConflictDialog denied(processes);denied.show();button(denied,"stopSignalRgbServiceAdmin")->click();
        QTRY_VERIFY(denied.findChild<QLabel*>("conflictStatus")->text().contains("Windows error 5"));QVERIFY(denied.isVisible());
        button(denied,"cancel")->click();
        processes.adminPollError.clear();processes.adminRestarts=true;processes.adminPendingPolls=0;
        ConflictDialog restarted(processes);restarted.show();button(restarted,"stopSignalRgbServiceAdmin")->click();
        QTRY_VERIFY(restarted.findChild<QLabel*>("conflictStatus")->text().contains("restarted"));
        QVERIFY(restarted.isVisible());QCOMPARE(processes.adminRequests,2);button(restarted,"cancel")->click();
        FakeProcesses failed;failed.adminSupported=true;failed.running={{42,"SignalRgbService.exe",{},"service",0,false}};
        failed.adminRequestError="Administrator request denied";failed.failScanOnAdminRequest=true;
        ConflictDialog combined(failed);combined.show();button(combined,"stopSignalRgbServiceAdmin")->click();
        const auto message=combined.findChild<QLabel*>("conflictStatus")->text();
        QVERIFY(message.contains("scan failed"));QVERIFY(message.contains("request denied"));QVERIFY(combined.isVisible());
        QVERIFY(button(combined,"ignore")->isEnabled());button(combined,"cancel")->click();
    }
    void administratorStopQueryFailureAndCancellation() {
        FakeProcesses processes;processes.adminSupported=true;processes.running={{42,"SignalRgbService.exe",{},"service",0,false}};
        processes.adminQueryError="Service query failed (Windows error 1060)";
        ConflictDialog missing(processes);missing.show();QVERIFY(!button(missing,"stopSignalRgbServiceAdmin")->isEnabled());
        QVERIFY(missing.findChild<QLabel*>("conflictStatus")->text().contains("1060"));button(missing,"cancel")->click();
        processes.adminQueryError.clear();processes.adminNeverStops=true;
        ConflictDialog cancelled(processes);cancelled.show();button(cancelled,"stopSignalRgbServiceAdmin")->click();
        button(cancelled,"cancel")->click();const int remaining=processes.adminPendingPolls;QTest::qWait(250);
        QCOMPARE(processes.adminPendingPolls,remaining);QCOMPARE(processes.adminRequests,1);
        processes.adminActive=false; // A separate fresh fixture operation follows the cancelled UI.
        ConflictDialog failedQuery(processes);failedQuery.show();button(failedQuery,"stopSignalRgbServiceAdmin")->click();
        processes.adminQueryError="Service status unknown";
        QTRY_VERIFY(failedQuery.findChild<QLabel*>("conflictStatus")->text().contains("unknown"));
        QVERIFY(failedQuery.isVisible());button(failedQuery,"cancel")->click();
    }
    void administratorStopTimeoutIsVisible() {
        FakeProcesses processes;processes.adminSupported=true;processes.adminPendingPolls=1000;
        processes.running={{42,"SignalRgbService.exe",{},"service",0,false}};
        ConflictDialog dialog(processes);dialog.show();button(dialog,"stopSignalRgbServiceAdmin")->click();
        QTRY_VERIFY_WITH_TIMEOUT(dialog.findChild<QLabel*>("conflictStatus")->text().contains("timed out"),34000);
        QVERIFY(dialog.isVisible());QVERIFY(button(dialog,"ignore")->isEnabled());QCOMPARE(processes.adminRequests,1);
        button(dialog,"stopSignalRgbServiceAdmin")->click();QCOMPARE(processes.adminRequests,1);
        QVERIFY(dialog.findChild<QLabel*>("conflictStatus")->text().contains("already active"));
        button(dialog,"cancel")->click();
    }
    void serviceDialogPaths() {
        for (const auto* action : {"terminateSelected", "terminateAll"}) {
            FakeProcesses processes;
            processes.running = {{42, "SignalRgbService.exe", "C:/rgb/SignalRgbService.exe", "service", 10, false}};
            ConflictDialog dialog(processes); dialog.show();
            dialog.findChild<QTableWidget*>("conflictTable")->selectRow(0);
            button(dialog, action)->click();
            QCOMPARE(processes.graceful, (QVector<quint32>{42}));
            QTRY_COMPARE_WITH_TIMEOUT(dialog.result(), int(QDialog::Accepted), 2500);
            QVERIFY(!processes.forced.isEmpty());
        }
    }
    void identityGuard() {
        ConflictProcess process{11, "SignalRgb.exe", "C:/rgb/SignalRgb.exe", "", 100, false};
        QVERIFY(matchesProcessIdentity(process, 100, "C:/rgb/signalrgb.EXE"));
        QVERIFY(!matchesProcessIdentity(process, 101, process.path));
        QVERIFY(!matchesProcessIdentity(process, 100, "C:/other/SignalRgb.exe"));
        process.executable = "OpenRGB.exe";
        QVERIFY(!matchesProcessIdentity(process, 100, process.path));
        process.started = 0;
        QVERIFY(!matchesProcessIdentity(process, 0, process.path));
    }
    void nativeReadOnlyScan() {
#ifdef Q_OS_WIN
        SystemConflictProcesses system;
        for (const auto& process : system.scan()) {
            QVERIFY(process.pid != 0);
            QVERIFY(process.pid != QCoreApplication::applicationPid());
            QVERIFY(!conflictReason(process.executable).isEmpty());
        }
#endif
    }
    void ignoreCancelAndEmpty() {
        FakeProcesses processes;
        ConflictDialog dialog(processes);
        QVERIFY(processes.graceful.isEmpty()); QVERIFY(processes.forced.isEmpty());
        QVERIFY(!button(dialog, "terminateSelected")->isEnabled());
        button(dialog, "ignore")->click();
        QCOMPARE(dialog.result(), int(QDialog::Accepted));
        ConflictDialog cancelled(processes);
        button(cancelled, "cancel")->click();
        QCOMPARE(cancelled.result(), int(QDialog::Rejected));
        QVERIFY(processes.graceful.isEmpty());
        processes.running.clear();
        ConflictDialog empty(processes);
        QTimer::singleShot(1000, &empty, &QDialog::reject);
        QCOMPARE(empty.exec(), int(QDialog::Accepted));
    }
    void selectionAndAll() {
        FakeProcesses processes;
        ConflictDialog dialog(processes);
        dialog.show();
        auto* table = dialog.findChild<QTableWidget*>("conflictTable");
        QVERIFY(dialog.grab().save("startup-conflicts-preview.png"));
        table->selectRow(1);
        button(dialog, "terminateSelected")->click();
        QCOMPARE(processes.graceful, (QVector<quint32>{22}));
        QVERIFY(!button(dialog, "ignore")->isEnabled());
        QTRY_COMPARE_WITH_TIMEOUT(processes.forced.size(), 1, 2500);
        QCOMPARE(processes.forced[0], quint32(22));
        QCOMPARE(table->rowCount(), 1);
        button(dialog, "terminateAll")->click();
        QTRY_COMPARE_WITH_TIMEOUT(processes.forced.size(), 2, 2500);
        QTRY_COMPARE(dialog.result(), int(QDialog::Accepted));
    }
    void asynchronousExitNeedsOneClick() {
        FakeProcesses processes;processes.exitScanDelay=3;
        ConflictDialog dialog(processes);dialog.show();
        QTest::mouseClick(button(dialog,"terminateAll"),Qt::LeftButton);
        QCOMPARE(processes.graceful.size(),2);
        QVERIFY(!button(dialog,"terminateAll")->isEnabled());
        QTRY_COMPARE_WITH_TIMEOUT(dialog.result(),int(QDialog::Accepted),2500);
        QCOMPARE(processes.forced,(QVector<quint32>{11,22}));QVERIFY(processes.running.isEmpty());
    }
    void terminationDeadlineIsVisible() {
        FakeProcesses processes;processes.exitScanDelay=10000;
        ConflictDialog dialog(processes);dialog.show();button(dialog,"terminateAll")->click();
        QTRY_VERIFY_WITH_TIMEOUT(button(dialog,"terminateAll")->isEnabled(),4000);
        QCOMPARE(processes.forced.size(),2);QCOMPARE(processes.running.size(),2);
        QVERIFY(dialog.findChild<QLabel*>("conflictStatus")->text().contains("Still running after termination"));
        button(dialog,"cancel")->click();
    }
    void verificationScanFailureStaysVisible() {
        FakeProcesses processes;processes.exitScanDelay=3;
        ConflictDialog dialog(processes);dialog.show();button(dialog,"terminateAll")->click();
        QTRY_COMPARE_WITH_TIMEOUT(processes.forced.size(),2,2500);processes.failScan=true;
        QTRY_VERIFY(dialog.findChild<QLabel*>("conflictStatus")->text().contains("scan failed"));
        QVERIFY(dialog.isVisible());QVERIFY(button(dialog,"ignore")->isEnabled());
        QVERIFY(!button(dialog,"terminateAll")->isEnabled());button(dialog,"cancel")->click();
    }
    void deniedAndRestarted() {
        FakeProcesses processes;
        processes.denied = true;
        ConflictDialog dialog(processes);
        dialog.show();
        button(dialog, "terminateAll")->click();
        QTRY_VERIFY_WITH_TIMEOUT(button(dialog, "terminateAll")->isEnabled(), 2500);
        QVERIFY(dialog.findChild<QLabel*>("conflictStatus")->text().contains("Access denied"));
        QCOMPARE(dialog.findChild<QTableWidget*>("conflictTable")->rowCount(), 2);
        processes.denied = false; processes.restart = true;
        button(dialog, "terminateAll")->click();
        QTRY_COMPARE_WITH_TIMEOUT(processes.forced.size(), 2, 2500);
        QCOMPARE(dialog.findChild<QTableWidget*>("conflictTable")->rowCount(), 2);
        QVERIFY(dialog.isVisible());
    }
    void cancellationPreventsForce() {
        FakeProcesses processes;
        ConflictDialog dialog(processes);
        dialog.show();
        button(dialog, "terminateAll")->click();
        button(dialog, "cancel")->click();
        QTest::qWait(1400);
        QVERIFY(processes.forced.isEmpty());
    }
    void scanFailure() {
        FakeProcesses processes; processes.failScan = true;
        ConflictDialog dialog(processes);
        QVERIFY(dialog.findChild<QLabel*>("conflictStatus")->text().contains("scan failed"));
        QVERIFY(!button(dialog, "terminateAll")->isEnabled());
        QCOMPARE(dialog.result(), int(QDialog::Rejected));
    }
};
QTEST_MAIN(StartupTest)
#include "test_startup.moc"
