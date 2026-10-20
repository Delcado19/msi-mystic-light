#include "ui/LightingStudioWindow.h"

#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
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

const std::vector<std::string> kModeNames = {"Native", "Canvas"};
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

QComboBox* makeCombo(QWidget* parent, const QString& objectName, const std::vector<std::string>& items)
{
    auto* combo = new QComboBox(parent);
    combo->setObjectName(objectName);
    for (const std::string& item : items) combo->addItem(QString::fromStdString(item));
    return combo;
}

} // namespace

LightingStudioWindow::LightingStudioWindow(Settings& settings, std::vector<std::string> channelNames, QWidget* parent)
    : QWidget(parent)
    , settings_(settings)
    , channelNames_(std::move(channelNames))
{
    setWindowTitle(QStringLiteral("Lighting Studio"));

    canvasWidget_ = new LightingCanvasWidget(this);
    canvasWidget_->setMinimumSize(420, 320);

    profileCombo_ = new QComboBox(this);
    profileCombo_->setObjectName(QStringLiteral("profileCombo"));

    globalEffectCombo_ = makeCombo(this, QStringLiteral("globalEffectCombo"), kEffectNames);

    auto* panel = new QWidget(this);
    panel->setMinimumWidth(kPanelMinWidth);
    auto* panelLayout = new QVBoxLayout(panel);
    panelLayout->setSpacing(kPanelSpacing);

    auto* profileRow = new QHBoxLayout();
    profileRow->addWidget(new QLabel(QStringLiteral("Profile"), panel));
    profileRow->addWidget(profileCombo_, /*stretch=*/1);
    panelLayout->addLayout(profileRow);

    auto* globalRow = new QHBoxLayout();
    globalRow->addWidget(new QLabel(QStringLiteral("Global Effect"), panel));
    globalRow->addWidget(globalEffectCombo_, /*stretch=*/1);
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

    int gridRow = 0;
    for (const std::string& channelName : channelNames_) {
        ChannelRow row;
        const QString qname = QString::fromStdString(channelName);
        row.modeCombo = makeCombo(rowsContainer, QStringLiteral("mode:") + qname, kModeNames);
        row.sourceCombo = makeCombo(rowsContainer, QStringLiteral("source:") + qname, kSourceNames);
        row.effectCombo = makeCombo(rowsContainer, QStringLiteral("effect:") + qname, kEffectNames);

        grid->addWidget(new QLabel(qname, rowsContainer), gridRow, 0);
        grid->addWidget(row.modeCombo, gridRow, 1);
        grid->addWidget(row.sourceCombo, gridRow, 2);
        grid->addWidget(row.effectCombo, gridRow, 3);
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
    // (the brief's own construction test relies on this).
    ensureProfileAndEntries();
    for (const Profile& p : settings_.profiles) {
        profileCombo_->addItem(QString::fromStdString(p.name));
    }
    repopulateFromActiveProfile();

    connect(profileCombo_, &QComboBox::currentIndexChanged, this, &LightingStudioWindow::onProfileComboChanged);
    connect(globalEffectCombo_, &QComboBox::currentIndexChanged, this, &LightingStudioWindow::onGlobalEffectComboChanged);
    connect(canvasWidget_, &LightingCanvasWidget::channelPlacementChanged, this, &LightingStudioWindow::onCanvasChannelPlacementChanged);
    connect(canvasWidget_, &LightingCanvasWidget::globalEffectReferencePointChanged, this, &LightingStudioWindow::onCanvasGlobalReferencePointChanged);

    for (auto& [channelName, row] : rows_) {
        connect(row.modeCombo, &QComboBox::currentIndexChanged, this, [this, channelName] { onRowChanged(channelName); });
        connect(row.sourceCombo, &QComboBox::currentIndexChanged, this, [this, channelName] { onRowChanged(channelName); });
        connect(row.effectCombo, &QComboBox::currentIndexChanged, this, [this, channelName] { onRowChanged(channelName); });
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
        const bool exists = std::any_of(profile->channelSettings.begin(), profile->channelSettings.end(),
            [&name](const ChannelSettings& cs) { return cs.channelName == name; });
        if (exists) continue;

        ChannelSettings cs;
        cs.channelName = name;
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
    // handler can run. Defensive fallback instead of dereferencing nothing.
    ChannelSettings cs;
    cs.channelName = channelName;
    profile->channelSettings.push_back(std::move(cs));
    return profile->channelSettings.back();
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
    const bool isCanvas = row.modeCombo->currentIndex() == 1;
    row.sourceCombo->setEnabled(isCanvas);
    const bool isIndependent = row.sourceCombo->currentIndex() == 0;
    row.effectCombo->setEnabled(isCanvas && isIndependent);
}

void LightingStudioWindow::repopulateFromActiveProfile()
{
    Profile* profile = settings_.activeProfile(); // never null; ensureProfileAndEntries() runs before every call site

    {
        const QSignalBlocker blocker(profileCombo_);
        const int idx = profileCombo_->findText(QString::fromStdString(profile->name));
        if (idx >= 0) profileCombo_->setCurrentIndex(idx);
    }

    for (const std::string& channelName : channelNames_) {
        const ChannelSettings& cs = channelSettingsFor(channelName);
        ChannelRow& row = rows_.at(channelName);
        {
            const QSignalBlocker modeBlocker(row.modeCombo);
            const QSignalBlocker sourceBlocker(row.sourceCombo);
            const QSignalBlocker effectBlocker(row.effectCombo);
            row.modeCombo->setCurrentIndex(cs.mode == ChannelMode::Canvas ? 1 : 0);
            row.sourceCombo->setCurrentIndex(cs.source == CanvasSource::GlobalFollow ? 1 : 0);
            const int effectIdx = row.effectCombo->findText(QString::fromStdString(cs.effectName));
            row.effectCombo->setCurrentIndex(effectIdx >= 0 ? effectIdx : 0);
        }
        updateRowEnabledState(channelName);
        applyChannelToCanvas(channelName, cs);
    }

    {
        const QSignalBlocker blocker(globalEffectCombo_);
        const int idx = globalEffectCombo_->findText(QString::fromStdString(profile->globalEffect.effectName));
        globalEffectCombo_->setCurrentIndex(idx >= 0 ? idx : 0);
    }
    applyGlobalEffectToCanvas(profile->globalEffect);
}

void LightingStudioWindow::onRowChanged(const std::string& channelName)
{
    updateRowEnabledState(channelName);

    const ChannelRow& row = rows_.at(channelName);
    ChannelSettings& cs = channelSettingsFor(channelName);
    cs.mode = (row.modeCombo->currentIndex() == 1) ? ChannelMode::Canvas : ChannelMode::Native;
    cs.source = (row.sourceCombo->currentIndex() == 1) ? CanvasSource::GlobalFollow : CanvasSource::Independent;
    // Settings.h documents effectName as "empty if mode == Native" --
    // switching back to Canvas just re-reads the combo, so clearing here
    // costs nothing and keeps persisted Native-mode entries matching that
    // contract.
    cs.effectName = (cs.mode == ChannelMode::Native) ? std::string() : row.effectCombo->currentText().toStdString();
    // placement comes from dragging on the canvas (onCanvasChannelPlacementChanged),
    // not from this row; staticColor/nativeSettings have no editing UI in
    // this pass (deferred -- see task-15-report.md) and are left as-is.

    applyChannelToCanvas(channelName, cs);
    emit channelAssignmentChanged(channelName);
}

void LightingStudioWindow::onGlobalEffectComboChanged()
{
    Profile* profile = settings_.activeProfile();
    profile->globalEffect.effectName = globalEffectCombo_->currentText().toStdString();

    applyGlobalEffectToCanvas(profile->globalEffect);
    emit globalEffectChanged();
}

void LightingStudioWindow::onProfileComboChanged()
{
    settings_.activeProfileName = profileCombo_->currentText().toStdString();
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
