// Unit test for AutostartWin (Task 11): registers/unregisters the app under
// HKCU\Software\Microsoft\Windows\CurrentVersion\Run so it can launch at
// Windows login. Uses a throwaway value name under the REAL registry key
// (sanctioned by the plan brief — this name is never used by anything else,
// and the test cleans up after itself via setEnabled(false) either way).
#include <QtTest/QtTest>

#include "platform/win/AutostartWin.h"

class AutostartTest : public QObject
{
    Q_OBJECT

private slots:
    void testEnableDisableRoundTrip();
    void cleanup(); // QtTest runs this after every test, pass or fail
};

void AutostartTest::testEnableDisableRoundTrip()
{
    AutostartWin autostart(L"MsiMysticLightAppTest_DoNotKeep", L"C:\\does\\not\\matter.exe");
    QVERIFY(!autostart.isEnabled());
    QVERIFY(autostart.setEnabled(true));
    QVERIFY(autostart.isEnabled());
    QVERIFY(autostart.setEnabled(false));
    QVERIFY(!autostart.isEnabled());
}

// QVERIFY returns early on failure, so without this the throwaway value
// could be left behind in the real Run key after a failed assertion above,
// making every subsequent run fail too. This guarantees removal regardless
// of how the test above exits.
void AutostartTest::cleanup()
{
    AutostartWin autostart(L"MsiMysticLightAppTest_DoNotKeep", L"");
    autostart.setEnabled(false);
}

QTEST_MAIN(AutostartTest)
#include "test_autostart.moc"
