// Device sidebar, fitted spatial preview, and selected-device inspector.
// Edits update the shared Settings model; main.cpp owns hardware writes
// and debounced persistence. Selecting a device or pausing the preview
// never modifies a hardware assignment.
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
class QListWidget;
class QFormLayout;
class DeviceLedWidget;
class QLabel;
class QCheckBox;
class QTabWidget;
class QStackedWidget;

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
    // the motherboard's JRGB1 and IOShield are Native-only. main.cpp
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

    void setDeviceComponents(const std::string& name, const std::vector<Component>& components);
    bool editComponentGeometry(const std::string& name,const std::vector<Component>& components);
    bool configureComponents(const std::string& name, const std::vector<Component>& components);
    bool setRegionColor(const std::string& name, int index, MsiColor color);
    bool createCustomEffect(const CustomEffectDefinition& definition);
    bool identifyRegion(const std::string& name, int index);
    void stopRegionIdentification();
    void setPersistenceStatus(bool saved);
    LightingCanvasWidget* canvas() const; // exposed for tests and for main.cpp to connect Canvas::setAssignment on changes

    // Public mutators backing the color-swatch buttons and profile
    // create/rename/delete buttons (Task 23). Each one is the single place
    // its corresponding button's click handler funnels into -- ColorPicker/
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
    // Temporary hardware-only override; index -1 restores the saved assignment.
    void customEffectsChanged();
    void regionIdentificationRequested(const std::string& name, int index);
    // Fired whenever any row's mode/source/effect/placement/staticColor/
    // native lightingMode/native color1 changes (Task 23 added the last
    // three), or the active profile switches -- main.cpp listens to push the
    // new assignment into the running Canvas (Task 18), not just to
    // Settings. A profile switch -- or create/rename/delete (Task 23: all
    // three route through refreshFromSettings(), same as a switch) -- fires
    // channelAssignmentChanged once per channel in channelNames_ (every
    // channel's assignment changes at once) plus globalEffectChanged,
    // followed by activeProfileChanged last.
    void channelAssignmentChanged(const std::string& channelName);
    void globalEffectChanged();
    void activeProfileChanged(const std::string& profileName);

protected:
    void hideEvent(QHideEvent* event) override;

private:
    // One row's mode/source/effect pickers in the side panel, plus (Task 23)
    // a staticColor swatch button and a minimal Native-mode editor
    // (lightingMode combo + color1 swatch button) -- see Part 2 of
    // task-23-report.md for what's deliberately NOT exposed here (every
    // other NativeModeSettings field keeps its current default, unedited).
    struct ChannelRow {
        QFormLayout* form = nullptr;
        DeviceLedWidget* hardwarePreview=nullptr;
        QString componentSignature;
        QListWidget* components = nullptr;
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
    // yet: motherboard channels start with steady white; a Canvas-only peripheral instead defaults to Canvas/
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

    void updateSelectedDevice();
    std::map<std::string, std::vector<Component>> defaultComponents_;
    DeviceLedWidget* ledMap_ = nullptr;
    QCheckBox* identifyToggle_ = nullptr;
    QTabWidget* workspaceViews_=nullptr;
    QStackedWidget* hardwarePages_=nullptr;
    CustomEffectDefinition previewRecipe_;
    QLabel* layoutEffectBadge_=nullptr;
    QLabel* persistenceStatus_=nullptr;
    bool persistenceSucceeded_=true;
    std::string identificationChannel_;
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
    QListWidget* deviceList_ = nullptr;
    QStackedWidget* devicePages_ = nullptr;
};
