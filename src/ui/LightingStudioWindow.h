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
class QPushButton;

class LightingStudioWindow : public QWidget {
    Q_OBJECT
public:
    // channelNames: every channel this build knows about (6 motherboard +
    // 4 peripherals), used to populate the side panel and the canvas even
    // before any Settings data exists for a channel (fresh/empty-profile
    // case -- see ensureProfileAndEntries()).
    //
    // availableModesByChannel (Task 23): which of {Native, Canvas} channel
    // X's own driver actually honors -- e.g. the 4 peripheral drivers are
    // Canvas-only (their setNativeMode is a confirmed permanent no-op), and
    // the motherboard's JRGB1 is Native-only (no Gen1Adv area). main.cpp
    // builds this once at startup from each driver's IDriver::supportsMode()
    // (the authoritative, hardware-topology-owning source -- see IDriver.h),
    // so this window never has to re-derive per-device mode rules as magic
    // strings of its own. A channel with no entry (or an empty vector) here
    // is treated as supporting BOTH modes -- this is also what every
    // pre-Task-23 test and call site gets by passing nothing, via the
    // default argument, so none of them had to change to learn about modes
    // they don't care about.
    LightingStudioWindow(Settings& settings, std::vector<std::string> channelNames,
        std::map<std::string, std::vector<ChannelMode>> availableModesByChannel = {},
        QWidget* parent = nullptr);

    LightingCanvasWidget* canvas() const; // exposed for tests and for main.cpp to connect Canvas::setAssignment on changes

    // Public mutators backing the color-swatch buttons and profile
    // create/rename/delete buttons (Task 23). Each one is the single place
    // its corresponding button's click handler funnels into -- QColorDialog/
    // QInputDialog are modal and cannot be driven from an automated test, so
    // tests call these directly instead and assert on the resulting Settings
    // state + emitted signal, exactly as this window's existing row-combo
    // tests already do for onRowChanged.
    void setChannelColor(const std::string& channelName, const MsiColor& color);
    void setChannelNativeColor(const std::string& channelName, const MsiColor& color);
    void setGlobalEffectColor(const MsiColor& color);

    // Profile CRUD (Task 23): thin wrappers around the corresponding
    // Settings:: methods (see Settings.h) that also re-sync this window's UI/
    // canvas and re-fire the usual change signals via refreshFromSettings(),
    // so main.cpp's listeners (tray profile submenu, debounced save) pick up
    // the change the same way they already do for a profile switch. A no-op
    // Settings:: call (e.g. a duplicate/empty name, or deleting the last
    // profile) is likewise a no-op here: nothing is re-synced or re-emitted.
    void createProfile(const std::string& name);
    void renameActiveProfile(const std::string& newName);
    void deleteActiveProfile();

    // Call this after changing settings_.activeProfileName from OUTSIDE
    // this window -- e.g. Task 18's tray profile-switch path (plan Step 5:
    // "refresh the LightingStudioWindow") sets the name on the Settings
    // reference this window already holds, then must call this so the
    // canvas, every row's combos, and the profile selector all repaint to
    // match the newly active profile. Internal switches via profileCombo_
    // already do this themselves (onProfileComboChanged) -- this is the
    // external-trigger equivalent, including the same signal fan-out
    // (channelAssignmentChanged per channel, globalEffectChanged,
    // activeProfileChanged).
    void refreshFromSettings();

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
    // One row's mode/source/effect pickers in the side panel, plus (Task 23)
    // a staticColor swatch button and a minimal Native-mode editor
    // (lightingMode combo + color1 swatch button) -- see Part 2 of
    // task-23-report.md for what's deliberately NOT exposed here (every
    // other NativeModeSettings field keeps its current default, unedited).
    struct ChannelRow {
        QComboBox* modeCombo = nullptr;
        QComboBox* sourceCombo = nullptr;
        QComboBox* effectCombo = nullptr;
        QPushButton* colorButton = nullptr;       // staticColor; meaningful only per effectUsesColor()
        QComboBox* nativeModeCombo = nullptr;     // NativeModeSettings::lightingMode; meaningful only in Native mode
        QPushButton* nativeColorButton = nullptr; // NativeModeSettings::color1; meaningful only in Native mode
    };

    // Guarantees settings_.profiles is non-empty (pushes a "Default" profile
    // if it was empty -- fresh/corrupt Settings case named in the dispatch)
    // and that the active profile has a ChannelSettings entry for every
    // name in channelNames_ (a profile loaded/switched-to may be missing
    // some, including a brand-new profile just created by createProfile()).
    // Also clamps any EXISTING entry whose persisted mode this channel's
    // driver doesn't actually support (Task 23: e.g. a hand-edited
    // settings.json predating this fix, or JRGB1 forced to Canvas) to a mode
    // it does support, instead of leaving a row with no matching combo item.
    // Safe to call repeatedly; a no-op once everything already exists and
    // already validates.
    void ensureProfileAndEntries();
    // Finds (or, defensively, appends) channelName's entry in the active
    // profile. Never holds the returned reference across a later call that
    // might resize profile->channelSettings (vector reference invalidation).
    ChannelSettings& channelSettingsFor(const std::string& channelName);

    // Which of {Native, Canvas} channelName's driver actually supports, per
    // the constructor's availableModesByChannel_ -- defaults to "both" for a
    // channel with no entry (see the constructor's doc comment).
    std::vector<ChannelMode> availableModesFor(const std::string& channelName) const;
    // Builds a fresh ChannelSettings for a channel with no persisted entry
    // yet: motherboard channels (Native available) keep ChannelSettings' own
    // Native default; a Canvas-only peripheral instead defaults to Canvas/
    // StaticColor/a visible color (Task 23) -- Native is a confirmed
    // permanent no-op for all 4 peripheral drivers, so leaving a fresh
    // peripheral channel at the plain ChannelSettings default would mean it
    // never lights up until the user manually flips it to Canvas.
    ChannelSettings makeDefaultChannelSettings(const std::string& channelName) const;
    // If cs.mode isn't in availableModesFor(channelName) (a persisted but
    // now-unsupported mode), clamps it to the first mode that IS supported,
    // and -- if that clamp lands on Canvas with an empty effectName (e.g. a
    // channel forced out of Native, whose effectName contract is "empty") --
    // fills in the same Canvas-default effect/color a fresh peripheral entry
    // gets, so the channel doesn't silently stay dark.
    void clampModeIfUnsupported(const std::string& channelName, ChannelSettings& cs) const;

    void applyChannelToCanvas(const std::string& channelName, const ChannelSettings& cs);
    void applyGlobalEffectToCanvas(const GlobalEffectSettings& g);
    // Pushes every channelNames_ entry's current Settings state into both
    // the row widgets (combo indices, signal-blocked) and canvasWidget_, and
    // the active profile's global effect into canvasWidget_. Also rebuilds
    // profileCombo_'s item list from scratch (Task 23: profiles.size() can
    // change between calls now that profiles can be created/renamed/
    // deleted, not just switched). Called once at construction and again on
    // every profile switch/create/rename/delete.
    void repopulateFromActiveProfile();
    void updateRowEnabledState(const std::string& channelName);
    // Enables globalColorButton_ only when the active global effect actually
    // reads a color (effectUsesColor()); called whenever the global effect
    // combo or the active profile changes.
    void updateGlobalColorButtonEnabledState();

    void onRowChanged(const std::string& channelName);
    void onGlobalEffectComboChanged();
    void onProfileComboChanged();
    void onCanvasChannelPlacementChanged(const std::string& channelName, CanvasPlacement newPlacement);
    void onCanvasGlobalReferencePointChanged(int x, int y);

    Settings& settings_;
    std::vector<std::string> channelNames_;
    std::map<std::string, std::vector<ChannelMode>> availableModesByChannel_;
    LightingCanvasWidget* canvasWidget_ = nullptr;
    QComboBox* profileCombo_ = nullptr;
    QPushButton* newProfileButton_ = nullptr;
    QPushButton* renameProfileButton_ = nullptr;
    QPushButton* deleteProfileButton_ = nullptr;
    QComboBox* globalEffectCombo_ = nullptr;
    QPushButton* globalColorButton_ = nullptr;
    std::map<std::string, ChannelRow> rows_;
};
