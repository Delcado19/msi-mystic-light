#pragma once
#include <QByteArray>

// GUI test executables must also run when started directly (not only through ctest, which sets
// these variables). Without them Qt aborts with "could not initialize the Qt platform plugin".
// Runs during static initialization, before QTEST_MAIN creates the QApplication; explicit
// environment values still win. TEST_QPA_PLUGIN_DIR is set per target in tests/CMakeLists.txt.
namespace {
const bool offscreenTestPlatformConfigured = [] {
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "offscreen");
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM_PLUGIN_PATH")) qputenv("QT_QPA_PLATFORM_PLUGIN_PATH", TEST_QPA_PLUGIN_DIR);
#ifdef Q_OS_WIN
    // The offscreen plugin has no Windows font database; ctest sets the same directory.
    if (!qEnvironmentVariableIsSet("QT_QPA_FONTDIR")) qputenv("QT_QPA_FONTDIR", qgetenv("WINDIR") + "/Fonts");
#endif
    return true;
}();
}
