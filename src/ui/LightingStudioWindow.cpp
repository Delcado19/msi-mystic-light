#include "ui/LightingStudioWindow.h"

#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <algorithm>
#include <memory>
#include <utility>

#include "effects/BreathingEffect.h"
#include "effects/CometEffect.h"
#include "effects/MarqueeEffect.h"
#include "effects/RadialRainbowEffect.h"
#include "effects/RainbowWaveEffect.h"
#include "effects/SpiralEffect.h"
#include "effects/StaticColorEffect.h"
#include "effects/SwirlCirclesEffect.h"
#include "ui/LightingCanvasWidget.h"

namespace {

constexpr int kPanelSpacing = 12;
constexpr int kRowSpacing = 6;
constexpr int kPanelMinWidth = 320;
constexpr int kSwatchSize = 22; // small square color-swatch buttons, row and global alike

// Default staticColor for a freshly-created peripheral channel (Task 23):
// full white. Peripherals default to Canvas mode (see
// LightingStudioWindow::makeDefaultChannelSettings) since Native is a
// confirmed permanent no-op for all 4 peripheral drivers -- leaving
// staticColor at its own MsiColor{} default (black) would mean the device is
// "configured" but still invisible out of the box, defeating the point of
// defaulting to Canvas at all. White is simplest/most visible; any
// non-black color would satisfy the same requirement.
constexpr MsiColor kDefaultPeripheralColor{255, 255, 255};

const std::vector<std::string> kSourceNames = {"Independent", "GlobalFollow"};
// "StaticColor" matches the name already used by Settings/main.cpp (not
// "Fill" -- see task-15-brief.md note 4, resolved against the existing
// codebase convention).
const std::vector<std::string> kEffectNames = {
    "StaticColor", "RainbowWave", "Breathing", "RadialRainbow",
    "Spiral", "Comet", "Marquee", "SwirlCircles"};

// Minimal effectName -> IEffect factory, local to this window. There is no
// shared factory anywhere in the codebase yet (main.cpp's own
// buildAssignment() has the same kind of local if/else chain for its two
// known names); Task 18's final wire-up builds its own instance of this
// same mapping rather than sharing one with this file, per the plan
// (docs/superpowers/plans/2026-10-19-lighting-studio.md, Task 18, Step 3).
// Deliberately tiny: no effect-parameter configuration UI, just enough to
// construct something to hand to setChannel()/setGlobalEffect() per row.
std::shared_ptr<IEffect> buildEffect(const std::string& effectName, const MsiColor& color, int referenceX, int referenceY)
{
    // ponytail: CometEffect's trackWidthPx has no stored Settings field to
    // read (no canvas-width concept persisted); fixed to a sane constant
    // for now. Add a real field if Comet's track width ever needs tuning.
    constexpr int kCometTrackWidthPx = 300;
    if (effectName == "StaticColor") return std::make_shared<StaticColorEffect>(color);
    if (effectName == "RainbowWave") return std::make_shared<RainbowWaveEffect>();
    if (effectName == "Breathing") return std::make_shared<BreathingEffect>(color);
    if (effectName == "RadialRainbow") return std::make_shared<RadialRainbowEffect>(referenceX, referenceY);
    if (effectName == "Spiral") return std::make_shared<SpiralEffect>(referenceX, referenceY);
    if (effectName == "Comet") return std::make_shared<CometEffect>(color, kCometTrackWidthPx);
    if (effectName == "Marquee") return std::make_shared<MarqueeEffect>(color);
    if (effectName == "SwirlCircles") return std::make_shared<SwirlCirclesEffect>(referenceX, referenceY);
    return nullptr; // unknown/empty name -- Canvas and LightingCanvasWidget both tolerate a null effect
}

// Which of kEffectNames' effects actually read the `color` parameter --
// mirrors buildEffect()'s own constructor-argument list above exactly, not a
// second, independent guess (Task 23). StaticColor/Breathing/Comet/Marquee
// all take a color; RainbowWave/RadialRainbow/Spiral/SwirlCircles don't
// (their only parameters are period/center/tightness). Used to gate both the
// per-row staticColor swatch and the global-effect color swatch -- see
// Settings.h's corrected field comment on ChannelSettings::staticColor.
bool effectUsesColor(const std::string& effectName)
{
    return effectName == "StaticColor" || effectName == "Breathing"
        || effectName == "Comet" || effectName == "Marquee";
}

// Native-mode lighting-mode choices offered by the row's minimal Native
// editor (Task 23, Part 2's "at minimum lightingMode+color1" ask).
// Deliberately excludes MsiLightingMode::Advanced: that value is the Canvas
// per-LED arming mode (see MsiMotherboardDriver::setChannelMode's comment on
// why entering Canvas mode arms a zone to Advanced) -- it has no meaning as
// a user-chosen NATIVE-mode value and would display nothing on real
// hardware if picked here.
const std::vector<std::pair<MsiLightingMode, std::string>> kNativeLightingModes = {
    {MsiLightingMode::Off, "Off"}, {MsiLightingMode::Wave, "Wave"},
    {MsiLightingMode::Steady, "Steady"}, {MsiLightingMode::Flame, "Flame"},
    {MsiLightingMode::Breathing, "Breathing"}, {MsiLightingMode::ColorRing, "ColorRing"},
    {MsiLightingMode::Lightning, "Lightning"}, {MsiLightingMode::Recreation, "Recreation"},
    {MsiLightingMode::Meteor, "Meteor"}, {MsiLightingMode::GodLike, "GodLike"},
};

std::string modeName(ChannelMode mode)
{
    return mode == ChannelMode::Canvas ? "Canvas" : "Native";
}

// MsiColor<->QColor conversion (Task 23): no such helper existed anywhere in
// the codebase before this -- LightingCanvasWidget.cpp just inlines
// QColor(c.r, c.g, c.b) at its few call sites -- so this is one small,
// obviously-named pair added where QColorDialog first needs both
// directions, not a speculative shared utility.
QColor toQColor(const MsiColor& c)
{
    return QColor(c.r, c.g, c.b);
}

MsiColor toMsiColor(const QColor& c)
{
    return MsiColor{static_cast<uint8_t>(c.red()), static_cast<uint8_t>(c.green()), static_cast<uint8_t>(c.blue())};
}

void updateColorSwatch(QPushButton* button, const MsiColor& color)
{
    // A flat-filled button is a simpler, more honest "this is the current
    // color" indicator than an icon/pixmap would be, and needs no asset.
    button->setStyleSheet(QStringLiteral("background-color: %1; border: 1px solid #555;").arg(toQColor(color).name()));
}

QComboBox* makeCombo(QWidget* parent, const QString& objectName, const std::vector<std::string>& items)
{
    auto* combo = new QComboBox(parent);
    combo->setObjectName(objectName);
    for (const std::string& item : items) combo->addItem(QString::fromStdString(item));
    return combo;
}

} // namespace

LightingStudioWindow::LightingStudioWindow(Settings& settings, std::vector<std::string> channelNames,
    std::map<std::string, std::vector<ChannelMode>> availableModesByChannel, QWidget* parent)
    : QWidget(parent)
    , settings_(settings)
    , channelNames_(std::move(channelNames))
    , availableModesByChannel_(std::move(availableModesByChannel))
{
    setWindowTitle(QStringLiteral("Lighting Studio"));

    canvasWidget_ = new LightingCanvasWidget(this);
    canvasWidget_->setMinimumSize(420, 320);

    profileCombo_ = new QComboBox(this);
    profileCombo_->setObjectName(QStringLiteral("profileCombo"));
    // Profile create/rename/delete (Task 23): small inline buttons next to
    // the combo, matching this window's existing dense/utilitarian layout
    // rather than a dedicated profile-management dialog. Text labels, not
    // icons -- no icon asset exists anywhere in this codebase yet.
    newProfileButton_ = new QPushButton(QStringLiteral("+"), this);
    newProfileButton_->setObjectName(QStringLiteral("newProfileButton"));
    newProfileButton_->setToolTip(QStringLiteral("New profile"));
    renameProfileButton_ = new QPushButton(QStringLiteral("Rename"), this);
    renameProfileButton_->setObjectName(QStringLiteral("renameProfileButton"));
    deleteProfileButton_ = new QPushButton(QStringLiteral("-"), this);
    deleteProfileButton_->setObjectName(QStringLiteral("deleteProfileButton"));
    deleteProfileButton_->setToolTip(QStringLiteral("Delete profile"));

    globalEffectCombo_ = makeCombo(this, QStringLiteral("globalEffectCombo"), kEffectNames);
    globalColorButton_ = new QPushButton(this);
    globalColorButton_->setObjectName(QStringLiteral("globalColorButton"));
    globalColorButton_->setFixedSize(kSwatchSize, kSwatchSize);
    globalColorButton_->setToolTip(QStringLiteral("Global effect color"));

    auto* panel = new QWidget(this);
    panel->setMinimumWidth(kPanelMinWidth);
    auto* panelLayout = new QVBoxLayout(panel);
    panelLayout->setSpacing(kPanelSpacing);

    auto* profileRow = new QHBoxLayout();
    profileRow->addWidget(new QLabel(QStringLiteral("Profile"), panel));
    profileRow->addWidget(profileCombo_, /*stretch=*/1);
    profileRow->addWidget(newProfileButton_);
    profileRow->addWidget(renameProfileButton_);
    profileRow->addWidget(deleteProfileButton_);
    panelLayout->addLayout(profileRow);

    auto* globalRow = new QHBoxLayout();
    globalRow->addWidget(new QLabel(QStringLiteral("Global Effect"), panel));
    globalRow->addWidget(globalEffectCombo_, /*stretch=*/1);
    globalRow->addWidget(globalColorButton_);
    panelLayout->addLayout(globalRow);

    auto* scrollArea = new QScrollArea(panel);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    auto* rowsContainer = new QWidget(scrollArea);
    auto* grid = new QGridLayout(rowsContainer);
    grid->setVerticalSpacing(kRowSpacing);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(2, 1);
    grid->setColumnStretch(3, 1);
    grid->setColumnStretch(5, 1);

    int gridRow = 0;
    for (const std::string& channelName : channelNames_) {
        ChannelRow row;
        const QString qname = QString::fromStdString(channelName);

        // Mode combo offers only the modes this channel's driver actually
        // supports (Task 23, Part 1) -- never a hardcoded Native/Canvas
        // pair. Item data carries the real ChannelMode value so combo index
        // is never treated as "0=Native,1=Canvas" (that assumption breaks
        // for a single-mode channel).
        row.modeCombo = new QComboBox(rowsContainer);
        row.modeCombo->setObjectName(QStringLiteral("mode:") + qname);
        for (ChannelMode mode : availableModesFor(channelName)) {
            row.modeCombo->addItem(QString::fromStdString(modeName(mode)), static_cast<int>(mode));
        }

        row.sourceCombo = makeCombo(rowsContainer, QStringLiteral("source:") + qname, kSourceNames);
        row.effectCombo = makeCombo(rowsContainer, QStringLiteral("effect:") + qname, kEffectNames);

        row.colorButton = new QPushButton(rowsContainer);
        row.colorButton->setObjectName(QStringLiteral("color:") + qname);
        row.colorButton->setFixedSize(kSwatchSize, kSwatchSize);
        row.colorButton->setToolTip(QStringLiteral("Static color"));

        row.nativeModeCombo = new QComboBox(rowsContainer);
        row.nativeModeCombo->setObjectName(QStringLiteral("nativeMode:") + qname);
        for (const auto& [mode, label] : kNativeLightingModes) {
            row.nativeModeCombo->addItem(QString::fromStdString(label), static_cast<int>(mode));
        }

        row.nativeColorButton = new QPushButton(rowsContainer);
        row.nativeColorButton->setObjectName(QStringLiteral("nativeColor:") + qname);
        row.nativeColorButton->setFixedSize(kSwatchSize, kSwatchSize);
        row.nativeColorButton->setToolTip(QStringLiteral("Native mode color"));

        grid->addWidget(new QLabel(qname, rowsContainer), gridRow, 0);
        grid->addWidget(row.modeCombo, gridRow, 1);
        grid->addWidget(row.sourceCombo, gridRow, 2);
        grid->addWidget(row.effectCombo, gridRow, 3);
        grid->addWidget(row.colorButton, gridRow, 4);
        grid->addWidget(row.nativeModeCombo, gridRow, 5);
        grid->addWidget(row.nativeColorButton, gridRow, 6);
        ++gridRow;

        rows_.emplace(channelName, row);
    }

    scrollArea->setWidget(rowsContainer);
    panelLayout->addWidget(scrollArea, /*stretch=*/1);

    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(canvasWidget_, /*stretch=*/2);
    mainLayout->addWidget(panel, /*stretch=*/1);

    // Settings/row initial state is applied before any of our own signals
    // are connected below, so construction can never spuriously emit
    // channelAssignmentChanged/globalEffectChanged/activeProfileChanged
    // (the brief's own construction test relies on this). profileCombo_'s
    // items are populated inside repopulateFromActiveProfile() itself now
    // (Task 23: it must also be rebuildable after a create/rename/delete),
    // so there is no separate one-time population loop here anymore.
    ensureProfileAndEntries();
    repopulateFromActiveProfile();

    connect(profileCombo_, &QComboBox::currentIndexChanged, this, &LightingStudioWindow::onProfileComboChanged);
    connect(globalEffectCombo_, &QComboBox::currentIndexChanged, this, &LightingStudioWindow::onGlobalEffectComboChanged);
    connect(canvasWidget_, &LightingCanvasWidget::channelPlacementChanged, this, &LightingStudioWindow::onCanvasChannelPlacementChanged);
    connect(canvasWidget_, &LightingCanvasWidget::globalEffectReferencePointChanged, this, &LightingStudioWindow::onCanvasGlobalReferencePointChanged);

    connect(globalColorButton_, &QPushButton::clicked, this, [this] {
        Profile* profile = settings_.activeProfile();
        if (!profile) return;
        const QColor picked = QColorDialog::getColor(toQColor(profile->globalEffect.color), this, QStringLiteral("Global Effect Color"));
        if (picked.isValid()) setGlobalEffectColor(toMsiColor(picked));
    });

    connect(newProfileButton_, &QPushButton::clicked, this, [this] {
        bool ok = false;
        const QString name = QInputDialog::getText(this, QStringLiteral("New Profile"), QStringLiteral("Name:"), QLineEdit::Normal, QString(), &ok);
        if (ok && !name.trimmed().isEmpty()) createProfile(name.trimmed().toStdString());
    });
    connect(renameProfileButton_, &QPushButton::clicked, this, [this] {
        Profile* profile = settings_.activeProfile();
        const QString current = profile ? QString::fromStdString(profile->name) : QString();
        bool ok = false;
        const QString name = QInputDialog::getText(this, QStringLiteral("Rename Profile"), QStringLiteral("Name:"), QLineEdit::Normal, current, &ok);
        if (ok && !name.trimmed().isEmpty()) renameActiveProfile(name.trimmed().toStdString());
    });
    connect(deleteProfileButton_, &QPushButton::clicked, this, [this] {
        // Confirm: deleting a profile is destructive (every channel
        // assignment in it is gone) and this button has no undo.
        const auto reply = QMessageBox::question(this, QStringLiteral("Delete Profile"),
            QStringLiteral("Delete the current profile? This cannot be undone."));
        if (reply == QMessageBox::Yes) deleteActiveProfile();
    });

    for (auto& [channelName, row] : rows_) {
        connect(row.modeCombo, &QComboBox::currentIndexChanged, this, [this, channelName] { onRowChanged(channelName); });
        connect(row.sourceCombo, &QComboBox::currentIndexChanged, this, [this, channelName] { onRowChanged(channelName); });
        connect(row.effectCombo, &QComboBox::currentIndexChanged, this, [this, channelName] { onRowChanged(channelName); });
        connect(row.nativeModeCombo, &QComboBox::currentIndexChanged, this, [this, channelName] { onRowChanged(channelName); });
        connect(row.colorButton, &QPushButton::clicked, this, [this, channelName] {
            const QColor picked = QColorDialog::getColor(toQColor(channelSettingsFor(channelName).staticColor), this, QStringLiteral("Channel Color"));
            if (picked.isValid()) setChannelColor(channelName, toMsiColor(picked));
        });
        connect(row.nativeColorButton, &QPushButton::clicked, this, [this, channelName] {
            const QColor picked = QColorDialog::getColor(toQColor(channelSettingsFor(channelName).nativeSettings.color1), this, QStringLiteral("Native Mode Color"));
            if (picked.isValid()) setChannelNativeColor(channelName, toMsiColor(picked));
        });
    }
}

LightingCanvasWidget* LightingStudioWindow::canvas() const
{
    return canvasWidget_;
}

void LightingStudioWindow::ensureProfileAndEntries()
{
    if (settings_.profiles.empty()) {
        // Fresh/corrupt Settings case (task-15-brief.md note 2): construct
        // and show every known channel with sensible defaults instead of
        // crashing on a null activeProfile().
        Profile defaultProfile;
        defaultProfile.name = "Default";
        settings_.profiles.push_back(std::move(defaultProfile));
        settings_.activeProfileName = "Default";
    }

    Profile* profile = settings_.activeProfile(); // never null past the guard above
    for (const std::string& name : channelNames_) {
        const auto found = std::find_if(profile->channelSettings.begin(), profile->channelSettings.end(),
            [&name](const ChannelSettings& cs) { return cs.channelName == name; });
        if (found != profile->channelSettings.end()) {
            // Entry already exists (loaded from disk, or left over from a
            // previous call) -- Task 23: clamp it if it names a mode this
            // channel's driver doesn't actually support (e.g. a hand-edited
            // settings.json predating this fix, or JRGB1 forced to Canvas).
            clampModeIfUnsupported(name, *found);
            continue;
        }

        ChannelSettings cs = makeDefaultChannelSettings(name);
        // Staggers freshly-added channels across the canvas instead of
        // stacking every one at (0,0) -- purely a "something sensible to
        // look at before the user drags it" default, not a real layout.
        const size_t index = profile->channelSettings.size();
        cs.placement = CanvasPlacement{static_cast<double>(20 + (index % 5) * 90), static_cast<double>(20 + (index / 5) * 60)};
        profile->channelSettings.push_back(std::move(cs));
    }
}

ChannelSettings& LightingStudioWindow::channelSettingsFor(const std::string& channelName)
{
    Profile* profile = settings_.activeProfile();
    for (ChannelSettings& cs : profile->channelSettings) {
        if (cs.channelName == channelName) return cs;
    }
    // Shouldn't happen: ensureProfileAndEntries() guarantees every
    // channelNames_ entry exists in the active profile before any row
    // handler can run. Defensive fallback instead of dereferencing nothing
    // -- routed through the same defaulting helper ensureProfileAndEntries()
    // uses, so even this fallback path gives a peripheral channel a visible
    // Canvas default rather than an invisible Native one.
    profile->channelSettings.push_back(makeDefaultChannelSettings(channelName));
    return profile->channelSettings.back();
}

std::vector<ChannelMode> LightingStudioWindow::availableModesFor(const std::string& channelName) const
{
    auto it = availableModesByChannel_.find(channelName);
    if (it == availableModesByChannel_.end() || it->second.empty()) {
        // No entry (e.g. a call site/test that doesn't pass this map at
        // all, or a channel name main.cpp's driver query didn't cover) --
        // treat as "supports both", preserving every pre-Task-23 caller's
        // behavior unchanged.
        return {ChannelMode::Native, ChannelMode::Canvas};
    }
    return it->second;
}

ChannelSettings LightingStudioWindow::makeDefaultChannelSettings(const std::string& channelName) const
{
    ChannelSettings cs;
    cs.channelName = channelName;
    const std::vector<ChannelMode> modes = availableModesFor(channelName);
    if (std::find(modes.begin(), modes.end(), ChannelMode::Native) == modes.end()) {
        // Native isn't offered at all -- a Canvas-only peripheral. Native is
        // a confirmed permanent no-op for all 4 peripheral drivers
        // (setNativeMode always returns false), so leaving this at
        // ChannelSettings' own Native default would mean the device never
        // lights up until the user manually flips it to Canvas (Task 23).
        cs.mode = ChannelMode::Canvas;
        cs.source = CanvasSource::Independent;
        cs.effectName = "StaticColor";
        cs.staticColor = kDefaultPeripheralColor;
    }
    // Motherboard channels (Native available, including JRGB1) keep
    // ChannelSettings' own Native default -- unchanged, still safe.
    return cs;
}

void LightingStudioWindow::clampModeIfUnsupported(const std::string& channelName, ChannelSettings& cs) const
{
    const std::vector<ChannelMode> modes = availableModesFor(channelName);
    if (modes.empty() || std::find(modes.begin(), modes.end(), cs.mode) != modes.end()) {
        return; // either no restriction is known, or the persisted mode is already fine
    }

    cs.mode = modes.front();
    if (cs.mode == ChannelMode::Canvas && cs.effectName.empty()) {
        // Same bug class as a fresh peripheral default (see
        // makeDefaultChannelSettings): clamping Native -> Canvas must not
        // leave effectName empty (buildEffect("") returns nullptr, the
        // channel stays dark forever) -- fill in the same visible default.
        cs.source = CanvasSource::Independent;
        cs.effectName = "StaticColor";
        cs.staticColor = kDefaultPeripheralColor;
    }
}

void LightingStudioWindow::applyChannelToCanvas(const std::string& channelName, const ChannelSettings& cs)
{
    if (cs.mode == ChannelMode::Native) {
        // Native-mode channels still get a box on the canvas (so the user
        // can arrange the physical-mirror layout before ever touching
        // Canvas mode), but with no own effect -- Canvas itself never
        // samples a Native channel's placement; only this preview does.
        canvasWidget_->setChannel(channelName, cs.placement, CanvasSource::Independent, nullptr);
        return;
    }

    std::shared_ptr<IEffect> effect;
    if (cs.source == CanvasSource::Independent) {
        effect = buildEffect(cs.effectName, cs.staticColor, 0, 0);
    }
    // GlobalFollow channels pass no own effect: LightingCanvasWidget draws
    // them as an outline over the shared background instead (see its
    // paintEvent), matching how Canvas itself samples the shared global
    // effect for a GlobalFollow channel rather than any per-channel effect.
    canvasWidget_->setChannel(channelName, cs.placement, cs.source, effect);
}

void LightingStudioWindow::applyGlobalEffectToCanvas(const GlobalEffectSettings& g)
{
    canvasWidget_->setGlobalEffect(buildEffect(g.effectName, g.color, g.referenceX, g.referenceY));
}

void LightingStudioWindow::updateRowEnabledState(const std::string& channelName)
{
    ChannelRow& row = rows_.at(channelName);
    const bool isCanvas = static_cast<ChannelMode>(row.modeCombo->currentData().toInt()) == ChannelMode::Canvas;
    row.sourceCombo->setEnabled(isCanvas);
    const bool isIndependent = row.sourceCombo->currentIndex() == 0;
    row.effectCombo->setEnabled(isCanvas && isIndependent);
    // Gated on the STORED cs.effectName, not row.effectCombo->currentText():
    // repopulateFromActiveProfile() sets this combo via findText(cs.effectName),
    // which falls back to index 0 ("StaticColor") whenever cs.effectName is
    // empty or unrecognized (e.g. every fresh profile's globalEffect-style
    // empty default, or a hand-edited settings.json's Canvas-mode entry with
    // no "effectName" field) -- that fallback is a DISPLAY choice only, and
    // gating on it would enable this button while buildEffect(cs.effectName)
    // still resolves to nullptr (an empty name), i.e. a swatch the user can
    // click with no visible effect. onRowChanged() calls this AFTER writing
    // cs, so this always reads the value that's actually about to be used.
    const bool colorMeaningful = isCanvas && isIndependent && effectUsesColor(channelSettingsFor(channelName).effectName);
    row.colorButton->setEnabled(colorMeaningful);

    // Native-mode minimal editor (Part 2): only meaningful while the row
    // itself is in Native mode.
    row.nativeModeCombo->setEnabled(!isCanvas);
    row.nativeColorButton->setEnabled(!isCanvas);
}

void LightingStudioWindow::updateGlobalColorButtonEnabledState()
{
    // Same reasoning as updateRowEnabledState's colorMeaningful above: gate on
    // the STORED globalEffect.effectName (every fresh profile's
    // GlobalEffectSettings{} default-constructs this to "", which
    // globalEffectCombo_'s own findText-with-fallback then silently displays
    // as "StaticColor") rather than the combo's possibly-stale display text,
    // so the swatch is never enabled for a selection that hasn't actually
    // been written into Settings yet.
    const Profile* profile = settings_.activeProfile();
    const std::string effectName = profile ? profile->globalEffect.effectName : std::string();
    globalColorButton_->setEnabled(effectUsesColor(effectName));
}

void LightingStudioWindow::repopulateFromActiveProfile()
{
    Profile* profile = settings_.activeProfile(); // never null; ensureProfileAndEntries() runs before every call site

    {
        // Rebuilt from scratch every call (Task 23: profiles.size() can now
        // change between calls -- create/rename/delete, not just switch --
        // matching TrayApp::setAvailableProfiles' own "rebuild, don't diff"
        // precedent). Blocked so clear()'s own currentIndexChanged(-1)
        // never reaches onProfileComboChanged (which would otherwise set
        // activeProfileName to an empty string).
        const QSignalBlocker blocker(profileCombo_);
        profileCombo_->clear();
        for (const Profile& p : settings_.profiles) {
            profileCombo_->addItem(QString::fromStdString(p.name));
        }
        const int idx = profileCombo_->findText(QString::fromStdString(profile->name));
        if (idx >= 0) profileCombo_->setCurrentIndex(idx);
    }
    // Guard against deleting the last remaining profile (Settings::
    // removeActiveProfile() already refuses it; this keeps the button
    // itself from inviting the attempt).
    deleteProfileButton_->setEnabled(settings_.profiles.size() > 1);

    for (const std::string& channelName : channelNames_) {
        const ChannelSettings& cs = channelSettingsFor(channelName);
        ChannelRow& row = rows_.at(channelName);
        {
            const QSignalBlocker modeBlocker(row.modeCombo);
            const QSignalBlocker sourceBlocker(row.sourceCombo);
            const QSignalBlocker effectBlocker(row.effectCombo);
            const QSignalBlocker nativeModeBlocker(row.nativeModeCombo);
            // findData, not a hardcoded index: cs.mode is guaranteed to be
            // one of this combo's items by ensureProfileAndEntries()'s own
            // clamp, but falling back to index 0 defensively costs nothing.
            const int modeIdx = row.modeCombo->findData(static_cast<int>(cs.mode));
            row.modeCombo->setCurrentIndex(modeIdx >= 0 ? modeIdx : 0);
            row.sourceCombo->setCurrentIndex(cs.source == CanvasSource::GlobalFollow ? 1 : 0);
            const int effectIdx = row.effectCombo->findText(QString::fromStdString(cs.effectName));
            row.effectCombo->setCurrentIndex(effectIdx >= 0 ? effectIdx : 0);
            // If cs.nativeSettings.lightingMode isn't one of this combo's
            // items (e.g. a hand-edited file set it to Advanced, which this
            // Native-mode combo deliberately excludes -- see
            // kNativeLightingModes), just show index 0; don't write
            // anything back into cs (this block is signal-blocked, so
            // onRowChanged can't fire from this anyway).
            const int nativeModeIdx = row.nativeModeCombo->findData(static_cast<int>(cs.nativeSettings.lightingMode));
            row.nativeModeCombo->setCurrentIndex(nativeModeIdx >= 0 ? nativeModeIdx : 0);
        }
        updateColorSwatch(row.colorButton, cs.staticColor);
        updateColorSwatch(row.nativeColorButton, cs.nativeSettings.color1);
        updateRowEnabledState(channelName);
        applyChannelToCanvas(channelName, cs);
    }

    {
        const QSignalBlocker blocker(globalEffectCombo_);
        // No index-0 fallback here (unlike the row effect combos): a fresh
        // profile's globalEffect.effectName is "" (no global effect), and
        // displaying "StaticColor" for it made picking StaticColor a
        // same-index no-op that fired no signal -- the user saw StaticColor
        // selected, a disabled color swatch, and no way to enable it short of
        // picking another effect first (Task 23 review, follow-up to
        // 17d9a4f). -1 shows an empty selection that matches the stored value.
        const int idx = globalEffectCombo_->findText(QString::fromStdString(profile->globalEffect.effectName));
        globalEffectCombo_->setCurrentIndex(idx);
    }
    updateColorSwatch(globalColorButton_, profile->globalEffect.color);
    updateGlobalColorButtonEnabledState();
    applyGlobalEffectToCanvas(profile->globalEffect);
}

void LightingStudioWindow::onRowChanged(const std::string& channelName)
{
    const ChannelRow& row = rows_.at(channelName);
    ChannelSettings& cs = channelSettingsFor(channelName);
    cs.mode = static_cast<ChannelMode>(row.modeCombo->currentData().toInt());
    cs.source = (row.sourceCombo->currentIndex() == 1) ? CanvasSource::GlobalFollow : CanvasSource::Independent;
    // Settings.h documents effectName as "empty if mode == Native" --
    // switching back to Canvas just re-reads the combo, so clearing here
    // costs nothing and keeps persisted Native-mode entries matching that
    // contract.
    cs.effectName = (cs.mode == ChannelMode::Native) ? std::string() : row.effectCombo->currentText().toStdString();
    // Minimal Native-mode editor (Part 2): lightingMode always synced from
    // its combo, regardless of cs.mode -- harmless since the field is
    // documented "meaningful only when mode == Native", and keeps this
    // handler simple (no extra gating needed).
    cs.nativeSettings.lightingMode = static_cast<MsiLightingMode>(row.nativeModeCombo->currentData().toInt());
    // placement comes from dragging on the canvas (onCanvasChannelPlacementChanged),
    // not from this row; staticColor/nativeSettings.color1 are edited via
    // their own dedicated swatch buttons (setChannelColor/
    // setChannelNativeColor), not this handler.

    // Called AFTER cs is written, not before: updateRowEnabledState()'s
    // colorMeaningful check now reads cs.effectName (see its own comment),
    // so it needs the just-written value, not the previous one.
    updateRowEnabledState(channelName);

    applyChannelToCanvas(channelName, cs);
    emit channelAssignmentChanged(channelName);
}

void LightingStudioWindow::setChannelColor(const std::string& channelName, const MsiColor& color)
{
    ChannelSettings& cs = channelSettingsFor(channelName);
    cs.staticColor = color;
    updateColorSwatch(rows_.at(channelName).colorButton, color);
    applyChannelToCanvas(channelName, cs);
    emit channelAssignmentChanged(channelName);
}

void LightingStudioWindow::setChannelNativeColor(const std::string& channelName, const MsiColor& color)
{
    ChannelSettings& cs = channelSettingsFor(channelName);
    cs.nativeSettings.color1 = color;
    updateColorSwatch(rows_.at(channelName).nativeColorButton, color);
    applyChannelToCanvas(channelName, cs);
    emit channelAssignmentChanged(channelName);
}

void LightingStudioWindow::setGlobalEffectColor(const MsiColor& color)
{
    Profile* profile = settings_.activeProfile();
    if (!profile) return;
    profile->globalEffect.color = color;
    updateColorSwatch(globalColorButton_, color);
    applyGlobalEffectToCanvas(profile->globalEffect);
    emit globalEffectChanged();
}

void LightingStudioWindow::createProfile(const std::string& name)
{
    if (!settings_.addProfile(name)) return; // empty/duplicate name
    settings_.activeProfileName = name; // a newly created profile becomes the active one, as the user expects
    refreshFromSettings(); // ensureProfileAndEntries() fills in every channel's defaults for it
}

void LightingStudioWindow::renameActiveProfile(const std::string& newName)
{
    if (!settings_.renameActiveProfile(newName)) return;
    // The ACTIVE profile itself doesn't change identity here (just its
    // name), but profileCombo_'s items and main.cpp's tray submenu both
    // need to catch up -- refreshFromSettings() already rebuilds
    // profileCombo_ from scratch and fires activeProfileChanged
    // unconditionally, which is exactly what's needed here too.
    refreshFromSettings();
}

void LightingStudioWindow::deleteActiveProfile()
{
    if (!settings_.removeActiveProfile()) return; // guarded against deleting the last remaining profile
    refreshFromSettings();
}

void LightingStudioWindow::onGlobalEffectComboChanged()
{
    Profile* profile = settings_.activeProfile();
    profile->globalEffect.effectName = globalEffectCombo_->currentText().toStdString();
    updateGlobalColorButtonEnabledState();

    applyGlobalEffectToCanvas(profile->globalEffect);
    emit globalEffectChanged();
}

void LightingStudioWindow::onProfileComboChanged()
{
    settings_.activeProfileName = profileCombo_->currentText().toStdString();
    refreshFromSettings();
}

// Shared by both the in-UI path (onProfileComboChanged, after it writes
// profileCombo_'s current text into settings_.activeProfileName) and the
// external path (a caller -- e.g. Task 18's tray profile-switch wiring --
// that already changed settings_.activeProfileName itself, via the
// Settings& this window holds, and now needs this window to catch up).
void LightingStudioWindow::refreshFromSettings()
{
    ensureProfileAndEntries(); // the newly active profile may be missing entries for some channelNames_
    repopulateFromActiveProfile();

    // Every channel's assignment changed at once (new profile, new
    // mode/source/effect/placement for all of them), so this fires
    // channelAssignmentChanged once per channel rather than silently
    // skipping to just activeProfileChanged -- per the header's documented
    // contract, listeners (Task 18) react to each individually just like a
    // row edit, plus the shared global effect.
    for (const std::string& channelName : channelNames_) {
        emit channelAssignmentChanged(channelName);
    }
    emit globalEffectChanged();
    emit activeProfileChanged(settings_.activeProfileName);
}

void LightingStudioWindow::onCanvasChannelPlacementChanged(const std::string& channelName, CanvasPlacement newPlacement)
{
    // The widget has already applied this placement to its own channels_
    // entry (see LightingCanvasWidget::mouseMoveEvent) -- this handler only
    // persists it into Settings, it must not call setChannel() back in.
    channelSettingsFor(channelName).placement = newPlacement;
    emit channelAssignmentChanged(channelName);
}

void LightingStudioWindow::onCanvasGlobalReferencePointChanged(int x, int y)
{
    // Likewise already applied to the live effect object in place (see
    // LightingCanvasWidget::mouseMoveEvent) -- persist only.
    Profile* profile = settings_.activeProfile();
    profile->globalEffect.referenceX = x;
    profile->globalEffect.referenceY = y;
    emit globalEffectChanged();
}
