#include "LightingWindow.h"
#include "ComponentEditor.h"
#include "ChannelEditor.h"
#include "StudioWidgets.h"
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QToolButton>
#include <QStyle>
#include <QSplitter>
#include <QStyledItemDelegate>
#include <QApplication>
#include <QActionGroup>
#include <QGridLayout>
#include <QCheckBox>
#include <QCloseEvent>
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
#include <QDialogButtonBox>
#include <QTreeWidgetItemIterator>
#include <QSet>
#include <cmath>
#include <QLineEdit>
#include <QListWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace {
class LayerDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter* painter,const QStyleOptionViewItem& option,const QModelIndex& index) const override {
        QStyleOptionViewItem item(option); initStyleOption(&item,index);
        const auto* widget=item.widget; auto* style=widget ? widget->style() : QApplication::style();
        if(index.column()!=0) {
            style->drawControl(QStyle::CE_ItemViewItem,&item,painter,widget);
            const auto* tree=qobject_cast<const QTreeWidget*>(widget);
            if(tree && tree->currentIndex()==index) {
                painter->save(); painter->setPen(QPen(QColor("#b6cdff"),1.5)); painter->setBrush(Qt::NoBrush);
                painter->drawRoundedRect(item.rect.adjusted(3,5,-3,-5),4,4); painter->restore();
            }
            return;
        }
        const auto textRect=style->subElementRect(QStyle::SE_ItemViewItemText,&item,widget).adjusted(2,0,-4,0);
        // Paint the effect subtitle without putting it into the editable layer name.
        const auto name=item.text; item.text.clear(); style->drawControl(QStyle::CE_ItemViewItem,&item,painter,widget);
        painter->save(); auto font=item.font; font.setBold(true); painter->setFont(font);
        painter->setPen(QColor("#e8eaf0"));
        painter->drawText(QRect(textRect.x(),textRect.center().y()-19,textRect.width(),22),Qt::AlignVCenter,
            painter->fontMetrics().elidedText(name,Qt::ElideRight,textRect.width()));
        font.setBold(false); font.setPointSizeF(std::max(8.0,font.pointSizeF()-1)); painter->setFont(font); painter->setPen(QColor("#b9c8e4"));
        painter->drawText(QRect(textRect.x(),textRect.center().y()+3,textRect.width(),20),Qt::AlignVCenter,
            painter->fontMetrics().elidedText("Effect: "+index.data(Qt::UserRole+1).toString(),Qt::ElideRight,textRect.width())); painter->restore();
    }
    QSize sizeHint(const QStyleOptionViewItem& option,const QModelIndex& index) const override {
        auto size=QStyledItemDelegate::sizeHint(option,index); size.setHeight(58); return size;
    }
};
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
    std::function<QVector<ProfileCandidate>()> scan, QWidget* parent, QVector<DeviceProfile> profiles)
    : QMainWindow(parent), document_(std::move(catalog), nullptr, std::move(profiles))
{
    setWindowTitle("Lighting — MSI Mystic Light Standalone");
    resize(1440, 900);
    setMinimumSize(1060, 680);
    applyStudioStyle(this);
    auto* fileMenu = menuBar()->addMenu("File");
    auto* open = fileMenu->addAction("Open…"); open->setShortcut(QKeySequence::Open);
    connect(open, &QAction::triggered, this, [this] {
        const auto file = QFileDialog::getOpenFileName(this, "Open lighting document", {}, "Lighting documents (*.lighting.json)");
        if (file.isEmpty() || !confirmDiscard()) return;
        try { loadProject(file); }
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
    auto* toolbar = addToolBar("History"); toolbar->setObjectName("historyToolbar");
    toolbar->setMovable(false);
    toolbar->addAction(open); toolbar->addAction(save); toolbar->addSeparator();
    // Keep dynamic undo descriptions in the menu, not in a toolbar that steals canvas width.
    auto* undoButton=new QToolButton; undoButton->setDefaultAction(undo); undoButton->setText("Undo"); toolbar->addWidget(undoButton);
    auto* redoButton=new QToolButton; redoButton->setDefaultAction(redo); redoButton->setText("Redo"); toolbar->addWidget(redoButton);
    connect(undo,&QAction::changed,undoButton,[undoButton] { undoButton->setText("Undo"); });
    connect(redo,&QAction::changed,redoButton,[redoButton] { redoButton->setText("Redo"); });

    tabs_ = new QTabWidget;
    tabs_->setObjectName("lightingViews");
    tabs_->setDocumentMode(true);
    layoutCanvas_ = new LightingCanvas(document_, LightingCanvas::Context::Layout);
    effectsCanvas_ = new LightingCanvas(document_, LightingCanvas::Context::Effects);
    layoutCanvas_->setObjectName("layoutCanvas");
    effectsCanvas_->setObjectName("effectsCanvas");
    tabs_->addTab(layoutCanvas_, "Layout");
    tabs_->addTab(effectsCanvas_, "Effects");
    applicationPages_=new QStackedWidget; applicationPages_->setObjectName("applicationPages");
    applicationPages_->addWidget(tabs_); setCentralWidget(applicationPages_);
    auto* outputDialog=new QDialog(this); outputDialog->setObjectName("outputDialog");
    outputDialog->setWindowTitle("Hardware output"); outputDialog->resize(760,540);
    deviceOutput_=new QVBoxLayout(outputDialog);
    connect(this,&LightingWindow::outputRequested,outputDialog,[outputDialog] { outputDialog->show(); outputDialog->raise(); outputDialog->activateWindow(); });

    auto* dock = new QDockWidget("Properties", this); dock->setObjectName("propertiesDock");
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    auto* contexts = new QStackedWidget;
    auto* sidebarDock=new QDockWidget("Lighting",this); sidebarDock->setObjectName("layersDock");
    sidebarDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    sidebarDock->setMinimumWidth(250); sidebarDock->setMaximumWidth(330);
    auto* sidebarContexts=new QStackedWidget; sidebarDock->setWidget(sidebarContexts);
    addDockWidget(Qt::LeftDockWidgetArea,sidebarDock);
    auto* componentPanel=new QWidget; auto* componentStack=new QVBoxLayout(componentPanel);
    componentStack->setContentsMargins(12,12,12,12); componentStack->setSpacing(10);
    sidebarContexts->addWidget(componentPanel);
    auto* layersPanel=new QWidget; auto* layerStack=new QVBoxLayout(layersPanel);
    layerStack->setContentsMargins(12,12,12,12); layerStack->setSpacing(10);
    sidebarContexts->addWidget(layersPanel);
    dock->setWidget(contexts);
    auto addPanel = [contexts](QWidget* panel) {
        auto* scroll = new QScrollArea;
        scroll->setWidgetResizable(true);
        scroll->setWidget(panel);
        contexts->addWidget(scroll);
    };
    addDockWidget(Qt::RightDockWidgetArea, dock);
    dock->setMinimumWidth(380); dock->setMaximumWidth(540);
    auto* layoutPanel = new QWidget;
    auto* layoutControls = new QVBoxLayout(layoutPanel);
    layoutControls->setContentsMargins(14,12,14,12); layoutControls->setSpacing(10);
    addPanel(layoutPanel);
    layoutControls->addWidget(new QLabel("Components"));
    catalog_ = new QComboBox;
    catalog_->setObjectName("componentCatalog");
    for (const auto& component : document_.catalog()) catalog_->addItem(component.name, component.id);
    layoutControls->addWidget(catalog_);
    button(layoutControls, "Add component", [this] {
        if (document_.state().placements.size() >= 256) { statusBar()->showMessage("Document limit: 256 components."); return; }
        if (catalog_->currentIndex() >= 0) layoutCanvas_->select(document_.addComponent(catalog_->currentData().toString()));
    })->setObjectName("addComponent");
    button(layoutControls, "Create component", [this] {
        ComponentDefinition initial; initial.id = "custom.component"; initial.name = "Custom component";
        initial.positions = {{0, 0}}; initial.mapping = {0};
        ComponentEditor editor(initial, false, this);
        if (editor.exec() != QDialog::Accepted) return;
        try {
            const auto component = editor.component(); document_.defineComponent(component);
            catalog_->setCurrentIndex(catalog_->findData(component.id));
        } catch (const std::exception& error) { statusBar()->showMessage(QString::fromUtf8(error.what()), 6000); }
    });
    button(layoutControls, "Edit component geometry", [this] {
        const auto id = catalog_->currentData().toString();
        if (!document_.catalog().contains(id)) return;
        ComponentEditor editor(document_.catalog()[id], true, this);
        if (editor.exec() != QDialog::Accepted) return;
        try { document_.defineComponent(editor.component(), true); }
        catch (const std::exception& error) { statusBar()->showMessage(QString::fromUtf8(error.what()), 6000); }
    })->setObjectName("editComponent");
    button(layoutControls, "Import component JSON", [this] {
        const auto file = QFileDialog::getOpenFileName(this, "Import component definition", {}, "Component definitions (*.json)");
        if (file.isEmpty()) return;
        try {
            document_.importComponent(file);
            statusBar()->showMessage("Imported component definition. Select it and add an instance to Layout.", 6000);
        } catch (const std::exception& error) { statusBar()->showMessage(QString::fromUtf8(error.what()), 6000); }
    });
    auto* removeComponent=button(layoutControls, "Remove selected component", [this] {
        const auto id = layoutCanvas_->selection();
        if (id.isNull()) return;
        try {
            document_.edit("Remove component", [&](LightingState& state) {
                for(auto& assignment:state.assignments) assignment.placements.removeAll(id);
                // An empty direct chain has no samples; remove its output scope and layer references atomically.
                QSet<QUuid> empty;
                for(const auto& assignment:state.assignments) if(assignment.software && assignment.placements.isEmpty()) empty.insert(assignment.id);
                state.assignments.removeIf([&](const auto& assignment) { return empty.contains(assignment.id); });
                for(auto& layer:state.layers) for(const auto& removed:empty) layer.channelTargets.removeAll(removed);
                state.placements.removeIf([&](const auto& placement) { return placement.id == id; });
            });
            layoutCanvas_->select({});
        } catch (const std::exception& error) {
            statusBar()->showMessage("Cannot remove component: " + QString::fromUtf8(error.what()), 6000);
        }
    });
    removeComponent->setObjectName("removeComponent");
    auto* library=new QWidget; auto* libraryControls=new QVBoxLayout(library); libraryControls->setContentsMargins(0,0,0,0);
    while(auto* item=layoutControls->takeAt(0)) { if(item->widget()) libraryControls->addWidget(item->widget()); delete item; }
    disclosure(componentStack,"Component library",library);
    components_=new QTreeWidget; components_->setObjectName("layoutComponents");
    components_->setHeaderHidden(true); components_->setRootIsDecorated(true); componentStack->insertWidget(0,components_,1);
    connect(components_,&QTreeWidget::currentItemChanged,this,[this](QTreeWidgetItem* item) {
        if(item) {
            layoutCanvas_->select(item->data(0,Qt::UserRole).toUuid());
            auto* header=item->parent() ? item->parent() : item;
            selectedHeader_=header->data(0,Qt::UserRole+1).toMap();
            refreshHeader();
        }
    });
    connect(components_,&QTreeWidget::itemClicked,this,[this](QTreeWidgetItem* item,int) {
        layoutCanvas_->select(item->data(0,Qt::UserRole).toUuid());
    });
    auto* layoutTitle=new QLabel("Arrange your lighting"); layoutTitle->setProperty("heading",true); layoutControls->addWidget(layoutTitle);
    auto* layoutHint=new QLabel("Select a header to move its group, or a child to move one component.");
    layoutHint->setWordWrap(true); layoutHint->setProperty("muted",true); layoutControls->addWidget(layoutHint);
    auto* placementPrecision=new QWidget; auto* placementForm = new QFormLayout(placementPrecision);
    auto* placementSection=disclosure(layoutControls,"Precise placement",placementPrecision);
    placementSection->findChild<QToolButton*>()->setChecked(true);
    const QStringList placementLabels{"Position X", "Position Y", "Scale X %", "Scale Y %", "Rotation deg"};
    for (int i = 0; i < placementLabels.size(); ++i) {
        auto* control = new QDoubleSpinBox;
        control->setObjectName(QString("placement%1").arg(i));
        control->setDecimals(2);
        control->setRange(i < 2 ? -10000 : i < 4 ? 1 : -360, i < 4 ? 10000 : 360);
        control->setKeyboardTracking(false);
        placementForm->addRow(placementLabels[i], control);
        placementControls_.append(control);
        connect(control, &QDoubleSpinBox::valueChanged, this, [this, i](double value) {
            const auto id = layoutCanvas_->selection();
            if (id.isNull()) return;
            try {
                document_.edit("Transform component", [&](LightingState& state) {
                    for (auto& placement : state.placements) if (placement.id == id) {
                        if (i == 0) placement.origin.setX(value);
                        if (i == 1) placement.origin.setY(value);
                        if (i == 2) placement.scale.setWidth(value / 100);
                        if (i == 3) placement.scale.setHeight(value / 100);
                        if (i == 4) placement.rotation = value;
                    }
                });
            } catch (const std::exception& error) {
                statusBar()->showMessage(QString::fromUtf8(error.what()), 6000);
                refreshPlacement();
            }
        });
    }
    connect(layoutCanvas_, &LightingCanvas::selectionChanged, this, [this] {
        refreshPlacement(); const QSignalBlocker block(components_);
        for(QTreeWidgetItemIterator it(components_); *it; ++it)
            if(!layoutCanvas_->selection().isNull() && (*it)->data(0,Qt::UserRole).toUuid()==layoutCanvas_->selection()) {
                components_->setCurrentItem(*it); auto* header=(*it)->parent() ? (*it)->parent() : *it;
                if(header) selectedHeader_=header->data(0,Qt::UserRole+1).toMap();
            }
        refreshHeader();
    });
    layoutControls->addWidget(removeComponent);
    auto* instructions = new QLabel("Drag LEDs to move their component.\nArrow keys move; Shift moves by 10.\nWheel zooms; middle drag pans.\n\nLayout changes sampling positions only.");
    instructions->setWordWrap(true);
    instructions->setProperty("muted",true);
    layoutControls->addWidget(instructions);
    layoutControls->addStretch();

    // Match the original studio: headers beside the layout, settings beside the selection.
    auto* headerPanel=new QWidget; auto* headerControls=new QVBoxLayout(headerPanel);
    headerControls->setContentsMargins(0,0,0,0);
    headerTitle_=new QLabel("Select a header"); headerTitle_->setProperty("heading",true); headerControls->addWidget(headerTitle_);
    headerSummary_=new QLabel; headerSummary_->setWordWrap(true); headerSummary_->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum); headerControls->addWidget(headerSummary_);
    button(headerControls,"Add fans / strips",[this] { addHeaderComponents(); })->setObjectName("addHeaderComponents");
    button(headerControls,"Header settings",[this] {
        if(selectedHeader_.isEmpty()) return;
        try {
            ChannelEditor editor(document_,selectedHeaderAssignment(),this);
            if(editor.exec()==QDialog::Accepted) document_.assignChannel(editor.assignment());
        } catch(const std::exception& error) { statusBar()->showMessage(QString::fromUtf8(error.what()),6000); }
    })->setObjectName("headerSettings");
    layoutControls->insertWidget(0,headerPanel);
    button(componentStack,"Refresh headers",[this] { scanHeaders(); })->setObjectName("refreshHeaders");
    scan_=std::move(scan);
    QTimer::singleShot(0,this,[this] { scanHeaders(); });

    auto* previewToolbar = addToolBar("Preview"); previewToolbar->setObjectName("previewToolbar");
    playing_ = new QPushButton("Play"); playing_->setCheckable(true); playing_->setObjectName("playPreview");
    playing_->setIcon(style()->standardIcon(QStyle::SP_MediaPlay)); playing_->setAccessibleName("Play or pause the effect preview");
    previewTime_ = new QDoubleSpinBox; previewTime_->setObjectName("previewTime");
    previewTime_->setRange(0, 86400); previewTime_->setDecimals(2); previewTime_->setSuffix(" s");
    previewTime_->setKeyboardTracking(false); previewTime_->setAccessibleName("Preview time in seconds");
    previewToolbar->setMovable(false);
    previewToolbar->addWidget(playing_); previewToolbar->addWidget(previewTime_);
    previewChannel_=new QComboBox; previewChannel_->setObjectName("previewChannel");
    previewChannel_->setMaximumWidth(180);
    previewChannel_->addItem("All layers",QUuid{}); previewToolbar->addWidget(previewChannel_);
    auto* fit=previewToolbar->addAction("Fit canvas"); fit->setObjectName("fitCanvas"); fit->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    connect(fit,&QAction::triggered,this,[this] { (tabs_->currentIndex()==0 ? layoutCanvas_ : effectsCanvas_)->fitToView(); });
    auto* output=previewToolbar->addAction("Output: Stopped"); output->setObjectName("reviewOutput");
    output->setToolTip("Review channel assignments, connect devices or stop output");
    previewToolbar->removeAction(output); toolbar->addSeparator(); toolbar->addAction(output);
    connect(output,&QAction::triggered,this,&LightingWindow::outputRequested);
    connect(previewChannel_,&QComboBox::currentIndexChanged,this,&LightingWindow::updatePreview);
    auto* timer = new QTimer(this); timer->setInterval(33);
    connect(playing_, &QPushButton::toggled, this, [this, timer](bool play) {
        playing_->setText(play ? "Pause" : "Play"); playing_->setIcon(style()->standardIcon(play ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay));
        if (play) { playbackStart_ = previewTime_->value(); previewClock_.restart(); timer->start(); }
        else timer->stop();
    });
    connect(timer, &QTimer::timeout, this, [this] {
        previewTime_->setValue(std::min(86400.0, playbackStart_ + previewClock_.elapsed()/1000.0));
        if (previewTime_->value() >= 86400) playing_->setChecked(false);
    });
    connect(previewTime_, &QDoubleSpinBox::editingFinished, this, [this] { playing_->setChecked(false); });
    connect(previewTime_, &QDoubleSpinBox::valueChanged, this, [this] { updatePreview(); });
    auto* effectPanel = new QWidget;
    auto* effectControls = new QVBoxLayout(effectPanel);
    effectControls->setContentsMargins(14,12,14,12); effectControls->setSpacing(10);
    previewOverview_=new QLabel; previewOverview_->setObjectName("compositionPreview");
    previewOverview_->setFixedSize(96,60);
    previewOverview_->setAlignment(Qt::AlignCenter); previewOverview_->setToolTip("Live composition preview for the selected output channel");
    auto* inspectorHeading=new QHBoxLayout; effectControls->addLayout(inspectorHeading);
    inspectorHeading->addWidget(previewOverview_);
    inspectorTitle_=new QLabel("Select a layer"); inspectorTitle_->setProperty("heading",true); inspectorTitle_->setWordWrap(true); inspectorHeading->addWidget(inspectorTitle_,1);
    inspectorTabs_=new QTabWidget; inspectorTabs_->setObjectName("layerInspectorTabs");
    effectControls->addWidget(inspectorTabs_);
    auto* appearance=new QWidget; auto* appearanceControls=new QVBoxLayout(appearance); appearanceControls->setContentsMargins(0,12,0,0); appearanceControls->setSpacing(12);
    auto* motion=new QWidget; auto* motionControls=new QVBoxLayout(motion); motionControls->setContentsMargins(0,12,0,0);
    auto* maskProperties=new QWidget; auto* maskControls=new QVBoxLayout(maskProperties); maskControls->setContentsMargins(0,12,0,0); maskControls->setSpacing(10);
    inspectorTabs_->addTab(appearance,"Effect"); inspectorTabs_->addTab(motion,"Motion"); inspectorTabs_->addTab(maskProperties,"Mask");
    connect(inspectorTabs_,&QTabWidget::currentChanged,this,[this](int page) {
        editingMask_=page==2; if(page!=2) { effectsCanvas_->setCropTool(false); findChild<QCheckBox*>("cropTool")->setChecked(false); }
        refreshProperties();
    });
    addPanel(effectPanel);
    button(layerStack, "New layer", [this] {
        if (document_.state().layers.size() >= 64) { statusBar()->showMessage("Document limit: 64 layers."); return; }
        const auto kind=chooseEffect(this); if(!kind) return;
        editingMask_=false; inspectorTabs_->setCurrentIndex(0);
        selectedLayer_=document_.addLayer(*kind); refresh();
    })->setProperty("primary",true);
    button(layerStack, "Import image...", [this] {
        const auto file = QFileDialog::getOpenFileName(this, "Import image", {}, "Images (*.png *.jpg *.jpeg *.gif *.bmp *.webp)");
        if (file.isEmpty()) return;
        try { selectedLayer_ = document_.importImage(file); editingMask_ = false; refresh(); }
        catch (const std::exception& error) { QMessageBox::warning(this, "Import failed", QString::fromUtf8(error.what())); }
    });
    auto* tree = new LayerTree;
    layers_ = tree;
    layers_->setObjectName("effectLayers");
    layers_->setMinimumHeight(180);
    layers_->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    layers_->setHeaderLabels({"Layer / effect", "Source", "Mask"});
    layers_->setRootIsDecorated(false);
    layers_->setItemDelegate(new LayerDelegate(layers_));
    layers_->setSelectionMode(QAbstractItemView::SingleSelection);
    layers_->setSelectionBehavior(QAbstractItemView::SelectRows);
    layers_->setDragDropMode(QAbstractItemView::InternalMove);
    layers_->setDefaultDropAction(Qt::MoveAction);
    layers_->setIconSize({44, 32});
    layers_->header()->setStretchLastSection(false);
    layers_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    layers_->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    layers_->header()->setSectionResizeMode(2, QHeaderView::Fixed);
    layers_->setColumnWidth(1, 52); layers_->setColumnWidth(2, 52);
    layerStack->addWidget(layers_,1);
    connect(layers_, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item, int column) {
        selectedLayer_ = item->data(0, Qt::UserRole).toUuid();
        if (column > 0) { editingMask_ = column == 2; inspectorTabs_->setCurrentIndex(editingMask_ ? 2 : 0); }
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
        const auto name = item->text(0).section('\n',0,0).trimmed();
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
    button(layerStack, "Duplicate layer", [this] {
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
    button(layerStack, "Delete layer", [this] {
        if (selectedLayer_.isNull()) return;
        const auto id = selectedLayer_;
        document_.edit("Delete layer", [&](LightingState& state) {
            state.layers.removeIf([&](const auto& layer) { return layer.id == id; });
        });
    });
    auto* form = new QFormLayout;
    appearanceControls->addLayout(form);
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
    blend_->setToolTip("How this layer combines with visible layers below it: Normal replaces, Multiply darkens, Screen lightens, Overlay increases contrast.");
    form->addRow("Blend mode", blend_);
    connect(blend_, &QComboBox::currentIndexChanged, this, [this](int index) {
        updateLayer("Layer blend mode", [index](auto& layer) { layer.blend = static_cast<LayerBlend>(index); });
    });
    auto* routing=new QWidget; auto* routingControls=new QVBoxLayout(routing); routingControls->setContentsMargins(0,0,0,0);
    disclosure(appearanceControls,"Output channels",routing);
    allChannels_=new QCheckBox("Layer targets all software channels"); allChannels_->setObjectName("allChannels");
    routingControls->addWidget(allChannels_);
    channelTargets_=new QListWidget; channelTargets_->setObjectName("layerChannelTargets"); channelTargets_->setMaximumHeight(120);
    routingControls->addWidget(channelTargets_);
    connect(allChannels_,&QCheckBox::toggled,this,[this](bool all) {
        updateLayer("Layer channel scope",[all](EffectLayer& layer) { layer.allChannels=all; layer.channelTargets.clear(); });
    });
    connect(channelTargets_,&QListWidget::itemChanged,this,[this](QListWidgetItem*) {
        QVector<QUuid> targets;
        for(int row=0;row<channelTargets_->count();++row) if(channelTargets_->item(row)->checkState()==Qt::Checked)
            targets.append(channelTargets_->item(row)->data(Qt::UserRole).toUuid());
        const auto selected=selectedLayer_;
        // Defer rebuilding the list until its itemChanged emission has completed.
        QTimer::singleShot(0,this,[this,targets,selected] {
            if(selectedLayer_==selected) updateLayer("Layer channel targets",[targets](EffectLayer& layer) { layer.allChannels=false; layer.channelTargets=targets; });
        });
    });
    color_=new ColorEditor; color_->setObjectName("effectColor"); appearanceControls->addWidget(color_);
    connect(color_,&ColorEditor::colorChanged,this,[this](QColor color) { updateLayer("Effect color",[color](auto& layer) { layer.color=color; }); });
    secondaryColor_=new ColorEditor; secondaryColor_->setObjectName("secondaryEffectColor"); appearanceControls->addWidget(secondaryColor_);
    connect(secondaryColor_,&ColorEditor::colorChanged,this,[this](QColor color) { updateLayer("Secondary color",[color](auto& layer) { layer.secondaryColor=color; }); });
    gradientColors_=new ColorListEditor(2,16); gradientColors_->setObjectName("gradientColors"); appearanceControls->addWidget(gradientColors_);
    connect(gradientColors_,&ColorListEditor::colorsChanged,this,[this](const QVector<QColor>& colors) {
        updateLayer("Gradient colors",[colors](auto& layer) { layer.gradientColors=colors; layer.color=colors.first(); layer.secondaryColor=colors.last(); });
    });
    effectType_ = new QComboBox;
    effectType_->setObjectName("selectedEffectType");
    for (const auto& definition : effectDefinitions())
        effectType_->addItem(definition.label, static_cast<int>(definition.kind));
    appearanceControls->addWidget(effectType_);
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
        value->setAccessibleName(animationLabels[i]);
        animationForm->addRow(animationLabels[i], sliderField(value));
        connect(value, &QDoubleSpinBox::valueChanged, this, [this] {
            const double spin = animation_[0]->value(), pulse = animation_[1]->value(), hue = animation_[2]->value();
            updateLayer("Animation rates", [=](auto& layer) { layer.spin = spin; layer.pulse = pulse; layer.hueShift = hue; });
        });
    }
    auto* advancedContent=new QWidget; auto* advancedForm=new QGridLayout(advancedContent); advancedForm->setContentsMargins(0,0,0,0);
    int advancedIndex=0;
    for (const auto& parameter : advancedParameters()) {
        auto* value = new QDoubleSpinBox; value->setObjectName(parameter.id);
        value->setRange(parameter.minimum,parameter.maximum);
        value->setDecimals(parameter.id == "particleCount" || parameter.id == "particleSeed" ? 0 : 2);
        value->setKeyboardTracking(false); advanced_.append(value);
        auto* label=new QLabel(parameter.label); label->setWordWrap(true); label->setMaximumWidth(86);
        value->setFixedWidth(72); value->setProperty("parameterLabel",QVariant::fromValue(static_cast<QObject*>(label)));
        const int column=(advancedIndex/6)*2,row=advancedIndex%6; ++advancedIndex;
        advancedForm->addWidget(label,row,column); advancedForm->addWidget(value,row,column+1);
        connect(value,&QDoubleSpinBox::valueChanged,this,[this,member=parameter.member](double number) {
            updateLayer("Effect parameter",[=](auto& layer) { layer.*member = number; });
        });
    }
    colorCycle_=new ColorListEditor(2,16); colorCycle_->setObjectName("colorCycle");
    advancedForm->addWidget(new QLabel("Color cycle palette"),6,0,1,4);
    advancedForm->addWidget(colorCycle_,7,0,1,4);
    connect(colorCycle_,&ColorListEditor::colorsChanged,this,[this](const QVector<QColor>& colors) {
        updateLayer("Cycle colors",[colors](auto& layer) { layer.colorCycle=colors; });
    });
    motionControls->addWidget(new QLabel("Play preview to see motion on the canvas."));
    motionControls->addWidget(animationControls_);
    motionControls->addWidget(advancedContent);
    motionControls->addStretch();
    maskLinked_ = new QCheckBox("Link mask to effect"); maskLinked_->setObjectName("maskLinked");
    maskControls->addWidget(maskLinked_);
    connect(maskLinked_,&QCheckBox::toggled,this,[this](bool linked) {
        updateLayer("Link mask",[=](auto& layer) { layer.maskLinked = linked; });
    });
    cropControls_ = new QWidget; auto* cropForm = new QGridLayout(cropControls_);
    cropForm->setContentsMargins(0,0,0,0);
    const QStringList cropLabels{"Crop left", "Crop top", "Crop width", "Crop height"};
    for (int i=0;i<4;++i) {
        auto* value = new QDoubleSpinBox; value->setObjectName("imageCrop"+QString::number(i));
        value->setRange(i<2 ? 0 : 0.001,1); value->setDecimals(3); value->setKeyboardTracking(false);
        crop_.append(value); value->setFixedWidth(72);
        auto* label=new QLabel(cropLabels[i]); label->setWordWrap(true); value->setProperty("parameterLabel",QVariant::fromValue(static_cast<QObject*>(label)));
        cropForm->addWidget(label,i/2,(i%2)*2); cropForm->addWidget(value,i/2,(i%2)*2+1);
        connect(value,&QDoubleSpinBox::valueChanged,this,[this] {
            const QRectF crop(crop_[0]->value(),crop_[1]->value(),crop_[2]->value(),crop_[3]->value());
            updateLayer("Image crop",[=](auto& layer) { layer.crop=crop; }); refreshProperties();
        });
    }
    auto* cropSection=disclosure(appearanceControls,"Precise image crop",cropControls_);
    cropSection->setObjectName("cropSection");
    cropSection->findChild<QToolButton*>()->setChecked(true);
    auto* resetCrop=button(appearanceControls,"Reset image crop",[this] { updateLayer("Reset crop",[](auto& layer) { layer.crop={0,0,1,1}; }); });
    resetCrop->setObjectName("resetCrop");

    imageFit_ = new QComboBox;
    imageFit_->setObjectName("imageFit");
    imageFit_->setToolTip("Preserve aspect ratio; fill crops equally from opposite edges.");
    imageFit_->addItems({"Fit whole image", "Fill and crop"});
    appearanceControls->addWidget(imageFit_);
    connect(imageFit_, &QComboBox::currentIndexChanged, this, [this](int index) {
        updateLayer("Image fit", [index](auto& layer) { layer.imageFill = index == 1; });
    });
    geometryControls_ = new QWidget;
    auto* geometryForm = new QGridLayout(geometryControls_);
    geometryForm->setContentsMargins(0, 0, 0, 0);
    const QStringList geometryLabels{"Center X", "Center Y", "Width", "Height", "Rotation °"};
    for (int i = 0; i < geometryLabels.size(); ++i) {
        auto* value = new QDoubleSpinBox;
        value->setObjectName("effectGeometry" + QString::number(i));
        value->setDecimals(1);
        value->setKeyboardTracking(false);
        value->setRange(i < 2 ? -10000 : i == 4 ? -360 : 1, i == 4 ? 360 : 10000);
        geometry_.append(value);
        value->setFixedWidth(72); auto* label=new QLabel(geometryLabels[i]); label->setWordWrap(true);
        value->setProperty("parameterLabel",QVariant::fromValue(static_cast<QObject*>(label)));
        geometryForm->addWidget(label,i/2,(i%2)*2); geometryForm->addWidget(value,i/2,(i%2)*2+1);
        connect(value, &QDoubleSpinBox::valueChanged, this, [this] {
            const QPointF center(geometry_[0]->value(), geometry_[1]->value());
            const QSizeF extent(geometry_[2]->value(), geometry_[3]->value());
            const double rotation = geometry_[4]->value();
            updateLayer("Effect geometry", [center, extent, rotation](auto& layer) {
                layer.center = center; layer.extent = extent; layer.rotation = rotation;
            });
        });
    }
    auto* geometrySection=disclosure(appearanceControls,"Precise transform",geometryControls_);
    geometrySection->setObjectName("geometrySection");
    geometrySection->findChild<QToolButton*>()->setChecked(true);
    repetitions_ = new QSpinBox;
    repetitions_->setObjectName("effectRepetitions");
    repetitions_->setRange(1, 64);
    repetitions_->setPrefix("Repetitions: ");
    appearanceControls->addWidget(repetitions_);
    connect(repetitions_, &QSpinBox::valueChanged, this, [this](int value) {
        updateLayer("Effect repetitions", [value](auto& layer) { layer.repetitions = value; });
    });
    effectTarget_ = button(appearanceControls, "Edit effect", [this] { editingMask_ = false; inspectorTabs_->setCurrentIndex(0); findChild<QCheckBox*>("cropTool")->setChecked(false); effectsCanvas_->setCropTool(false); refreshProperties(); });
    maskTarget_ = button(appearanceControls, "Edit mask", [this] { editingMask_ = true; inspectorTabs_->setCurrentIndex(2); refreshProperties(); });
    effectTarget_->setCheckable(true); maskTarget_->setCheckable(true);
    auto* tools = new QToolBar("Effect tools", this);
    tools->setObjectName("effectTools");
    tools->setAllowedAreas(Qt::LeftToolBarArea);
    tools->setMovable(false);
    addToolBar(Qt::LeftToolBarArea, tools);
    appearanceControls->removeWidget(effectTarget_); appearanceControls->removeWidget(maskTarget_);
    effectTarget_->setText("Move"); effectTarget_->setToolTip("Move or transform the selected effect (V)"); effectTarget_->setShortcut(QKeySequence(Qt::Key_V));
    maskTarget_->setText("Brush"); maskTarget_->setToolTip("Paint the layer mask (B)"); maskTarget_->setShortcut(QKeySequence(Qt::Key_B));
    effectTarget_->setObjectName("moveEffectTool"); maskTarget_->setObjectName("paintMaskTool");
    effectTarget_->setAccessibleName("Move effect"); maskTarget_->setAccessibleName("Paint mask");
    effectTarget_->setText(""); maskTarget_->setText("");
    effectTarget_->setIcon(studioToolIcon("move")); maskTarget_->setIcon(studioToolIcon("brush"));
    effectTarget_->setFixedSize(42,42); maskTarget_->setFixedSize(42,42);
    tools->addWidget(effectTarget_); tools->addWidget(maskTarget_);
    auto* cropTool=new QCheckBox; cropTool->setObjectName("cropTool"); cropTool->setParent(this); cropTool->hide();
    auto* cropButton=new QToolButton; cropButton->setIcon(studioToolIcon("crop")); cropButton->setCheckable(true); cropButton->setFixedSize(42,42);
    cropButton->setObjectName("cropImageTool");
    cropButton->setToolTip("Crop image on canvas (C)"); cropButton->setAccessibleName("Crop image"); cropButton->setShortcut(QKeySequence(Qt::Key_C)); tools->addWidget(cropButton);
    connect(cropButton,&QToolButton::toggled,cropTool,&QCheckBox::setChecked);
    connect(cropTool,&QCheckBox::toggled,cropButton,&QToolButton::setChecked);
    connect(cropTool,&QCheckBox::toggled,this,[this](bool crop) {
        if (crop) editingMask_=false;
        effectsCanvas_->setCropTool(crop); refreshProperties();
    });
    connect(effectsCanvas_,&LightingCanvas::effectGestureStarted,this,[this] {
        // Source geometry is authored at base time, not at a channel's offset clock.
        playing_->setChecked(false); previewChannel_->setCurrentIndex(0); previewTime_->setValue(0);
    });
    auto* effectPalette=new QWidget; auto* paletteGrid=new QGridLayout(effectPalette); paletteGrid->setContentsMargins(0,8,0,0); paletteGrid->setSpacing(4);
    int effectIndex=0;
    for(const auto& definition:effectDefinitions()) {
        auto* tool=new QToolButton; tool->setObjectName("addEffect_"+definition.id); tool->setFixedSize(34,34);
        EffectLayer sample; sample.effect=definition.kind; sample.color=QColor("#91aaff"); sample.secondaryColor=QColor("#ff91ba");
        tool->setIcon(QIcon(QPixmap::fromImage(renderEffect(sample,1).copy(QRect(180,60,280,280)).scaled(28,28,Qt::KeepAspectRatio,Qt::SmoothTransformation))));
        if(definition.kind==EffectKind::Image) tool->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
        tool->setStyleSheet("QToolButton { padding: 2px; }"); tool->setIconSize({28,28}); tool->setAccessibleName("Add "+definition.label); tool->setToolTip("Add "+definition.label+" as a new effect layer with its own mask.");
        connect(tool,&QToolButton::clicked,this,[this,kind=definition.kind] {
            if(kind==EffectKind::Image) {
                for(auto* control:findChildren<QPushButton*>()) if(control->text()=="Import image...") { control->click(); break; }
            } else if(document_.state().layers.size()<64) {
                editingMask_=false; inspectorTabs_->setCurrentIndex(0); selectedLayer_=document_.addLayer(kind); refresh();
            }
        });
        paletteGrid->addWidget(tool,effectIndex/2,effectIndex%2); ++effectIndex;
    }
    tools->addSeparator(); tools->addWidget(effectPalette);
    tools->hide();
    connect(tabs_, &QTabWidget::currentChanged, tools, [tools](int index) { tools->setVisible(index == 1); });
    maskEnabled_ = new QCheckBox("Enable mask");
    maskControls->addWidget(maskEnabled_);
    connect(maskEnabled_, &QCheckBox::toggled, this, [this](bool enabled) {
        updateLayer("Toggle mask", [enabled](auto& layer) { layer.maskEnabled = enabled; });
    });
    auto* showMask = new QCheckBox("Show grayscale mask");
    maskControls->addWidget(showMask);
    connect(showMask, &QCheckBox::toggled, effectsCanvas_, &LightingCanvas::setShowMask);
    auto* maskTool = new QComboBox;
    maskTool->addItems({"Mask brush", "Mask gradient · black → white"});
    maskControls->addWidget(maskTool);
    connect(maskTool, &QComboBox::currentIndexChanged, this, [this](int index) {
        effectsCanvas_->setMaskTool(static_cast<LightingCanvas::MaskTool>(index));
    });
    auto* brushForm = new QFormLayout;
    maskControls->addLayout(brushForm);
    auto spin = [brushForm](const QString& name, int low, int high, int value) {
        auto* control = new QDoubleSpinBox; control->setDecimals(0); control->setRange(low, high); control->setValue(value); control->setAccessibleName(name);
        brushForm->addRow(name, sliderField(control)); return control;
    };
    auto* size = spin("Brush diameter", 2, 320, 60);
    auto* hardness = spin("Hardness %", 0, 100, 30);
    auto* brushOpacity = spin("Brush opacity %", 0, 100, 100);
    auto* gray = spin("Mask gray", 0, 255, 0);
    gray->setToolTip("0 hides; 255 reveals; intermediate values create partial transparency.");
    for (auto* control : {size, hardness, brushOpacity, gray})
        connect(control, &QDoubleSpinBox::valueChanged, this, [this, size, hardness, brushOpacity, gray] {
            effectsCanvas_->setBrush(size->value() / 2.0, hardness->value() / 100.0, brushOpacity->value() / 100.0, gray->value());
        });
    button(maskControls, "Fill mask white", [this] { updateLayer("Reveal layer", [](auto& layer) { layer.mask.fill(255); }); });
    button(maskControls, "Fill mask black", [this] { updateLayer("Hide layer with mask", [](auto& layer) { layer.mask.fill(0); }); });
    button(maskControls, "Invert mask", [this] { updateLayer("Invert mask", [](auto& layer) { layer.mask.invertPixels(); }); });
    maskControls->addStretch(); appearanceControls->addStretch(); effectControls->addStretch();
    auto* layerHint=new QLabel("Drag to reorder. Double-click a name to rename.\nSelect a source or mask thumbnail to edit it.");
    layerHint->setWordWrap(true); layerHint->setProperty("muted",true); layerStack->addWidget(layerHint);
    connect(tabs_, &QTabWidget::currentChanged, sidebarContexts, &QStackedWidget::setCurrentIndex);
    connect(tabs_, &QTabWidget::currentChanged, contexts, &QStackedWidget::setCurrentIndex);
    connect(effectsCanvas_,&LightingCanvas::addRequested,this,[this] {
        for(auto* control:findChildren<QPushButton*>()) if(control->text()=="New layer") { control->click(); break; }
    });
    connect(layoutCanvas_,&LightingCanvas::addRequested,this,[this] { addHeaderComponents(); });
    connect(&document_, &LightingDocument::changed, this, &LightingWindow::refresh);
    connect(document_.history(), &QUndoStack::cleanChanged, this, [this](bool clean) { setWindowModified(!clean); });
    setWindowTitle("Lighting Studio[*] — MSI Mystic Light Standalone");
    statusBar()->showMessage("Lighting Studio - Create effects, arrange components, then review output.");
    auto* viewMenu=menuBar()->addMenu("View"); viewMenu->setObjectName("workspaceViewMenu");
    viewMenu->addAction(sidebarDock->toggleViewAction()); viewMenu->addAction(dock->toggleViewAction());
    auto* resetWorkspace=viewMenu->addAction("Reset workspace"); resetWorkspace->setObjectName("resetWorkspace");
    connect(resetWorkspace,&QAction::triggered,this,[this,sidebarDock,dock] {
        const bool studio=applicationPages_->currentIndex()==0; sidebarDock->setVisible(studio); dock->setVisible(studio); resizeDocks({sidebarDock,dock},{260,390},Qt::Horizontal);
        layoutCanvas_->fitToView(); effectsCanvas_->fitToView();
    });
    auto* navigation=addToolBar("Workspace"); navigation->setObjectName("workspaceNavigation"); navigation->setMovable(false);
    insertToolBar(toolbar,navigation); auto* group=new QActionGroup(this); group->setExclusive(true);
    const QStringList sections{"Layout", "Effects"};
    for(int index=0;index<2;++index) {
        auto* action=navigation->addAction(sections[index]); action->setObjectName("workspace"+sections[index]); action->setCheckable(true); group->addAction(action);
        connect(action,&QAction::triggered,this,[this,index,tools,previewToolbar] {
            tabs_->setCurrentIndex(index); previewToolbar->show(); tools->setVisible(index==1);
        });
    }
    connect(tabs_,&QTabWidget::currentChanged,this,[this](int index) { findChild<QAction*>(index==0 ? "workspaceLayout" : "workspaceEffects")->setChecked(true); });
    tabs_->tabBar()->hide();
    findChild<QAction*>("workspaceLayout")->trigger();
    resizeDocks({sidebarDock,dock},{260,390},Qt::Horizontal);
    installParameterTooltips(this);
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

void LightingWindow::scanHeaders()
{
    try {
        discovered_=scan_(); refresh();
        statusBar()->showMessage(discovered_.isEmpty() ? "No supported controller detected. Saved layouts remain available." : "Controllers detected. Select a header to attach fans and strips.");
    } catch(const std::exception& error) {
        statusBar()->showMessage("Header discovery failed: "+QString::fromUtf8(error.what()));
    }
}

ChannelAssignment LightingWindow::selectedHeaderAssignment() const
{
    ChannelAssignment result; result.profileId=selectedHeader_.value("profileId").toString();
    result.endpoint=selectedHeader_.value("path").toString(); result.channelId=selectedHeader_.value("channelId").toString();
    for(const auto& assignment:document_.state().assignments)
        if(assignment.profileId==result.profileId && assignment.endpoint==result.endpoint && assignment.channelId==result.channelId) return assignment;
    const auto& channel=assignmentChannel(assignmentProfile(document_.profiles(),result.profileId),result.channelId);
    result.software=channel.direct;
    if(!channel.nativeModes.isEmpty()) result.native.id=channel.nativeModes.first();
    return result;
}

void LightingWindow::refreshHeader()
{
    const bool selected=!selectedHeader_.isEmpty();
    findChild<QPushButton*>("addHeaderComponents")->setEnabled(selected);
    findChild<QPushButton*>("headerSettings")->setEnabled(selected);
    if(!selected) { headerTitle_->setText("No header selected"); headerSummary_->setText("Detected headers appear on the left. Select one to attach your components."); return; }
    const auto assignment=selectedHeaderAssignment();
    const auto& profile=assignmentProfile(document_.profiles(),assignment.profileId);
    const auto& channel=assignmentChannel(profile,assignment.channelId);
    int leds=0;
    for(const auto& id:assignment.placements) for(const auto& placement:document_.state().placements)
        if(placement.id==id) leds+=document_.catalog()[placement.componentId].positions.size();
    headerTitle_->setText(assignment.channelId);
    headerSummary_->setText(QString("%1\n%2 components / %3 of %4 LEDs\n%5").arg(profile.name).arg(assignment.placements.size()).arg(leds).arg(channel.maxLeds)
        .arg(channel.direct ? "Effects canvas supported" : "Native lighting only"));
}

void LightingWindow::addHeaderComponents()
{
    if(selectedHeader_.isEmpty()) { statusBar()->showMessage("Select a detected header on the left, or refresh headers after connecting your controller."); return; }
    const auto assignment=selectedHeaderAssignment();
    const auto& channel=assignmentChannel(assignmentProfile(document_.profiles(),assignment.profileId),assignment.channelId);
    QDialog dialog(this); dialog.setObjectName("attachComponentsDialog"); dialog.setWindowTitle("Attach components to "+assignment.channelId);
    auto* layout=new QVBoxLayout(&dialog);
    auto* hint=new QLabel("Enter the components connected in physical chain order: fans first, then strips. LED counts must match your hardware.");
    hint->setWordWrap(true); layout->addWidget(hint);
    auto* form=new QFormLayout; layout->addLayout(form);
    auto spin=[&](const QString& name,const QString& label,int minimum,int maximum,int value) {
        auto* control=new QSpinBox; control->setObjectName(name); control->setRange(minimum,maximum); control->setValue(value); form->addRow(label,control); return control;
    };
    auto* fans=spin("fanCount","Fans",0,64,1);
    auto* fanLeds=spin("fanLeds","LEDs per fan",1,channel.maxLeds,8);
    auto* strips=spin("stripCount","Strips",0,64,0);
    auto* stripLeds=spin("stripLeds","LEDs per strip",1,channel.maxLeds,8);
    fans->setToolTip("Number of fans to append to this header's physical chain.");
    strips->setToolTip("Number of strips to append after the fans. Reorder the chain in Header settings if needed.");
    fanLeds->setToolTip("Actual addressable LED count of each fan; use the manufacturer's specification.");
    stripLeds->setToolTip("Actual addressable LED count of each strip; total LEDs must fit this header's capacity.");
    auto* error=new QLabel; error->setObjectName("attachError"); error->setWordWrap(true); layout->addWidget(error);
    auto* buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel); layout->addWidget(buttons);
    buttons->button(QDialogButtonBox::Ok)->setText("Attach components");
    connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    connect(buttons,&QDialogButtonBox::accepted,&dialog,[&,assignment] {
        if(fans->value()+strips->value()==0) { error->setText("Choose at least one component."); return; }
        try {
            // Geometry, chain and instances share one undo command; capacity failure leaves everything untouched.
            document_.edit("Attach fans and strips",[&](LightingState& state) {
                auto updated=assignment;
                for(int kind=0;kind<2;++kind) {
                    const int count=kind==0 ? fans->value() : strips->value(), leds=kind==0 ? fanLeds->value() : stripLeds->value();
                    if(count==0) continue;
                    ComponentDefinition definition; definition.id=QString("generic.%1.%2").arg(kind==0 ? "fan" : "strip").arg(leds);
                    definition.name=QString("%1 (%2 LEDs)").arg(kind==0 ? "Fan" : "Strip").arg(leds);
                    // Catalog units become 20 canvas pixels in componentPoints().
                    for(int led=0;led<leds;++led) {
                        const double angle=6.283185307179586*led/leds;
                        definition.positions.append(kind==0 ? QPointF(1.3+1.3*std::cos(angle),1.3+1.3*std::sin(angle)) : QPointF(led*0.4,0));
                        definition.mapping.append(led);
                    }
                    // Existing document geometry with this ID remains authoritative.
                    if(!state.components.contains(definition.id)) state.components.insert(definition.id,definition);
                    else if(state.components[definition.id].positions.size()!=leds)
                        throw std::invalid_argument("Existing component geometry has a different LED count; edit it in Component library first");
                    for(int n=0;n<count;++n) {
                        ComponentPlacement placement; placement.componentId=definition.id;
                        const int index=state.placements.size(); placement.origin={40.0+(index%7)*80,40.0+(index/7)*80};
                        state.placements.append(placement); updated.placements.append(placement.id);
                    }
                }
                bool replaced=false;
                for(auto& existing:state.assignments) if(existing.id==updated.id) { existing=updated; replaced=true; }
                if(!replaced) state.assignments.append(updated);
            });
            dialog.accept();
        } catch(const std::exception& failure) { error->setText(QString::fromUtf8(failure.what())); }
    });
    installParameterTooltips(&dialog); dialog.exec();
}

void LightingWindow::refreshPlacement()
{
    const ComponentPlacement* selected = nullptr;
    for (const auto& placement : document_.state().placements)
        if (placement.id == layoutCanvas_->selection()) selected = &placement;
    findChild<QPushButton*>("removeComponent")->setEnabled(selected!=nullptr);
    for (int i = 0; i < placementControls_.size(); ++i) {
        const QSignalBlocker block(placementControls_[i]);
        placementControls_[i]->setEnabled(selected != nullptr);
        if (selected) {
            const double values[]{selected->origin.x(), selected->origin.y(),
                selected->scale.width() * 100, selected->scale.height() * 100, selected->rotation};
            placementControls_[i]->setValue(values[i]);
        }
    }
}

void LightingWindow::refresh()
{
    refreshPlacement();
    {
        const QSignalBlocker block(components_); components_->clear();
        QVector<QVariantMap> headers;
        auto append=[&](const QString& profile,const QString& endpoint,const QString& channel) {
            QVariantMap data{{"profileId",profile},{"path",endpoint},{"channelId",channel}};
            if(!headers.contains(data)) headers.append(data);
        };
        for(const auto& candidate:discovered_)
            for(const auto& profile:document_.profiles()) if(profile.id==candidate.profileId)
                for(const auto& channel:profile.channels) append(profile.id,candidate.device.path,channel.id);
        // Saved chains stay editable while disconnected; discovery never rewrites their exact endpoint.
        for(const auto& assignment:document_.state().assignments) append(assignment.profileId,assignment.endpoint,assignment.channelId);
        QSet<QUuid> assigned;
        for(const auto& data:headers) {
            bool online=false;
            for(const auto& candidate:discovered_) if(candidate.profileId==data["profileId"] && candidate.device.path==data["path"]) online=true;
            auto* group=new QTreeWidgetItem(components_,{data["channelId"].toString()+(online ? "" : " (offline)")});
            group->setData(0,Qt::UserRole+1,data); group->setExpanded(true);
            const auto& profile=assignmentProfile(document_.profiles(),data["profileId"].toString());
            group->setToolTip(0,profile.name+"\n"+data["path"].toString());
            if(data==selectedHeader_) components_->setCurrentItem(group);
            for(const auto& assignment:document_.state().assignments)
                if(assignment.profileId==data["profileId"] && assignment.endpoint==data["path"] && assignment.channelId==data["channelId"]) {
                    group->setData(0,Qt::UserRole,assignment.id);
                    for(int i=0;i<assignment.placements.size();++i)
                        for(const auto& placement:document_.state().placements) if(placement.id==assignment.placements[i]) {
                            assigned.insert(placement.id);
                            auto* child=new QTreeWidgetItem(group,{QString::number(i+1)+". "+document_.catalog()[placement.componentId].name});
                            child->setData(0,Qt::UserRole,placement.id);
                            if(placement.id==layoutCanvas_->selection()) components_->setCurrentItem(child);
                        }
                }
        }
        QTreeWidgetItem* unassigned=nullptr;
        for(const auto& placement:document_.state().placements) if(!assigned.contains(placement.id)) {
            if(!unassigned) { unassigned=new QTreeWidgetItem(components_,{"Unassigned components"}); unassigned->setExpanded(true); }
            auto* child=new QTreeWidgetItem(unassigned,{document_.catalog()[placement.componentId].name});
            child->setData(0,Qt::UserRole,placement.id);
            if(placement.id==layoutCanvas_->selection()) components_->setCurrentItem(child);
        }
        if(selectedHeader_.isEmpty() && !headers.isEmpty()) { selectedHeader_=headers.first(); components_->setCurrentItem(components_->topLevelItem(0)); }
    }
    refreshHeader();
    {
        const QSignalBlocker block(previewChannel_); const auto before=previewChannel_->currentData();
        previewChannel_->clear(); previewChannel_->addItem("All layers",QUuid{});
        for(const auto& assignment:document_.state().assignments) if(assignment.software)
            previewChannel_->addItem(assignment.channelId+" / "+assignment.endpoint,assignment.id);
        const int index=previewChannel_->findData(before); previewChannel_->setCurrentIndex(index<0 ? 0 : index);
    }
    const auto currentComponent = catalog_->currentData();
    {
        const QSignalBlocker block(catalog_); catalog_->clear();
        for (const auto& component : document_.catalog()) catalog_->addItem(component.name, component.id);
        const int previous = catalog_->findData(currentComponent);
        if (previous >= 0) catalog_->setCurrentIndex(previous);
    }
    findChild<QPushButton*>("addComponent")->setEnabled(catalog_->count() > 0);
    findChild<QPushButton*>("editComponent")->setEnabled(catalog_->count() > 0);
    QSignalBlocker blockTree(layers_);
    layers_->clear();
    const EffectLayer* selected = nullptr;
    for (auto it = document_.state().layers.crbegin(); it != document_.state().layers.crend(); ++it) {
        auto* item = new QTreeWidgetItem(layers_);
        item->setText(0, it->name);
        item->setData(0,Qt::UserRole+1,effectDefinition(it->effect).label);
        item->setToolTip(0,it->name+" - Effect: "+effectDefinition(it->effect).label);
        item->setData(0,Qt::AccessibleDescriptionRole,"Effect: "+effectDefinition(it->effect).label);
        item->setFlags((item->flags() | Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsDragEnabled) & ~Qt::ItemIsDropEnabled);
        item->setCheckState(0, it->visible ? Qt::Checked : Qt::Unchecked);
        item->setData(0, Qt::UserRole, it->id);
        item->setIcon(1, QIcon(QPixmap::fromImage(renderEffect(*it).scaled(40, 25, Qt::KeepAspectRatio, Qt::SmoothTransformation))));
        item->setIcon(2, QIcon(QPixmap::fromImage(it->mask.scaled(40, 25))));
        item->setToolTip(1, "Select effect: " + effectDefinition(it->effect).label); item->setToolTip(2, "Select grayscale mask");
        if (it->id == selectedLayer_) { selected = &*it; layers_->setCurrentItem(item, editingMask_ ? 2 : 1); }
    }
    if (!selected) {
        selectedLayer_=document_.state().layers.isEmpty() ? QUuid{} : document_.state().layers.back().id;
        if(!selectedLayer_.isNull()) layers_->setCurrentItem(layers_->topLevelItem(0),editingMask_ ? 2 : 1);
    }
    refreshProperties();
    updatePreview();
}

void LightingWindow::installOutputPanel(QWidget* panel)
{
    deviceOutput_->addWidget(panel);
}

void LightingWindow::setOutputState(const QString& state)
{
    findChild<QAction*>("reviewOutput")->setText("Output: "+state);
}

double LightingWindow::previewSeconds() const { return previewTime_->value(); }
void LightingWindow::updatePreview()
{
    const auto seconds=previewTime_->value(); auto channel=previewChannel_->currentData().toUuid();
    bool found=false; for(const auto& assignment:document_.state().assignments) if(assignment.id==channel && assignment.software) found=true;
    if(!found) channel={};
    previewFrame_=document_.render(seconds); // Canonical global composition for default-clock output channels.
    const auto displayed=channel.isNull() ? previewFrame_ : document_.renderForChannel(channel,seconds);
    const double localTime=channel.isNull() ? seconds : document_.channelTime(channel,seconds);
    layoutCanvas_->setPreviewFrame(displayed,localTime,channel); effectsCanvas_->setPreviewFrame(displayed,localTime,channel);
    QImage overview(displayed.size(),QImage::Format_ARGB32_Premultiplied); overview.fill(QColor("#161920"));
    { QPainter painter(&overview); painter.drawImage(0,0,displayed); }
    previewOverview_->setPixmap(QPixmap::fromImage(overview).scaled(96,60,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    emit previewFrameChanged(previewFrame_);
}

void LightingWindow::refreshProperties()
{
    QSignalBlocker blockTransparency(transparency_), blockBlend(blend_), blockMask(maskEnabled_), blockType(effectType_), blockRepeat(repetitions_);
    const EffectLayer* selected = nullptr;
    for (const auto& layer : document_.state().layers) if (layer.id == selectedLayer_) selected = &layer;
    {
        const QSignalBlocker blockAll(allChannels_),blockTargets(channelTargets_);
        allChannels_->setEnabled(selected!=nullptr); allChannels_->setChecked(selected && selected->allChannels);
        channelTargets_->clear(); channelTargets_->setEnabled(selected && !selected->allChannels);
        channelTargets_->setVisible(selected && !selected->allChannels);
        allChannels_->setVisible(selected && (!document_.state().assignments.isEmpty() || !selected->allChannels));
        for(const auto& assignment:document_.state().assignments) if(assignment.software) {
            auto* item=new QListWidgetItem(assignment.channelId+" / "+assignment.endpoint,channelTargets_);
            item->setData(Qt::UserRole,assignment.id); item->setFlags(item->flags()|Qt::ItemIsUserCheckable);
            item->setCheckState(selected && selected->channelTargets.contains(assignment.id) ? Qt::Checked : Qt::Unchecked);
        }
    }
    effectsCanvas_->select(selectedLayer_);
    effectsCanvas_->setMaskTarget(editingMask_);
    effectTarget_->setChecked(!editingMask_); maskTarget_->setChecked(editingMask_);
    for (QWidget* widget : std::initializer_list<QWidget*>{transparency_, blend_, color_, gradientColors_, effectTarget_, maskTarget_, maskEnabled_})
        widget->setEnabled(selected != nullptr);
    if (editingMask_) { playing_->setChecked(false); previewChannel_->setCurrentIndex(0); previewTime_->setValue(0); }
    playing_->setEnabled(!editingMask_);
    inspectorTabs_->setEnabled(selected!=nullptr);
    inspectorTitle_->setText(selected ? effectDefinition(selected->effect).label : "Select a layer");
    animationControls_->setVisible(selected && !editingMask_);
    findChild<QPushButton*>("resetCrop")->setVisible(selected && !editingMask_ && selected->effect==EffectKind::Image);
    findChild<QWidget*>("cropSection")->setVisible(selected && !editingMask_ && selected->effect == EffectKind::Image);
    maskLinked_->setEnabled(selected != nullptr);
    auto* cropTool=findChild<QCheckBox*>("cropTool");
    if ((!selected || selected->effect!=EffectKind::Image) && cropTool->isChecked()) cropTool->setChecked(false);
    cropTool->setEnabled(selected && !editingMask_ && selected->effect==EffectKind::Image && !selected->image.isNull());
    findChild<QToolButton*>("cropImageTool")->setEnabled(cropTool->isEnabled());
    effectType_->setEnabled(selected && !editingMask_);
    color_->setVisible(!editingMask_ && selected && selected->effect != EffectKind::Image && !isGradient(selected->effect));
    gradientColors_->setVisible(selected && !editingMask_ && isGradient(selected->effect));
    imageFit_->setVisible(selected && !editingMask_ && selected->effect == EffectKind::Image);
    secondaryColor_->setVisible(selected && !editingMask_ && isParticle(selected->effect));
    findChild<QWidget*>("geometrySection")->setVisible(selected && !editingMask_ && selected->effect != EffectKind::StaticColor);
    repetitions_->setVisible(selected && !editingMask_ && (selected->effect == EffectKind::Stripes || selected->effect == EffectKind::Waves));
    if (selected) {
        transparency_->setValue(selected->transparency);
        findChild<QLabel*>("transparencyValue")->setText(QString::number(selected->transparency) + "%");
        transparency_->setToolTip(QString::number(selected->transparency) + "% transparent. 0% is fully visible; 100% is hidden. The mask additionally controls local coverage.");
        blend_->setCurrentIndex(static_cast<int>(selected->blend));
        const double rates[]{selected->spin, selected->pulse, selected->hueShift};
        for (int i = 0; i < 3; ++i) {
            const QSignalBlocker block(animation_[i]); animation_[i]->setValue(rates[i]);
        }
        animation_[0]->setEnabled(selected->effect != EffectKind::StaticColor);
        animation_[2]->setEnabled(true);
        const QSignalBlocker blockLink(maskLinked_), blockCycle(colorCycle_);
        maskLinked_->setChecked(selected->maskLinked);
        colorCycle_->setColors(selected->colorCycle);
        gradientColors_->setColors(selected->gradientColors.isEmpty() ? QVector<QColor>{selected->color,selected->secondaryColor} : selected->gradientColors);
        colorCycle_->setEnabled(selected->effect != EffectKind::Image);
        const auto& parameters = advancedParameters();
        for (int i=0;i<advanced_.size();++i) {
            const QSignalBlocker block(advanced_[i]); advanced_[i]->setValue(selected->*(parameters[i].member));
            const bool particle = parameters[i].id.startsWith("particle");
            const bool show = !particle || isParticle(selected->effect);
            advanced_[i]->setVisible(show);
            if(auto* label=qobject_cast<QWidget*>(advanced_[i]->property("parameterLabel").value<QObject*>())) label->setVisible(show);
            advanced_[i]->setEnabled((!particle || isParticle(selected->effect)) &&
                (parameters[i].id != "cycleRate" || selected->effect != EffectKind::Image));
        }
        const double crop[]{selected->crop.x(),selected->crop.y(),selected->crop.width(),selected->crop.height()};
        for (int i=0;i<4;++i) { const QSignalBlocker block(crop_[i]); crop_[i]->setValue(crop[i]); }
        const QSignalBlocker blockFit(imageFit_);
        imageFit_->setCurrentIndex(selected->imageFill ? 1 : 0);
        color_->setColor(selected->color);
        maskEnabled_->setChecked(selected->maskEnabled);
        effectType_->setCurrentIndex(effectType_->findData(static_cast<int>(selected->effect)));
        secondaryColor_->setColor(selected->secondaryColor);
        repetitions_->setValue(selected->repetitions);
        const QVector<double> values{selected->center.x(), selected->center.y(), selected->extent.width(), selected->extent.height(), selected->rotation};
        for (int i = 0; i < geometry_.size(); ++i) {
            const QSignalBlocker block(geometry_[i]);
            geometry_[i]->setValue(values[i]);
        }
        geometry_[3]->setEnabled(selected->effect != EffectKind::LinearGradient &&
            selected->effect != EffectKind::Stripes && selected->effect != EffectKind::Waves);
    }
    syncSliderFields(this);
}

void LightingWindow::closeEvent(QCloseEvent* event)
{
    if (!confirmDiscard()) { event->ignore(); return; }
    emit closing();
    event->accept();
}

void LightingWindow::loadProject(const QString& path, bool recovered, const QString& original)
{
    emit projectLoading();
    document_.load(path); fileName_ = recovered ? original : path;
    if (recovered) document_.history()->resetClean();
    findChild<QAction*>("workspaceEffects")->trigger();
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
