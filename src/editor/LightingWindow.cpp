#include "LightingWindow.h"
#include <QCheckBox>
#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QDockWidget>
#include <QDropEvent>
#include <QFormLayout>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>
#include <QTimer>
#include <QLineEdit>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace {
class LayerTree : public QTreeWidget {
public:
    using QTreeWidget::QTreeWidget;
    std::function<void()> reordered;
protected:
    void dropEvent(QDropEvent* event) override {
        QTreeWidget::dropEvent(event);
        if (event->isAccepted() && reordered) reordered();
    }
};
QPushButton* button(QVBoxLayout* layout, const QString& label, std::function<void()> action)
{
    auto* result = new QPushButton(label);
    layout->addWidget(result);
    QObject::connect(result, &QPushButton::clicked, result, std::move(action));
    return result;
}
QIcon colorIcon(QColor color)
{
    QPixmap pixmap(32, 24);
    pixmap.fill(color);
    return QIcon(pixmap);
}
}

LightingWindow::LightingWindow(ComponentCatalog catalog,
    std::function<QVector<ProfileCandidate>()> scan, QWidget* parent)
    : QMainWindow(parent), document_(std::move(catalog))
{
    setWindowTitle("Lighting — MSI Mystic Light Standalone");
    resize(1200, 760);
    auto* fileMenu = menuBar()->addMenu("File");
    auto* open = fileMenu->addAction("Open…"); open->setShortcut(QKeySequence::Open);
    connect(open, &QAction::triggered, this, [this] {
        const auto file = QFileDialog::getOpenFileName(this, "Open lighting document", {}, "Lighting documents (*.lighting.json)");
        if (file.isEmpty() || !confirmDiscard()) return;
        try { document_.load(file); fileName_ = file; }
        catch (const std::exception& error) { QMessageBox::warning(this, "Open failed", QString::fromUtf8(error.what())); }
    });
    auto* save = fileMenu->addAction("Save…"); save->setShortcut(QKeySequence::Save);
    connect(save, &QAction::triggered, this, [this] { saveDocument(); });
    auto* saveAs = fileMenu->addAction("Save as…"); saveAs->setShortcut(QKeySequence::SaveAs);
    connect(saveAs, &QAction::triggered, this, [this] {
        const auto previous = fileName_; fileName_.clear();
        if (!saveDocument()) fileName_ = previous;
    });
    auto* editMenu = menuBar()->addMenu("Edit");
    auto* undo = document_.history()->createUndoAction(this, "Undo");
    undo->setShortcut(QKeySequence::Undo);
    editMenu->addAction(undo);
    auto* redo = document_.history()->createRedoAction(this, "Redo");
    redo->setShortcuts({QKeySequence::Redo, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z)});
    editMenu->addAction(redo);
    auto* toolbar = addToolBar("History");
    toolbar->addAction(undo); toolbar->addAction(redo);

    tabs_ = new QTabWidget;
    tabs_->setObjectName("lightingViews");
    layoutCanvas_ = new LightingCanvas(document_, LightingCanvas::Context::Layout);
    effectsCanvas_ = new LightingCanvas(document_, LightingCanvas::Context::Effects);
    layoutCanvas_->setObjectName("layoutCanvas");
    effectsCanvas_->setObjectName("effectsCanvas");
    tabs_->addTab(layoutCanvas_, "Layout");
    tabs_->addTab(effectsCanvas_, "Effects");
    setCentralWidget(tabs_);

    auto* dock = new QDockWidget("Properties", this);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    auto* contexts = new QStackedWidget;
    dock->setWidget(contexts);
    auto addPanel = [contexts](QWidget* panel) {
        auto* scroll = new QScrollArea;
        scroll->setWidgetResizable(true);
        scroll->setWidget(panel);
        contexts->addWidget(scroll);
    };
    addDockWidget(Qt::RightDockWidgetArea, dock);
    dock->setMinimumWidth(320);
    auto* layoutPanel = new QWidget;
    auto* layoutControls = new QVBoxLayout(layoutPanel);
    addPanel(layoutPanel);
    layoutControls->addWidget(new QLabel("Components"));
    catalog_ = new QComboBox;
    for (const auto& component : document_.catalog()) catalog_->addItem(component.name, component.id);
    layoutControls->addWidget(catalog_);
    button(layoutControls, "Add component", [this] {
        if (document_.state().placements.size() >= 256) { statusBar()->showMessage("Document limit: 256 components."); return; }
        if (catalog_->currentIndex() >= 0) layoutCanvas_->select(document_.addComponent(catalog_->currentData().toString()));
    })->setEnabled(catalog_->count() > 0);
    button(layoutControls, "Remove selected component", [this] {
        const auto id = layoutCanvas_->selection();
        if (id.isNull()) return;
        document_.edit("Remove component", [&](LightingState& state) {
            state.placements.removeIf([&](const auto& placement) { return placement.id == id; });
        });
        layoutCanvas_->select({});
    });
    auto* instructions = new QLabel("Drag LEDs to move their component.\nArrow keys move; Shift moves by 10.\nWheel zooms; middle drag pans.\n\nLayout changes sampling positions only.");
    instructions->setWordWrap(true);
    layoutControls->addWidget(instructions);
    layoutControls->addStretch();

    // Candidate selection is explicit and staged only: no transport is opened here.
    auto* candidates = new QComboBox;
    candidates->setObjectName("profileCandidates");
    candidates->setMinimumContentsLength(16);
    candidates->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    button(layoutControls, "Scan supported devices", [this, scan, candidates] {
        candidates->clear();
        try {
            const auto found = scan();
            candidates->addItem("Select profile / endpoint…");
            for (const auto& candidate : found) {
                const auto& device = candidate.device;
                candidates->addItem(QString("%1 · interface %2 · %3").arg(candidate.profileId).arg(device.interfaceNumber).arg(device.path),
                    QVariantMap{{"profileId", candidate.profileId}, {"path", device.path}});
                candidates->setItemData(candidates->count() - 1, device.path, Qt::ToolTipRole);
            }
            statusBar()->showMessage(QString("%1 profile/endpoint candidates; no device opened.").arg(found.size()));
        } catch (const std::exception& error) {
            QMessageBox::warning(this, "Discovery failed", QString::fromUtf8(error.what()));
        }
    });
    layoutControls->addWidget(candidates);
    layoutControls->addWidget(new QLabel("Selection does not connect or assign channels."));

    auto* previewToolbar = addToolBar("Preview");
    playing_ = new QCheckBox("Play preview"); playing_->setObjectName("playPreview");
    previewTime_ = new QDoubleSpinBox; previewTime_->setObjectName("previewTime");
    previewTime_->setRange(0, 86400); previewTime_->setDecimals(2); previewTime_->setSuffix(" s");
    previewTime_->setKeyboardTracking(false); previewTime_->setAccessibleName("Preview time in seconds");
    previewToolbar->addWidget(playing_); previewToolbar->addWidget(previewTime_);
    auto* timer = new QTimer(this); timer->setInterval(33);
    connect(playing_, &QCheckBox::toggled, this, [this, timer](bool play) {
        if (play) { playbackStart_ = previewTime_->value(); previewClock_.restart(); timer->start(); }
        else timer->stop();
    });
    connect(timer, &QTimer::timeout, this, [this] {
        previewTime_->setValue(std::min(86400.0, playbackStart_ + previewClock_.elapsed()/1000.0));
        if (previewTime_->value() >= 86400) playing_->setChecked(false);
    });
    connect(previewTime_, &QDoubleSpinBox::editingFinished, this, [this] { playing_->setChecked(false); });
    connect(previewTime_, &QDoubleSpinBox::valueChanged, this, [this](double seconds) {
        const auto frame=document_.render(seconds);
        layoutCanvas_->setPreviewFrame(frame,seconds); effectsCanvas_->setPreviewFrame(frame,seconds);
    });
    auto* effectPanel = new QWidget;
    auto* effectControls = new QVBoxLayout(effectPanel);
    addPanel(effectPanel);
    auto* newEffect = new QComboBox;
    newEffect->setObjectName("newEffectType");
    for (const auto& definition : effectDefinitions())
        newEffect->addItem(definition.label, static_cast<int>(definition.kind));
    effectControls->addWidget(newEffect);
    button(effectControls, "New layer", [this, newEffect] {
        if (document_.state().layers.size() >= 64) { statusBar()->showMessage("Document limit: 64 layers."); return; }
        editingMask_ = false;
        selectedLayer_ = document_.addLayer(static_cast<EffectKind>(newEffect->currentData().toInt()));
        refresh();
    });
    button(effectControls, "Import image...", [this] {
        const auto file = QFileDialog::getOpenFileName(this, "Import image", {}, "Images (*.png *.jpg *.jpeg *.gif *.bmp *.webp)");
        if (file.isEmpty()) return;
        try { selectedLayer_ = document_.importImage(file); editingMask_ = false; refresh(); }
        catch (const std::exception& error) { QMessageBox::warning(this, "Import failed", QString::fromUtf8(error.what())); }
    });
    auto* tree = new LayerTree;
    layers_ = tree;
    layers_->setObjectName("effectLayers");
    layers_->setMinimumHeight(180); layers_->setMaximumHeight(280); // Long property forms must not collapse the layer list.
    layers_->setHeaderLabels({"Layer", "Effect", "Mask"});
    layers_->setRootIsDecorated(false);
    layers_->setSelectionMode(QAbstractItemView::SingleSelection);
    layers_->setSelectionBehavior(QAbstractItemView::SelectItems);
    layers_->setDragDropMode(QAbstractItemView::InternalMove);
    layers_->setDefaultDropAction(Qt::MoveAction);
    layers_->setIconSize({40, 25});
    layers_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    layers_->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    layers_->header()->setSectionResizeMode(2, QHeaderView::Fixed);
    layers_->setColumnWidth(1, 55); layers_->setColumnWidth(2, 55);
    effectControls->addWidget(layers_);
    connect(layers_, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item, int column) {
        selectedLayer_ = item->data(0, Qt::UserRole).toUuid();
        if (column > 0) editingMask_ = column == 2;
        refreshProperties();
    });
    connect(layers_, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem* item) {
        if (!item) return;
        selectedLayer_ = item->data(0, Qt::UserRole).toUuid();
        refreshProperties();
    });
    connect(layers_, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem* item, int column) {
        if (column != 0) return;
        const auto id = item->data(0, Qt::UserRole).toUuid();
        const auto name = item->text(0).trimmed();
        const bool visible = item->checkState(0) == Qt::Checked;
        // Rebuilding the panel inside QTreeWidget::setData would invalidate its item.
        QMetaObject::invokeMethod(this, [this, id, name, visible] {
            if (name.isEmpty()) { refresh(); return; }
            document_.edit("Edit layer", [&](LightingState& state) {
                for (auto& layer : state.layers) if (layer.id == id) { layer.name = name; layer.visible = visible; }
            });
        }, Qt::QueuedConnection);
    });
    tree->reordered = [this] {
        QVector<QUuid> order;
        for (int i = layers_->topLevelItemCount() - 1; i >= 0; --i)
            order.append(layers_->topLevelItem(i)->data(0, Qt::UserRole).toUuid());
        document_.edit("Reorder layers", [&](LightingState& state) {
            auto before = state.layers;
            for (int i = 0; i < order.size(); ++i)
                for (const auto& layer : before) if (layer.id == order[i]) state.layers[i] = layer;
        });
    };
    button(effectControls, "Duplicate layer", [this] {
        if (document_.state().layers.size() >= 64) { statusBar()->showMessage("Document limit: 64 layers."); return; }
        for (const auto& layer : document_.state().layers) if (layer.id == selectedLayer_) {
            auto copy = layer;
            copy.id = QUuid::createUuid(); copy.name += " copy";
            const auto id = selectedLayer_;
            document_.edit("Duplicate layer", [&](LightingState& state) {
                for (int i = 0; i < state.layers.size(); ++i) if (state.layers[i].id == id) { state.layers.insert(i + 1, copy); break; }
            });
            selectedLayer_ = copy.id; refresh(); break;
        }
    });
    button(effectControls, "Delete layer", [this] {
        if (selectedLayer_.isNull()) return;
        const auto id = selectedLayer_;
        document_.edit("Delete layer", [&](LightingState& state) {
            state.layers.removeIf([&](const auto& layer) { return layer.id == id; });
        });
    });
    auto* form = new QFormLayout;
    effectControls->addLayout(form);
    transparency_ = new QSlider(Qt::Horizontal);
    transparency_->setRange(0, 100);
    transparency_->setObjectName("layerTransparency");
    transparency_->setAccessibleName("Layer transparency percent");
    auto* transparencyValue = new QLabel("0%");
    transparencyValue->setObjectName("transparencyValue");
    form->addRow("Transparency", transparency_); form->addRow("", transparencyValue);
    connect(transparency_, &QSlider::valueChanged, this, [this, transparencyValue](int value) {
        transparencyValue->setText(QString::number(value) + "%");
        updateLayer("Layer transparency", [value](auto& layer) { layer.transparency = value; });
    });
    // Live feedback while dragging, with a single undo step for the whole gesture.
    connect(transparency_, &QSlider::sliderPressed, this, [this] { document_.history()->beginMacro("Layer transparency"); });
    connect(transparency_, &QSlider::sliderReleased, this, [this] {
        document_.history()->endMacro();
    });
    blend_ = new QComboBox;
    blend_->addItems({"Normal", "Multiply", "Screen", "Overlay"});
    form->addRow("Blend mode", blend_);
    connect(blend_, &QComboBox::currentIndexChanged, this, [this](int index) {
        updateLayer("Layer blend mode", [index](auto& layer) { layer.blend = static_cast<LayerBlend>(index); });
    });
    color_ = button(effectControls, "Effect color…", [this] {
        for (const auto& layer : document_.state().layers) if (layer.id == selectedLayer_) {
            const auto color = QColorDialog::getColor(layer.color, this, "Effect color", QColorDialog::ShowAlphaChannel);
            if (color.isValid()) updateLayer("Effect color", [color](auto& changed) { changed.color = color; });
            break;
        }
    });
    secondaryColor_ = button(effectControls, "Second effect color…", [this] {
        for (const auto& layer : document_.state().layers) if (layer.id == selectedLayer_) {
            const auto color = QColorDialog::getColor(layer.secondaryColor, this, "Second effect color", QColorDialog::ShowAlphaChannel);
            if (color.isValid()) updateLayer("Gradient color", [color](auto& changed) { changed.secondaryColor = color; });
            break;
        }
    });
    effectType_ = new QComboBox;
    effectType_->setObjectName("selectedEffectType");
    for (const auto& definition : effectDefinitions())
        effectType_->addItem(definition.label, static_cast<int>(definition.kind));
    effectControls->addWidget(effectType_);
    connect(effectType_, &QComboBox::currentIndexChanged, this, [this](int) {
        const auto kind = static_cast<EffectKind>(effectType_->currentData().toInt());
        updateLayer("Change effect type", [kind](auto& layer) {
            if (layer.name == effectDefinition(layer.effect).label) layer.name = effectDefinition(kind).label;
            layer.effect = kind;
        });
    });
    animationControls_ = new QWidget;
    auto* animationForm = new QFormLayout(animationControls_);
    animationForm->setContentsMargins(0, 0, 0, 0);
    const QStringList animationLabels{"Spin deg/s", "Pulse Hz", "Hue deg/s"};
    for (int i = 0; i < 3; ++i) {
        auto* value = new QDoubleSpinBox;
        value->setObjectName("animationRate" + QString::number(i));
        value->setRange(i == 1 ? 0 : -360, i == 1 ? 10 : 360);
        value->setKeyboardTracking(false); animation_.append(value);
        animationForm->addRow(animationLabels[i], value);
        connect(value, &QDoubleSpinBox::valueChanged, this, [this] {
            const double spin = animation_[0]->value(), pulse = animation_[1]->value(), hue = animation_[2]->value();
            updateLayer("Animation rates", [=](auto& layer) { layer.spin = spin; layer.pulse = pulse; layer.hueShift = hue; });
        });
    }
    for (const auto& parameter : advancedParameters()) {
        auto* value = new QDoubleSpinBox; value->setObjectName(parameter.id);
        value->setRange(parameter.minimum,parameter.maximum);
        value->setDecimals(parameter.id == "particleCount" || parameter.id == "particleSeed" ? 0 : 2);
        value->setKeyboardTracking(false); advanced_.append(value);
        animationForm->addRow(parameter.label,value);
        connect(value,&QDoubleSpinBox::valueChanged,this,[this,member=parameter.member](double number) {
            updateLayer("Effect parameter",[=](auto& layer) { layer.*member = number; });
        });
    }
    colorCycle_ = new QLineEdit; colorCycle_->setObjectName("colorCycle");
    colorCycle_->setPlaceholderText("#ff0000, #00ff00, #0000ff");
    animationForm->addRow("Cycle colors",colorCycle_);
    connect(colorCycle_,&QLineEdit::editingFinished,this,[this] {
        QVector<QColor> colors;
        for (const auto& text : colorCycle_->text().split(',')) colors.append(QColor(text.trimmed()));
        for (const auto& layer : document_.state().layers) if (layer.id==selectedLayer_ && layer.colorCycle==colors) return;
        updateLayer("Cycle colors",[colors](auto& layer) { layer.colorCycle = colors; });
        refreshProperties();
    });
    effectControls->addWidget(animationControls_);
    maskLinked_ = new QCheckBox("Link mask to effect"); maskLinked_->setObjectName("maskLinked");
    effectControls->addWidget(maskLinked_);
    connect(maskLinked_,&QCheckBox::toggled,this,[this](bool linked) {
        updateLayer("Link mask",[=](auto& layer) { layer.maskLinked = linked; });
    });
    cropControls_ = new QWidget; auto* cropForm = new QFormLayout(cropControls_);
    cropForm->setContentsMargins(0,0,0,0);
    const QStringList cropLabels{"Crop left", "Crop top", "Crop width", "Crop height"};
    for (int i=0;i<4;++i) {
        auto* value = new QDoubleSpinBox; value->setObjectName("imageCrop"+QString::number(i));
        value->setRange(i<2 ? 0 : 0.001,1); value->setDecimals(3); value->setKeyboardTracking(false);
        crop_.append(value); cropForm->addRow(cropLabels[i],value);
        connect(value,&QDoubleSpinBox::valueChanged,this,[this] {
            const QRectF crop(crop_[0]->value(),crop_[1]->value(),crop_[2]->value(),crop_[3]->value());
            updateLayer("Image crop",[=](auto& layer) { layer.crop=crop; }); refreshProperties();
        });
    }
    effectControls->addWidget(cropControls_);
    auto* resetCrop=button(effectControls,"Reset image crop",[this] { updateLayer("Reset crop",[](auto& layer) { layer.crop={0,0,1,1}; }); });
    resetCrop->setObjectName("resetCrop");

    imageFit_ = new QComboBox;
    imageFit_->setObjectName("imageFit");
    imageFit_->setToolTip("Preserve aspect ratio; fill crops equally from opposite edges.");
    imageFit_->addItems({"Fit whole image", "Fill and crop"});
    effectControls->addWidget(imageFit_);
    connect(imageFit_, &QComboBox::currentIndexChanged, this, [this](int index) {
        updateLayer("Image fit", [index](auto& layer) { layer.imageFill = index == 1; });
    });
    geometryControls_ = new QWidget;
    auto* geometryForm = new QFormLayout(geometryControls_);
    geometryForm->setContentsMargins(0, 0, 0, 0);
    const QStringList geometryLabels{"Center X", "Center Y", "Width", "Height", "Rotation °"};
    for (int i = 0; i < geometryLabels.size(); ++i) {
        auto* value = new QDoubleSpinBox;
        value->setObjectName("effectGeometry" + QString::number(i));
        value->setDecimals(1);
        value->setKeyboardTracking(false);
        value->setRange(i < 2 ? -10000 : i == 4 ? -360 : 1, i == 4 ? 360 : 10000);
        geometry_.append(value);
        geometryForm->addRow(geometryLabels[i], value);
        connect(value, &QDoubleSpinBox::valueChanged, this, [this] {
            const QPointF center(geometry_[0]->value(), geometry_[1]->value());
            const QSizeF extent(geometry_[2]->value(), geometry_[3]->value());
            const double rotation = geometry_[4]->value();
            updateLayer("Effect geometry", [center, extent, rotation](auto& layer) {
                layer.center = center; layer.extent = extent; layer.rotation = rotation;
            });
        });
    }
    effectControls->addWidget(geometryControls_);
    for (QWidget* widget : std::initializer_list<QWidget*>{imageFit_,cropControls_,resetCrop,geometryControls_}) {
        effectControls->removeWidget(widget); effectControls->insertWidget(effectControls->indexOf(animationControls_),widget);
    }
    repetitions_ = new QSpinBox;
    repetitions_->setObjectName("effectRepetitions");
    repetitions_->setRange(1, 64);
    repetitions_->setPrefix("Repetitions: ");
    effectControls->addWidget(repetitions_);
    connect(repetitions_, &QSpinBox::valueChanged, this, [this](int value) {
        updateLayer("Effect repetitions", [value](auto& layer) { layer.repetitions = value; });
    });
    effectTarget_ = button(effectControls, "Edit effect", [this] { editingMask_ = false; findChild<QCheckBox*>("cropTool")->setChecked(false); effectsCanvas_->setCropTool(false); refreshProperties(); });
    maskTarget_ = button(effectControls, "Edit mask", [this] { editingMask_ = true; refreshProperties(); });
    effectTarget_->setCheckable(true); maskTarget_->setCheckable(true);
    auto* tools = new QToolBar("Effect tools", this);
    tools->setAllowedAreas(Qt::LeftToolBarArea);
    tools->setMovable(false);
    addToolBar(Qt::LeftToolBarArea, tools);
    effectControls->removeWidget(effectTarget_); effectControls->removeWidget(maskTarget_);
    tools->addWidget(effectTarget_); tools->addWidget(maskTarget_);
    auto* cropTool=new QCheckBox("Crop image"); cropTool->setObjectName("cropTool"); tools->addWidget(cropTool);
    connect(cropTool,&QCheckBox::toggled,this,[this](bool crop) {
        if (crop) editingMask_=false;
        effectsCanvas_->setCropTool(crop); refreshProperties();
    });
    connect(effectsCanvas_,&LightingCanvas::effectGestureStarted,this,[this] {
        playing_->setChecked(false); previewTime_->setValue(0);
    });
    tools->hide();
    connect(tabs_, &QTabWidget::currentChanged, tools, [tools](int index) { tools->setVisible(index == 1); });
    maskEnabled_ = new QCheckBox("Enable mask");
    effectControls->addWidget(maskEnabled_);
    connect(maskEnabled_, &QCheckBox::toggled, this, [this](bool enabled) {
        updateLayer("Toggle mask", [enabled](auto& layer) { layer.maskEnabled = enabled; });
    });
    auto* showMask = new QCheckBox("Show grayscale mask");
    effectControls->addWidget(showMask);
    connect(showMask, &QCheckBox::toggled, effectsCanvas_, &LightingCanvas::setShowMask);
    auto* maskTool = new QComboBox;
    maskTool->addItems({"Mask brush", "Mask gradient · black → white"});
    effectControls->addWidget(maskTool);
    connect(maskTool, &QComboBox::currentIndexChanged, this, [this](int index) {
        effectsCanvas_->setMaskTool(static_cast<LightingCanvas::MaskTool>(index));
    });
    auto* brushForm = new QFormLayout;
    effectControls->addLayout(brushForm);
    auto spin = [brushForm](const QString& name, int low, int high, int value) {
        auto* control = new QSpinBox; control->setRange(low, high); control->setValue(value);
        brushForm->addRow(name, control); return control;
    };
    auto* size = spin("Brush diameter", 2, 320, 60);
    auto* hardness = spin("Hardness %", 0, 100, 30);
    auto* brushOpacity = spin("Brush opacity %", 0, 100, 100);
    auto* gray = spin("Mask gray", 0, 255, 0);
    gray->setToolTip("0 hides; 255 reveals; intermediate values create partial transparency.");
    for (auto* control : {size, hardness, brushOpacity, gray})
        connect(control, &QSpinBox::valueChanged, this, [this, size, hardness, brushOpacity, gray] {
            effectsCanvas_->setBrush(size->value() / 2.0, hardness->value() / 100.0, brushOpacity->value() / 100.0, gray->value());
        });
    button(effectControls, "Fill mask white", [this] { updateLayer("Reveal layer", [](auto& layer) { layer.mask.fill(255); }); });
    button(effectControls, "Fill mask black", [this] { updateLayer("Hide layer with mask", [](auto& layer) { layer.mask.fill(0); }); });
    button(effectControls, "Invert mask", [this] { updateLayer("Invert mask", [](auto& layer) { layer.mask.invertPixels(); }); });
    auto* note = new QLabel("Offline effect preview\nNo hardware output or channel assignments yet.\nWheel zooms; middle drag pans; Escape cancels strokes.");
    note->setWordWrap(true);
    effectControls->addWidget(note);
    connect(tabs_, &QTabWidget::currentChanged, contexts, &QStackedWidget::setCurrentIndex);
    connect(&document_, &LightingDocument::changed, this, &LightingWindow::refresh);
    connect(document_.history(), &QUndoStack::cleanChanged, this, [this](bool clean) { setWindowModified(!clean); });
    setWindowTitle("Lighting[*] — MSI Mystic Light Standalone");
    statusBar()->showMessage("Offline editor · no devices opened. Layout and Effects are separate editing views.");
    refresh();
}

void LightingWindow::updateLayer(const QString& label, const std::function<void(EffectLayer&)>& operation)
{
    if (selectedLayer_.isNull()) return;
    try {
        document_.edit(label, [&](LightingState& state) {
            for (auto& layer : state.layers) if (layer.id == selectedLayer_) operation(layer);
        });
    } catch (const std::exception& error) {
        statusBar()->showMessage(QString::fromUtf8(error.what()), 6000);
        refreshProperties(); // Invalid typed input must not escape a Qt signal handler or alter the document.
    }
}

void LightingWindow::refresh()
{
    QSignalBlocker blockTree(layers_);
    layers_->clear();
    const EffectLayer* selected = nullptr;
    for (auto it = document_.state().layers.crbegin(); it != document_.state().layers.crend(); ++it) {
        auto* item = new QTreeWidgetItem(layers_);
        item->setText(0, it->name);
        item->setFlags((item->flags() | Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsDragEnabled) & ~Qt::ItemIsDropEnabled);
        item->setCheckState(0, it->visible ? Qt::Checked : Qt::Unchecked);
        item->setData(0, Qt::UserRole, it->id);
        item->setIcon(1, QIcon(QPixmap::fromImage(renderEffect(*it).scaled(40, 25, Qt::KeepAspectRatio, Qt::SmoothTransformation))));
        item->setIcon(2, QIcon(QPixmap::fromImage(it->mask.scaled(40, 25))));
        item->setToolTip(1, "Select effect: " + effectDefinition(it->effect).label); item->setToolTip(2, "Select grayscale mask");
        if (it->id == selectedLayer_) { selected = &*it; layers_->setCurrentItem(item, editingMask_ ? 2 : 1); }
    }
    if (!selected) selectedLayer_ = {};
    refreshProperties();
}

void LightingWindow::refreshProperties()
{
    QSignalBlocker blockTransparency(transparency_), blockBlend(blend_), blockMask(maskEnabled_), blockType(effectType_), blockRepeat(repetitions_);
    const EffectLayer* selected = nullptr;
    for (const auto& layer : document_.state().layers) if (layer.id == selectedLayer_) selected = &layer;
    effectsCanvas_->select(selectedLayer_);
    effectsCanvas_->setMaskTarget(editingMask_);
    effectTarget_->setChecked(!editingMask_); maskTarget_->setChecked(editingMask_);
    for (QWidget* widget : std::initializer_list<QWidget*>{transparency_, blend_, color_, effectTarget_, maskTarget_, maskEnabled_})
        widget->setEnabled(selected != nullptr);
    if (editingMask_) { playing_->setChecked(false); previewTime_->setValue(0); }
    playing_->setEnabled(!editingMask_);
    animationControls_->setVisible(selected && !editingMask_);
    findChild<QPushButton*>("resetCrop")->setVisible(selected && !editingMask_ && selected->effect==EffectKind::Image);
    cropControls_->setVisible(selected && !editingMask_ && selected->effect == EffectKind::Image);
    maskLinked_->setEnabled(selected != nullptr);
    auto* cropTool=findChild<QCheckBox*>("cropTool");
    if ((!selected || selected->effect!=EffectKind::Image) && cropTool->isChecked()) cropTool->setChecked(false);
    cropTool->setEnabled(selected && !editingMask_ && selected->effect==EffectKind::Image && !selected->image.isNull());
    effectType_->setEnabled(selected && !editingMask_);
    color_->setVisible(!editingMask_ && selected && selected->effect != EffectKind::Image);
    imageFit_->setVisible(selected && !editingMask_ && selected->effect == EffectKind::Image);
    secondaryColor_->setVisible(selected && !editingMask_ && (isGradient(selected->effect) || isParticle(selected->effect)));
    geometryControls_->setVisible(selected && !editingMask_ && selected->effect != EffectKind::StaticColor);
    repetitions_->setVisible(selected && !editingMask_ && (selected->effect == EffectKind::Stripes || selected->effect == EffectKind::Waves));
    if (selected) {
        transparency_->setValue(selected->transparency);
        findChild<QLabel*>("transparencyValue")->setText(QString::number(selected->transparency) + "%");
        transparency_->setToolTip(QString::number(selected->transparency) + "% transparent");
        blend_->setCurrentIndex(static_cast<int>(selected->blend));
        const double rates[]{selected->spin, selected->pulse, selected->hueShift};
        for (int i = 0; i < 3; ++i) {
            const QSignalBlocker block(animation_[i]); animation_[i]->setValue(rates[i]);
        }
        animation_[0]->setEnabled(selected->effect != EffectKind::StaticColor);
        animation_[2]->setEnabled(true);
        const QSignalBlocker blockLink(maskLinked_), blockCycle(colorCycle_);
        maskLinked_->setChecked(selected->maskLinked);
        QStringList colors; for (const auto& color : selected->colorCycle) colors.append(color.name(QColor::HexArgb));
        colorCycle_->setText(colors.join(", "));
        colorCycle_->setEnabled(selected->effect != EffectKind::Image);
        const auto& parameters = advancedParameters();
        for (int i=0;i<advanced_.size();++i) {
            const QSignalBlocker block(advanced_[i]); advanced_[i]->setValue(selected->*(parameters[i].member));
            const bool particle = parameters[i].id.startsWith("particle");
            const bool show = !particle || isParticle(selected->effect);
            advanced_[i]->setVisible(show);
            if (auto* form=qobject_cast<QFormLayout*>(animationControls_->layout()))
                if (auto* label=form->labelForField(advanced_[i])) label->setVisible(show);
            advanced_[i]->setEnabled((!particle || isParticle(selected->effect)) &&
                (parameters[i].id != "cycleRate" || selected->effect != EffectKind::Image));
        }
        const double crop[]{selected->crop.x(),selected->crop.y(),selected->crop.width(),selected->crop.height()};
        for (int i=0;i<4;++i) { const QSignalBlocker block(crop_[i]); crop_[i]->setValue(crop[i]); }
        const QSignalBlocker blockFit(imageFit_);
        imageFit_->setCurrentIndex(selected->imageFill ? 1 : 0);
        color_->setIcon(colorIcon(selected->color));
        maskEnabled_->setChecked(selected->maskEnabled);
        effectType_->setCurrentIndex(effectType_->findData(static_cast<int>(selected->effect)));
        secondaryColor_->setIcon(colorIcon(selected->secondaryColor));
        repetitions_->setValue(selected->repetitions);
        const QVector<double> values{selected->center.x(), selected->center.y(), selected->extent.width(), selected->extent.height(), selected->rotation};
        for (int i = 0; i < geometry_.size(); ++i) {
            const QSignalBlocker block(geometry_[i]);
            geometry_[i]->setValue(values[i]);
        }
        geometry_[3]->setEnabled(selected->effect != EffectKind::LinearGradient &&
            selected->effect != EffectKind::Stripes && selected->effect != EffectKind::Waves);
    }
}

void LightingWindow::closeEvent(QCloseEvent* event)
{
    if (!confirmDiscard()) { event->ignore(); return; }
    event->accept();
}

bool LightingWindow::saveDocument()
{
    const auto path = fileName_.isEmpty() ? QFileDialog::getSaveFileName(this,
        "Save lighting document", "Untitled.lighting.json", "Lighting documents (*.lighting.json)") : fileName_;
    if (path.isEmpty()) return false;
    try { document_.save(path); fileName_ = path; statusBar()->showMessage("Saved " + path); return true; }
    catch (const std::exception& error) { QMessageBox::warning(this, "Save failed", QString::fromUtf8(error.what())); return false; }
}

bool LightingWindow::confirmDiscard()
{
    if (document_.history()->isClean()) return true;
    const auto answer = QMessageBox::question(this, "Unsaved lighting document",
        "Save your layout, layers and masks before continuing?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    return answer == QMessageBox::Discard || (answer == QMessageBox::Save && saveDocument());
}
