// Unit test for TrayApp: QSystemTrayIcon/QMenu wrapper with Quit (Task 10),
// "Open Lighting Studio", and a profile quick-switch submenu (Task 16).
// Uses app.contextMenu() rather than making the tray icon visible, so this
// runs without a real system tray (headless/CI).
// Do NOT call start() here (see Task 10 brief) — it calls trayIcon_.show()
// which needs a real desktop tray and would behave unpredictably headless.
#include <QtTest/QtTest>

#include "ui/TrayApp.h"

class TrayAppTest : public QObject
{
    Q_OBJECT

private slots:
    void testContextMenuHasQuitActionAndEmitsSignal();
    void testOpenStudioMenuEntry_triggersOpenStudioRequested();
    void testSetAvailableProfiles_thenClickingAnEntry_emitsProfileSelectedWithThatName();
    void testSetAvailableProfiles_marksTheActiveProfileChecked();

private:
    // Finds a top-level QAction by text, or the "Profile" submenu's QMenu.
    static QAction* findAction(QMenu* menu, const QString& text);
    static QMenu* findProfileSubmenu(QMenu* menu);
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

QAction* TrayAppTest::findAction(QMenu* menu, const QString& text)
{
    for (QAction* a : menu->actions()) {
        if (a->text() == text) return a;
    }
    return nullptr;
}

QMenu* TrayAppTest::findProfileSubmenu(QMenu* menu)
{
    for (QAction* a : menu->actions()) {
        if (a->menu() != nullptr) return a->menu();
    }
    return nullptr;
}

void TrayAppTest::testOpenStudioMenuEntry_triggersOpenStudioRequested()
{
    TrayApp app;
    QAction* openStudioAction = findAction(app.contextMenu(), "Open Lighting Studio");
    QVERIFY(openStudioAction != nullptr);
    QSignalSpy spy(&app, &TrayApp::openStudioRequested);
    openStudioAction->trigger();
    QCOMPARE(spy.count(), 1);
}

void TrayAppTest::testSetAvailableProfiles_thenClickingAnEntry_emitsProfileSelectedWithThatName()
{
    TrayApp app;
    app.setAvailableProfiles({"Gaming", "Movie", "Calm"}, "Gaming");

    QMenu* profileMenu = findProfileSubmenu(app.contextMenu());
    QVERIFY(profileMenu != nullptr);
    QAction* movieAction = findAction(profileMenu, "Movie");
    QVERIFY(movieAction != nullptr);

    QSignalSpy spy(&app, &TrayApp::profileSelected);
    movieAction->trigger();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<std::string>(), std::string("Movie"));

    // The submenu is radio-style (an exclusive QActionGroup), not plain
    // checkboxes: selecting a different entry must uncheck the one that
    // was previously active, not leave both checked.
    QVERIFY(movieAction->isChecked());
    QVERIFY(!findAction(profileMenu, "Gaming")->isChecked());
}

void TrayAppTest::testSetAvailableProfiles_marksTheActiveProfileChecked()
{
    TrayApp app;
    app.setAvailableProfiles({"Gaming", "Movie", "Calm"}, "Movie");

    QMenu* profileMenu = findProfileSubmenu(app.contextMenu());
    QVERIFY(profileMenu != nullptr);

    QAction* gamingAction = findAction(profileMenu, "Gaming");
    QAction* movieAction = findAction(profileMenu, "Movie");
    QAction* calmAction = findAction(profileMenu, "Calm");
    QVERIFY(gamingAction != nullptr);
    QVERIFY(movieAction != nullptr);
    QVERIFY(calmAction != nullptr);

    QVERIFY(!gamingAction->isChecked());
    QVERIFY(movieAction->isChecked());
    QVERIFY(!calmAction->isChecked());

    // Calling it again replaces the submenu's contents rather than
    // accumulating duplicates (e.g. a profile renamed/removed in the
    // Studio window must be reflected, not left stale alongside the new list).
    app.setAvailableProfiles({"Gaming", "Calm"}, "Calm");
    profileMenu = findProfileSubmenu(app.contextMenu());
    QCOMPARE(profileMenu->actions().size(), 2);
    QVERIFY(findAction(profileMenu, "Movie") == nullptr);
    QAction* calmAgain = findAction(profileMenu, "Calm");
    QVERIFY(calmAgain != nullptr);
    QVERIFY(calmAgain->isChecked());
}

QTEST_MAIN(TrayAppTest)
#include "test_tray_app.moc"
