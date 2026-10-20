// Unit tests for Settings (Task 9, extended by Task 13): JSON persistence
// of per-channel user configuration, now organized into named Profiles with
// per-channel canvas source/placement and a canvas-wide GlobalEffectSettings
// (Task 13). Covers the round trip, the v1->profiles migration, the
// dangling-active-profile fallback, and the Review Focus item Task 9 owns --
// a missing or corrupted settings file (first run, or after a manual edit)
// must leave Settings unchanged and never crash, rather than wiping out the
// caller's defaults.
#include <cmath>

#include <QtTest/QtTest>

#include "settings/Settings.h"

class SettingsTest : public QObject
{
    Q_OBJECT

private slots:
    void testSaveAndLoadRoundTrip_multipleProfilesWithPlacementAndSource();
    void testLoadFromFile_legacyChannelsOnlyFileMigratesToDefaultProfile();
    void testLoadFromFile_danglingActiveProfileFallsBackToFirst();
    void testActiveProfile_emptyProfilesReturnsNullptr();
    void testLoadFromFile_missingFileLeavesSettingsUnchanged();
    void testLoadFromFile_corruptedJsonLeavesSettingsUnchanged();
    void testLoadFromFile_validJsonMissingFieldsUsesDefaults();
    void testLoadFromFile_wrongFieldTypesUseDefaults();
    void testLoadFromFile_outOfRangeNativeValuesAreClamped();
    void testLoadFromFile_extremePlacementIsClamped();
    void testLoadFromFile_structurallyInvalidJsonLeavesSettingsUnchanged();
    void testSaveToFile_unwritablePathFails();

    // Task 23: profile create/rename/delete (CRUD), added because the UI
    // previously had no way to do any of this -- only switch between
    // profiles already present in a hand-edited settings file.
    void testAddProfile_appendsNewProfileWithGivenName();
    void testAddProfile_rejectsEmptyName();
    void testAddProfile_rejectsDuplicateName();
    void testRenameActiveProfile_updatesNameAndActiveProfileName();
    void testRenameActiveProfile_rejectsEmptyName();
    void testRenameActiveProfile_rejectsCollisionWithDifferentProfile();
    void testRenameActiveProfile_allowsRenamingToItsOwnCurrentName();
    void testRenameActiveProfile_resolvesStaleActiveProfileNameViaActiveProfile();
    void testRemoveActiveProfile_removesItAndActivatesANeighbor();
    void testRemoveActiveProfile_refusesWhenOnlyOneProfileLeft();
    void testRemoveActiveProfile_refusesWhenProfilesEmpty();
};

// Extends the pre-Task-13 round-trip test to cover the new shape: 2
// profiles, a GlobalFollow channel with a non-zero placement, and a
// non-default GlobalEffectSettings -- everything must come back exactly
// equal after a save/load round trip.
void SettingsTest::testSaveAndLoadRoundTrip_multipleProfilesWithPlacementAndSource()
{
    Settings original;

    Profile profileA;
    profileA.name = "Gaming";
    ChannelSettings canvasChannel;
    canvasChannel.channelName = "JARGB1";
    canvasChannel.mode = ChannelMode::Canvas;
    canvasChannel.source = CanvasSource::GlobalFollow;
    canvasChannel.placement = CanvasPlacement{12.5, -3.0};
    canvasChannel.effectName = "RainbowWave";
    profileA.channelSettings.push_back(canvasChannel);
    ChannelSettings nativeChannel;
    nativeChannel.channelName = "JRGB1";
    nativeChannel.mode = ChannelMode::Native;
    nativeChannel.nativeSettings = NativeModeSettings{MsiLightingMode::Breathing, {1,2,3},{},{},{}, 1,false,false,true,4,2,20};
    profileA.channelSettings.push_back(nativeChannel);
    profileA.globalEffect = GlobalEffectSettings{"RadialRainbow", {10, 20, 30}, 7, 9};
    original.profiles.push_back(profileA);

    Profile profileB;
    profileB.name = "Subtle";
    ChannelSettings staticChannel;
    staticChannel.channelName = "JARGB2";
    staticChannel.mode = ChannelMode::Canvas;
    staticChannel.source = CanvasSource::Independent;
    staticChannel.effectName = "StaticColor";
    staticChannel.staticColor = {5, 6, 7};
    profileB.channelSettings.push_back(staticChannel);
    original.profiles.push_back(profileB);

    original.activeProfileName = "Subtle";

    QString path = QDir::temp().filePath("msi_app_settings_test.json");
    QVERIFY(original.saveToFile(path.toStdString()));

    Settings loaded;
    QVERIFY(loaded.loadFromFile(path.toStdString()));
    QCOMPARE(static_cast<int>(loaded.profiles.size()), 2);
    QVERIFY(loaded.profiles[0] == original.profiles[0]);
    QVERIFY(loaded.profiles[1] == original.profiles[1]);
    QCOMPARE(loaded.activeProfileName, std::string("Subtle"));
    QFile::remove(path);
}

// A v1-style file -- top-level {"channels": [...]}, no "profiles" key --
// must still load, as a single Profile named "Default" containing those
// channels, so existing users' settings.json keeps working after upgrading.
void SettingsTest::testLoadFromFile_legacyChannelsOnlyFileMigratesToDefaultProfile()
{
    QString path = QDir::temp().filePath("msi_app_settings_legacy_test.json");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(R"({"channels":[{"channelName":"JARGB1","mode":"Canvas","effectName":"RainbowWave"},{"channelName":"JRGB1","mode":"Native"}]})");
    f.close();

    Settings s;
    QVERIFY(s.loadFromFile(path.toStdString()));
    QCOMPARE(static_cast<int>(s.profiles.size()), 1);
    QCOMPARE(s.profiles[0].name, std::string("Default"));
    QCOMPARE(s.activeProfileName, std::string("Default"));
    QCOMPARE(static_cast<int>(s.profiles[0].channelSettings.size()), 2);

    ChannelSettings expectedFirst;
    expectedFirst.channelName = "JARGB1";
    expectedFirst.mode = ChannelMode::Canvas;
    expectedFirst.effectName = "RainbowWave";
    QVERIFY(s.profiles[0].channelSettings[0] == expectedFirst); // full-struct match, not just name/effect

    ChannelSettings expectedSecond;
    expectedSecond.channelName = "JRGB1";
    expectedSecond.mode = ChannelMode::Native;
    QVERIFY(s.profiles[0].channelSettings[1] == expectedSecond);
    QVERIFY(s.profiles[0].globalEffect == GlobalEffectSettings{}); // v1 files have no global effect -> defaults
    QFile::remove(path);
}

// "activeProfile" naming a profile not present in "profiles" must not
// crash; activeProfile() falls back to the first profile (Review Focus).
void SettingsTest::testLoadFromFile_danglingActiveProfileFallsBackToFirst()
{
    QString path = QDir::temp().filePath("msi_app_settings_dangling_test.json");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(R"({"profiles":[{"name":"First","channels":[]},{"name":"Second","channels":[]}],"activeProfile":"DoesNotExist"})");
    f.close();

    Settings s;
    QVERIFY(s.loadFromFile(path.toStdString()));
    QCOMPARE(static_cast<int>(s.profiles.size()), 2);
    Profile* active = s.activeProfile();
    QVERIFY(active != nullptr);
    QVERIFY(active == &s.profiles[0]);
    QCOMPARE(active->name, std::string("First"));
    QFile::remove(path);
}

void SettingsTest::testActiveProfile_emptyProfilesReturnsNullptr()
{
    Settings s;
    QVERIFY(s.profiles.empty());
    QVERIFY(s.activeProfile() == nullptr);
}

void SettingsTest::testLoadFromFile_missingFileLeavesSettingsUnchanged()
{
    Settings s;
    Profile marker;
    marker.name = "Marker";
    s.profiles.push_back(marker);
    s.activeProfileName = "Marker";
    QVERIFY(!s.loadFromFile("/does/not/exist/anywhere.json"));
    QCOMPARE(static_cast<int>(s.profiles.size()), 1);
    QCOMPARE(s.profiles[0].name, std::string("Marker"));
    QCOMPARE(s.activeProfileName, std::string("Marker"));
}

void SettingsTest::testLoadFromFile_corruptedJsonLeavesSettingsUnchanged()
{
    QString path = QDir::temp().filePath("msi_app_settings_corrupt_test.json");
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.write("{ this is not valid json ][");
    f.close();

    Settings s;
    Profile marker;
    marker.name = "Marker";
    s.profiles.push_back(marker);
    QVERIFY(!s.loadFromFile(path.toStdString()));
    QCOMPARE(static_cast<int>(s.profiles.size()), 1);
    QFile::remove(path);
}

// A manual edit of the settings file is well-formed JSON but missing fields
// (e.g. a hand-trimmed channel entry without "nativeSettings") isn't a
// parse failure, so loadFromFile should succeed and fill in defaults for
// whatever is missing rather than rejecting the whole file. Pre-existing
// v1 test fixture shape ({"channels": [...]}, no "profiles" key) kept
// verbatim per the brief: it must still pass, now via the migration path
// (channelSettingsFromJson is shared between both paths).
void SettingsTest::testLoadFromFile_validJsonMissingFieldsUsesDefaults()
{
    QString path = QDir::temp().filePath("msi_app_settings_partial_test.json");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(R"({"channels":[{"channelName":"JARGB2"}]})");
    f.close();

    Settings s;
    Profile marker;
    marker.name = "Marker";
    s.profiles.push_back(marker);
    QVERIFY(s.loadFromFile(path.toStdString()));
    QCOMPARE(static_cast<int>(s.profiles.size()), 1);
    QCOMPARE(s.profiles[0].name, std::string("Default")); // migrated
    QCOMPARE(static_cast<int>(s.profiles[0].channelSettings.size()), 1);
    QCOMPARE(s.profiles[0].channelSettings[0].channelName, std::string("JARGB2"));
    ChannelSettings expected;
    expected.channelName = "JARGB2";
    QVERIFY(s.profiles[0].channelSettings[0] == expected);
    QFile::remove(path);
}

// A field present with the wrong JSON type (a string where a number/bool/
// object is expected) must default that field rather than crash or corrupt
// the rest of the entry -- same Review Focus as the missing-field case
// above, but for type mismatches instead of absent keys. Pre-existing v1
// fixture shape kept, with the two new fields (source/placement) added to
// the same wrong-type sweep since they go through the same JSON path.
void SettingsTest::testLoadFromFile_wrongFieldTypesUseDefaults()
{
    QString path = QDir::temp().filePath("msi_app_settings_wrongtypes_test.json");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(R"({"channels":[{"channelName":"JARGB2","mode":42,"source":7,"placement":"nowhere","staticColor":"red","nativeSettings":{"brightness":"high","selectAll":"yes"}}]})");
    f.close();

    Settings s;
    Profile marker;
    marker.name = "Marker";
    s.profiles.push_back(marker);
    QVERIFY(s.loadFromFile(path.toStdString()));
    QCOMPARE(static_cast<int>(s.profiles.size()), 1);
    ChannelSettings expected;
    expected.channelName = "JARGB2";
    QVERIFY(s.profiles[0].channelSettings[0] == expected);
    QFile::remove(path);
}

// An out-of-range value in a hand-edited settings.json must be clamped to
// what MsiProtocol.cpp's bit-packing can safely encode, not passed through
// -- these values reach the real board within one render tick, so a wild
// value here is exactly the "unconfirmed bytes to hardware" case the
// project's safety rule exists to prevent (whole-branch review finding).
// Pre-existing v1 fixture shape kept, now via the migration path.
void SettingsTest::testLoadFromFile_outOfRangeNativeValuesAreClamped()
{
    QString path = QDir::temp().filePath("msi_app_settings_outofrange_test.json");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(R"({"channels":[{"channelName":"JARGB2","nativeSettings":{"lightingMode":999,"colorCount":0,"brightness":42,"speed":-5}}]})");
    f.close();

    Settings s;
    QVERIFY(s.loadFromFile(path.toStdString()));
    QCOMPARE(static_cast<int>(s.profiles.size()), 1);
    QCOMPARE(static_cast<int>(s.profiles[0].channelSettings.size()), 1);
    const NativeModeSettings& n = s.profiles[0].channelSettings[0].nativeSettings;
    QCOMPARE(n.lightingMode, MsiLightingMode::Steady); // 999 is invalid -> falls back to the default
    QCOMPARE(n.colorCount, static_cast<uint8_t>(1));   // 0 clamped up to the documented 1-4 range
    QCOMPARE(n.brightness, static_cast<uint8_t>(5));   // 42 clamped down to the documented 0-5 range
    QCOMPARE(n.speed, static_cast<uint8_t>(0));         // -5 clamped up to the documented 0-2 range
    QFile::remove(path);
}

// Review Focus (Task 3's deferred item, now actionable): a placement value
// too large/non-finite to survive Canvas.cpp's `static_cast<int>(placement
// + localCoord)` is undefined behavior, not just a visually wrong position.
// JSON can't encode NaN/Infinity, but 1e300 parses fine as a double, so the
// clamp must be exercised through the real load path, not just unit-tested
// in isolation.
void SettingsTest::testLoadFromFile_extremePlacementIsClamped()
{
    QString path = QDir::temp().filePath("msi_app_settings_extremeplacement_test.json");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(R"({"profiles":[{"name":"Default","channels":[{"channelName":"JARGB1","source":"GlobalFollow","placement":{"x":1e300,"y":-1e300}}]}],"activeProfile":"Default"})");
    f.close();

    Settings s;
    QVERIFY(s.loadFromFile(path.toStdString()));
    const CanvasPlacement& p = s.profiles[0].channelSettings[0].placement;
    QVERIFY(std::isfinite(p.x));
    QVERIFY(std::isfinite(p.y));
    QVERIFY(p.x < 1e7 && p.x > -1e7); // clamped, not passed through raw
    QVERIFY(p.y < 1e7 && p.y > -1e7);
    QFile::remove(path);
}

// Covers the loadFromFile failure branches: well-formed JSON whose top
// level isn't an object, a top-level object with no "channels" key and no
// "profiles" key (fails v1-migration's own structural check), a "channels"
// present but not an array (same, v1 path), and a "profiles" key present
// but not an array. All must fail the same way as a parse error: return
// false and leave Settings untouched.
void SettingsTest::testLoadFromFile_structurallyInvalidJsonLeavesSettingsUnchanged()
{
    const QStringList badDocuments = {
        "[1,2,3]",                 // valid JSON, but top level isn't an object
        "{}",                      // valid object, but no "channels" and no "profiles" key
        R"({"channels":{}})",      // "channels" present but not an array (v1 path)
        R"({"channels":"nope"})",  // "channels" present but a string (v1 path)
        R"({"profiles":{}})",      // "profiles" present but not an array
        R"({"profiles":"nope"})"   // "profiles" present but a string
    };
    QString path = QDir::temp().filePath("msi_app_settings_structural_test.json");
    for (const QString& bad : badDocuments) {
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        f.write(bad.toUtf8());
        f.close();

        Settings s;
        Profile marker;
        marker.name = "Marker";
        s.profiles.push_back(marker);
        QVERIFY(!s.loadFromFile(path.toStdString()));
        QCOMPARE(static_cast<int>(s.profiles.size()), 1);
        QCOMPARE(s.profiles[0].name, std::string("Marker"));
    }
    QFile::remove(path);
}

void SettingsTest::testSaveToFile_unwritablePathFails()
{
    Settings s;
    Profile marker;
    marker.name = "Marker";
    s.profiles.push_back(marker);
    QVERIFY(!s.saveToFile("/does/not/exist/subdir/settings.json"));
}

void SettingsTest::testAddProfile_appendsNewProfileWithGivenName()
{
    Settings s;
    Profile p; p.name = "Default";
    s.profiles.push_back(p);
    s.activeProfileName = "Default";

    QVERIFY(s.addProfile("Night"));
    QCOMPARE(static_cast<int>(s.profiles.size()), 2);
    QCOMPARE(s.profiles[1].name, std::string("Night"));
    QVERIFY(s.profiles[1].channelSettings.empty()); // caller fills in channel defaults, not addProfile itself
    QCOMPARE(s.activeProfileName, std::string("Default")); // unchanged: addProfile doesn't switch
}

void SettingsTest::testAddProfile_rejectsEmptyName()
{
    Settings s;
    QVERIFY(!s.addProfile(""));
    QVERIFY(s.profiles.empty());
}

void SettingsTest::testAddProfile_rejectsDuplicateName()
{
    Settings s;
    Profile p; p.name = "Default";
    s.profiles.push_back(p);
    QVERIFY(!s.addProfile("Default"));
    QCOMPARE(static_cast<int>(s.profiles.size()), 1); // no duplicate appended
}

void SettingsTest::testRenameActiveProfile_updatesNameAndActiveProfileName()
{
    Settings s;
    Profile p; p.name = "Default";
    s.profiles.push_back(p);
    s.activeProfileName = "Default";

    QVERIFY(s.renameActiveProfile("Gaming"));
    QCOMPARE(s.profiles[0].name, std::string("Gaming"));
    QCOMPARE(s.activeProfileName, std::string("Gaming"));
}

void SettingsTest::testRenameActiveProfile_rejectsEmptyName()
{
    Settings s;
    Profile p; p.name = "Default";
    s.profiles.push_back(p);
    s.activeProfileName = "Default";

    QVERIFY(!s.renameActiveProfile(""));
    QCOMPARE(s.profiles[0].name, std::string("Default")); // unchanged
}

void SettingsTest::testRenameActiveProfile_rejectsCollisionWithDifferentProfile()
{
    Settings s;
    Profile a; a.name = "Default";
    Profile b; b.name = "Night";
    s.profiles = {a, b};
    s.activeProfileName = "Default";

    QVERIFY(!s.renameActiveProfile("Night")); // would collide with profiles[1]
    QCOMPARE(s.profiles[0].name, std::string("Default"));
    QCOMPARE(s.activeProfileName, std::string("Default"));
}

void SettingsTest::testRenameActiveProfile_allowsRenamingToItsOwnCurrentName()
{
    Settings s;
    Profile p; p.name = "Default";
    s.profiles.push_back(p);
    s.activeProfileName = "Default";

    QVERIFY(s.renameActiveProfile("Default")); // not a collision with itself
    QCOMPARE(s.profiles[0].name, std::string("Default"));
}

// renameActiveProfile goes through activeProfile()'s own stale-name
// fallback: if activeProfileName names a profile that no longer exists,
// "the active profile" is profiles.front(), not a nonexistent entry -- the
// rename must act on that fallback target, same as every other Settings
// caller that uses activeProfile().
void SettingsTest::testRenameActiveProfile_resolvesStaleActiveProfileNameViaActiveProfile()
{
    Settings s;
    Profile p; p.name = "First";
    s.profiles.push_back(p);
    s.activeProfileName = "DoesNotExist";

    QVERIFY(s.renameActiveProfile("Renamed"));
    QCOMPARE(s.profiles[0].name, std::string("Renamed"));
    QCOMPARE(s.activeProfileName, std::string("Renamed"));
}

void SettingsTest::testRemoveActiveProfile_removesItAndActivatesANeighbor()
{
    Settings s;
    Profile a; a.name = "Default";
    Profile b; b.name = "Night";
    s.profiles = {a, b};
    s.activeProfileName = "Default";

    QVERIFY(s.removeActiveProfile());
    QCOMPARE(static_cast<int>(s.profiles.size()), 1);
    QCOMPARE(s.profiles[0].name, std::string("Night"));
    QCOMPARE(s.activeProfileName, std::string("Night"));
}

void SettingsTest::testRemoveActiveProfile_refusesWhenOnlyOneProfileLeft()
{
    Settings s;
    Profile p; p.name = "Default";
    s.profiles.push_back(p);
    s.activeProfileName = "Default";

    QVERIFY(!s.removeActiveProfile());
    QCOMPARE(static_cast<int>(s.profiles.size()), 1); // untouched: never left with zero profiles
}

void SettingsTest::testRemoveActiveProfile_refusesWhenProfilesEmpty()
{
    Settings s;
    QVERIFY(!s.removeActiveProfile());
    QVERIFY(s.profiles.empty());
}

QTEST_MAIN(SettingsTest)
#include "test_settings.moc"
