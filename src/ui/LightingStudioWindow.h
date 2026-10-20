// Main window ("Lighting Studio"): LightingCanvasWidget (Task 14) on the
// left, a side panel on the right with a profile selector, a global-effect
// picker, and one mode/source/effect row per known channel. Writes directly
// into the Settings instance it's given -- it does not own persistence
// (main.cpp still calls Settings::saveToFile on quit, per the plan's
// Task 18). Task 15 of the Lighting Studio plan; see
// docs/superpowers/specs/2026-10-19-lighting-studio-design.md and
// .superpowers/sdd/2026-10-19-lighting-studio/task-15-brief.md.
#pragma once

#include <QWidget>

#include <map>
#include <string>
#include <vector>

#include "canvas/Canvas.h" // CanvasPlacement
#include "settings/Settings.h"

class LightingCanvasWidget;
class QComboBox;

class LightingStudioWindow : public QWidget {
    Q_OBJECT
public:
    // channelNames: every channel this build knows about (6 motherboard +
    // 4 peripherals), used to populate the side panel and the canvas even
    // before any Settings data exists for a channel (fresh/empty-profile
    // case -- see ensureProfileAndEntries()).
    LightingStudioWindow(Settings& settings, std::vector<std::string> channelNames, QWidget* parent = nullptr);

    LightingCanvasWidget* canvas() const; // exposed for tests and for main.cpp to connect Canvas::setAssignment on changes

signals:
    // Fired whenever any row's mode/source/effect/placement changes, or the
    // active profile switches -- main.cpp listens to push the new
    // assignment into the running Canvas (Task 18), not just to Settings.
    // A profile switch fires channelAssignmentChanged once per channel in
    // channelNames_ (every channel's assignment changes at once) plus
    // globalEffectChanged, followed by activeProfileChanged last.
    void channelAssignmentChanged(const std::string& channelName);
    void globalEffectChanged();
    void activeProfileChanged(const std::string& profileName);

private:
    // One row's mode/source/effect pickers in the side panel. No color/
    // native-settings editing UI in this pass (see task-15-report.md,
    // "Deferred") -- StaticColor/native-mode values are only ever read from
    // whatever Settings already holds for that channel.
    struct ChannelRow {
        QComboBox* modeCombo = nullptr;
        QComboBox* sourceCombo = nullptr;
        QComboBox* effectCombo = nullptr;
    };

    // Guarantees settings_.profiles is non-empty (pushes a "Default" profile
    // if it was empty -- fresh/corrupt Settings case named in the dispatch)
    // and that the active profile has a ChannelSettings entry for every
    // name in channelNames_ (a profile loaded/switched-to may be missing
    // some). Safe to call repeatedly; a no-op once everything already
    // exists.
    void ensureProfileAndEntries();
    // Finds (or, defensively, appends) channelName's entry in the active
    // profile. Never holds the returned reference across a later call that
    // might resize profile->channelSettings (vector reference invalidation).
    ChannelSettings& channelSettingsFor(const std::string& channelName);

    void applyChannelToCanvas(const std::string& channelName, const ChannelSettings& cs);
    void applyGlobalEffectToCanvas(const GlobalEffectSettings& g);
    // Pushes every channelNames_ entry's current Settings state into both
    // the row widgets (combo indices, signal-blocked) and canvasWidget_, and
    // the active profile's global effect into canvasWidget_. Called once at
    // construction and again on every profile switch.
    void repopulateFromActiveProfile();
    void updateRowEnabledState(const std::string& channelName);

    void onRowChanged(const std::string& channelName);
    void onGlobalEffectComboChanged();
    void onProfileComboChanged();
    void onCanvasChannelPlacementChanged(const std::string& channelName, CanvasPlacement newPlacement);
    void onCanvasGlobalReferencePointChanged(int x, int y);

    Settings& settings_;
    std::vector<std::string> channelNames_;
    LightingCanvasWidget* canvasWidget_ = nullptr;
    QComboBox* profileCombo_ = nullptr;
    QComboBox* globalEffectCombo_ = nullptr;
    std::map<std::string, ChannelRow> rows_;
};
