#pragma once

#include "LightingCanvas.h"
#include "discovery/HardwareDiscovery.h"
#include <QMainWindow>
#include <QElapsedTimer>
#include <QVariantMap>

class QTreeWidget;
class QTreeWidgetItem;
class QComboBox;
class QSlider;
class QCheckBox;
class QPushButton;
class QTabWidget;
class QDoubleSpinBox;
class QSpinBox;
class QLineEdit;
class QListWidget;
class QLabel;
class QStackedWidget;
class QVBoxLayout;
class ColorEditor;
class ColorListEditor;

class LightingWindow : public QMainWindow {
    Q_OBJECT
public:
    LightingWindow(ComponentCatalog catalog,
        std::function<QVector<ProfileCandidate>()> scan, QWidget* parent = nullptr, QVector<DeviceProfile> profiles = {}, bool automaticDiscovery = true, QString libraryPath = {});
    ~LightingWindow() override;
    LightingDocument& document() { return document_; }
    QVariantMap workspacePanelPreferences() const;
    void restoreWorkspacePanelPreferences(const QVariantMap& panels);
    double previewSeconds() const;
    QVector<ComponentPlacement> previewPlacements() const { return layoutCanvas_->previewPlacements(); }
    const QVector<ProfileCandidate>& discoveredHardware() const { return discovered_; }
    void setOutputState(const QString& state);
    void installOutputPanel(QWidget* panel);
    void restoreLegacyHardware(const QString& path);
    QString projectFile() const { return fileName_; }
    void loadProject(const QString& path, bool recovered = false, const QString& original = {}, const QJsonArray& playbackDefinitions = {});
    void saveProject(const QString& path);
    const QImage& previewFrame() const { return previewFrame_; }
signals:
    void previewFrameChanged(const QImage& frame);
    void identifyRequested(const QUuid& placement); // ServicePanel forwards it to the installed service.
    void hardwareDiscovered();
    void closing();
    void projectLoading();
    void outputRequested();
protected:
    void closeEvent(QCloseEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void paintEvent(QPaintEvent*) override;
    bool eventFilter(QObject*,QEvent*) override;
private:
    void refresh();
    void scanHeaders();
    void refreshHeader();
    QString offlineProfile_;
    void addHeaderComponents();
    void refreshComponentCatalog();
    ChannelAssignment selectedHeaderAssignment() const;
    ChannelAssignment connectorAssignment(const QVariantMap& connector) const;
    void moveComponentToConnector(const QUuid& id, ChannelAssignment connector);
    std::function<QVector<ProfileCandidate>()> scan_;
    QVector<ProfileCandidate> discovered_;
    QVariantMap selectedHeader_;
    QLabel *headerTitle_, *headerSummary_;
    void refreshProperties();
    void refreshToolOptions();
    void updatePreview();
    void flashComponent(const QUuid& placement);
    quint64 identifyGeneration_ = 0; // Newest identify flash; older expiry timers do nothing.
    void refreshPlacement();
    void syncSelectionTrees();
    void updateWorkspacePanels();
    void cyclePanelFocus(bool backwards);
    void moveSelectedLayer(int direction);
    void setLabelsHeld(bool held);
    void removeSelectedComponents();
    void updateMotion(const QString& label,const std::function<void(EffectLayer&)>& operation);
    void updateLayer(const QString& label, const std::function<void(EffectLayer&)>& operation);
    bool saveDocument();
    void refreshProjectName();
    bool confirmDiscard();
    LightingDocument document_;
    QImage previewFrame_;
    LightingCanvas *layoutCanvas_, *effectsCanvas_, *hardwarePreview_;
    QTabWidget* tabs_;
    QStackedWidget* applicationPages_;
    QVBoxLayout* deviceOutput_;
    QTreeWidget* layers_=nullptr;
    QTreeWidget* components_=nullptr;
    QTabWidget* inspectorTabs_;
    QLabel* inspectorTitle_;
    QComboBox *catalog_, *blend_;
    QComboBox* previewChannel_;
    QCheckBox* allChannels_;
    QListWidget* channelTargets_;
    ColorEditor *secondaryColor_, *sourceColor_;
    ColorListEditor* gradientColors_;
    QWidget* geometryControls_;
    QVector<QDoubleSpinBox*> geometry_;
    QVector<QDoubleSpinBox*> placementControls_;
    QDoubleSpinBox* repetitions_;
    QSlider* transparency_;
    ColorEditor* color_;
    QColor foreground_=Qt::white;
    QPushButton *effectTarget_;
    QCheckBox* maskEnabled_;
    QComboBox* imageFit_;
    QWidget* animationControls_;
    QVector<QDoubleSpinBox*> animation_;
    QPushButton* playing_;
    QDoubleSpinBox* previewTime_;
    QVector<QDoubleSpinBox*> advanced_, crop_;
    QWidget* cropControls_;
    ColorListEditor* colorCycle_;
    QCheckBox* maskLinked_;
    QElapsedTimer previewClock_;
    double playbackStart_ = 0;
    QUuid selectedLayer_, activeLayer_;
    bool editingMask_ = false;
    bool labelsHeld_=false;
    bool compactWorkspace_=false, navigatorShown_=true, inspectorShown_=true, inspectorBeforeCompact_=true, adjustingPanels_=false;
    QString fileName_;
};

// Project files always end in ".lighting.json" (owner decision 2026-11-19): the open dialog filters on it
// and the uninstaller backs up only *.json. A trailing ".json" is replaced; any other name gets the suffix.
QString withProjectSuffix(const QString& path);
