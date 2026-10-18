// Unit tests for Settings (Task 9): JSON persistence of per-channel user
// configuration. Covers the round trip, and the Review Focus item this task
// owns — a missing or corrupted settings file (first run, or after a manual
// edit) must leave channelSettings unchanged and never crash, rather than
// wiping out the caller's defaults.
#include <QtTest/QtTest>

#include "settings/Settings.h"

class SettingsTest : public QObject
{
    Q_OBJECT

private slots:
    void testSaveAndLoadRoundTrip();
    void testLoadFromFile_missingFileLeavesSettingsUnchanged();
    void testLoadFromFile_corruptedJsonLeavesSettingsUnchanged();
    void testLoadFromFile_validJsonMissingFieldsUsesDefaults();
    void testLoadFromFile_wrongFieldTypesUseDefaults();
    void testLoadFromFile_structurallyInvalidJsonLeavesSettingsUnchanged();
    void testSaveToFile_unwritablePathFails();
};

void SettingsTest::testSaveAndLoadRoundTrip()
{
    Settings original;
    original.channelSettings.push_back({"JARGB1", ChannelMode::Canvas, "RainbowWave", {}, {}});
    original.channelSettings.push_back({"JRGB1", ChannelMode::Native, "", {}, NativeModeSettings{MsiLightingMode::Breathing, {1,2,3},{},{},{}, 1,false,false,true,4,2,20}});
    QString path = QDir::temp().filePath("msi_app_settings_test.json");
    QVERIFY(original.saveToFile(path.toStdString()));

    Settings loaded;
    QVERIFY(loaded.loadFromFile(path.toStdString()));
    QCOMPARE(static_cast<int>(loaded.channelSettings.size()), 2);
    QVERIFY(loaded.channelSettings[0] == original.channelSettings[0]);
    QVERIFY(loaded.channelSettings[1] == original.channelSettings[1]);
    QFile::remove(path);
}

void SettingsTest::testLoadFromFile_missingFileLeavesSettingsUnchanged()
{
    Settings s;
    s.channelSettings.push_back({"Marker", ChannelMode::Native, "", {}, {}});
    QVERIFY(!s.loadFromFile("/does/not/exist/anywhere.json"));
    QCOMPARE(static_cast<int>(s.channelSettings.size()), 1);
    QCOMPARE(s.channelSettings[0].channelName, std::string("Marker"));
}

void SettingsTest::testLoadFromFile_corruptedJsonLeavesSettingsUnchanged()
{
    QString path = QDir::temp().filePath("msi_app_settings_corrupt_test.json");
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.write("{ this is not valid json ][");
    f.close();

    Settings s;
    s.channelSettings.push_back({"Marker", ChannelMode::Native, "", {}, {}});
    QVERIFY(!s.loadFromFile(path.toStdString()));
    QCOMPARE(static_cast<int>(s.channelSettings.size()), 1);
    QFile::remove(path);
}

// A manual edit of the settings file is well-formed JSON but missing fields
// (e.g. a hand-trimmed "channels" entry without "nativeSettings") isn't a
// parse failure, so loadFromFile should succeed and fill in defaults for
// whatever is missing rather than rejecting the whole file.
void SettingsTest::testLoadFromFile_validJsonMissingFieldsUsesDefaults()
{
    QString path = QDir::temp().filePath("msi_app_settings_partial_test.json");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(R"({"channels":[{"channelName":"JARGB2"}]})");
    f.close();

    Settings s;
    s.channelSettings.push_back({"Marker", ChannelMode::Native, "", {}, {}});
    QVERIFY(s.loadFromFile(path.toStdString()));
    QCOMPARE(static_cast<int>(s.channelSettings.size()), 1);
    QCOMPARE(s.channelSettings[0].channelName, std::string("JARGB2"));
    QVERIFY((s.channelSettings[0] == ChannelSettings{"JARGB2", ChannelMode::Native, "", {}, {}}));
    QFile::remove(path);
}

// A field present with the wrong JSON type (a string where a number/bool/
// object is expected) must default that field rather than crash or corrupt
// the rest of the entry — same Review Focus as the missing-field case
// above, but for type mismatches instead of absent keys.
void SettingsTest::testLoadFromFile_wrongFieldTypesUseDefaults()
{
    QString path = QDir::temp().filePath("msi_app_settings_wrongtypes_test.json");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(R"({"channels":[{"channelName":"JARGB2","mode":42,"staticColor":"red","nativeSettings":{"brightness":"high","selectAll":"yes"}}]})");
    f.close();

    Settings s;
    s.channelSettings.push_back({"Marker", ChannelMode::Native, "", {}, {}});
    QVERIFY(s.loadFromFile(path.toStdString()));
    QCOMPARE(static_cast<int>(s.channelSettings.size()), 1);
    QVERIFY((s.channelSettings[0] == ChannelSettings{"JARGB2", ChannelMode::Native, "", {}, {}}));
    QFile::remove(path);
}

// Covers the two loadFromFile failure branches not exercised by the
// corrupted-JSON test above: well-formed JSON whose top level isn't an
// object, and a top-level object whose "channels" key isn't an array.
// Both must fail the same way as a parse error: return false and leave
// channelSettings untouched.
void SettingsTest::testLoadFromFile_structurallyInvalidJsonLeavesSettingsUnchanged()
{
    const QStringList badDocuments = {
        "[1,2,3]",               // valid JSON, but top level isn't an object
        "{}",                    // valid object, but no "channels" key at all
        R"({"channels":{}})",    // "channels" present but not an array
        R"({"channels":"nope"})" // "channels" present but a string
    };
    QString path = QDir::temp().filePath("msi_app_settings_structural_test.json");
    for (const QString& bad : badDocuments) {
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        f.write(bad.toUtf8());
        f.close();

        Settings s;
        s.channelSettings.push_back({"Marker", ChannelMode::Native, "", {}, {}});
        QVERIFY(!s.loadFromFile(path.toStdString()));
        QCOMPARE(static_cast<int>(s.channelSettings.size()), 1);
        QCOMPARE(s.channelSettings[0].channelName, std::string("Marker"));
    }
    QFile::remove(path);
}

void SettingsTest::testSaveToFile_unwritablePathFails()
{
    Settings s;
    s.channelSettings.push_back({"Marker", ChannelMode::Native, "", {}, {}});
    QVERIFY(!s.saveToFile("/does/not/exist/subdir/settings.json"));
}

QTEST_MAIN(SettingsTest)
#include "test_settings.moc"
