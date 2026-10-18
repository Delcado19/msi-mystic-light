// Unit test for TrayApp (Task 10): minimal QSystemTrayIcon/QMenu wrapper
// with a single "Quit" action. Uses app.contextMenu() rather than making the
// tray icon visible, so this runs without a real system tray (headless/CI).
// Do NOT call start() here (see Task 10 brief) — it calls trayIcon_.show()
// which needs a real desktop tray and would behave unpredictably headless.
#include <QtTest/QtTest>

#include "ui/TrayApp.h"

class TrayAppTest : public QObject
{
    Q_OBJECT

private slots:
    void testContextMenuHasQuitActionAndEmitsSignal();
};

void TrayAppTest::testContextMenuHasQuitActionAndEmitsSignal()
{
    TrayApp app;
    QMenu* menu = app.contextMenu();
    QVERIFY(menu != nullptr);
    QAction* quitAction = nullptr;
    for (QAction* a : menu->actions()) {
        if (a->text() == "Quit") { quitAction = a; break; }
    }
    QVERIFY(quitAction != nullptr);
    QSignalSpy spy(&app, &TrayApp::quitRequested);
    quitAction->trigger();
    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(TrayAppTest)
#include "test_tray_app.moc"
