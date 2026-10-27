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
        std::function<QVector<ProfileCandidate>()> scan, QWidget* parent = nullptr, QVector<DeviceProfile> profiles = {});
    ~LightingWindow() override;
    LightingDocument& document() { return document_; }
    double previewSeconds() const;
    QVector<ComponentPlacement> previewPlacements() const { return layoutCanvas_->previewPlacements(); }
    const QVector<ProfileCandidate>& discoveredHardware() const { return discovered_; }
    void setOutputState(const QString& state);
    void installOutputPanel(QWidget* panel);
    void restoreLegacyHardware(const QString& path);
    QString projectFile() const { return fileName_; }
    void loadProject(const QString& path, bool recovered = false, const QString& original = {});
    void saveProject(const QString& path);
    const QImage& previewFrame() const { return previewFrame_; }
signals:
    void previewFrameChanged(const QImage& frame);
    void hardwareDiscovered();
    void closing();
    void projectLoading();
    void outputRequested();
protected:
    void closeEvent(QCloseEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    bool eventFilter(QObject*,QEvent*) override;
private:
    void refresh();
    void scanHeaders();
    void refreshHeader();
    void addHeaderComponents();
    ChannelAssignment selectedHeaderAssignment() const;
    std::function<QVector<ProfileCandidate>()> scan_;
    QVector<ProfileCandidate> discovered_;
    QVariantMap selectedHeader_;
    QLabel *headerTitle_, *headerSummary_;
    void refreshProperties();
    void refreshToolOptions();
    void updatePreview();
    void refreshPlacement();
    void updateWorkspacePanels();
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
    QTreeWidget* layers_;
    QTreeWidget* components_;
    QTabWidget* inspectorTabs_;
    QLabel* inspectorTitle_;
    QComboBox *catalog_, *blend_;
    QComboBox* effectType_;
    QComboBox* previewChannel_;
    QCheckBox* allChannels_;
    QListWidget* channelTargets_;
    ColorEditor *secondaryColor_, *sourceColor_;
    ColorListEditor* gradientColors_;
    QWidget* geometryControls_;
    QVector<QDoubleSpinBox*> geometry_;
    QVector<QDoubleSpinBox*> placementControls_;
    QSpinBox* repetitions_;
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
    bool editingMask_ = false, resumePreview_=false;
    bool labelsHeld_=false;
    bool compactWorkspace_=false, navigatorShown_=true, inspectorShown_=true, inspectorBeforeCompact_=true, adjustingPanels_=false;
    QString fileName_;
};
