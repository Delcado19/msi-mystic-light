#pragma once

#include "LightingCanvas.h"
#include "discovery/HardwareDiscovery.h"
#include <QMainWindow>
#include <QElapsedTimer>

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

class LightingWindow : public QMainWindow {
    Q_OBJECT
public:
    LightingWindow(ComponentCatalog catalog,
        std::function<QVector<ProfileCandidate>()> scan, QWidget* parent = nullptr, QVector<DeviceProfile> profiles = {});
    LightingDocument& document() { return document_; }
    double previewSeconds() const;
    QString projectFile() const { return fileName_; }
    void loadProject(const QString& path, bool recovered = false, const QString& original = {});
    const QImage& previewFrame() const { return previewFrame_; }
signals:
    void previewFrameChanged(const QImage& frame);
    void closing();
protected:
    void closeEvent(QCloseEvent*) override;
private:
    void refresh();
    void refreshProperties();
    void updatePreview();
    void refreshPlacement();
    void updateLayer(const QString& label, const std::function<void(EffectLayer&)>& operation);
    bool saveDocument();
    bool confirmDiscard();
    LightingDocument document_;
    QImage previewFrame_;
    LightingCanvas *layoutCanvas_, *effectsCanvas_;
    QTabWidget* tabs_;
    QTreeWidget* layers_;
    QTreeWidget* assignments_;
    QComboBox *catalog_, *blend_;
    QComboBox* effectType_;
    QComboBox* previewChannel_;
    QCheckBox* allChannels_;
    QListWidget* channelTargets_;
    QPushButton* secondaryColor_;
    QWidget* geometryControls_;
    QVector<QDoubleSpinBox*> geometry_;
    QVector<QDoubleSpinBox*> placementControls_;
    QSpinBox* repetitions_;
    QSlider* transparency_;
    QPushButton *color_, *effectTarget_, *maskTarget_;
    QCheckBox* maskEnabled_;
    QComboBox* imageFit_;
    QWidget* animationControls_;
    QVector<QDoubleSpinBox*> animation_;
    QCheckBox* playing_;
    QDoubleSpinBox* previewTime_;
    QVector<QDoubleSpinBox*> advanced_, crop_;
    QWidget* cropControls_;
    QLineEdit* colorCycle_;
    QCheckBox* maskLinked_;
    QElapsedTimer previewClock_;
    double playbackStart_ = 0;
    QUuid selectedLayer_;
    bool editingMask_ = false;
    QString fileName_;
};
