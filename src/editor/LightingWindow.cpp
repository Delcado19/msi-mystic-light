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
    setCentralWidget(tabs_);

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
    dock->setMinimumWidth(290); dock->setMaximumWidth(340);
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
    button(layoutControls, "Remove selected component", [this] {
        const auto id = layoutCanvas_->selection();
        if (id.isNull()) return;
        try {
            document_.edit("Remove component", [&](LightingState& state) {
                state.placements.removeIf([&](const auto& placement) { return placement.id == id; });
            });
            layoutCanvas_->select({});
        } catch (const std::exception& error) {
            statusBar()->showMessage("Remove this instance from its channel chain first. " + QString::fromUtf8(error.what()), 6000);
        }
    });
    // Component actions live beside the component list; precision controls stay in the inspector.
    while(auto* item=layoutControls->takeAt(0)) { if(item->widget()) componentStack->addWidget(item->widget()); delete item; }
    components_=new QTreeWidget; components_->setObjectName("layoutComponents");
    components_->setHeaderHidden(true); components_->setRootIsDecorated(false); componentStack->insertWidget(1,components_,1);
    connect(components_,&QTreeWidget::currentItemChanged,this,[this](QTreeWidgetItem* item) {
        if(item) layoutCanvas_->select(item->data(0,Qt::UserRole).toUuid());
    });
    auto* layoutTitle=new QLabel("Arrange your lighting"); layoutTitle->setProperty("heading",true); layoutControls->addWidget(layoutTitle);
    auto* layoutHint=new QLabel("Select a component, then drag it on the canvas.\nUse the handles for effect layers in Effects.");
    layoutHint->setWordWrap(true); layoutHint->setProperty("muted",true); layoutControls->addWidget(layoutHint);
    auto* placementPrecision=new QWidget; auto* placementForm = new QFormLayout(placementPrecision);
    disclosure(layoutControls,"Precise placement",placementPrecision);
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
        for(int i=0;i<components_->topLevelItemCount();++i) if(components_->topLevelItem(i)->data(0,Qt::UserRole).toUuid()==layoutCanvas_->selection()) components_->setCurrentItem(components_->topLevelItem(i));
    });
    auto* instructions = new QLabel("Drag LEDs to move their component.\nArrow keys move; Shift moves by 10.\nWheel zooms; middle drag pans.\n\nLayout changes sampling positions only.");
    instructions->setWordWrap(true);
    layoutControls->addWidget(instructions);
    layoutControls->addStretch();

    auto* hardwarePage=new QWidget; auto* hardwareControls=new QVBoxLayout(hardwarePage);
    hardwareControls->setContentsMargins(12,12,12,12); hardwareControls->setSpacing(10);
    // Candidate selection is explicit and staged only: no transport is opened here.
    auto* candidates = new QComboBox;
    candidates->setObjectName("profileCandidates");
    candidates->setMinimumContentsLength(16);
    candidates->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    button(hardwareControls, "Scan supported devices", [this, scan, candidates] {
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
            statusBar()->showMessage(QString("%1 profile/endpoint candidates (read-only scan).").arg(found.size()));
        } catch (const std::exception& error) {
            QMessageBox::warning(this, "Discovery failed", QString::fromUtf8(error.what()));
        }
    });
    hardwareControls->addWidget(candidates);
    auto* channelChoice = new QComboBox; channelChoice->setObjectName("profileChannels");
    hardwareControls->addWidget(channelChoice);
    connect(candidates, &QComboBox::currentIndexChanged, this, [this,candidates,channelChoice] {
        channelChoice->clear();
        const auto profileId=candidates->currentData().toMap().value("profileId").toString();
        for(const auto& profile:document_.profiles()) if(profile.id==profileId)
            for(const auto& channel:profile.channels) channelChoice->addItem(channel.id,channel.id);
    });
    button(hardwareControls, "Configure channel", [this,candidates,channelChoice] {
        if(channelChoice->currentIndex()<0) return;
        const auto data=candidates->currentData().toMap();
        ChannelAssignment assignment; assignment.endpoint=data.value("path").toString();
        assignment.profileId=data.value("profileId").toString(); assignment.channelId=channelChoice->currentData().toString();
        for(const auto& existing:document_.state().assignments)
            if(existing.endpoint==assignment.endpoint && existing.channelId==assignment.channelId && existing.profileId==assignment.profileId) assignment=existing;
        try {
            ChannelEditor editor(document_,assignment,this);
            if(editor.exec()==QDialog::Accepted) document_.assignChannel(editor.assignment());
        } catch(const std::exception& error) { statusBar()->showMessage(QString::fromUtf8(error.what()),6000); }
    });
    assignments_=new QTreeWidget; assignments_->setObjectName("channelAssignments");
    assignments_->setHeaderLabels({"Channel", "Mode", "Instances"}); assignments_->setMinimumHeight(140); assignments_->setMaximumHeight(240);
    hardwareControls->addWidget(assignments_);
    button(hardwareControls, "Edit assigned channel", [this] {
        if(!assignments_->currentItem()) return;
        const auto id=assignments_->currentItem()->data(0,Qt::UserRole).toUuid();
        // Copy before opening the dialog; its accepted edit can invalidate document references.
        for(const auto assignment:document_.state().assignments) if(assignment.id==id) {
            try {
                ChannelEditor editor(document_,assignment,this);
                if(editor.exec()==QDialog::Accepted) document_.assignChannel(editor.assignment());
            } catch(const std::exception& error) { statusBar()->showMessage(QString::fromUtf8(error.what()),6000); }
            break;
        }
    });
    button(hardwareControls, "Remove channel assignment", [this] {
        if(!assignments_->currentItem()) return;
        const auto id=assignments_->currentItem()->data(0,Qt::UserRole).toUuid();
        document_.edit("Remove channel assignment", [&](LightingState& state) {
            state.assignments.removeIf([&](const auto& item) { return item.id==id; });
            for(auto& layer:state.layers) layer.channelTargets.removeAll(id);
        });
    });
    hardwareControls->addWidget(new QLabel("Assignments alone do not connect or write hardware."));

    hardwareControls->addStretch();
    auto* setupTabs=new QTabWidget; setupTabs->setObjectName("layoutSetupTabs");
    setupTabs->addTab(componentPanel,"Components"); setupTabs->addTab(hardwarePage,"Devices");
    sidebarContexts->insertWidget(0,setupTabs);

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
    previewOverview_->setMinimumHeight(120); previewOverview_->setMaximumHeight(160);
    previewOverview_->setAlignment(Qt::AlignCenter); previewOverview_->setToolTip("Live composition preview for the selected output channel");
    effectControls->addWidget(previewOverview_);
    inspectorTitle_=new QLabel("Select a layer"); inspectorTitle_->setProperty("heading",true); inspectorTitle_->setWordWrap(true); effectControls->addWidget(inspectorTitle_);
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
    color_ = button(appearanceControls, "Effect color…", [this] {
        for (const auto& layer : document_.state().layers) if (layer.id == selectedLayer_) {
            const auto color = QColorDialog::getColor(layer.color, this, "Effect color", QColorDialog::ShowAlphaChannel);
            if (color.isValid()) updateLayer("Effect color", [color](auto& changed) { changed.color = color; });
            break;
        }
    });
    secondaryColor_ = button(appearanceControls, "Second effect color…", [this] {
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
    auto* advancedContent=new QWidget; auto* advancedForm=new QFormLayout(advancedContent); advancedForm->setContentsMargins(0,0,0,0);
    for (const auto& parameter : advancedParameters()) {
        auto* value = new QDoubleSpinBox; value->setObjectName(parameter.id);
        value->setRange(parameter.minimum,parameter.maximum);
        value->setDecimals(parameter.id == "particleCount" || parameter.id == "particleSeed" ? 0 : 2);
        value->setKeyboardTracking(false); advanced_.append(value);
        advancedForm->addRow(parameter.label,value);
        connect(value,&QDoubleSpinBox::valueChanged,this,[this,member=parameter.member](double number) {
            updateLayer("Effect parameter",[=](auto& layer) { layer.*member = number; });
        });
    }
    colorCycle_ = new QLineEdit; colorCycle_->setObjectName("colorCycle");
    colorCycle_->setPlaceholderText("#ff0000, #00ff00, #0000ff");
    advancedForm->addRow("Cycle colors",colorCycle_);
    connect(colorCycle_,&QLineEdit::editingFinished,this,[this] {
        QVector<QColor> colors;
        for (const auto& text : colorCycle_->text().split(',')) colors.append(QColor(text.trimmed()));
        for (const auto& layer : document_.state().layers) if (layer.id==selectedLayer_ && layer.colorCycle==colors) return;
        updateLayer("Cycle colors",[colors](auto& layer) { layer.colorCycle = colors; });
        refreshProperties();
    });
    motionControls->addWidget(new QLabel("Play preview to see motion on the canvas."));
    motionControls->addWidget(animationControls_);
    disclosure(motionControls,"More motion & particle settings",advancedContent);
    motionControls->addStretch();
    maskLinked_ = new QCheckBox("Link mask to effect"); maskLinked_->setObjectName("maskLinked");
    maskControls->addWidget(maskLinked_);
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
    auto* cropSection=disclosure(appearanceControls,"Precise image crop",cropControls_);
    cropSection->setObjectName("cropSection");
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
    auto* geometrySection=disclosure(appearanceControls,"Precise transform",geometryControls_);
    geometrySection->setObjectName("geometrySection");
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
    connect(layoutCanvas_,&LightingCanvas::addRequested,this,[this] { findChild<QPushButton*>("addComponent")->click(); });
    connect(&document_, &LightingDocument::changed, this, &LightingWindow::refresh);
    connect(document_.history(), &QUndoStack::cleanChanged, this, [this](bool clean) { setWindowModified(!clean); });
    setWindowTitle("Lighting[*] — MSI Mystic Light Standalone");
    statusBar()->showMessage("Lighting Studio - Create effects, arrange components, then review output.");
    auto* viewMenu=menuBar()->addMenu("View"); viewMenu->setObjectName("workspaceViewMenu");
    viewMenu->addAction(sidebarDock->toggleViewAction()); viewMenu->addAction(dock->toggleViewAction());
    auto* resetWorkspace=viewMenu->addAction("Reset workspace"); resetWorkspace->setObjectName("resetWorkspace");
    connect(resetWorkspace,&QAction::triggered,this,[this,sidebarDock,dock] {
        sidebarDock->show(); dock->show(); resizeDocks({sidebarDock,dock},{280,310},Qt::Horizontal);
        layoutCanvas_->fitToView(); effectsCanvas_->fitToView();
    });
    tabs_->setCurrentIndex(1);
    resizeDocks({sidebarDock,dock},{280,310},Qt::Horizontal);
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

void LightingWindow::refreshPlacement()
{
    const ComponentPlacement* selected = nullptr;
    for (const auto& placement : document_.state().placements)
        if (placement.id == layoutCanvas_->selection()) selected = &placement;
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
        for(const auto& placement:document_.state().placements) {
            auto* item=new QTreeWidgetItem(components_,{document_.catalog()[placement.componentId].name});
            item->setData(0,Qt::UserRole,placement.id);
            if(placement.id==layoutCanvas_->selection()) components_->setCurrentItem(item);
        }
    }
    const auto selectedAssignment=assignments_->currentItem() ? assignments_->currentItem()->data(0,Qt::UserRole).toUuid() : QUuid{};
    assignments_->clear();
    for(const auto& assignment:document_.state().assignments) {
        auto* item=new QTreeWidgetItem(assignments_,{assignment.channelId,assignment.software ? "Software" : "Native",QString::number(assignment.placements.size())});
        item->setData(0,Qt::UserRole,assignment.id); item->setToolTip(0,assignment.profileId+"\n"+assignment.endpoint);
        if(assignment.id==selectedAssignment) assignments_->setCurrentItem(item);
    }
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
    previewOverview_->setPixmap(QPixmap::fromImage(overview).scaled(270,150,Qt::KeepAspectRatio,Qt::SmoothTransformation));
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
    for (QWidget* widget : std::initializer_list<QWidget*>{transparency_, blend_, color_, effectTarget_, maskTarget_, maskEnabled_})
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
    color_->setVisible(!editingMask_ && selected && selected->effect != EffectKind::Image);
    imageFit_->setVisible(selected && !editingMask_ && selected->effect == EffectKind::Image);
    secondaryColor_->setVisible(selected && !editingMask_ && (isGradient(selected->effect) || isParticle(selected->effect)));
    findChild<QWidget*>("geometrySection")->setVisible(selected && !editingMask_ && selected->effect != EffectKind::StaticColor);
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
            if (auto* form=qobject_cast<QFormLayout*>(advanced_[i]->parentWidget()->layout()))
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
