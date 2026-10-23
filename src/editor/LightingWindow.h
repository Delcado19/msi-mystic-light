#pragma once

#include "LightingCanvas.h"
#include "discovery/HardwareDiscovery.h"
#include <QMainWindow>

class QTreeWidget;
class QTreeWidgetItem;
class QComboBox;
class QSlider;
class QCheckBox;
class QPushButton;
class QTabWidget;
class QDoubleSpinBox;
class QSpinBox;

class LightingWindow : public QMainWindow {
    Q_OBJECT
public:
    LightingWindow(ComponentCatalog catalog,
        std::function<QVector<ProfileCandidate>()> scan, QWidget* parent = nullptr);
    LightingDocument& document() { return document_; }
protected:
    void closeEvent(QCloseEvent*) override;
private:
    void refresh();
    void refreshProperties();
    void updateLayer(const QString& label, const std::function<void(EffectLayer&)>& operation);
    bool saveDocument();
    bool confirmDiscard();
    LightingDocument document_;
    LightingCanvas *layoutCanvas_, *effectsCanvas_;
    QTabWidget* tabs_;
    QTreeWidget* layers_;
    QComboBox *catalog_, *blend_;
    QComboBox* effectType_;
    QPushButton* secondaryColor_;
    QWidget* geometryControls_;
    QVector<QDoubleSpinBox*> geometry_;
    QSpinBox* repetitions_;
    QSlider* transparency_;
    QPushButton *color_, *effectTarget_, *maskTarget_;
    QCheckBox* maskEnabled_;
    QComboBox* imageFit_;
    QUuid selectedLayer_;
    bool editingMask_ = false;
    QString fileName_;
};
