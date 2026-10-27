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
        SystemConflictProcesses system;
        QVERIFY(!system.stop({}, true).isEmpty());
        ConflictProcess invalid{123, "SignalRgb.exe", {}, "", 0, false};
        QVERIFY(!system.stop(invalid, true).isEmpty());
        invalid.executable = "LEDKeeper2.exe";
        QVERIFY(system.stop(invalid, true).contains("manually"));
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
