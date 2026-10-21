#include "ui/LayoutEditor.h"
#include "ui/HardwareLibrary.h"
#include "ui/HardwarePicker.h"
#include "canvas/LayoutGeometry.h"
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCompleter>
#include "ui/LightingStudioWindow.h"

#include "ui/DeviceLedWidget.h"
#include <QTabWidget>
#include <QCheckBox>
#include <QHideEvent>
#include <QScrollArea>
#include <QDialog>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QRegularExpression>
#include <QApplication>
#include <QFormLayout>
#include <QListWidget>
#include <QSplitter>
#include <QStackedWidget>
#include <QFont>
#include <QColor>
#include "ui/ColorPicker.h"
#include "ui/EffectEditor.h"
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMenu>
#include <QToolButton>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <cmath>
#include <algorithm>
#include <memory>
#include <utility>

#include "effects/EffectFactory.h"
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

constexpr int kSwatchSize = 40; // small square color-swatch buttons, row and global alike

// Default staticColor for a freshly-created peripheral channel (Task 23):
// full white. Peripherals default to Canvas mode (see
// LightingStudioWindow::makeDefaultChannelSettings) since Native is a
// confirmed permanent no-op for all 4 peripheral drivers -- leaving
// staticColor at its own MsiColor{} default (black) would mean the device is
// "configured" but still invisible out of the box, defeating the point of
// defaulting to Canvas at all. White is simplest/most visible; any
// non-black color would satisfy the same requirement.
constexpr MsiColor kDefaultPeripheralColor{255, 255, 255};

const std::vector<std::string> kSourceNames = {"Own effect", "Layout effect"};
// "StaticColor" matches the name already used by Settings/main.cpp (not
// "Fill" -- see task-15-brief.md note 4, resolved against the existing
// codebase convention).
const std::vector<std::string> kEffectNames = {
    "StaticColor", "RainbowWave", "Breathing", "RadialRainbow",
    "Spiral", "Comet", "Marquee", "SwirlCircles", "Aurora", "Ocean", "Ember", "Pastel", "Polar", "Sunset", "Wave", "Vertical", "Diagonal", "Radial", "Plasma", "Sweep", "Static gradient"};

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
// obviously-named pair added where the color picker first needs both
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
    const QColor fill = toQColor(color);
    // WCAG relative luminance picks readable black/white text for any RGB.
    const auto linear = [](double value) { return value <= 0.04045 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4); };
    const double light = 0.2126 * linear(fill.redF()) + 0.7152 * linear(fill.greenF()) + 0.0722 * linear(fill.blueF());
    button->setText(fill.name().toUpper());
    button->setAccessibleName(QStringLiteral("Color %1").arg(fill.name()));
    button->setStyleSheet(QStringLiteral(
        "QPushButton { background: %1; color: %2; border: 1px solid #697485; "
        "border-radius: 9px; padding: 8px 12px; font-weight: 600; } "
        "QPushButton:focus { border: 2px solid #71e2c5; } "
        "QPushButton:disabled { background: #242a33; color: #a3adbb; border-color: #424c5b; }")
        .arg(fill.name(), light > 0.179 ? QStringLiteral("#10151b") : QStringLiteral("#ffffff")));

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

    resize(1440, 880);
    setMinimumSize(1080, 700);
    QFont studioFont(QStringLiteral("Segoe UI"), 10);
    setFont(studioFont);
    setObjectName(QStringLiteral("lightingStudio"));
    setStyleSheet(QStringLiteral(R"(
        QWidget#lightingStudio { background: #11151b; color: #edf2f7; }
        QWidget { color: #edf2f7; font-family: 'Segoe UI'; }
        QFrame#sidebar, QFrame#inspector { background: #181e27; border: 1px solid #303947; border-radius: 14px; }
        QFrame#scenePanel { background: #131922; border: 1px solid #303947; border-radius: 14px; }
        QDialog, QMenu { background: #181e27; }
        QMenu::item { padding: 8px 18px; } QMenu::item:selected { background: #30483f; }
        QLineEdit, QSpinBox { background: #252d39; color: #edf2f7; border: 1px solid #69798b; border-radius: 6px; padding: 8px; }
        QLineEdit:focus, QSpinBox:focus { border: 2px solid #71e2c5; }
        QLabel { background: transparent; }
        QLabel[role="eyebrow"] { color: #9eabba; font-size: 11px; font-weight: 600; }
        QLabel[role="title"] { font-size: 19px; font-weight: 600; }
        QLabel[role="section"] { font-size: 17px; font-weight: 600; }
        QLabel[role="muted"] { color: #a9b4c3; }
        QComboBox, QPushButton, QToolButton { min-height: 22px; padding: 8px 10px; border: 1px solid #424c5b; border-radius: 8px; background: #252d39; }
        QComboBox:hover, QPushButton:hover { border-color: #8a9bad; background: #303b4a; }
        QPushButton:pressed { background: #35475a; }
        QPushButton[primary="true"] { background: #71e2c5; color: #10231d; border-color: #71e2c5; }
        QComboBox:focus, QPushButton:focus, QListWidget:focus { border: 2px solid #71e2c5; }
        QComboBox:disabled, QPushButton:disabled { color: #9ba6b5; background: #202630; border-color: #343e4c; }
        QComboBox::drop-down { border: none; width: 25px; }
        QComboBox::down-arrow { image: url(:/studio/chevron.svg); width: 12px; height: 8px; }
        QComboBox QAbstractItemView { background: #252d39; color: #edf2f7; selection-background-color: #365d58; padding: 6px; }
        QListWidget { background: transparent; border: none; outline: 0; padding: 2px; }
        QListWidget::item { padding: 8px 10px; margin: 3px 0; border: 1px solid transparent; border-radius: 9px; }
        QListWidget::item:hover { background: #252f3c; }
        QListWidget::item:selected { background: #253c3b; border: 1px solid #5bbfa7; color: #e0fff5; }
        QScrollArea, QStackedWidget { background: transparent; border: none; }
        QTabWidget::pane { border: none; }
        QTabBar::tab { background: #202a36; color: #a9b4c3; padding: 9px 15px; }
        QTabBar::tab:selected { background: #314a46; color: #e0fff5; }
        QScrollBar:vertical { background: #181e27; width: 8px; }
        QScrollBar::handle:vertical { background: #526071; min-height: 30px; border-radius: 4px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QSplitter::handle { background: #11151b; width: 10px; }
        QToolTip { background: #263341; color: #f1f5fa; border: 1px solid #69798b; padding: 7px; }
    )"));
    auto label = [](const QString& text, const char* role, QWidget* parent) {
        auto* result = new QLabel(text, parent);
        result->setProperty("role", role);
        result->setWordWrap(QByteArray(role) == "muted");
        return result;
    };
    auto deviceName = [](const std::string& name) -> QString {
        static const std::map<std::string, QString> names = {
            {"JARGB1", "ARGB header 1"}, {"JARGB2", "ARGB header 2"}, {"JARGB3", "ARGB header 3"},
            {"EZConnector", "EZ Connector"}, {"IOShield", "I/O shield"}, {"JRGB1", "RGB header"},
            {"RoccatVulcan", "Roccat Vulcan"}, {"RoccatKone", "Roccat Kone Aimo"},
            {"RazerBasiliskV3", "Razer Basilisk V3"}, {"SteelSeriesApex3", "SteelSeries Apex 3"}};
        const auto it = names.find(name);
        return it == names.end() ? QString::fromStdString(name) : it->second;
    };
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 12, 16, 10);
    root->setSpacing(12);
    auto* header = new QHBoxLayout();
    auto* brand = new QVBoxLayout();
    brand->setSpacing(3);
    brand->addWidget(label(QStringLiteral("Lighting Studio"), "title", this));
    header->addLayout(brand, 1);
    header->addStretch();
    auto* localBadge = label(QStringLiteral("LOCAL CONTROL  /  LIVE CHANGES"), "eyebrow", this);
    localBadge->hide();
    root->addLayout(header);
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setChildrenCollapsible(false);
    splitter->setHandleWidth(10);
    root->addWidget(splitter, 1);

    auto* sidebar = new QFrame(splitter);
    sidebar->setObjectName(QStringLiteral("sidebar"));
    sidebar->setMinimumWidth(225);
    auto* sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(16, 20, 16, 16);
    sideLayout->setSpacing(12);
    sideLayout->addWidget(label(QStringLiteral("WORKSPACE"), "eyebrow", sidebar));
    profileCombo_ = new QComboBox(sidebar);
    profileCombo_->setObjectName(QStringLiteral("profileCombo"));
    profileCombo_->setAccessibleName(QStringLiteral("Lighting profile"));
    sideLayout->addWidget(profileCombo_);
    auto* profileActions = new QHBoxLayout();
    newProfileButton_ = new QPushButton(QStringLiteral("New"), sidebar);
    newProfileButton_->setObjectName(QStringLiteral("newProfileButton"));
    renameProfileButton_ = new QPushButton(QStringLiteral("Rename"), sidebar);
    renameProfileButton_->setObjectName(QStringLiteral("renameProfileButton"));
    deleteProfileButton_ = new QPushButton(QStringLiteral("Delete"), sidebar);
    deleteProfileButton_->setObjectName(QStringLiteral("deleteProfileButton"));
    for (auto* button : {newProfileButton_, renameProfileButton_, deleteProfileButton_}) {
        button->setAccessibleName(button->text() + QStringLiteral(" profile"));
        button->hide();
    }
    delete profileActions;
    auto* profileMenu=new QMenu(this);for(auto* button:{newProfileButton_,renameProfileButton_,deleteProfileButton_}){
        auto* action=profileMenu->addAction(button->text()+" profile");connect(action,&QAction::triggered,button,&QPushButton::click);
    }
    connect(profileMenu,&QMenu::aboutToShow,this,[this,profileMenu]{int i=0;for(auto* button:{newProfileButton_,renameProfileButton_,deleteProfileButton_})profileMenu->actions()[i++]->setEnabled(button->isEnabled());});
    auto* manageProfile=new QToolButton(sidebar);manageProfile->setText("Profile options");manageProfile->setPopupMode(QToolButton::InstantPopup);manageProfile->setMenu(profileMenu);sideLayout->addWidget(manageProfile);
    sideLayout->addSpacing(10);
    auto* deviceHeading=label(QStringLiteral("DEVICES"), "eyebrow", sidebar);sideLayout->addWidget(deviceHeading);
    deviceList_ = new QListWidget(sidebar);
    deviceList_->setObjectName(QStringLiteral("deviceList"));
    deviceList_->setAccessibleName(QStringLiteral("Devices and motherboard zones"));
    sideLayout->addWidget(deviceList_, 1);
    auto* effectHelp=label("Effects light the whole layout. Create a palette here, then place your hardware in Layout.","muted",sidebar);sideLayout->addWidget(effectHelp,1);effectHelp->setAlignment(Qt::AlignTop|Qt::AlignLeft);effectHelp->hide();
    sideLayout->addWidget(label(QStringLiteral("GPU lighting is not supported in this build."), "muted", sidebar));

    auto* scenePanel = new QFrame(splitter);
    scenePanel->setObjectName(QStringLiteral("scenePanel"));
    auto* sceneLayout = new QVBoxLayout(scenePanel);
    sceneLayout->setContentsMargins(14, 14, 14, 12);
    sceneLayout->setSpacing(10);
    auto* sceneHeading = new QHBoxLayout();
    auto* viewTitle=label("Layout","section",scenePanel);sceneHeading->addWidget(viewTitle);
    sceneHeading->addStretch();
    layoutEffectBadge_=label("","muted",scenePanel);sceneHeading->addWidget(layoutEffectBadge_);
    auto* previewButton = new QPushButton(QStringLiteral("Pause preview"), scenePanel);
    previewButton->setObjectName(QStringLiteral("previewToggle"));
    previewButton->setCheckable(true);
    previewButton->setToolTip(QStringLiteral("Pause only the on-screen animation. Hardware lighting continues."));
    sceneHeading->addWidget(previewButton);
    sceneLayout->addLayout(sceneHeading);
    auto* viewHelp=label("Drag hardware to position it over the colors. Select a device to resize or rotate it.","muted",scenePanel);sceneLayout->addWidget(viewHelp);
    canvasWidget_ = new LightingCanvasWidget(scenePanel);
    canvasWidget_->setMinimumSize(340, 320);
    canvasWidget_->setFitToView(true);
    auto* views = new QTabWidget(scenePanel);workspaceViews_=views;
    views->setObjectName(QStringLiteral("workspaceViews"));
    views->addTab(canvasWidget_, QStringLiteral("Layout"));
    auto* detailPanel=new QWidget(views);auto* detailLayout=new QVBoxLayout(detailPanel);detailLayout->setContentsMargins(0,0,0,0);
    ledMap_ = new DeviceLedWidget(detailPanel);detailLayout->addWidget(ledMap_,1);
    auto* ledDialog=new QDialog(this);ledDialog->setObjectName("ledRegionsDialog");ledDialog->setWindowTitle("Device LED colors");ledDialog->resize(920,520);
    auto* ledDialogLayout=new QVBoxLayout(ledDialog);ledDialogLayout->addWidget(detailPanel);
    connect(ledDialog,&QDialog::finished,this,[this]{stopRegionIdentification();});
    hardwarePages_=new QStackedWidget(views);hardwarePages_->setObjectName("hardwarePages");
    auto* effectsPanel=new QWidget(views);auto* effectsLayout=new QVBoxLayout(effectsPanel);views->addTab(effectsPanel,"Effects");views->addTab(hardwarePages_,"Hardware");
    sceneLayout->addWidget(views, 1);
    ledMap_->regionClicked = [this](int index) {
        const int selected = deviceList_->currentRow();
        if (selected < 0) return;
        const auto name = channelNames_[selected];
        if(identifyToggle_->isChecked()) { identifyRegion(name,index); return; }
        auto& cs = channelSettingsFor(name);
        const auto it = cs.ledOverrides.find(index);
        const QColor initial = toQColor(it == cs.ledOverrides.end() ? cs.staticColor : it->second);
        const QColor chosen = ColorPicker::getColor(initial, this, QStringLiteral("Key / LED region color"));
        if (chosen.isValid()) setRegionColor(name,index,toMsiColor(chosen));
    };
    auto* resetRegions = new QPushButton(QStringLiteral("Clear fixed key / region colors"), scenePanel);
    resetRegions->setObjectName(QStringLiteral("clearRegionColors"));
    detailLayout->addWidget(resetRegions);
    identifyToggle_ = new QCheckBox(QStringLiteral("Identify clicked region: white only, other regions off"),scenePanel);
    identifyToggle_->setObjectName(QStringLiteral("identifyRegionToggle"));
    identifyToggle_->setToolTip(QStringLiteral("Temporary test. Uncheck, switch profiles, or close the window to restore your effect."));
    detailLayout->addWidget(identifyToggle_);
    connect(identifyToggle_,&QCheckBox::toggled,this,[this](bool enabled){if(!enabled)stopRegionIdentification();});

    connect(resetRegions, &QPushButton::clicked, this, [this] {
        const int index = deviceList_->currentRow(); if (index < 0) return;
        auto& cs = channelSettingsFor(channelNames_[index]);
        if (cs.ledOverrides.empty()) return;
        cs.ledOverrides.clear(); updateSelectedDevice();
        emit channelAssignmentChanged(cs.channelName);
    });

    previewButton->setChecked(canvasWidget_->previewPaused());
    ledMap_->setPreviewPaused(canvasWidget_->previewPaused());
    previewButton->setText(previewButton->isChecked() ? QStringLiteral("Resume preview") : QStringLiteral("Pause preview"));
    connect(previewButton, &QPushButton::toggled, this, [this, previewButton](bool paused) {
        canvasWidget_->setPreviewPaused(paused);
        ledMap_->setPreviewPaused(paused);
        previewButton->setText(paused ? QStringLiteral("Resume preview") : QStringLiteral("Pause preview"));
    });
    auto* globalPanel = new QFrame(scenePanel);
    auto* globalLayout = new QVBoxLayout(globalPanel);
    globalLayout->setContentsMargins(0, 8, 0, 0);
    globalLayout->addWidget(label(QStringLiteral("SHARED EFFECT"), "eyebrow", globalPanel));
    auto* globalRow = new QHBoxLayout();
    globalEffectCombo_ = makeCombo(this, QStringLiteral("globalEffectCombo"), kEffectNames);
    globalEffectCombo_->setPlaceholderText(QStringLiteral("Choose a shared effect"));
    globalEffectCombo_->setAccessibleName(QStringLiteral("Shared lighting effect"));
    globalColorButton_ = new QPushButton(this);
    globalColorButton_->setObjectName(QStringLiteral("globalColorButton"));
    globalColorButton_->setMinimumHeight(kSwatchSize);
    globalColorButton_->setToolTip(QStringLiteral("Shared effect color"));
    globalRow->addWidget(globalEffectCombo_, 1);
    globalRow->addWidget(globalColorButton_);
    globalLayout->addLayout(globalRow);
    globalLayout->addWidget(label(QStringLiteral("Devices using Canvas sample the colors beneath their positions."), "muted", globalPanel));
    effectsLayout->addWidget(globalPanel);
    auto* effectPreview=new EffectPreview(&previewRecipe_,globalPanel);effectPreview->setObjectName("selectedEffectPreview");globalLayout->addWidget(effectPreview,1);
    auto* effectActions=globalRow;
    auto* createEffect = new QPushButton("New effect",globalPanel);createEffect->setObjectName("createEffectButton");createEffect->setProperty("primary",true);effectActions->addWidget(createEffect);
    auto* editEffect=new QPushButton("Edit effect",globalPanel);editEffect->setObjectName("editEffectButton");effectActions->addWidget(editEffect);
    const auto openEffect=[this](bool create){
        const auto* profile=settings_.activeProfile();if(!profile)return;
        auto definition=effectDefinition(profile->globalEffect.effectName,profile->globalEffect.color,settings_.customEffects);
        if(create){definition={"My effect",{{255,0,80},{0,190,255},{100,255,60}},6000,720};int i=1;const auto exists=[this](const std::string& name){return std::any_of(settings_.customEffects.begin(),settings_.customEffects.end(),[&](const auto& d){return d.name==name;});};while(exists(definition.name))definition.name="My effect "+std::to_string(++i);}
        if(definition.name.empty())definition.name="My effect";
        definition.path="Canvas positions";
        EffectEditor editor(definition,this);editor.findChild<QComboBox*>("effectPath")->setEnabled(false);editor.saveEffect=[this](const auto& d){return createCustomEffect(d);};editor.exec();
    };
    connect(createEffect,&QPushButton::clicked,this,[openEffect]{openEffect(true);});connect(editEffect,&QPushButton::clicked,this,[openEffect]{openEffect(false);});
    auto* inspector = new QFrame(splitter);
    inspector->setObjectName(QStringLiteral("inspector"));
    inspector->setMinimumWidth(270);
    auto* inspectorLayout = new QVBoxLayout(inspector);
    inspectorLayout->setContentsMargins(20, 20, 20, 20);
    inspectorLayout->addWidget(label(QStringLiteral("SELECTED DEVICE"), "eyebrow", inspector));
    devicePages_ = new QStackedWidget(inspector);
    devicePages_->setObjectName(QStringLiteral("deviceInspector"));
    auto* inspectorScroll = new QScrollArea(inspector);
    inspectorScroll->setWidgetResizable(true); inspectorScroll->setFrameShape(QFrame::NoFrame);
    inspectorScroll->setWidget(devicePages_);
    inspectorLayout->addWidget(inspectorScroll, 1);
    for (const std::string& channelName : channelNames_) {
        ChannelRow row;
        const QString qname = QString::fromStdString(channelName);
        deviceList_->addItem(deviceName(channelName));
        deviceList_->item(deviceList_->count() - 1)->setToolTip(qname);
        auto* page = new QWidget(devicePages_);
        auto* pageLayout = new QVBoxLayout(page);
        pageLayout->setContentsMargins(0, 16, 0, 0);
        pageLayout->setSpacing(16);
        pageLayout->addWidget(label(deviceName(channelName), "section", page));
        const auto modes = availableModesFor(channelName);
        const bool nativeOnly = modes.size() == 1 && modes.front() == ChannelMode::Native;
        pageLayout->addWidget(label(nativeOnly ? QStringLiteral("Device effects only") : QStringLiteral("Individual or shared lighting"), "muted", page));
        auto* form = new QFormLayout();
        row.form = form;
        form->setRowWrapPolicy(QFormLayout::WrapAllRows);
        form->setVerticalSpacing(12);
        row.modeCombo = new QComboBox(page);
        row.modeCombo->setObjectName(QStringLiteral("mode:") + qname);
        // Display names distinguish control from the shared layout; enum data stays compatible.
        for (ChannelMode mode : modes) row.modeCombo->addItem(mode==ChannelMode::Native?"Device effects":"Software lighting", static_cast<int>(mode));
        row.sourceCombo = makeCombo(page, QStringLiteral("source:") + qname, kSourceNames);
        row.effectCombo = makeCombo(page, QStringLiteral("effect:") + qname, kEffectNames);
        row.colorButton = new QPushButton(page);
        row.colorButton->setObjectName(QStringLiteral("color:") + qname);
        row.colorButton->setMinimumHeight(kSwatchSize);
        row.nativeModeCombo = new QComboBox(page);
        row.nativeModeCombo->setObjectName(QStringLiteral("nativeMode:") + qname);
        for (const auto& [mode, text] : kNativeLightingModes) row.nativeModeCombo->addItem(QString::fromStdString(text), static_cast<int>(mode));
        row.nativeColorButton = new QPushButton(page);
        row.nativeColorButton->setObjectName(QStringLiteral("nativeColor:") + qname);
        row.nativeColorButton->setMinimumHeight(kSwatchSize);
        form->addRow(QStringLiteral("Lighting mode"), row.modeCombo);
        form->addRow(QStringLiteral("Color source"), row.sourceCombo);
        form->addRow(QStringLiteral("Effect"), row.effectCombo);
        form->addRow(QStringLiteral("Color"), row.colorButton);
        form->addRow(QStringLiteral("Device effect"), row.nativeModeCombo);
        form->addRow(QStringLiteral("Device color"), row.nativeColorButton);
        for (int i = 0; i < form->rowCount(); ++i) {
            QWidget* field = form->itemAt(i, QFormLayout::FieldRole)->widget();
            auto* fieldLabel = qobject_cast<QLabel*>(form->itemAt(i, QFormLayout::LabelRole)->widget());
            field->setAccessibleName(deviceName(channelName) + QStringLiteral(" ") + fieldLabel->text());
            fieldLabel->setBuddy(field);
        }
        pageLayout->addLayout(form);
        auto* geometry=new QFormLayout;
        for(int axis=0;axis<3;++axis) {
            auto* field=new QDoubleSpinBox(page);field->setObjectName(QString("layout%1:").arg(axis)+qname);
            field->setRange(axis==2?-360:10,axis==2?360:2000);field->setValue(axis==0?70:axis==1?40:0);field->setSuffix(axis==2?" deg":" px");
            geometry->addRow(axis==0?"Width":axis==1?"Height":"Rotation",field);
            connect(field,&QDoubleSpinBox::valueChanged,this,[this,channelName,axis](double value){auto& cs=channelSettingsFor(channelName);if(axis==0)cs.placement.width=value;else if(axis==1)cs.placement.height=value;else cs.placement.rotation=value;applyChannelToCanvas(channelName,cs);updateSelectedDevice();emit channelAssignmentChanged(channelName);});
        }
        pageLayout->addWidget(label("PLACEMENT","eyebrow",page));pageLayout->addLayout(geometry);
        auto* regions=new QPushButton("Edit LED colors...",page);regions->setObjectName("openLedRegions:"+qname);pageLayout->addWidget(regions);
        connect(regions,&QPushButton::clicked,this,[ledDialog]{ledDialog->show();ledDialog->raise();ledDialog->activateWindow();});
        if(channelName=="RoccatVulcan") {
            auto* calibrate=new QPushButton("Calibrate keyboard wire addresses",page);calibrate->setObjectName("calibrateKeyboard");pageLayout->addWidget(calibrate);
            connect(calibrate,&QPushButton::clicked,this,[this,channelName]{
                const auto& cs=channelSettingsFor(channelName);auto components=cs.hasCustomComponents()?cs.components:defaultComponents_[channelName];if(components.size()!=1)return;
                QDialog dialog(this);dialog.setWindowTitle("Calibrate actual keyboard addresses");dialog.resize(560,340);auto* form=new QFormLayout(&dialog);
                auto* help=new QLabel("Probe one wire address at a time. Choose the key that should receive it, then Save. This corrects the local mapping without assuming OpenRGB is right.",&dialog);help->setWordWrap(true);form->addRow(help);
                auto* key=new QComboBox(&dialog);key->setObjectName("calibrationKey");
                for(int i=0;i<components[0].ledCount;++i)if(components[0].ledNames[i].find("Wire slot")!=0){QString text=QString::fromStdString(components[0].ledNames[i]);if(i==0x0a)text="Left Windows";if(i==0x59)text="Right Ctrl";key->addItem(text+QString(" / current slot %1").arg(components[0].ledMapping[i]),i);}
                form->addRow("Physical key",key);auto* address=new QSpinBox(&dialog);address->setObjectName("calibrationAddress");address->setRange(0,components[0].ledCount-1);form->addRow("Actual wire slot",address);
                auto* identify=new QCheckBox("Identify slot: white only, all other keys off",&dialog);form->addRow(identify);
                connect(identify,&QCheckBox::toggled,&dialog,[&](bool enabled){if(enabled)identifyRegion(channelName,address->value());else stopRegionIdentification();});
                connect(address,&QSpinBox::valueChanged,&dialog,[&](int value){if(identify->isChecked())identifyRegion(channelName,value);});
                connect(key,&QComboBox::currentIndexChanged,&dialog,[&]{address->setValue(components[0].ledMapping[key->currentData().toInt()]);});
                key->setCurrentIndex(key->findData(0x0a));
                auto* buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,&dialog);form->addRow(buttons);
                connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);
                const int result=dialog.exec();stopRegionIdentification();if(result!=QDialog::Accepted)return;
                const int logical=key->currentData().toInt(),wire=address->value(),previous=components[0].ledMapping[logical];
                const auto other=std::find(components[0].ledMapping.begin(),components[0].ledMapping.end(),wire);if(other!=components[0].ledMapping.end())*other=previous;
                components[0].ledMapping[logical]=wire;editComponentGeometry(channelName,components);
            });
        }

        auto* layoutButton=new QPushButton("Arrange components / LED groups",page);layoutButton->setObjectName("editLayout:"+qname);pageLayout->addWidget(layoutButton);
        connect(layoutButton,&QPushButton::clicked,this,[this,channelName]{
            const auto& cs=channelSettingsFor(channelName);const auto components=cs.hasCustomComponents()?cs.components:defaultComponents_[channelName];
            if(components.empty())return;LayoutEditor editor(components,this);
            if(editor.exec()==QDialog::Accepted)editComponentGeometry(channelName,editor.components());
        });
        auto* hardwarePage=new QWidget(hardwarePages_);auto* hardwareLayout=new QVBoxLayout(hardwarePage);hardwareLayout->setContentsMargins(16,16,16,16);hardwareLayout->setSpacing(6);
        hardwarePage->setObjectName("hardwareChainPage");
        hardwarePage->setStyleSheet("QWidget#hardwareChainPage { background-color:#141b24; } QPushButton { min-height:24px; padding:4px 10px; } QComboBox { min-height:24px; padding:4px 10px; }");
        hardwareLayout->addWidget(label(deviceName(channelName)+" / connected hardware","section",hardwarePage));
        auto* topologyHint=label("Add the actual products in cable order. Each card is a physical component; its dots are addressable LEDs or groups.","muted",hardwarePage);topologyHint->setVisible(false);
        auto* hardwareScroll=new QScrollArea(hardwarePages_);hardwareScroll->setWidgetResizable(true);hardwareScroll->setFrameShape(QFrame::NoFrame);
        hardwarePage->setMinimumHeight(650);hardwareScroll->setWidget(hardwarePage);hardwarePages_->addWidget(hardwareScroll);
        const bool header = channelName == "JARGB1" || channelName == "JARGB2" || channelName == "JARGB3" || channelName == "EZConnector";
        if (header) {
            auto* page=hardwarePage;auto* pageLayout=hardwareLayout;
            pageLayout->addWidget(label(QStringLiteral("CONNECTED COMPONENTS / IN WIRE ORDER"),"eyebrow",page));
            row.components = new QListWidget(page);row.components->setMinimumHeight(160);row.components->setIconSize(QSize(40,40));
            row.components->setObjectName(QStringLiteral("components:")+qname);
            pageLayout->addWidget(row.components);
            auto* advanced=new QWidget(page);auto* advancedLayout=new QVBoxLayout(advanced);advancedLayout->setContentsMargins(0,0,0,0);advanced->setVisible(false);
            auto* catalog = new QComboBox(advanced);catalog->setObjectName("hardwareCatalog:"+qname);catalog->setEditable(true);catalog->setInsertPolicy(QComboBox::NoInsert);
            const auto populate=[this]{for(auto* list:findChildren<QComboBox*>())if(list->objectName().startsWith("hardwareCatalog:")){list->clear();for(const auto& c:hardwareLibrary())list->addItem(QString::fromStdString(c.brand.empty()?c.displayName:c.brand+" / "+c.displayName));for(const auto& c:settings_.hardwareCatalog)list->addItem("Custom / "+QString::fromStdString(c.displayName));}};populate();catalog->completer()->setCaseSensitivity(Qt::CaseInsensitive);catalog->setToolTip("Search by brand or product, then choose a definition");advancedLayout->addWidget(catalog);
            auto* actions = new QHBoxLayout();auto* add = new QPushButton(QStringLiteral("+ Add component to ")+qname,page);add->setObjectName("addHardware:"+qname);
            actions->addWidget(add,1);auto* manage=new QPushButton("Manage definitions",page);manage->setCheckable(true);actions->addWidget(manage);pageLayout->insertLayout(1,actions);pageLayout->addWidget(advanced);
            connect(manage,&QPushButton::toggled,advanced,&QWidget::setVisible);
            auto* definitions=new QHBoxLayout;advancedLayout->addLayout(definitions);auto* create=new QPushButton("New definition",page);create->setObjectName("createHardwareDefinition:"+qname);definitions->addWidget(create);
            auto* import=new QPushButton("Import JSON",page);definitions->addWidget(import);
            auto* forget=new QPushButton("Delete definition",page);definitions->addWidget(forget);
            connect(forget,&QPushButton::clicked,this,[this,catalog,populate]{const int i=catalog->currentIndex()-int(hardwareLibrary().size());if(i<0 || i>=int(settings_.hardwareCatalog.size()))return;settings_.hardwareCatalog.erase(settings_.hardwareCatalog.begin()+i);populate();emit customEffectsChanged();emit globalEffectChanged();});
            connect(import,&QPushButton::clicked,this,[this,populate]{const auto path=QFileDialog::getOpenFileName(this,"Import hardware component",{},"Component JSON (*.json)");if(path.isEmpty())return;QFile file(path);if(file.size()>128*1024 || !file.open(QIODevice::ReadOnly))return;try{auto c=Component::fromJson(QJsonDocument::fromJson(file.readAll()).object());if(!c.valid() || c.ledCount>120 || settings_.hardwareCatalog.size()>=128)throw std::invalid_argument("Invalid component");settings_.hardwareCatalog.push_back(c);populate();emit globalEffectChanged();}catch(const std::invalid_argument&){QMessageBox::warning(this,"Invalid definition","The definition needs 1-120 wire addresses, a unique address mapping and one coordinate/name per address.");}});
            connect(create,&QPushButton::clicked,this,[this,populate]{
                QDialog dialog(this);dialog.setWindowTitle("Create hardware");dialog.resize(500,330);auto* form=new QFormLayout(&dialog);
                auto* name=new QLineEdit(&dialog);name->setObjectName("hardwareName");name->setMaxLength(100);form->addRow("Product name",name);
                auto* type=new QComboBox(&dialog);type->addItems({"Fan","Strip"});form->addRow("Type",type);
                auto* count=new QSpinBox(&dialog);count->setObjectName("hardwareAddressCount");count->setRange(1,120);count->setValue(12);form->addRow("Addressable LEDs / groups",count);
                auto* physical=new QSpinBox(&dialog);physical->setObjectName("hardwareLedsPerAddress");physical->setRange(1,16);form->addRow("Physical LEDs per address",physical);
                auto* buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,&dialog);form->addRow(buttons);connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
                connect(buttons,&QDialogButtonBox::accepted,&dialog,[&]{if(name->text().trimmed().isEmpty() || settings_.hardwareCatalog.size()>=128)return;auto c=Component::makeGenericPlaceholder(count->value());c.type=type->currentText().toStdString();c.productName=c.displayName=name->text().trimmed().toStdString();c.physicalLedsPerAddress=physical->value();c.arrange(c.type=="Fan"?"Circle":"Line");c.layoutDefaults=c.ledCoordinates;settings_.hardwareCatalog.push_back(c);dialog.accept();});
                if(dialog.exec()==QDialog::Accepted){populate();emit globalEffectChanged();}
            });
            pageLayout->addWidget(label(QStringLiteral("Wire order determines addresses. Parallel hubs mirror outputs. Configure physical LEDs per address; software grouping keeps all wire addresses."),"muted",page));
            connect(add,&QPushButton::clicked,this,[this,channelName,create] {
                for(;;) {
                    auto components=channelSettingsFor(channelName).components;int used=0;for(const auto& c:components)used+=c.ledCount;
                    auto products=hardwareLibrary();products.insert(products.end(),settings_.hardwareCatalog.begin(),settings_.hardwareCatalog.end());
                    HardwarePicker picker(QString::fromStdString(channelName),std::move(products),120-used,this);
                    if(picker.exec()!=QDialog::Accepted){if(picker.customRequested){create->click();continue;}return;}
                    for(int i=0;i<picker.quantity();++i)components.push_back(instantiateHardware(picker.component(),int(components.size())));
                    if(!configureComponents(channelName,components))QMessageBox::warning(this,"Header capacity","This header supports at most 120 addressable slots. Nothing was changed.");
                    return;
                }
            });
        }

        hardwareLayout->addWidget(layoutButton);
        row.hardwarePreview=new DeviceLedWidget(hardwarePage);row.hardwarePreview->setMinimumHeight(220);hardwareLayout->addWidget(row.hardwarePreview,1);
        pageLayout->addStretch();
        pageLayout->addWidget(label(nativeOnly ? QStringLiteral("This zone uses effects built into the device. Spatial effects are unavailable.") : QStringLiteral("Native uses device effects. Canvas renders lighting from this workspace."), "muted", page));
        devicePages_->addWidget(page);
        rows_.emplace(channelName, row);
    }
    connect(deviceList_, &QListWidget::currentRowChanged, this, [this](int index) {
        if (index < 0 || index >= static_cast<int>(channelNames_.size())) return;
        stopRegionIdentification();
        devicePages_->setCurrentIndex(index);hardwarePages_->setCurrentIndex(index);
        canvasWidget_->setSelectedChannel(channelNames_[index]);
        if(settings_.activeProfile()) updateSelectedDevice();
    });
    connect(canvasWidget_, &LightingCanvasWidget::channelSelected, this, [this](const std::string& name) {
        const auto it = std::find(channelNames_.begin(), channelNames_.end(), name);
        if (it != channelNames_.end()) deviceList_->setCurrentRow(static_cast<int>(it - channelNames_.begin()));
    });
    connect(views,&QTabWidget::currentChanged,this,[this,inspector,viewTitle,viewHelp,previewButton,deviceHeading,effectHelp](int index){
        inspector->setVisible(index==0);deviceList_->setVisible(index!=1);deviceHeading->setVisible(index!=1);effectHelp->setVisible(index==1);
        viewTitle->setText(index==0?"Layout":index==1?"Effects":"Hardware");previewButton->setVisible(index==0);layoutEffectBadge_->setVisible(index==0);
        viewHelp->setText(index==0?"Drag hardware to position it over the colors. Select a device to resize or rotate it.":index==1?"Choose an effect or create your own palette. Lighting follows device positions in Layout.":"Choose a controller on the left, then add its connected fans and strips in cable order.");
    });
    deviceList_->setCurrentRow(channelNames_.empty() ? -1 : 0);
    splitter->setSizes({250, 800, 300});
    splitter->setStretchFactor(1, 1);
    auto* footer = new QHBoxLayout();
    persistenceStatus_=label(QStringLiteral("Live changes / settings loaded"),"muted",this);footer->addWidget(persistenceStatus_);
    footer->addStretch();
    footer->addWidget(label(QStringLiteral("Lighting preview / not device detection"), "muted", this));
    for (int i = 0; i < footer->count(); ++i) {
        if (auto* text = qobject_cast<QLabel*>(footer->itemAt(i)->widget())) text->setWordWrap(false);
    }
    root->addLayout(footer);

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
        const QColor picked = ColorPicker::getColor(toQColor(profile->globalEffect.color), this, QStringLiteral("Global Effect Color"));
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
            const QColor picked = ColorPicker::getColor(toQColor(channelSettingsFor(channelName).staticColor), this, QStringLiteral("Channel Color"));
            if (picked.isValid()) setChannelColor(channelName, toMsiColor(picked));
        });
        connect(row.nativeColorButton, &QPushButton::clicked, this, [this, channelName] {
            const QColor picked = ColorPicker::getColor(toQColor(channelSettingsFor(channelName).nativeSettings.color1), this, QStringLiteral("Native Mode Color"));
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
    const auto index = std::find(channelNames_.begin(), channelNames_.end(), channelName) - channelNames_.begin();
    cs.placement = {210.0 + static_cast<double>(index % 4) * 190.0,
                    145.0 + static_cast<double>(index / 4) * 155.0};
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
    } else {
        // Fresh board channels must be visible too. Existing black/off
        // choices are preserved because only missing entries use defaults.
        cs.nativeSettings.color1 = {255, 255, 255};
    }
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
    const bool header=channelName=="JARGB1" || channelName=="JARGB2" || channelName=="JARGB3" || channelName=="EZConnector";
    // Unconfigured/empty connectors belong in Hardware, not as fake devices on Layout.
    if(header && (!cs.hasCustomComponents() || cs.components.empty())){canvasWidget_->removeChannel(channelName);return;}
    canvasWidget_->setChannelComponents(channelName,!cs.hasCustomComponents()?defaultComponents_[channelName]:cs.components);
    canvasWidget_->setChannelOverrides(channelName,cs.ledOverrides);
    if (cs.mode == ChannelMode::Native) {
        // Native-mode channels still get a box on the canvas (so the user
        // can arrange the physical layout before switching modes). The neutral
        // silhouette does not imply that native lighting samples the field.
        canvasWidget_->setChannel(channelName, cs.placement, CanvasSource::Independent,
            std::make_shared<StaticColorEffect>(cs.nativeSettings.lightingMode == MsiLightingMode::Off ? MsiColor{} : cs.nativeSettings.color1));
        return;
    }

    std::shared_ptr<IEffect> effect;
    if (cs.source == CanvasSource::Independent) {
        effect = buildEffect(cs.effectName, cs.staticColor, 0, 0, settings_.customEffects);
    }
    // GlobalFollow samples the shared effect at the zone's scene position;
    // Independent uses its own local effect, matching the rendering engine.
    canvasWidget_->setChannel(channelName, cs.placement, cs.source, effect);
}

void LightingStudioWindow::applyGlobalEffectToCanvas(const GlobalEffectSettings& g)
{
    previewRecipe_=effectDefinition(g.effectName,g.color,settings_.customEffects);previewRecipe_.path="Canvas positions";
    if(layoutEffectBadge_)layoutEffectBadge_->setText("Effect: "+QString::fromStdString(previewRecipe_.name));
    auto effect=buildEffect(g.effectName,g.color,g.referenceX,g.referenceY,settings_.customEffects);canvasWidget_->setGlobalEffect(effect);
    if(auto* preview=dynamic_cast<EffectPreview*>(findChild<QWidget*>("selectedEffectPreview")))preview->renderedEffect=effect;
    updateSelectedDevice();
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
    auto* form = row.form;
    if (form) {
        form->setRowVisible(row.modeCombo,row.modeCombo->count()>1);
        form->setRowVisible(row.sourceCombo, isCanvas);
        form->setRowVisible(row.effectCombo, isCanvas && isIndependent);
        form->setRowVisible(row.colorButton, isCanvas && isIndependent);
        form->setRowVisible(row.nativeModeCombo, !isCanvas);
        form->setRowVisible(row.nativeColorButton, !isCanvas);
    }

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
    globalColorButton_->setEnabled(effectUsesColor(effectName));globalColorButton_->setVisible(effectUsesColor(effectName));
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
            while (row.effectCombo->count() > static_cast<int>(kEffectNames.size())) row.effectCombo->removeItem(row.effectCombo->count()-1);
            for(const auto& definition : settings_.customEffects) row.effectCombo->addItem(QString::fromStdString(definition.name),QString::fromStdString("custom:"+definition.name));
            // findData, not a hardcoded index: cs.mode is guaranteed to be
            // one of this combo's items by ensureProfileAndEntries()'s own
            // clamp, but falling back to index 0 defensively costs nothing.
            const int modeIdx = row.modeCombo->findData(static_cast<int>(cs.mode));
            row.modeCombo->setCurrentIndex(modeIdx >= 0 ? modeIdx : 0);
            row.sourceCombo->setCurrentIndex(cs.source == CanvasSource::GlobalFollow ? 1 : 0);
            const int effectIdx = (cs.effectName.find("custom:") == 0 ? row.effectCombo->findData(QString::fromStdString(cs.effectName)) : row.effectCombo->findText(QString::fromStdString(cs.effectName)));
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
    updateSelectedDevice();
    }

    {
        const QSignalBlocker blocker(globalEffectCombo_);
        while(globalEffectCombo_->count()>static_cast<int>(kEffectNames.size()))globalEffectCombo_->removeItem(globalEffectCombo_->count()-1);
        for(const auto& definition : settings_.customEffects)globalEffectCombo_->addItem(QString::fromStdString(definition.name),QString::fromStdString("custom:"+definition.name));
        // No index-0 fallback here (unlike the row effect combos): a fresh
        // profile's globalEffect.effectName is "" (no global effect), and
        // displaying "StaticColor" for it made picking StaticColor a
        // same-index no-op that fired no signal -- the user saw StaticColor
        // selected, a disabled color swatch, and no way to enable it short of
        // picking another effect first (Task 23 review, follow-up to
        // 17d9a4f). -1 shows an empty selection that matches the stored value.
        const int idx = (profile->globalEffect.effectName.find("custom:") == 0 ? globalEffectCombo_->findData(QString::fromStdString(profile->globalEffect.effectName)) : globalEffectCombo_->findText(QString::fromStdString(profile->globalEffect.effectName)));
        globalEffectCombo_->setCurrentIndex(idx);
    }
    updateColorSwatch(globalColorButton_, profile->globalEffect.color);
    updateGlobalColorButtonEnabledState();
    applyGlobalEffectToCanvas(profile->globalEffect);
}

void LightingStudioWindow::onRowChanged(const std::string& channelName)
{
    stopRegionIdentification();
    const ChannelRow& row = rows_.at(channelName);
    ChannelSettings& cs = channelSettingsFor(channelName);
    cs.mode = static_cast<ChannelMode>(row.modeCombo->currentData().toInt());
    cs.source = (row.sourceCombo->currentIndex() == 1) ? CanvasSource::GlobalFollow : CanvasSource::Independent;
    // Settings.h documents effectName as "empty if mode == Native" --
    // switching back to Canvas just re-reads the combo, so clearing here
    // costs nothing and keeps persisted Native-mode entries matching that
    // contract.
    cs.effectName = (cs.mode == ChannelMode::Native) ? std::string() : (row.effectCombo->currentData().isValid() ? row.effectCombo->currentData().toString() : row.effectCombo->currentText()).toStdString();
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
    updateSelectedDevice();
    emit channelAssignmentChanged(channelName);
}

void LightingStudioWindow::setChannelColor(const std::string& channelName, const MsiColor& color)
{
    ChannelSettings& cs = channelSettingsFor(channelName);
    cs.staticColor = color;
    updateColorSwatch(rows_.at(channelName).colorButton, color);
    applyChannelToCanvas(channelName, cs);
    updateSelectedDevice();
    emit channelAssignmentChanged(channelName);
}

void LightingStudioWindow::setChannelNativeColor(const std::string& channelName, const MsiColor& color)
{
    ChannelSettings& cs = channelSettingsFor(channelName);
    cs.nativeSettings.color1 = color;
    updateColorSwatch(rows_.at(channelName).nativeColorButton, color);
    applyChannelToCanvas(channelName, cs);
    updateSelectedDevice();
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
    // Review finding: a name collision fails silently here (addProfile()
    // returns false, below). A QMessageBox::warning() was tried as a fix but
    // reverted -- it's a MODAL dialog, and this method is called directly
    // from tests (e.g. testCreateProfile_rejectsDuplicateNameAsNoOp) with no
    // event loop to dismiss it, which hung and crashed the test suite rather
    // than just failing a test. Left as a known, documented minor gap
    // instead: a real fix needs the warning to not fire on the test's direct
    // call path (e.g. only shown from the button's own click handler, not
    // from this method), which is more invasive than this cleanup pass
    // warrants.
    if (!settings_.addProfile(name)) return; // empty/duplicate name
    settings_.activeProfileName = name; // a newly created profile becomes the active one, as the user expects
    refreshFromSettings(); // ensureProfileAndEntries() fills in every channel's defaults for it
}

void LightingStudioWindow::renameActiveProfile(const std::string& newName)
{
    if (!settings_.renameActiveProfile(newName)) return; // empty name or collides with a different existing profile; see createProfile()'s comment above
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
    profile->globalEffect.effectName = (globalEffectCombo_->currentData().isValid() ? globalEffectCombo_->currentData().toString() : globalEffectCombo_->currentText()).toStdString();
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
    stopRegionIdentification();
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
    updateSelectedDevice();
    emit channelAssignmentChanged(channelName);
}

void LightingStudioWindow::onCanvasGlobalReferencePointChanged(int x, int y)
{
    // Likewise already applied to the live effect object in place (see
    // LightingCanvasWidget::mouseMoveEvent) -- persist only.
    Profile* profile = settings_.activeProfile();
    profile->globalEffect.referenceX = x;
    profile->globalEffect.referenceY = y;
    updateSelectedDevice();emit globalEffectChanged();
}

void LightingStudioWindow::setDeviceComponents(const std::string& name, const std::vector<Component>& components)
{
    defaultComponents_[name]=components;
    if(rows_.count(name) && components.size()==1){auto& cs=channelSettingsFor(name);
        // Migrate only the unreadable legacy footprint; preserve custom sizes and coordinates.
        if(cs.placement.width==70 && cs.placement.height==40){const auto kind=deviceArtworkKind(components[0]);if(kind.startsWith("keyboard")){cs.placement.width=420;cs.placement.height=120;}else if(kind=="mouse"){cs.placement.width=80;cs.placement.height=130;}}
    }
    // Registering detected defaults must not overwrite saved custom geometry
    // on non-selected outputs during startup.
    if(rows_.count(name))applyChannelToCanvas(name,channelSettingsFor(name));
    updateSelectedDevice();
}

bool LightingStudioWindow::configureComponents(const std::string& name, const std::vector<Component>& components)
{
    stopRegionIdentification();
    if (name!="JARGB1" && name!="JARGB2" && name!="JARGB3" && name!="EZConnector") return false;
    if (!rows_.count(name)) return false;
    Channel validation(name,120);
    if (!components.empty() && !validation.replaceComponents(components)) return false;
    auto& cs=channelSettingsFor(name);
    if(!components.empty() && !cs.hasCustomComponents()){
        cs.mode=ChannelMode::Canvas;cs.source=CanvasSource::GlobalFollow;
        if(settings_.activeProfile()->globalEffect.effectName.empty()){settings_.activeProfile()->globalEffect.effectName="Wave";applyGlobalEffectToCanvas(settings_.activeProfile()->globalEffect);emit globalEffectChanged();}
        auto& row=rows_.at(name);const QSignalBlocker mode(row.modeCombo),source(row.sourceCombo);row.modeCombo->setCurrentIndex(row.modeCombo->findData(int(cs.mode)));row.sourceCombo->setCurrentIndex(1);
    }
    if(!components.empty()) {
        const auto after=layoutBounds(components),before=layoutBounds(cs.components);
        const double sx=cs.components.empty()?3.:(cs.placement.width-1)/qMax(1,before.right-before.left);
        const double sy=cs.components.empty()?3.:(cs.placement.height-1)/qMax(1,before.bottom-before.top);
        cs.placement.width=qBound(20.,(after.right-after.left)*sx+1,1050.);
        cs.placement.x=qBound(0.,cs.placement.x,1100.-cs.placement.width);
        cs.placement.height=qBound(20.,(after.bottom-after.top)*sy+1,650.);
        cs.placement.y=qBound(0.,cs.placement.y,700.-cs.placement.height);
    }
    cs.components=components;cs.componentsConfigured=true;
    // A changed wire chain invalidates previous per-address overrides.
    cs.ledOverrides.clear();
    applyChannelToCanvas(name,cs);
    updateRowEnabledState(name);updateSelectedDevice();emit channelAssignmentChanged(name);return true;
}

bool LightingStudioWindow::setRegionColor(const std::string& name, int index, MsiColor color)
{
    if(!rows_.count(name))return false;
    auto& cs=channelSettingsFor(name);
    const auto& components=!cs.hasCustomComponents()?defaultComponents_[name]:cs.components;
    int count=0;for(const auto& c:components)count+=c.ledCount;
    if(cs.mode!=ChannelMode::Canvas || index<0 || index>=count)return false;
    cs.ledOverrides[index]=color;updateSelectedDevice();emit channelAssignmentChanged(name);return true;
}

bool LightingStudioWindow::createCustomEffect(const CustomEffectDefinition& definition)
{
    if(QString::fromStdString(definition.name).trimmed().isEmpty() || QString::fromStdString(definition.name).size()>80 || definition.colors.size()<2 || definition.colors.size()>16
        || !CustomEffectDefinition::validPattern(definition.pattern) || !CustomEffectDefinition::validPath(definition.path) || !definition.validStops()
        || definition.periodMs<500 || definition.periodMs>120000 || definition.wavelength<40 || definition.wavelength>4000)return false;
    auto it=std::find_if(settings_.customEffects.begin(),settings_.customEffects.end(),[&](const auto& d){return d.name==definition.name;});
    if(it==settings_.customEffects.end()) { if(settings_.customEffects.size()>=64)return false;settings_.customEffects.push_back(definition); }
    else *it=definition;
    settings_.activeProfile()->globalEffect.effectName="custom:"+definition.name;
    refreshFromSettings();emit customEffectsChanged();return persistenceSucceeded_;
}

void LightingStudioWindow::updateSelectedDevice()
{
    const int index=deviceList_->currentRow();if(index<0 || index>=static_cast<int>(channelNames_.size()) || !settings_.activeProfile())return;
    const auto& name=channelNames_[index];const auto& cs=channelSettingsFor(name);
    if(auto* dialog=findChild<QDialog*>("ledRegionsDialog"))dialog->setWindowTitle(QString::fromStdString(name)+" / LED colors");
    const bool header=name=="JARGB1" || name=="JARGB2" || name=="JARGB3" || name=="EZConnector";
    const auto components=header && !cs.hasCustomComponents()?std::vector<Component>{}:!cs.hasCustomComponents()?defaultComponents_[name]:cs.components;
    ledMap_->setGeometryData(components);canvasWidget_->setChannelComponents(name,components);
    canvasWidget_->setChannelOverrides(name,cs.ledOverrides);
    auto effect=cs.mode==ChannelMode::Native?std::make_shared<StaticColorEffect>(cs.nativeSettings.lightingMode==MsiLightingMode::Off?MsiColor{}:cs.nativeSettings.color1)
        :cs.source==CanvasSource::GlobalFollow?buildEffect(settings_.activeProfile()->globalEffect.effectName,settings_.activeProfile()->globalEffect.color,settings_.activeProfile()->globalEffect.referenceX,settings_.activeProfile()->globalEffect.referenceY,settings_.customEffects)
        :buildEffect(cs.effectName,cs.staticColor,0,0,settings_.customEffects);
    ledMap_->setEffect(effect,cs.ledOverrides,cs.mode==ChannelMode::Canvas,cs.placement,cs.source==CanvasSource::GlobalFollow);
    if(auto* preview=rows_.at(name).hardwarePreview){preview->setGeometryData(components);preview->setEffect(effect,cs.ledOverrides,false,cs.placement,cs.source==CanvasSource::GlobalFollow);}
    for(int axis=0;axis<3;++axis)if(auto* field=findChild<QDoubleSpinBox*>(QString("layout%1:").arg(axis)+QString::fromStdString(name))){const QSignalBlocker blocker(field);field->setValue(axis==0?cs.placement.width:axis==1?cs.placement.height:cs.placement.rotation);}
    identifyToggle_->setEnabled(cs.mode==ChannelMode::Canvas);
    if(cs.mode!=ChannelMode::Canvas)stopRegionIdentification();
    auto* list=rows_.at(name).components;
    if(list) {
        QJsonArray signature;for(const auto& c:cs.components)signature.append(c.toJson());const QString key=QString::fromUtf8(QJsonDocument(signature).toJson(QJsonDocument::Compact))+(cs.hasCustomComponents()?"configured":"legacy");
        auto& row=rows_.at(name);if(key!=row.componentSignature){row.componentSignature=key;const int selected=list->currentRow();list->clear();int offset=0;
            for(size_t index=0;index<cs.components.size();++index){const auto& c=cs.components[index];
                const QString title=QString::fromStdString(c.displayName);const QString details=QString("%1 addresses / %2 physical LEDs / wire %3-%4").arg(c.ledCount).arg(c.ledCount*c.physicalLedsPerAddress).arg(offset+1).arg(offset+c.ledCount);
                auto* item=new QListWidgetItem(list);item->setData(Qt::AccessibleTextRole,title+" / "+details);item->setSizeHint(QSize(180,76));item->setToolTip(QString::fromStdString(c.brand+" / "+c.productName));
                auto* card=new QWidget(list);auto* cardLayout=new QHBoxLayout(card);cardLayout->setContentsMargins(8,4,8,4);
                auto* icon=new QLabel(card);icon->setPixmap(hardwareIcon(c).pixmap(56,48));cardLayout->addWidget(icon);
                auto* text=new QLabel(title+"\n"+details,card);text->setWordWrap(true);text->setAttribute(Qt::WA_TransparentForMouseEvents);cardLayout->addWidget(text,1);
                auto* remove=new QPushButton("Remove",card);remove->setObjectName(QString("removeHardware:%1:%2").arg(QString::fromStdString(name)).arg(index));remove->setAccessibleName("Remove "+title+" from "+QString::fromStdString(name));cardLayout->addWidget(remove);
                connect(remove,&QPushButton::clicked,this,[this,name,index]{auto chain=channelSettingsFor(name).components;if(index>=chain.size())return;chain.erase(chain.begin()+index);configureComponents(name,chain);});
                list->setItemWidget(item,card);offset+=c.ledCount;
            }
            if(cs.components.empty())list->addItem(cs.hasCustomComponents()?"No components / output cleared":"No components yet. Click + Add component above.");else list->setCurrentRow(qBound(0,selected,int(cs.components.size())-1));
        }
    }
}

bool LightingStudioWindow::identifyRegion(const std::string& name,int index)
{
    if(!rows_.count(name))return false;
    const auto& cs=channelSettingsFor(name);const auto& components=!cs.hasCustomComponents()?defaultComponents_[name]:cs.components;
    int count=0;for(const auto& component:components)count+=component.ledCount;
    if(cs.mode!=ChannelMode::Canvas || index<0 || index>=count)return false;
    if(!identificationChannel_.empty() && identificationChannel_!=name)stopRegionIdentification();
    identificationChannel_=name;emit regionIdentificationRequested(name,index);return true;
}

void LightingStudioWindow::stopRegionIdentification()
{
    const auto previous=identificationChannel_;identificationChannel_.clear();
    if(identifyToggle_) { const QSignalBlocker blocker(identifyToggle_);identifyToggle_->setChecked(false); }
    if(!previous.empty())emit regionIdentificationRequested(previous,-1);
}

void LightingStudioWindow::hideEvent(QHideEvent* event)
{
    stopRegionIdentification();QWidget::hideEvent(event);
}

bool LightingStudioWindow::editComponentGeometry(const std::string& name,const std::vector<Component>& components) {
    if(!rows_.count(name))return false;
    auto& cs=channelSettingsFor(name);const auto old=cs.hasCustomComponents()?cs.components:defaultComponents_[name];
    if(old.size()!=components.size())return false;
    for(size_t i=0;i<old.size();++i)if(!components[i].valid() || components[i].ledCount!=old[i].ledCount)return false;
    const auto before=layoutBounds(old),after=layoutBounds(components);
    // Retain the scene positions of unchanged LEDs when a sibling is moved or scaled.
    if(cs.source==CanvasSource::GlobalFollow) {
        const double sx=(cs.placement.width-1)/qMax(1,before.right-before.left),sy=(cs.placement.height-1)/qMax(1,before.bottom-before.top);
        const double dx=(after.left+after.right-before.left-before.right)*.5*sx,dy=(after.top+after.bottom-before.top-before.bottom)*.5*sy;
        const double angle=cs.placement.rotation*3.141592653589793/180.;
        const double width=qBound(10.,(after.right-after.left)*sx+1,2000.),height=qBound(10.,(after.bottom-after.top)*sy+1,2000.);
        cs.placement.x+=(cs.placement.width-width)/2+dx*std::cos(angle)-dy*std::sin(angle);
        cs.placement.y+=(cs.placement.height-height)/2+dx*std::sin(angle)+dy*std::cos(angle);
        cs.placement.width=width;cs.placement.height=height;
    }
    auto overrides=cs.ledOverrides;int offset=0;
    for(size_t c=0;c<old.size();++c){for(int i=0;i<old[c].ledCount;++i){const int from=offset+old[c].ledMapping[i],to=offset+components[c].ledMapping[i];if(from!=to)overrides.erase(to);}offset+=old[c].ledCount;}
    offset=0;for(size_t c=0;c<old.size();++c){for(int i=0;i<old[c].ledCount;++i){const auto fixed=cs.ledOverrides.find(offset+old[c].ledMapping[i]);if(fixed!=cs.ledOverrides.end())overrides[offset+components[c].ledMapping[i]]=fixed->second;}offset+=old[c].ledCount;}
    cs.ledOverrides=std::move(overrides);
    cs.components=components;cs.componentsConfigured=true;applyChannelToCanvas(name,cs);updateSelectedDevice();emit channelAssignmentChanged(name);return true;
}

void LightingStudioWindow::setPersistenceStatus(bool saved){persistenceSucceeded_=saved;if(persistenceStatus_)persistenceStatus_->setText(saved?"Saved to disk":"Save failed / settings could not be written");}
