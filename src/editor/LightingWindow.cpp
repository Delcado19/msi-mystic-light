#include "LightingWindow.h"
#include "SnapGrid.h"
#include "HardwareIdentity.h"
#include <QFileInfo>
#include <QKeyEvent>
#include <QAbstractSpinBox>
#include <QScreen>
#include "HardwarePicker.h"
#include "LegacyHardware.h"
#include <QStandardPaths>
#include <QDir>
#include <QDialog>
#include <QSplitter>
#include <QButtonGroup>
#include "ComponentEditor.h"
#include "ChannelEditor.h"
#include "StudioWidgets.h"
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QToolButton>
#include <QStyle>
#include <QSplitter>
#include <QButtonGroup>
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
#include <QTreeView>
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
        const auto name=item.text; item.text.clear();
        const auto eye=style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator,&item,widget).adjusted(-2,-2,2,2);
        item.features &= ~QStyleOptionViewItem::HasCheckIndicator;
        style->drawControl(QStyle::CE_ItemViewItem,&item,painter,widget);
        static const auto visible=studioToolIcon("eye"), hidden=studioToolIcon("eye-off");
        (index.data(Qt::CheckStateRole).toInt()==Qt::Checked ? visible : hidden).paint(painter,eye);
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
    void keyPressEvent(QKeyEvent* event) override {
        // Source/mask thumbnails have no check state; Space still toggles the selected layer's eye.
        if(event->key()==Qt::Key_Space && event->modifiers()==Qt::NoModifier && currentItem()) {
            if(!event->isAutoRepeat()) currentItem()->setCheckState(0,currentItem()->checkState(0)==Qt::Checked ? Qt::Unchecked : Qt::Checked);
            event->accept();return;
        }
        QTreeWidget::keyPressEvent(event);
    }
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
    auto* save = fileMenu->addAction("Save"); save->setObjectName("saveProject"); save->setShortcut(QKeySequence::Save);
    connect(save, &QAction::triggered, this, [this] { saveDocument(); });
    auto* saveAs = fileMenu->addAction("Save as…"); saveAs->setObjectName("saveProjectAs"); saveAs->setShortcut(QKeySequence::SaveAs);
    connect(saveAs, &QAction::triggered, this, [this] {
        const auto previous = fileName_; fileName_.clear();
        if (!saveDocument()) { fileName_ = previous; refreshProjectName(); }
    });
    auto* editMenu = menuBar()->addMenu("Edit");
    auto* undo = document_.history()->createUndoAction(this, "Undo");
    undo->setShortcut(QKeySequence::Undo);
    editMenu->addAction(undo);
    auto* redo = document_.history()->createRedoAction(this, "Redo");
    redo->setShortcuts({QKeySequence::Redo, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z)});
    editMenu->addAction(redo);
    auto* toolbar = addToolBar("History"); toolbar->setObjectName("historyToolbar");
    toolbar->setMovable(false); toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    open->setIcon(studioToolIcon("open")); save->setIcon(studioToolIcon("save"));
    undo->setIcon(studioToolIcon("undo")); redo->setIcon(studioToolIcon("redo"));
    toolbar->addAction(open); toolbar->addAction(save);
    saveAs->setText("Save as…"); toolbar->addAction(saveAs);
    auto* projectName=new QLabel("Untitled"); projectName->setObjectName("currentProjectName"); projectName->setMinimumWidth(80); projectName->setMaximumWidth(210); toolbar->addWidget(projectName);
    toolbar->addSeparator();
    // Keep dynamic undo descriptions in the menu, not in a toolbar that steals canvas width.
    auto* undoButton=new QToolButton; undoButton->setDefaultAction(undo); undoButton->setText("Undo"); toolbar->addWidget(undoButton);
    auto* redoButton=new QToolButton; redoButton->setDefaultAction(redo); redoButton->setText("Redo"); toolbar->addWidget(redoButton);
    undoButton->setToolButtonStyle(Qt::ToolButtonIconOnly); redoButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    connect(undo,&QAction::changed,undoButton,[undoButton] { undoButton->setText("Undo"); });
    connect(redo,&QAction::changed,redoButton,[redoButton] { redoButton->setText("Redo"); });

    tabs_ = new QTabWidget;
    tabs_->setObjectName("lightingViews");
    tabs_->setDocumentMode(true);
    layoutCanvas_ = new LightingCanvas(document_, LightingCanvas::Context::Layout);
    effectsCanvas_ = new LightingCanvas(document_, LightingCanvas::Context::Effects);
    layoutCanvas_->setObjectName("layoutCanvas");
    effectsCanvas_->setObjectName("effectsCanvas");
    auto wrapCanvas=[this](LightingCanvas* canvas) {
        auto* wrapper=new QWidget; auto* layout=new QVBoxLayout(wrapper);layout->setContentsMargins(0,0,0,0);layout->addWidget(canvas);
        connect(canvas,&LightingCanvas::fullscreenRequested,this,[this,canvas,layout] {
            if(auto* host=qobject_cast<QDialog*>(canvas->window());host && host->objectName()=="canvasFullscreen") {host->reject();return;}
            auto* host=new QDialog(this);host->setObjectName("canvasFullscreen");host->setAttribute(Qt::WA_DeleteOnClose);
            auto* full=new QVBoxLayout(host);
            const auto screenBounds=screen()->geometry(), available=screen()->availableGeometry();
            // Some Windows taskbars remain above fullscreen dialogs; keep controls inside the usable monitor area.
            full->setContentsMargins(std::max(0,available.left()-screenBounds.left()),std::max(8,available.top()-screenBounds.top()),
                std::max(0,screenBounds.right()-available.right()),std::max(8,screenBounds.bottom()-available.bottom()));
            auto* playback=new QPushButton(playing_->text(),host);playback->setObjectName("fullscreenCanvasPlayback");
            playback->setCheckable(true);playback->setChecked(playing_->isChecked());playback->setStyleSheet(playing_->styleSheet());
            full->addWidget(playback,0,Qt::AlignLeft);full->addWidget(canvas);
            connect(playback,&QPushButton::toggled,playing_,&QPushButton::setChecked);
            connect(playing_,&QPushButton::toggled,playback,[playback](bool running){playback->setChecked(running);playback->setText(running ? "Canvas Stop" : "Canvas Start");});
            connect(host,&QDialog::finished,this,[canvas,layout] {layout->addWidget(canvas);canvas->show();canvas->fitToView();});
            host->showFullScreen();canvas->fitToView();
        });
        return wrapper;
    };
    tabs_->addTab(wrapCanvas(layoutCanvas_), "Layout");
    auto* effectWorkspace=new QSplitter(Qt::Vertical); effectWorkspace->setObjectName("effectWorkspace");
    hardwarePreview_=new LightingCanvas(document_,LightingCanvas::Context::Layout); hardwarePreview_->setObjectName("hardwareLivePreview"); hardwarePreview_->setPreviewOnly();
    effectWorkspace->addWidget(wrapCanvas(effectsCanvas_)); effectWorkspace->addWidget(hardwarePreview_); effectWorkspace->setStretchFactor(0,4); effectWorkspace->setStretchFactor(1,1); effectWorkspace->setSizes({560,180});
    tabs_->addTab(effectWorkspace, "Effects");
    applicationPages_=new QStackedWidget; applicationPages_->setObjectName("applicationPages");
    applicationPages_->addWidget(tabs_); setCentralWidget(applicationPages_);
    connect(this,&LightingWindow::outputRequested,this,[this] { findChild<QAction*>("workspaceHardware")->trigger(); });

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
    for (const auto& component : document_.catalog()) catalog_->addItem(hardwareName(component), component.id);
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
    auto* removeComponent=button(layoutControls, "Remove selected component", [this] {removeSelectedComponents();});
    removeComponent->setObjectName("removeComponent");
    auto* library=new QWidget; auto* libraryControls=new QVBoxLayout(library); libraryControls->setContentsMargins(0,0,0,0);
    while(auto* item=layoutControls->takeAt(0)) { if(item->widget()) libraryControls->addWidget(item->widget()); delete item; }
    auto* hardwarePage=new QWidget; hardwarePage->setObjectName("hardwarePage");
    auto* hardwareColumns=new QHBoxLayout(hardwarePage); hardwareColumns->setContentsMargins(28,24,28,24); hardwareColumns->setSpacing(28);
    auto* hardwareOptions=new QWidget; auto* hardwareControls=new QVBoxLayout(hardwareOptions); hardwareControls->setContentsMargins(0,0,0,0);
    auto* hardwareOutput=new QWidget; deviceOutput_=new QVBoxLayout(hardwareOutput); deviceOutput_->setContentsMargins(0,0,0,0);
    hardwareColumns->addWidget(hardwareOptions,1); hardwareColumns->addWidget(hardwareOutput,2);
    auto* hardwareTitle=new QLabel("Hardware"); hardwareTitle->setProperty("heading",true); hardwareControls->addWidget(hardwareTitle);
    auto* hardwareHelp=new QLabel("Select a detected header, then attach your fans and strips. Configure component definitions here; arrange their positions in Layout.");
    hardwareHelp->setWordWrap(true); hardwareControls->addWidget(hardwareHelp);
    disclosure(hardwareControls,"Custom component definitions",library);
    const auto editDeviceLeds = [this] {
        const auto id=layoutCanvas_->selection();
        for (const auto& placement : document_.state().placements) if (placement.id==id) {
            ComponentEditor editor(document_.catalog().value(placement.componentId), true, this, true);
            if (editor.exec()==QDialog::Accepted) {
                try { document_.setComponentGeometry(id, editor.component().positions); }
                catch (const std::exception& error) { statusBar()->showMessage(QString::fromUtf8(error.what()),6000); }
            }
            return;
        }
    };
    auto* editHardwareLeds=button(hardwareControls,"Edit LED arrangement",editDeviceLeds);
    editHardwareLeds->setObjectName("editHardwareLeds");
    auto* editLayoutLeds=button(layoutControls,"Edit LED arrangement",editDeviceLeds);
    editLayoutLeds->setObjectName("editLayoutLeds");
    const auto updateLedActions=[this,editHardwareLeds,editLayoutLeds] {
        bool placed=false;
        for (const auto& placement:document_.state().placements) if(placement.id==layoutCanvas_->selection()) placed=true;
        for(auto* action:{editHardwareLeds,editLayoutLeds}) {
            action->setEnabled(placed);
            action->setToolTip("Select a device, then arrange its individual LEDs; other devices keep their geometry");
        }
    };
    connect(layoutCanvas_,&LightingCanvas::selectionChanged,this,updateLedActions);
    connect(&document_,&LightingDocument::changed,this,updateLedActions);
    updateLedActions();
    auto* detachHardware=button(hardwareControls,"Remove selected component",[removeComponent] { removeComponent->click(); }); detachHardware->setObjectName("removeHardwareComponent"); detachHardware->setEnabled(false);
    connect(layoutCanvas_,&LightingCanvas::selectionChanged,detachHardware,[this,detachHardware] {
        bool placed=false; for(const auto& placement:document_.state().placements) if(placement.id==layoutCanvas_->selection()) placed=true;
        detachHardware->setEnabled(placed);
    });
    button(hardwareControls,"Import previous hardware layout...",[this] {
        const auto path=QFileDialog::getOpenFileName(this,"Import previous Lighting Studio hardware",qEnvironmentVariable("APPDATA")+"/MsiMysticLightApp/settings.json","Previous settings (*.json)");
        if(!path.isEmpty()) restoreLegacyHardware(path);
    });
    auto* hardwareStatus=new QLabel("Canvas output starts automatically for configured, detected hardware. Attach your fans and strips to their headers.");
    hardwareStatus->setObjectName("hardwareStatus"); hardwareStatus->setWordWrap(true); hardwareControls->addWidget(hardwareStatus);
    hardwareControls->addStretch();

    auto* hardwareScroll=new QScrollArea; hardwareScroll->setWidgetResizable(true); hardwareScroll->setWidget(hardwarePage);
    tabs_->addTab(hardwareScroll,"Hardware");
    components_=new QTreeWidget; components_->setObjectName("layoutComponents");
    components_->setHeaderLabels({"Header / component","Manufacturer"});
    auto* hardwareTree=new QTreeView; hardwareTree->setObjectName("hardwareComponents"); hardwareTree->setMinimumHeight(220);
    // Both workspaces share the same inventory model and selection; editing never maintains a second hardware list.
    hardwareTree->setModel(components_->model()); hardwareTree->setSelectionModel(components_->selectionModel()); hardwareControls->insertWidget(2,hardwareTree,1);
    hardwareTree->header()->setSectionResizeMode(0,QHeaderView::Stretch); hardwareTree->header()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
    components_->setColumnHidden(1,true); // Keep the narrow Layout sidebar focused on names; Hardware exposes the manufacturer column.
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
    auto* layoutHint=new QLabel("Select a group or component. Drag its corners to resize, the round handle to rotate, or its LEDs to move.");
    layoutHint->setWordWrap(true); layoutHint->setProperty("muted",true); layoutControls->addWidget(layoutHint);
    auto* placementPrecision=new QWidget; auto* placementForm = new QFormLayout(placementPrecision);
    auto* placementSection=disclosure(layoutControls,"Position, size and rotation",placementPrecision); placementSection->setObjectName("placementSection");
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
    auto* instructions = new QLabel("Drag a component to move it; drag a group label to move the group.\nDrag corners to resize; the circle rotates.\nArrow keys move; Shift moves by 10.\nWheel zooms; middle drag pans.\n\nLayout changes sampling positions only.");
    instructions->setWordWrap(true);
    instructions->setProperty("muted",true);
    layoutControls->addWidget(instructions);
    layoutControls->addStretch();

    // Match the original studio: headers beside the layout, settings beside the selection.
    auto* headerPanel=new QWidget; auto* headerControls=new QVBoxLayout(headerPanel);
    headerControls->setContentsMargins(0,0,0,0);
    headerTitle_=new QLabel("Select a header"); headerTitle_->setProperty("heading",true); headerControls->addWidget(headerTitle_);
    headerSummary_=new QLabel; headerSummary_->setWordWrap(true); headerSummary_->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum); headerControls->addWidget(headerSummary_);
    auto* addHardware=new QPushButton("Attach fans or strips..."); addHardware->setObjectName("addHeaderComponents");
    addHardware->setToolTip("Choose products and quantities to attach to a detected header"); hardwareControls->insertWidget(2,addHardware);
    connect(addHardware,&QPushButton::clicked,this,[this] { addHeaderComponents(); });
    button(headerControls,"Header settings",[this] {
        if(selectedHeader_.isEmpty()) return;
        try {
            ChannelEditor editor(document_,selectedHeaderAssignment(),this);
            if(editor.exec()==QDialog::Accepted) document_.assignChannel(editor.assignment());
        } catch(const std::exception& error) { statusBar()->showMessage(QString::fromUtf8(error.what()),6000); }
    })->setObjectName("headerSettings");
    layoutControls->insertWidget(0,headerPanel);
    auto* refreshHardware=button(hardwareControls,"Refresh",[this] { scanHeaders(); }); refreshHardware->setObjectName("refreshHeaders");
    auto* hardwareActions=new QHBoxLayout;
    for(auto* control:{addHardware,detachHardware,refreshHardware}) { hardwareControls->removeWidget(control); hardwareActions->addWidget(control); }
    addHardware->setText("Add..."); detachHardware->setText("Remove");
    refreshHardware->setToolTip("Refresh connected hardware and verify supported RAM identity through PawnIO. Discovery does not change colors; configured canvas output resumes automatically.");
    hardwareControls->insertLayout(3,hardwareActions);
    scan_=std::move(scan);
    QTimer::singleShot(0,this,[this] { scanHeaders(); });

    auto* previewToolbar = addToolBar("Preview"); previewToolbar->setObjectName("previewToolbar");
    playing_ = new QPushButton("Canvas Start"); playing_->setCheckable(true); playing_->setObjectName("playPreview");
    playing_->setIcon(style()->standardIcon(QStyle::SP_MediaPlay)); playing_->setAccessibleName("Canvas Start/Stop");
    playing_->setStyleSheet("QPushButton { background: #244b39; color: #effff4; border: 1px solid #5eaa7e; padding: 7px 14px; } QPushButton:checked { background: #6a2b32; color: #fff3f3; border-color: #da717b; }");
    previewTime_ = new QDoubleSpinBox(this); previewTime_->setObjectName("previewTime");
    previewTime_->setRange(0, 86400); previewTime_->setDecimals(2); previewTime_->setSuffix(" s");
    previewTime_->setKeyboardTracking(false); previewTime_->setAccessibleName("Preview time in seconds");
    previewToolbar->setMovable(false);
    previewToolbar->addWidget(playing_); previewTime_->hide(); // Keep the animation clock internal, without a numeric toolbar field.
    previewChannel_=new QComboBox; previewChannel_->setObjectName("previewChannel");
    previewChannel_->setMaximumWidth(180);
    previewChannel_->addItem("All layers",QUuid{}); previewToolbar->addWidget(previewChannel_);
    auto* fit=previewToolbar->addAction("Fit canvas"); fit->setObjectName("fitCanvas"); fit->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    connect(fit,&QAction::triggered,this,[this] { (tabs_->currentIndex()==0 ? layoutCanvas_ : effectsCanvas_)->fitToView(); });
    for(const auto& zoom:QVector<QPair<QString,double>>{{"Zoom out",1/1.2},{"Zoom in",1.2}}) {
        auto* action=previewToolbar->addAction(zoom.first); action->setToolTip(zoom.first+" (mouse wheel also zooms)");
        connect(action,&QAction::triggered,this,[this,factor=zoom.second] { (tabs_->currentIndex()==0 ? layoutCanvas_ : effectsCanvas_)->zoomBy(factor); });
    }
    auto* snap=previewToolbar->addAction("Snap to grid"); snap->setObjectName("snapToGrid");
    snap->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_G));snap->setShortcutContext(Qt::ApplicationShortcut);
    snap->setCheckable(true);snap->setChecked(SnapGrid::load().enabled);
    snap->setToolTip("Snap positions and resize handles to the world grid; rotation uses 15 degrees. Hold Alt to move freely. Ctrl+Shift+G toggles snapping, including fullscreen.");
    auto* gridSize=new QSpinBox;gridSize->setObjectName("gridSpacing");gridSize->setRange(1,100);gridSize->setSuffix(" px");
    gridSize->setValue(int(SnapGrid::load().spacing));gridSize->setKeyboardTracking(false);
    gridSize->setAccessibleName("Snap grid spacing");gridSize->setToolTip("Grid spacing in canvas pixels. LED arrangement uses the same physical scale; gradient palette stops use this percentage.");
    previewToolbar->addWidget(gridSize);
    auto updateGrid=[this,snap,gridSize] {
        SnapGrid{snap->isChecked(),double(gridSize->value())}.save();
        layoutCanvas_->gridChanged();effectsCanvas_->gridChanged();update();
    };
    connect(snap,&QAction::toggled,this,updateGrid);connect(gridSize,&QSpinBox::valueChanged,this,updateGrid);
    auto* output=previewToolbar->addAction("Output: Stopped"); output->setObjectName("reviewOutput");
    output->setToolTip("Show connected hardware and output status; canvas output starts automatically");
    // Output status is informational; Hardware already has its own navigation button.
    previewToolbar->removeAction(output); output->setEnabled(false);
    auto* outputStatus=new QLabel("Output: Stopped"); outputStatus->setObjectName("toolbarOutputStatus");
    outputStatus->setParent(this); outputStatus->hide();
    connect(previewChannel_,&QComboBox::currentIndexChanged,this,&LightingWindow::updatePreview);
    connect(layoutCanvas_,&LightingCanvas::layoutPreviewChanged,this,[this] { emit previewFrameChanged(previewFrame_); });
    auto* timer = new QTimer(this); timer->setTimerType(Qt::PreciseTimer); timer->setInterval(16);
    connect(playing_, &QPushButton::toggled, this, [this, timer](bool play) {
        playing_->setText(play ? "Canvas Stop" : "Canvas Start"); playing_->setIcon(style()->standardIcon(play ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay));
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
    inspectorTitle_=new QLabel("Select a layer"); inspectorTitle_->setProperty("heading",true); inspectorTitle_->setWordWrap(true); effectControls->addWidget(inspectorTitle_);
    inspectorTabs_=new QTabWidget; inspectorTabs_->setObjectName("layerInspectorTabs");
    effectControls->addWidget(inspectorTabs_);
    auto* appearance=new QWidget; auto* appearanceControls=new QVBoxLayout(appearance); appearanceControls->setContentsMargins(0,12,0,0); appearanceControls->setSpacing(12);
    auto* motion=new QWidget; auto* motionControls=new QVBoxLayout(motion); motionControls->setContentsMargins(0,12,0,0);
    auto* maskProperties=new QWidget; auto* maskControls=new QVBoxLayout(maskProperties); maskControls->setContentsMargins(0,12,0,0); maskControls->setSpacing(10);
    inspectorTabs_->addTab(appearance,"Effect"); inspectorTabs_->addTab(motion,"Motion"); inspectorTabs_->addTab(maskProperties,"Mask");
    connect(inspectorTabs_,&QTabWidget::currentChanged,this,[this](int page) {
        if(page==2) { playing_->setChecked(false); previewChannel_->setCurrentIndex(0); previewTime_->setValue(0); }
        editingMask_=page==2; if(page!=2) { effectsCanvas_->setCropTool(false); findChild<QCheckBox*>("cropTool")->setChecked(false); }
        refreshProperties();
    });
    addPanel(effectPanel);
    button(layerStack, "New layer", [this] {
        if (document_.state().layers.size() >= 64) { statusBar()->showMessage("Document limit: 64 layers."); return; }
        editingMask_=false; inspectorTabs_->setCurrentIndex(0);
        selectedLayer_=document_.addPaintLayer(); refresh();
    })->setObjectName("newLayer");
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
    })->setObjectName("deleteLayer");
    auto* createActions=new QHBoxLayout; auto* manageActions=new QHBoxLayout;
    for(auto* control:layersPanel->findChildren<QPushButton*>(QString{},Qt::FindDirectChildrenOnly)) {

        if(control->text()=="Duplicate layer" || control->text()=="Delete layer") { layerStack->removeWidget(control); manageActions->addWidget(control); }
    }
    delete createActions; layerStack->addLayout(manageActions);
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
    blend_ = new QComboBox; blend_->setObjectName("layerBlendMode");
    blend_->addItems({"Normal", "Multiply", "Screen", "Overlay"});
    blend_->setToolTip("How this layer combines with visible layers below it: Normal replaces, Multiply darkens, Screen lightens, Overlay increases contrast. The bottom layer has no backdrop; select an upper layer to see blending.");
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
    connect(color_,&ColorEditor::colorChanged,this,[this](QColor color) {
        foreground_=color;
        // Brush color is tool state; changing it must not recolor a procedural source under the brush.
        if(effectsCanvas_->tool()==LightingCanvas::Tool::Move)
            for(const auto& layer:document_.state().layers) if(layer.id==selectedLayer_ && layer.effect!=EffectKind::Paint)
                { updateLayer("Effect color",[color](auto& selected) { selected.color=color; }); break; }
    });
    secondaryColor_=new ColorEditor; secondaryColor_->setObjectName("secondaryEffectColor"); appearanceControls->addWidget(secondaryColor_);
    connect(secondaryColor_,&ColorEditor::colorChanged,this,[this](QColor color) { updateLayer("Secondary color",[color](auto& layer) { layer.secondaryColor=color; }); });
    gradientColors_=new ColorListEditor(2,16); gradientColors_->setGradientMode(); gradientColors_->setObjectName("gradientColors"); appearanceControls->addWidget(gradientColors_);
    connect(gradientColors_,&ColorListEditor::stopsChanged,this,[this] {
        const auto colors=gradientColors_->colors(); const auto positions=gradientColors_->positions();
        updateLayer("Gradient stops",[colors,positions](auto& layer) { layer.gradientColors=colors; layer.gradientPositions=positions; layer.color=colors.first(); layer.secondaryColor=colors.last(); });
    });
    connect(gradientColors_,&ColorListEditor::gestureStarted,this,[this] { document_.history()->beginMacro("Gradient stop"); });
    connect(gradientColors_,&ColorListEditor::gestureFinished,this,[this] { document_.history()->endMacro(); });
    effectType_ = new QComboBox;
    effectType_->setObjectName("selectedEffectType");
    for (const auto& definition : effectDefinitions())
        if(definition.kind!=EffectKind::Paint && definition.kind!=EffectKind::Image)
            effectType_->addItem(isParticle(definition.kind) ? QString(definition.label).replace("Particle ","") : definition.label, static_cast<int>(definition.kind));
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
    const QStringList animationLabels{"Rotation / sec", "Flashes / sec", "Color shift / sec"};
    for (int i = 0; i < 3; ++i) {
        auto* value = new QDoubleSpinBox;
        value->setObjectName("animationRate" + QString::number(i));
        value->setRange(i == 1 ? 0 : -360, i == 1 ? 10 : 360);
        value->setKeyboardTracking(false); animation_.append(value);
        value->setAccessibleName(animationLabels[i]);
        animationForm->addRow(animationLabels[i], sliderField(value));
        connect(value, &QDoubleSpinBox::valueChanged, this, [this] {
            const double spin = animation_[0]->value(), pulse = animation_[1]->value(), hue = animation_[2]->value();
            updateMotion("Animation rates", [=](auto& layer) { layer.spin = spin; layer.pulse = pulse; layer.hueShift = hue; });
        });
    }
    auto* motionScope=new QCheckBox("Apply animation to all visible layers"); motionScope->setObjectName("allLayerMotion");
    motionScope->setToolTip("Unchecked: only the selected layer changes. Checked: motion settings affect every visible layer, including the gradient behind a shape.");
    motionControls->addWidget(motionScope); motionControls->addWidget(animationControls_);
    auto* particles=new QWidget; particles->setObjectName("particleParameters"); auto* particleForm=new QFormLayout(particles); particleForm->setContentsMargins(0,0,0,0);
    motionControls->addWidget(particles);
    auto* advancedContent=new QWidget; auto* advancedForm=new QFormLayout(advancedContent); advancedForm->setContentsMargins(0,0,0,0);
    for(const auto& parameter:advancedParameters()) {
        auto* value=new QDoubleSpinBox; value->setObjectName(parameter.id); value->setRange(parameter.minimum,parameter.maximum);
        value->setDecimals(parameter.id=="particleCount" || parameter.id=="particleSeed" ? 0 : 2); value->setKeyboardTracking(false); advanced_.append(value);
        const bool primary=parameter.id=="particleCount" || parameter.id=="particleSpeed" || parameter.id=="particleSize";
        if(primary) {
            auto* field=sliderField(value); value->setProperty("visibilityContainer",QVariant::fromValue(static_cast<QObject*>(field)));
            auto* label=new QLabel(parameter.label); value->setProperty("parameterLabel",QVariant::fromValue(static_cast<QObject*>(label))); particleForm->addRow(label,field);
        } else if(parameter.id!="cycleRate") {
            auto* label=new QLabel(parameter.label); label->setWordWrap(true); value->setFixedWidth(72); value->setProperty("parameterLabel",QVariant::fromValue(static_cast<QObject*>(label)));
            auto* field=sliderField(value); value->setProperty("visibilityContainer",QVariant::fromValue(static_cast<QObject*>(field)));
            advancedForm->addRow(label,field);
        }
        connect(value,&QDoubleSpinBox::valueChanged,this,[this,member=parameter.member](double number) { updateMotion("Effect parameter",[=](auto& layer) { layer.*member=number; }); });
    }
    auto* details=disclosure(motionControls,"More animation settings",advancedContent); details->setObjectName("advancedMotion");
    colorCycle_=new ColorListEditor(2,16); colorCycle_->setObjectName("colorCycle");
    auto* cycleContent=new QWidget; auto* cycleControls=new QVBoxLayout(cycleContent);
    auto* enableCycle=new QCheckBox("Enable color animation"); enableCycle->setObjectName("enableColorCycle"); cycleControls->addWidget(enableCycle);
    connect(enableCycle,&QCheckBox::toggled,this,[this](bool on) {
        updateMotion("Enable color animation",[=](auto& layer) { layer.cycleRate=on ? (layer.cycleRate>0 ? layer.cycleRate : .25) : 0; });
        if(on && !editingMask_) playing_->setChecked(true);
    });
    auto* cycleHint=new QLabel("Fade through these colors over time. Set loops / sec above zero to animate; zero uses the normal effect colors.");
    cycleHint->setWordWrap(true); cycleControls->addWidget(cycleHint);
    auto* cycleForm=new QFormLayout; cycleControls->addLayout(cycleForm);
    for(auto* value:advanced_) if(value->objectName()=="cycleRate") cycleForm->addRow("Palette loops / sec",sliderField(value));
    cycleControls->addWidget(colorCycle_);
    disclosure(motionControls,"Animate through these colors",cycleContent);
    connect(colorCycle_,&ColorListEditor::colorsChanged,this,[this](const QVector<QColor>& colors) { updateMotion("Cycle colors",[colors](auto& layer) { layer.colorCycle=colors; }); });
    connect(colorCycle_,&ColorListEditor::gestureStarted,this,[this] { document_.history()->beginMacro("Cycle color"); });
    connect(colorCycle_,&ColorListEditor::gestureFinished,this,[this] { document_.history()->endMacro(); });
    effectType_->setProperty("appearanceContainer",QVariant::fromValue(static_cast<QObject*>(appearance)));
    effectType_->setProperty("motionContainer",QVariant::fromValue(static_cast<QObject*>(motion)));
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
    cropSection->findChild<QToolButton*>()->setChecked(false);
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
    auto* geometrySection=disclosure(appearanceControls,"Position and size (optional)",geometryControls_);
    geometrySection->setObjectName("geometrySection");
    geometrySection->findChild<QToolButton*>()->setChecked(false);
    repetitions_ = new QSpinBox;
    repetitions_->setObjectName("effectRepetitions");
    repetitions_->setRange(1, 64);
    repetitions_->setPrefix("Repetitions: ");
    appearanceControls->addWidget(repetitions_);
    connect(repetitions_, &QSpinBox::valueChanged, this, [this](int value) {
        updateLayer("Effect repetitions", [value](auto& layer) { layer.repetitions = value; });
    });
    auto* tools=new QToolBar("Drawing tools",this); tools->setObjectName("effectTools"); tools->setMovable(false);
    tools->setAllowedAreas(Qt::LeftToolBarArea); addToolBar(Qt::LeftToolBarArea,tools);
    auto* toolGroup=new QButtonGroup(this); toolGroup->setExclusive(true);
    const QStringList toolNames{"Move", "Pencil", "Line", "Rectangle", "Ellipse", "Gradient", "Fill", "Eraser"};
    const QList<int> keys{Qt::Key_V,Qt::Key_B,Qt::Key_L,Qt::Key_U,Qt::Key_O,Qt::Key_G,Qt::Key_F,Qt::Key_E};
    for(int index=0;index<toolNames.size();++index) {
        auto* tool=new QPushButton; tool->setObjectName(index==0 ? "moveEffectTool" : toolNames[index].toLower()+"Tool"); tool->setFixedSize(40,40);
        tool->setIcon(studioToolIcon(toolNames[index].toLower())); tool->setCheckable(true); tool->setShortcut(QKeySequence(keys[index]));
        const QStringList descriptions{"Move the selected layer. Drag corners to resize and the round handle to rotate.","Draw on the active paint layer. Procedural effects receive a new paint layer.","Drag from the start to the end of a straight line.","Drag a rectangle. Use Filled shapes for a solid shape.","Drag an ellipse. Use Filled shapes for a solid shape.","Drag to set a gradient. Afterwards drag either arrow endpoint to change its length and direction.","Fill a connected area with the foreground color. On a procedural or image source, place only that region on a new paint layer above it.","Erase paint pixels. On a procedural effect, erase its mask so the effect stays editable."};
        tool->setToolTip(toolNames[index]+" ("+QChar(keys[index])+")\n"+descriptions[index]); tool->setAccessibleName(toolNames[index]); tools->addWidget(tool); toolGroup->addButton(tool,index);
        if(index==0) { effectTarget_=tool; tool->setChecked(true); }
        connect(tool,&QPushButton::clicked,this,[this,index] { effectsCanvas_->setCropTool(false); findChild<QCheckBox*>("cropTool")->setChecked(false); effectsCanvas_->setTool(static_cast<LightingCanvas::Tool>(index)); refreshProperties(); });
    }
    for(const auto& family:QStringList{"Shapes","Particles"}) {
        auto* flyout=new QToolButton; flyout->setObjectName(family.toLower()+"Tool"); flyout->setIcon(studioToolIcon(family.toLower()));
        flyout->setAccessibleName(family); flyout->setToolTip(family+": choose a source to add as a new layer"); flyout->setFixedSize(40,40); flyout->setPopupMode(QToolButton::InstantPopup);
        auto* menu=new QMenu(family,flyout); flyout->setMenu(menu);
        for(const auto& definition:effectDefinitions()) if(family=="Shapes" ? definition.kind>=EffectKind::Circle && definition.kind<=EffectKind::Moon : isParticle(definition.kind)) {
            auto* action=menu->addAction(definition.label); action->setObjectName("create_"+definition.id);
            connect(action,&QAction::triggered,this,[this,kind=definition.kind] {
                if(document_.state().layers.size()>=64) { statusBar()->showMessage("Layer limit reached."); return; }
                editingMask_=false; findChild<QCheckBox*>("cropTool")->setChecked(false);
                selectedLayer_=document_.addLayer(kind); effectsCanvas_->setCropTool(false); effectsCanvas_->setTool(LightingCanvas::Tool::Move);
                inspectorTabs_->setCurrentIndex(isParticle(kind) ? 1 : 0); effectTarget_->setChecked(true); refresh();
            });
        }
        tools->addWidget(flyout);
    }
    auto* cropTool=new QCheckBox; cropTool->setObjectName("cropTool"); cropTool->setParent(this); cropTool->hide();
    auto* cropButton=new QToolButton; cropButton->setIcon(studioToolIcon("crop")); cropButton->setCheckable(true); cropButton->setFixedSize(40,40); cropButton->setObjectName("cropImageTool");
    cropButton->setToolTip("Crop imported image (C)"); cropButton->setAccessibleName("Crop image"); cropButton->setShortcut(QKeySequence(Qt::Key_C)); tools->addWidget(cropButton);
    connect(cropButton,&QToolButton::toggled,cropTool,&QCheckBox::setChecked); connect(cropTool,&QCheckBox::toggled,cropButton,&QToolButton::setChecked);
    connect(cropTool,&QCheckBox::toggled,this,[this](bool crop) { if(crop) { editingMask_=false; effectsCanvas_->setTool(LightingCanvas::Tool::Move); } effectsCanvas_->setCropTool(crop); });
    connect(effectsCanvas_,&LightingCanvas::effectGestureStarted,this,[this] {
        resumePreview_=playing_->isChecked(); playing_->setChecked(false); previewChannel_->setCurrentIndex(0);
        // Transform the frame under the pointer; resetting animated geometry makes visible shapes unclickable.
        if(effectsCanvas_->tool()!=LightingCanvas::Tool::Move || editingMask_) previewTime_->setValue(0);
    });
    connect(effectsCanvas_,&LightingCanvas::maskEditingRequested,this,[this] { inspectorTabs_->setCurrentIndex(2); });
    connect(effectsCanvas_,&LightingCanvas::editingError,this,[this](const QString& message) { statusBar()->showMessage(message,8000); });
    connect(effectsCanvas_,&LightingCanvas::gestureEnded,this,[this] { if(resumePreview_) playing_->setChecked(true); resumePreview_=false; });
    connect(effectsCanvas_,&LightingCanvas::selectionChanged,this,[this] {
        if(selectedLayer_!=effectsCanvas_->selection()) { selectedLayer_=effectsCanvas_->selection(); refresh(); }
    });
    auto* toolOptions=new QToolBar("Tool options",this); toolOptions->setObjectName("drawingOptions"); toolOptions->setMovable(false); addToolBarBreak(); addToolBar(toolOptions);
    appearanceControls->removeWidget(color_); color_->setMaximumWidth(190); toolOptions->addWidget(color_)->setObjectName("foregroundColorAction");
    auto* shapeFill=new QCheckBox("Filled shapes"); shapeFill->setObjectName("shapeFill"); shapeFill->setChecked(true); toolOptions->addWidget(shapeFill)->setObjectName("shapeFillAction");
    tools->hide(); toolOptions->hide();
    connect(tabs_,&QTabWidget::currentChanged,this,[this,tools](int index) { tools->setVisible(index==1); refreshToolOptions(); });
    maskEnabled_ = new QCheckBox("Enable mask");
    maskControls->addWidget(maskEnabled_);
    connect(maskEnabled_, &QCheckBox::toggled, this, [this](bool enabled) {
        updateLayer("Toggle mask", [enabled](auto& layer) { layer.maskEnabled = enabled; });
    });
    auto* showMask = new QCheckBox("Show grayscale mask");
    maskControls->addWidget(showMask);
    connect(showMask, &QCheckBox::toggled, effectsCanvas_, &LightingCanvas::setShowMask);
    auto* brushForm = new QFormLayout;
    maskControls->addLayout(brushForm);
    auto spin = [brushForm](const QString& name, int low, int high, int value) {
        auto* control = new QDoubleSpinBox; control->setDecimals(0); control->setRange(low, high); control->setValue(value); control->setAccessibleName(name);
        brushForm->addRow(name, sliderField(control)); return control;
    };
    auto* size=new QDoubleSpinBox; size->setAccessibleName("Brush diameter"); size->setObjectName("brushDiameter"); size->setRange(1,320); size->setValue(6); size->setSuffix(" px"); size->setFixedWidth(90);
    auto* stroke=sliderField(size); stroke->setMinimumWidth(180); stroke->setMaximumWidth(250);
    stroke->findChild<QSlider*>()->setObjectName("brushSizeSlider");
    toolOptions->addWidget(new QLabel("  Stroke "))->setObjectName("strokeLabelAction"); toolOptions->addWidget(stroke)->setObjectName("strokeSizeAction");
    const auto syncDrawing=[this,size,shapeFill] { effectsCanvas_->setDrawing(color_->color(),size->value(),shapeFill->isChecked()); };
    connect(color_,&ColorEditor::colorChanged,this,[syncDrawing] { syncDrawing(); }); connect(size,&QDoubleSpinBox::valueChanged,this,[syncDrawing] { syncDrawing(); }); connect(shapeFill,&QCheckBox::toggled,this,[syncDrawing] { syncDrawing(); });
    syncDrawing();
    auto* toolHint=new QLabel; toolHint->setObjectName("toolHint"); toolHint->setProperty("muted",true); toolOptions->addWidget(toolHint);
    auto* hardness = spin("Hardness %", 0, 100, 30);
    auto* brushOpacity = spin("Brush opacity %", 0, 100, 100);
    auto* gray = spin("Mask gray", 0, 255, 0);
    gray->setToolTip("0 hides; 255 reveals; intermediate values create partial transparency.");
    const auto syncBrush=[this,size,hardness,brushOpacity,gray] {
        effectsCanvas_->setBrush(size->value()/2.0,hardness->value()/100.0,brushOpacity->value()/100.0,gray->value());
    };
    for (auto* control : {size, hardness, brushOpacity, gray}) connect(control,&QDoubleSpinBox::valueChanged,this,syncBrush);
    syncBrush(); // The initial mask brush must match the displayed diameter before the first size edit.
    button(maskControls, "Fill mask white", [this] { updateLayer("Reveal layer", [](auto& layer) { layer.mask.fill(255); }); });
    button(maskControls, "Fill mask black", [this] { updateLayer("Hide layer with mask", [](auto& layer) { layer.mask.fill(0); }); });
    button(maskControls, "Invert mask", [this] { updateLayer("Invert mask", [](auto& layer) { layer.mask.invertPixels(); }); });
    maskControls->addStretch(); appearanceControls->addStretch(); effectControls->addStretch();
    auto* layerHint=new QLabel("Drag to reorder. Double-click to rename.\nClick the mask thumbnail to edit transparency.");
    layerHint->setWordWrap(true); layerHint->setProperty("muted",true); layerStack->addWidget(layerHint);
    connect(tabs_, &QTabWidget::currentChanged,this,[sidebarContexts,contexts,sidebarDock,dock](int index) {
        sidebarDock->setVisible(index!=2); dock->setVisible(index!=2);
        if(index<2) { sidebarContexts->setCurrentIndex(index); contexts->setCurrentIndex(index); }
    });
    connect(effectsCanvas_,&LightingCanvas::addRequested,this,[this] {
        for(auto* control:findChildren<QPushButton*>()) if(control->text()=="New layer") { control->click(); break; }
    });
    connect(layoutCanvas_,&LightingCanvas::addRequested,this,[this] { addHeaderComponents(); });
    connect(&document_, &LightingDocument::changed, this, &LightingWindow::refresh);
    connect(document_.history(), &QUndoStack::cleanChanged, this, [this](bool clean) { setWindowModified(!clean); });
    setWindowTitle("Lighting Studio[*] — MSI Mystic Light Standalone");
    statusBar()->showMessage("Lighting Studio - Create effects and arrange components; canvas output is automatic.");
    auto* viewMenu=menuBar()->addMenu("View"); viewMenu->setObjectName("workspaceViewMenu");
    viewMenu->addAction(sidebarDock->toggleViewAction()); viewMenu->addAction(dock->toggleViewAction());
    auto* resetWorkspace=viewMenu->addAction("Reset workspace"); resetWorkspace->setObjectName("resetWorkspace");
    connect(resetWorkspace,&QAction::triggered,this,[this,sidebarDock,dock] {
        const bool studio=applicationPages_->currentIndex()==0; sidebarDock->setVisible(studio); dock->setVisible(studio); resizeDocks({sidebarDock,dock},{260,390},Qt::Horizontal);
        layoutCanvas_->fitToView(); effectsCanvas_->fitToView();
    });
    auto* navigation=addToolBar("Workspace"); navigation->setObjectName("workspaceNavigation"); navigation->setMovable(false);
    insertToolBar(toolbar,navigation); auto* group=new QActionGroup(this); group->setExclusive(true);
    const QStringList sections{"Layout", "Effects", "Hardware"};
    for(int index=0;index<3;++index) {
        auto* action=navigation->addAction(sections[index]); action->setObjectName("workspace"+sections[index]); action->setCheckable(true); group->addAction(action);
        connect(action,&QAction::triggered,this,[this,index,tools,previewToolbar] {
            tabs_->setCurrentIndex(index); previewToolbar->setVisible(index!=2); tools->setVisible(index==1);
        });
    }
    connect(tabs_,&QTabWidget::currentChanged,this,[this](int index) { findChild<QAction*>(index==0 ? "workspaceLayout" : index==1 ? "workspaceEffects" : "workspaceHardware")->setChecked(true); });
    tabs_->tabBar()->hide();
    findChild<QAction*>("workspaceLayout")->trigger();
    resizeDocks({sidebarDock,dock},{260,390},Qt::Horizontal);
    installParameterTooltips(this);
    refresh();
    for(QWidget* target : {static_cast<QWidget*>(layoutCanvas_),static_cast<QWidget*>(effectsCanvas_),static_cast<QWidget*>(layers_),static_cast<QWidget*>(components_),static_cast<QWidget*>(hardwareTree)}) target->installEventFilter(this);
    playing_->setChecked(true);
}

void LightingWindow::updateMotion(const QString& label,const std::function<void(EffectLayer&)>& operation)
{
    if(!findChild<QCheckBox*>("allLayerMotion")->isChecked()) { updateLayer(label,operation); return; }
    try { document_.edit(label,[&](auto& state) { for(auto& layer:state.layers) if(layer.visible) operation(layer); }); }
    catch(const std::exception& error) { statusBar()->showMessage(QString::fromUtf8(error.what()),6000); refreshProperties(); }
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

void LightingWindow::restoreLegacyHardware(const QString& path)
{
    try {
        const auto notes=importLegacyHardware(document_,path,scan_());
        findChild<QLabel*>("hardwareStatus")->setText(notes.join("\n"));
        statusBar()->showMessage(notes.first(),15000);
    } catch(const std::exception& error) { findChild<QLabel*>("hardwareStatus")->setText("Restore failed: "+QString::fromUtf8(error.what())); }
}

void LightingWindow::scanHeaders()
{
    try {
        discovered_=scan_();
        for(const auto& candidate:discovered_) {
            const auto& profile=assignmentProfile(document_.profiles(),candidate.profileId);
            const bool dedicated=profile.driverId=="lighting.peripherals" || profile.driverId=="lighting.auxiliary";
            for(const auto& channel:profile.channels) {
                bool configured=false;
                for(const auto& assignment:document_.state().assignments) if(assignment.endpoint==candidate.device.path && assignment.channelId==channel.id) configured=true;
                const auto componentId=channel.address.value("componentId").toString();
                if(configured || (!dedicated && channel.id!="IOShield")) continue;
                if(!document_.catalog().contains(componentId) && document_.installedCatalog().contains(componentId))
                    document_.defineComponent(document_.installedCatalog()[componentId]);
                if(!document_.catalog().contains(componentId)) continue;
                document_.edit("Detect integrated lighting",[&](LightingState& state) {
                    ComponentPlacement placement; placement.componentId=componentId;
                    placement.origin={80.0+100*(state.placements.size()%5),260.0};
                    ChannelAssignment assignment; assignment.endpoint=candidate.device.path; assignment.profileId=profile.id;
                    assignment.channelId=channel.id; assignment.software=true; assignment.placements={placement.id};
                    state.placements.append(placement); state.assignments.append(assignment);
                });
            }
        }
        refresh(); emit hardwareDiscovered();
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
    findChild<QPushButton*>("addHeaderComponents")->setEnabled(true);
    findChild<QPushButton*>("headerSettings")->setEnabled(selected);
    if(!selected) { headerTitle_->setText("No header selected"); headerSummary_->setText("Detected headers appear on the left. Select one to attach your components."); return; }
    const auto assignment=selectedHeaderAssignment();
    const auto& profile=assignmentProfile(document_.profiles(),assignment.profileId);
    const auto& channel=assignmentChannel(profile,assignment.channelId);
    findChild<QPushButton*>("addHeaderComponents")->setEnabled(profile.driverId!="lighting.peripherals" && profile.driverId!="lighting.auxiliary" && channel.id!="IOShield");
    int leds=0;
    for(const auto& id:assignment.placements) for(const auto& placement:document_.state().placements)
        if(placement.id==id) leds+=document_.catalog()[placement.componentId].positions.size();
    for(const auto& placement:document_.state().placements) if(placement.id==layoutCanvas_->selection()) {
        const auto& component=document_.catalog()[placement.componentId]; headerTitle_->setText(hardwareName(component));
        headerSummary_->setText(QString("%1 / %2 LEDs\nDrag this component on the layout canvas.").arg(assignment.channelId).arg(component.positions.size())); return;
    }
    headerTitle_->setText(assignment.channelId);
    headerSummary_->setText(QString("%1\n%2 components / %3 of %4 LEDs\n%5").arg(profile.name).arg(assignment.placements.size()).arg(leds).arg(channel.maxLeds)
        .arg(channel.direct ? "Effects canvas supported" : "Native lighting only"));
}

void LightingWindow::addHeaderComponents()
{
    QVector<HeaderChoice> choices;
    for(int i=0;i<components_->topLevelItemCount();++i) {
        const auto data=components_->topLevelItem(i)->data(0,Qt::UserRole+1).toMap(); if(data.isEmpty()) continue;
        const auto& profile=assignmentProfile(document_.profiles(),data.value("profileId").toString());
        if(profile.driverId=="lighting.peripherals" || profile.driverId=="lighting.auxiliary") continue; // Dedicated devices are detected, never attached to ARGB headers.
        const auto& channel=assignmentChannel(profile,data.value("channelId").toString());
        if(channel.id=="IOShield") continue; // Integrated shared-color zone, not an attachable fan header.
        int used=0;
        for(const auto& assignment:document_.state().assignments) if(assignment.profileId==profile.id && assignment.endpoint==data.value("path") && assignment.channelId==channel.id)
            for(const auto& id:assignment.placements) for(const auto& placement:document_.state().placements) if(placement.id==id) used+=document_.catalog()[placement.componentId].positions.size();
        choices.append({profile.name+" / "+channel.id,data,channel.maxLeds-used,channel.fixedLeds ? channel.fixedLeds-used : 0});
    }
    HardwarePicker picker(document_.catalog(),choices,selectedHeader_,this);
    if(picker.exec()!=QDialog::Accepted) return;
    selectedHeader_=picker.header(); const auto assignment=selectedHeaderAssignment();
    try {
        const auto component=picker.component(); const int count=picker.quantity();
        document_.edit("Attach hardware",[&](LightingState& state) {
            if(!state.components.contains(component.id) && state.components.size()>=256) throw std::invalid_argument("Component definition limit reached");
            if(!state.components.contains(component.id)) state.components.insert(component.id,component);
            auto updated=assignment;
            QRectF bounds; for(const auto& point:component.positions) bounds=bounds.united(QRectF(point,QSizeF(.001,.001)));
            const double scale=3.0/std::max(3.0,std::max(bounds.width(),bounds.height()));
            for(int index=0;index<count;++index) {
                ComponentPlacement placement; placement.componentId=component.id; const int position=state.placements.size();
                placement.scale={scale,scale}; placement.origin={50.0+(position%7)*80-bounds.left()*20*scale,60.0+(position/7)*80-bounds.top()*20*scale};
                state.placements.append(placement); updated.placements.append(placement.id);
            }
            bool replaced=false; for(auto& existing:state.assignments) if(existing.id==updated.id) { existing=updated; replaced=true; }
            if(!replaced) state.assignments.append(updated);
        });
    } catch(const std::exception& error) { statusBar()->showMessage(QString::fromUtf8(error.what()),8000); }
}

void LightingWindow::refreshPlacement()
{
    const ComponentPlacement* selected = nullptr;
    for (const auto& placement : document_.state().placements)
        if (placement.id == layoutCanvas_->selection()) selected = &placement;
    findChild<QPushButton*>("removeComponent")->setEnabled(selected!=nullptr);
    findChild<QWidget*>("placementSection")->setVisible(selected!=nullptr);
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
    hardwarePreview_->setVisible(!document_.state().placements.isEmpty()); // Give the space back to authoring when there is no hardware to preview.
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
                            auto* child=new QTreeWidgetItem(group,{hardwareName(document_.catalog()[placement.componentId],i+1)});
                            child->setData(0,Qt::UserRole,placement.id);
                            child->setText(1,hardwareManufacturer(document_.catalog()[placement.componentId]));
                            if(placement.id==layoutCanvas_->selection()) components_->setCurrentItem(child);
                        }
                }
        }
        QTreeWidgetItem* unassigned=nullptr;
        for(const auto& placement:document_.state().placements) if(!assigned.contains(placement.id)) {
            if(!unassigned) { unassigned=new QTreeWidgetItem(components_,{"Unassigned components"}); unassigned->setExpanded(true); }
            auto* child=new QTreeWidgetItem(unassigned,{hardwareName(document_.catalog()[placement.componentId])});
            child->setData(0,Qt::UserRole,placement.id);
            child->setText(1,hardwareManufacturer(document_.catalog()[placement.componentId]));
            if(placement.id==layoutCanvas_->selection()) components_->setCurrentItem(child);
        }
        if(selectedHeader_.isEmpty() && !headers.isEmpty()) { selectedHeader_=headers.first(); components_->setCurrentItem(components_->topLevelItem(0)); }
    }
    findChild<QTreeView*>("hardwareComponents")->expandAll();
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
        for (const auto& component : document_.catalog()) catalog_->addItem(hardwareName(component), component.id);
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
        item->setToolTip(0,it->name+" - Effect: "+effectDefinition(it->effect).label+"\nClick the eye or press Space to show/hide this layer.");
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
    auto* action=findChild<QAction*>("reviewOutput"); action->setText("Output: "+state);
    const auto description=state=="Running" ? QString("Output is running. Canvas edits and playback update software channels live.") :
        state=="Stopped" ? QString("Output is stopped. Screen preview sends no colors. Open Hardware to resume output.") : "Output: "+state;
    findChild<QLabel*>("hardwareStatus")->setText(description);
    const auto* detail=findChild<QLabel*>("outputStatus");
    action->setToolTip(description+(detail ? "\n"+detail->text() : QString{}));
    auto* label=findChild<QLabel*>("toolbarOutputStatus"); label->setText("Output: "+state); label->setToolTip(action->toolTip());
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
    layoutCanvas_->setPreviewFrame(previewFrame_,seconds); effectsCanvas_->setPreviewFrame(displayed,localTime,channel); hardwarePreview_->setPreviewFrame(previewFrame_,seconds);
    emit previewFrameChanged(previewFrame_);
}

void LightingWindow::refreshToolOptions()
{
    auto* options=findChild<QToolBar*>("drawingOptions"); if(!options) return;
    const EffectLayer* selected=nullptr;
    for(const auto& layer:document_.state().layers) if(layer.id==selectedLayer_) selected=&layer;
    const auto tool=effectsCanvas_->tool();
    const bool drawing=tool!=LightingCanvas::Tool::Move && tool!=LightingCanvas::Tool::Eraser && (tool!=LightingCanvas::Tool::Gradient || !selected || !isGradient(selected->effect));
    const bool color=!editingMask_ && (drawing || (selected && selected->effect!=EffectKind::Image && !isGradient(selected->effect)));
    const bool stroke=(editingMask_ || tool!=LightingCanvas::Tool::Move) && tool!=LightingCanvas::Tool::Gradient && tool!=LightingCanvas::Tool::Fill;
    const bool shape=tool==LightingCanvas::Tool::Rectangle || tool==LightingCanvas::Tool::Ellipse;
    findChild<QAction*>("foregroundColorAction")->setVisible(color); color_->setVisible(color);
    findChild<QAction*>("shapeFillAction")->setVisible(shape);
    findChild<QAction*>("strokeLabelAction")->setVisible(stroke);
    findChild<QAction*>("strokeSizeAction")->setVisible(stroke);
    const QStringList hints{"Move: drag the selected layer", "Pencil: draw on the canvas", "Line: drag from start to end", "Rectangle: drag to draw", "Ellipse: drag to draw", "Gradient: drag an arrow; drag either endpoint to adjust", "Fill: click a connected area", "Eraser: remove pixels or hide the source mask"};
    findChild<QLabel*>("toolHint")->setText("  "+hints[int(tool)]);
    // Photoshop-style options keep a stable row so switching tools never shifts the canvas under the pointer.
    options->setVisible(tabs_->currentIndex()==1);
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

    for (QWidget* widget : std::initializer_list<QWidget*>{transparency_, blend_, color_, gradientColors_, maskEnabled_})
        widget->setEnabled(selected != nullptr);
    // Editing a procedural source with Eraser selects its mask; that must never lock Canvas Start.
    playing_->setEnabled(true);
    inspectorTabs_->setEnabled(selected!=nullptr);
    inspectorTitle_->setText(selected ? effectDefinition(selected->effect).label : "Select a layer");
    animationControls_->setVisible(selected && !editingMask_);
    findChild<QWidget*>("particleParameters")->setVisible(selected && isParticle(selected->effect));
    findChild<QPushButton*>("resetCrop")->setVisible(selected && !editingMask_ && selected->effect==EffectKind::Image);
    findChild<QWidget*>("cropSection")->setVisible(selected && !editingMask_ && selected->effect == EffectKind::Image);
    maskLinked_->setEnabled(selected != nullptr);
    auto* cropTool=findChild<QCheckBox*>("cropTool");
    if ((!selected || selected->effect!=EffectKind::Image) && cropTool->isChecked()) cropTool->setChecked(false);
    cropTool->setEnabled(selected && !editingMask_ && selected->effect==EffectKind::Image && !selected->image.isNull());
    findChild<QToolButton*>("cropImageTool")->setEnabled(cropTool->isEnabled());
    effectType_->setEnabled(selected && !editingMask_);
    effectType_->setVisible(selected && !editingMask_ && (isParticle(selected->effect) || isGradient(selected->effect)));
    if(selected && (isParticle(selected->effect) || isGradient(selected->effect))) {
        auto* host=qobject_cast<QWidget*>(effectType_->property(isParticle(selected->effect) ? "motionContainer" : "appearanceContainer").value<QObject*>());
        if(effectType_->parentWidget()!=host) qobject_cast<QVBoxLayout*>(host->layout())->insertWidget(0,effectType_);
        effectType_->clear();
        for(const auto& definition:effectDefinitions()) if(isParticle(selected->effect) ? isParticle(definition.kind) : isGradient(definition.kind))
            effectType_->addItem(QString(definition.label).replace("Particle ",""),static_cast<int>(definition.kind));
    }
    color_->setVisible(!editingMask_ && selected && selected->effect != EffectKind::Image && !isGradient(selected->effect));
    gradientColors_->setVisible(selected && !editingMask_ && (isGradient(selected->effect) || selected->gradientFill || effectsCanvas_->tool()==LightingCanvas::Tool::Gradient));
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
        animation_[2]->setEnabled(selected->effect!=EffectKind::Image && selected->effect!=EffectKind::Paint);
        const QSignalBlocker blockLink(maskLinked_), blockCycle(colorCycle_);
        maskLinked_->setChecked(selected->maskLinked);
        colorCycle_->setColors(selected->colorCycle);
        const auto stops=selected->gradientColors.isEmpty() ? QVector<QColor>{selected->color,selected->secondaryColor} : selected->gradientColors;
        gradientColors_->setStops(stops,selected->gradientPositions); effectsCanvas_->setGradientStops(stops,selected->gradientPositions);
        colorCycle_->setEnabled(selected->effect != EffectKind::Image);
        findChild<QCheckBox*>("enableColorCycle")->setEnabled(selected->effect != EffectKind::Image);
        { const QSignalBlocker block(findChild<QCheckBox*>("enableColorCycle")); findChild<QCheckBox*>("enableColorCycle")->setChecked(selected->cycleRate>0); }
        const auto& parameters = advancedParameters();
        for (int i=0;i<advanced_.size();++i) {
            const QSignalBlocker block(advanced_[i]); advanced_[i]->setValue(selected->*(parameters[i].member));
            const bool particle = parameters[i].id.startsWith("particle");
            const bool show = !particle || isParticle(selected->effect);
            advanced_[i]->setVisible(show);
            if(auto* field=qobject_cast<QWidget*>(advanced_[i]->property("visibilityContainer").value<QObject*>())) field->setVisible(show);
            if(auto* label=qobject_cast<QWidget*>(advanced_[i]->property("parameterLabel").value<QObject*>())) label->setVisible(show);
            advanced_[i]->setEnabled((!particle || isParticle(selected->effect)) &&
                (parameters[i].id != "cycleRate" || selected->effect != EffectKind::Image));
        }
        const double crop[]{selected->crop.x(),selected->crop.y(),selected->crop.width(),selected->crop.height()};
        for (int i=0;i<4;++i) { const QSignalBlocker block(crop_[i]); crop_[i]->setValue(crop[i]); }
        const QSignalBlocker blockFit(imageFit_);
        imageFit_->setCurrentIndex(selected->imageFill ? 1 : 0);
        const bool brush=effectsCanvas_->tool()!=LightingCanvas::Tool::Move || selected->effect==EffectKind::Paint;
        color_->setColor(brush ? foreground_ : selected->color);
        effectsCanvas_->setDrawing(brush ? foreground_ : selected->color,findChild<QDoubleSpinBox*>("brushDiameter")->value(),findChild<QCheckBox*>("shapeFill")->isChecked());
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
    refreshToolOptions();
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
    refreshProjectName();
    if (recovered) document_.history()->resetClean();
    findChild<QAction*>("workspaceEffects")->trigger();
}

bool LightingWindow::saveDocument()
{
    const auto path = fileName_.isEmpty() ? QFileDialog::getSaveFileName(this,
        "Save lighting document", "Untitled.lighting.json", "Lighting documents (*.lighting.json)") : fileName_;
    if (path.isEmpty()) return false;
    try { saveProject(path); return true; }
    catch (const std::exception& error) { QMessageBox::warning(this, "Save failed", QString::fromUtf8(error.what())); return false; }
}

void LightingWindow::saveProject(const QString& path)
{
    if(path.trimmed().isEmpty()) throw std::invalid_argument("A project filename is required");
    // Adopt the new name only after the atomic QSaveFile commit succeeds.
    document_.save(path); fileName_=path; refreshProjectName(); statusBar()->showMessage("Saved "+path);
}

bool LightingWindow::confirmDiscard()
{
    if (document_.history()->isClean()) return true;
    const auto answer = QMessageBox::question(this, "Unsaved lighting document",
        "Save your layout, layers and masks before continuing?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    return answer == QMessageBox::Discard || (answer == QMessageBox::Save && saveDocument());
}

void LightingWindow::refreshProjectName()
{
    const auto name=fileName_.isEmpty() ? QString("Untitled") : QFileInfo(fileName_).fileName();
    auto* label=findChild<QLabel*>("currentProjectName"); label->setText(label->fontMetrics().elidedText(name,Qt::ElideMiddle,200)); label->setToolTip(fileName_);
    setWindowTitle(name+"[*] — Lighting Studio");
}

void LightingWindow::removeSelectedComponents()
{
    const auto selected=layoutCanvas_->selection();QSet<QUuid> ids;
    for(const auto& placement:document_.state().placements) if(placement.id==selected) ids.insert(selected);
    for(const auto& assignment:document_.state().assignments) if(assignment.id==selected)
        for(const auto& id:assignment.placements) ids.insert(id);
    if(ids.isEmpty()) return;
    try {
        document_.edit("Remove components",[&](LightingState& state) {
            for(auto& assignment:state.assignments) assignment.placements.removeIf([&](const auto& id){return ids.contains(id);});
            // Removing a chain's last component must also remove its output and effect-target references.
            QSet<QUuid> empty;
            for(const auto& assignment:state.assignments) if(assignment.software && assignment.placements.isEmpty()) empty.insert(assignment.id);
            state.assignments.removeIf([&](const auto& assignment){return empty.contains(assignment.id);});
            for(auto& layer:state.layers) for(const auto& id:empty) layer.channelTargets.removeAll(id);
            state.placements.removeIf([&](const auto& placement){return ids.contains(placement.id);});
        });
        layoutCanvas_->select({});
    } catch(const std::exception& error) {statusBar()->showMessage("Cannot remove component: "+QString::fromUtf8(error.what()),6000);}
}
bool LightingWindow::eventFilter(QObject* watched,QEvent* event)
{
    if(event->type()==QEvent::KeyPress) {
        const auto* key=static_cast<QKeyEvent*>(event);
        if(key->key()==Qt::Key_Delete && !(key->modifiers() & ~Qt::KeypadModifier)) {
            // Text/spin editors own Delete while typing; never turn a field edit into object deletion.
            const auto* focus=QApplication::focusWidget();
            if(qobject_cast<const QLineEdit*>(focus) || qobject_cast<const QAbstractSpinBox*>(focus)) return QMainWindow::eventFilter(watched,event);
            if(!key->isAutoRepeat()) {
                if(watched==effectsCanvas_ || watched==layers_) findChild<QPushButton*>("deleteLayer")->click();
                else removeSelectedComponents();
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(watched,event);
}
