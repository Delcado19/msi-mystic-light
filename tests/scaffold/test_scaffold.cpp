// Scaffold test: verifies the CMake/Qt Test/CTest harness itself works end to
// end (configure -> build -> ctest discovery -> QtTest execution). Replaced
// by real protocol/UI tests starting in Task 2; keep until those exist.
#include <QtTest/QtTest>

class ScaffoldTest : public QObject
{
    Q_OBJECT

private slots:
    void sanityCheck();
};

void ScaffoldTest::sanityCheck()
{
    QVERIFY(1 + 1 == 2);
}

QTEST_MAIN(ScaffoldTest)
#include "test_scaffold.moc"
