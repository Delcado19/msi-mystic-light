#include "ParameterSpinBox.h"
#include "LightingWindow.h"
#include "HardwareArtwork.h"
#include "SnapGrid.h"
#include "HardwareIdentity.h"
#include <QFileInfo>
#include <QDateTime>
#include <QKeyEvent>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QAbstractSpinBox>
#include <QScreen>
#include "HardwarePicker.h"
#include "LegacyHardware.h"
#include <QStandardPaths>
#include <QDir>
#include <QDialog>
#include <QSplitter>
#include <QSplitterHandle>
#include <QEnterEvent>
#include <QButtonGroup>
#include "ComponentEditorPage.h"
#include "ChannelEditor.h"
#include "StudioWidgets.h"
#include "StudioTheme.h"
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
#include <QResizeEvent>
#include <QComboBox>
#include <QDockWidget>
#include <QDropEvent>
#include <QDrag>
#include <QMimeData>
#include <QFormLayout>
#include <QFileDialog>
#include <QInputDialog>
#include <QSaveFile>
#include <QJsonDocument>
#include <QHeaderView>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QScrollArea>
#include <QScrollBar>
#include <QSlider>
#include <QRandomGenerator>
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
#include <QWidgetAction>
#include <QVBoxLayout>
#include <QRegularExpression>

namespace {
// R3: shared LED outputs and Header settings redesign. R4: aligned rows, Space panning, layers-only drawing options, logo right, auto-fit follows edits. R5: saved layouts no longer move the logo left; undistorted effect area covering all housings; numbered keys. R6: menu cleanup, one unit per wheel notch, trimmed larger logo. R7: audit fixes (PawnIO path, HID padding, Apex 3, service idle, resume reconnect, installer). R9: identify flash, shared-aware LED counts, LED column and component ranges. R10: service survives hung device opens, conflict dialog checks every discovered device. R11: UAC stop per process, outputs reinitialized after a conflict stop. R12: CodeRabbit review fixes (probe-gated opening, physical device match, installer results). R13: exited family members count as stopped. R14: scan deadline, stale writer filter. R15: 12 s scan deadline, stale scan results rejected. R16: gray stops take a hue, live drag output, menus only where they act, sorted effect library. R17: edits stream during a drag without a rescan, Tools hidden in Devices, effect categories as submenus. R18: canvas drags stream live, menus show only what acts in the current view. R19: no 5 s rescan while all outputs run, output stalls logged. R20: presence check off the output thread (150 ms LED freeze each second). R21: uninstaller backs up user files, never kills the editor. R22: tray icon and name, Layout menu without Add components. R23: product name Lighting Studio for settings and session data. R24: CodeRabbit fixes for PR #3, picker keeps the color on an outside click, Return applies.
constexpr auto studioBuildId="GS-06-R46";
void setEffectControlRange(QDoubleSpinBox* field,double minimum,double maximum,double current)
{
    // Keep legacy values visible without rewriting projects; sliders use only the practical range.
    field->setProperty("sliderMinimum",minimum);field->setProperty("sliderMaximum",maximum);
    field->setRange(std::min(minimum,current),std::max(maximum,current));field->setValue(current);
}
QPair<double,double> practicalEffectRange(const EffectParameter& parameter,EffectKind kind)
{
    const auto& id=parameter.id;
    // Expose the renderer's supported rate for every pattern; the former UI caps hid valid fast motion.
    if(id=="patternSpeed") return {parameter.minimum,parameter.maximum};
    if(id=="patternWidth") return {.01,kind==EffectKind::Visor ? .5 : .25};
    if(id=="patternAmplitude") return {0,kind==EffectKind::Dna ? .45 : .4};
    if(id=="patternCells") return {2,32};
    if(id=="patternSites") return {4,24};
    if(id=="patternDetail") return {1,16};
    if(id=="particleCount" && kind==EffectKind::Lightning) return {1,12};
    if(id=="particleCount" && kind==EffectKind::Fireworks) return {1,400};
    if(id=="particleSpeed" && kind==EffectKind::Fireworks) return {0,120};
    if(id=="driftX" || id=="driftY") return {-500,500};
    if(id=="zoom" || id=="warp" || id=="warpY") return {0,50};
    if(id=="breathe" || id=="cycleRate" || id=="trails") return {0,1};
    if(id=="particleCount" || id=="particleSpeed") return {parameter.minimum,200};
    if(id=="particleSize") return {1,20};
    return {parameter.minimum,parameter.maximum};
}
class PreviewDivider final : public QSplitterHandle {
public:
    explicit PreviewDivider(QSplitter* parent):QSplitterHandle(Qt::Vertical,parent) {
        setObjectName("effectPreviewDivider");setCursor(Qt::SplitVCursor);
        setAccessibleName("Resize effect canvas and live hardware preview");
        setToolTip("Drag up or down to resize the effect canvas and live hardware preview");
    }
    QSize sizeHint() const override {return {80,18};}
protected:
    void enterEvent(QEnterEvent* event) override {QSplitterHandle::enterEvent(event);update();}
    void leaveEvent(QEvent* event) override {QSplitterHandle::leaveEvent(event);update();}
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(),QColor("#252e3d"));
        painter.setPen(QPen(QColor(underMouse() ? "#bfd3f3" : "#7f9ac2"),2));
        painter.drawLine(10,height()/2,width()-42,height()/2);
        painter.setPen(Qt::NoPen);painter.setBrush(QColor("#38465b"));painter.drawRoundedRect(QRectF(width()-37,1,30,height()-2),3,3);
        painter.setPen(QPen(QColor("#e0eaff"),1.6,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        const int x=width()-22;
        painter.drawLine(x,3,x,height()-3);
        painter.drawLine(x,3,x-4,7);painter.drawLine(x,3,x+4,7);
        painter.drawLine(x,height()-3,x-4,height()-7);painter.drawLine(x,height()-3,x+4,height()-7);
    }
};
class PreviewSplitter final : public QSplitter {
public:
    PreviewSplitter():QSplitter(Qt::Vertical) {setHandleWidth(18);setChildrenCollapsible(true);}
protected:
    QSplitterHandle* createHandle() override {return new PreviewDivider(this);}
};
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
        // Custom names retain their effect subtitle; default names already identify the effect.
        const auto name=item.text; item.text.clear();
        const bool subtitle=name.compare(index.data(Qt::UserRole+1).toString(),Qt::CaseInsensitive)!=0;
        const auto eye=style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator,&item,widget).adjusted(-2,-2,2,2);
        item.features &= ~QStyleOptionViewItem::HasCheckIndicator;
        style->drawControl(QStyle::CE_ItemViewItem,&item,painter,widget);
        // One row marker avoids cyan seams at every Source/Mask cell boundary.
        if(item.state & QStyle::State_Selected)painter->fillRect(QRect(item.rect.left(),item.rect.top(),2,item.rect.height()),studioThemeColor("@accent"));
        static const auto visible=studioToolIcon("eye"), hidden=studioToolIcon("eye-off");
        (index.data(Qt::CheckStateRole).toInt()==Qt::Checked ? visible : hidden).paint(painter,eye);
        painter->save(); auto font=item.font; font.setBold(true); painter->setFont(font);
        painter->setPen(QColor("#e8eaf0"));
        painter->drawText(QRect(textRect.x(),subtitle ? textRect.center().y()-19 : textRect.center().y()-11,textRect.width(),22),Qt::AlignVCenter,
            painter->fontMetrics().elidedText(name,Qt::ElideRight,textRect.width()));
        font.setBold(false); font.setPointSizeF(std::max(8.0,font.pointSizeF()-1)); painter->setFont(font); painter->setPen(QColor("#b9c8e4"));
        if(subtitle)painter->drawText(QRect(textRect.x(),textRect.center().y()+3,textRect.width(),20),Qt::AlignVCenter,
            painter->fontMetrics().elidedText(index.data(Qt::UserRole+1).toString(),Qt::ElideRight,textRect.width())); painter->restore();
    }
    QSize sizeHint(const QStyleOptionViewItem& option,const QModelIndex& index) const override {
        const auto name=index.siblingAtColumn(0);
        auto size=QStyledItemDelegate::sizeHint(option,index);
        size.setHeight(name.data().toString().compare(name.data(Qt::UserRole+1).toString(),Qt::CaseInsensitive)==0 ? 44 : 58);return size;
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
void updatePlaybackButton(QPushButton* button,bool playing)
{
    // Labelled: the bare red square read as an unknown control (critique 2026-11-26).
    button->setText(playing ? "Stop" : "Play");button->setMinimumWidth(84);button->setIconSize({18,18});
    button->setIcon(studioToolIcon(playing ? "preview-stop" : "preview-play"));
    button->setAccessibleName(playing ? "Stop preview" : "Play preview");
    button->setToolTip(playing ? "Stop preview animation; keep the current frame" : "Play preview animation");
}
QIcon colorIcon(QColor color)
{
    QPixmap pixmap(32, 24);
    pixmap.fill(color);
    return QIcon(pixmap);
}
}

namespace {
class DeviceTree : public QTreeView {
public:
    using QTreeView::QTreeView;
    std::function<void(const QUuid&,const QVariantMap&)> componentDropped;
protected:
    void startDrag(Qt::DropActions) override {
        const auto item=currentIndex().siblingAtColumn(0);
        if (!item.parent().isValid()) return;
        const auto id=item.data(Qt::UserRole).toUuid();
        if (id.isNull()) return;
        auto* drag=new QDrag(this);
        auto* mime=new QMimeData;
        mime->setData("application/x-lighting-component",id.toByteArray());
        drag->setMimeData(mime);
        drag->exec(Qt::MoveAction);
    }
    void dragEnterEvent(QDragEnterEvent* event) override {
        if (event->source()==this) event->acceptProposedAction();
    }
    void dragMoveEvent(QDragMoveEvent* event) override {
        if (event->source()==this && !connectorAt(event->position().toPoint()).isEmpty()) event->acceptProposedAction();
    }
    void dropEvent(QDropEvent* event) override {
        if (event->source()!=this || !componentDropped) return;
        const auto connector=connectorAt(event->position().toPoint());
        if (connector.isEmpty()) return;
        const auto id=QUuid::fromString(QString::fromUtf8(event->mimeData()->data("application/x-lighting-component")));
        if (id.isNull()) return;
        event->acceptProposedAction();
        componentDropped(id,connector);
    }
private:
    QVariantMap connectorAt(const QPoint& point) const {
        auto item=indexAt(point).siblingAtColumn(0);
        if (item.parent().isValid()) item=item.parent();
        return item.data(Qt::UserRole+1).toMap();
    }
};

// Trim transparent margins so the logo fills the header row (owner report 2026-11-14).
QPixmap studioLogo()
{
    QImage image(":/branding/lighting-studio-header.png");image=image.convertToFormat(QImage::Format_ARGB32);
    QRect used;for(int y=0;y<image.height();++y)for(int x=0;x<image.width();++x)if(qAlpha(image.pixel(x,y))>8)used=used.united(QRect(x,y,1,1));
    if(!used.isEmpty())image=image.copy(used.adjusted(-4,-4,4,4).intersected(image.rect()));
    // Slim header (owner decision 2026-11-26): 56 px instead of 140; the large banner is the splash screen.
    return QPixmap::fromImage(image.scaled(320,56,Qt::KeepAspectRatio,Qt::SmoothTransformation));
}
}
LightingWindow::LightingWindow(ComponentCatalog catalog,
    std::function<QVector<ProfileCandidate>()> scan, QWidget* parent, QVector<DeviceProfile> profiles, bool automaticDiscovery, QString libraryPath)
    : QMainWindow(parent), document_(std::move(catalog), nullptr, std::move(profiles), libraryPath)
{
    setWindowTitle("Lighting Studio");
    setProperty("studioBackground",true);
    resize(1440, 900);
    setMinimumSize(1060, 680);
    applyStudioStyle(this);
    auto* fileMenu = menuBar()->addMenu("File");
    auto* open = fileMenu->addAction("Open…"); open->setShortcut(QKeySequence::Open);
    connect(open, &QAction::triggered, this, [this] {
        const auto file = QFileDialog::getOpenFileName(this, "Open lighting document", QDir::homePath(), "Lighting documents (*.lighting.json)");
        if (file.isEmpty() || !confirmDiscard()) return;
        for (;;) {
            try { loadProject(file); break; }
            catch (const MissingComponents& missing) {
                const auto choice = QMessageBox::question(this, "Missing component definitions",
                    "Import definitions for the following components?\n\n" + missing.ids.join("\n"));
                if (choice != QMessageBox::Yes) break;
                const auto definitions = QFileDialog::getOpenFileNames(this, "Import missing components",
                    QDir::homePath(), "Component definitions (*.json)");
                if (definitions.isEmpty()) break;
                try {
                    for (const auto& definition : definitions) document_.importComponent(definition);
                } catch (const std::exception& error) {
                    QMessageBox::warning(this, "Import failed", QString::fromUtf8(error.what()));
                    break;
                }
            } catch (const std::exception& error) {
                QMessageBox::warning(this, "Open failed", QString::fromUtf8(error.what()));
                break;
            }
        }
    });
    auto* save = fileMenu->addAction("Save"); save->setObjectName("saveProject"); save->setShortcut(QKeySequence::Save);
    connect(save, &QAction::triggered, this, [this] { saveDocument(); });
    auto* saveAs = fileMenu->addAction("Save as…"); saveAs->setObjectName("saveProjectAs"); saveAs->setShortcut(QKeySequence::SaveAs);
    connect(saveAs, &QAction::triggered, this, [this] {
        const auto previous = fileName_; fileName_.clear();
        if (!saveDocument()) { fileName_ = previous; refreshProjectName(); }
    });
    auto* help=menuBar()->addMenu("Help");
    auto* shortcuts=help->addAction("Keyboard shortcuts");shortcuts->setObjectName("keyboardShortcuts");
    // Grouped key/action table (owner finding 2026-11-26: the old message box was cramped and unreadable).
    connect(shortcuts,&QAction::triggered,this,[this]{
        const QList<QPair<QString,QList<QPair<QString,QString>>>> groups{
            {"Project",{{"Ctrl+O","Open layout"},{"Ctrl+S","Save layout"},{"Ctrl+Shift+S","Save layout as"},{"Ctrl+Z","Undo"},{"Ctrl+Y / Ctrl+Shift+Z","Redo"}}},
            {"View",{{"Space + drag","Move the canvas view"},{"Mouse wheel","Zoom at the cursor"},{"Ctrl+0","Fit canvas"},{"Hold L","Show component names"}}},
            {"Tools",{{"V","Move"},{"H","Pan"},{"B","Pencil"},{"I","Line"},{"U","Rectangle"},{"O","Ellipse"},{"G","Gradient"},{"F","Fill"},{"E","Eraser"},{"C","Crop image"}}},
            {"Selection",{{"Ctrl+D","Duplicate selection"},{"Del","Delete selection"},{"Ctrl+PgUp / Ctrl+PgDn","Move layer up / down"},{"Ctrl+Shift+G","Snap to grid on / off"},{"Hold Alt","Place freely, ignoring the grid"}}},
            {"Panels and fields",{{"Ctrl+Shift+L","Navigator on / off"},{"Ctrl+Shift+P","Inspector on / off"},{"F6 / Shift+F6","Next / previous panel"},{"Alt+R","Reset the focused number field"}}}};
        QString html="<table cellspacing=\"0\" cellpadding=\"5\">";
        for(const auto& group:groups) {
            html+="<tr><td colspan=\"2\" style=\"padding-top:12px\"><b>"+group.first.toHtmlEscaped()+"</b></td></tr>";
            for(const auto& entry:group.second)html+="<tr><td style=\"padding-right:28px\"><b>"+entry.first.toHtmlEscaped()+"</b></td><td>"+entry.second.toHtmlEscaped()+"</td></tr>";
        }
        html+="</table><p>The Component Editor lists its own keys under its ? button.</p>";
        QDialog dialog(this);dialog.setWindowTitle("Keyboard shortcuts");dialog.setObjectName("keyboardShortcutsDialog");
        auto* layout=new QVBoxLayout(&dialog);auto* scroll=new QScrollArea;scroll->setWidgetResizable(true);scroll->setFrameShape(QFrame::NoFrame);
        auto* text=new QLabel(html);text->setObjectName("keyboardShortcutsTable");text->setTextFormat(Qt::RichText);text->setContentsMargins(8,0,8,0);
        scroll->setWidget(text);layout->addWidget(scroll,1);
        auto* close=new QDialogButtonBox(QDialogButtonBox::Close);layout->addWidget(close);
        connect(close,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
        dialog.resize(520,640);dialog.exec();});
    auto* about=help->addAction("About Lighting Studio");about->setObjectName("aboutLightingStudio");
    connect(about,&QAction::triggered,this,[this]{QDialog dialog(this);dialog.setWindowTitle("Lighting Studio");auto* stack=new QVBoxLayout(&dialog);stack->addWidget(studioBanner(&dialog,210));auto* description=new QLabel(QString("Lighting Studio — build %1\nRGB layout, effects and hardware control.\nBanner artwork depicts a conceptual interface.").arg(studioBuildId));description->setWordWrap(true);stack->addWidget(description);auto* executable=new QLabel(QCoreApplication::applicationFilePath());executable->setObjectName("runningExecutablePath");executable->setWordWrap(true);executable->setTextFormat(Qt::PlainText);executable->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);stack->addWidget(executable);auto* close=new QDialogButtonBox(QDialogButtonBox::Close);stack->addWidget(close);connect(close,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);dialog.resize(800,300);dialog.exec();});
    auto* artworkCredits=help->addAction("Artwork credits");artworkCredits->setObjectName("artworkCredits");
    connect(artworkCredits,&QAction::triggered,this,[this] {
        QMessageBox::about(this,"Artwork credits",
            "<p>Fan artwork adapted from <b>Computer fan</b> by Delapouite, "
            "<a href=\"https://game-icons.net/1x1/delapouite/computer-fan.html\">game-icons.net</a>, "
            "<a href=\"https://creativecommons.org/licenses/by/3.0/\">CC BY 3.0</a>.</p>"
            "<p>Changes: square housing, muted colors, transparent background. "
            "Other technical hardware drawings: Lighting Studio contributors, GPL-2.0-or-later.</p>");
    });
    auto* editMenu = menuBar()->addMenu("Edit");
    undoGroup_ = new QUndoGroup(this); undoGroup_->addStack(document_.history()); undoGroup_->setActiveStack(document_.history());
    auto* undo = undoGroup_->createUndoAction(this, "Undo"); undo->setObjectName("editUndo");
    undo->setShortcut(QKeySequence::Undo);
    editMenu->addAction(undo);
    auto* redo = undoGroup_->createRedoAction(this, "Redo"); redo->setObjectName("editRedo");
    redo->setShortcuts({QKeySequence::Redo, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z)});
    editMenu->addAction(redo);
    editMenu->addSeparator();
    // Program settings, stored per user in QSettings and never in the project (owner request 2026-11-24).
    auto* settingsAction=editMenu->addAction("Settings...");settingsAction->setObjectName("programSettings");
    connect(settingsAction,&QAction::triggered,this,[this] {
        QDialog dialog(this);dialog.setWindowTitle("Settings");
        auto* form=new QFormLayout(&dialog);
        auto* disconnected=new QComboBox;disconnected->setObjectName("disconnectedDevicesSetting");
        disconnected->addItem("Hide","hide");disconnected->addItem("Dim","dim");
        disconnected->setCurrentIndex(std::max(0,disconnected->findData(QSettings().value("editor/disconnectedDevices","hide"))));
        disconnected->setToolTip("Devices in the project that the last hardware scan did not find. They keep their layout and effects.");
        form->addRow("Disconnected devices",disconnected);
        auto* splash=new QCheckBox("Show the start screen");splash->setObjectName("showSplashSetting");
        splash->setChecked(QSettings().value("editor/showSplash",true).toBool());
        splash->setToolTip("Shows the large Lighting Studio banner while the program starts.");form->addRow("Start",splash);
        auto* ajazz=new QCheckBox("AJAZZ AK35I V4 keyboard (experimental, I own this exact model)");ajazz->setObjectName("ajazzAk35iV4Setting");
        ajazz->setChecked(QSettings().value("devices/ajazzAk35iV4",false).toBool());
        ajazz->setToolTip("Other AJAZZ keyboards use the same USB identity, so this profile is never chosen automatically.\nTakes effect after restarting Lighting Studio.");
        form->addRow("Keyboards",ajazz);
        auto* buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);form->addRow(buttons);
        connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
        if(dialog.exec()!=QDialog::Accepted)return;
        QSettings().setValue("editor/disconnectedDevices",disconnected->currentData());
        QSettings().setValue("editor/showSplash",splash->isChecked());
        QSettings().setValue("devices/ajazzAk35iV4",ajazz->isChecked());
        refresh(); // A setting is not a document change, so nothing else repaints the canvases.
    });
    auto* toolbar = studioHeaderToolbar("History",this);addToolBar(toolbar); toolbar->setObjectName("historyToolbar");
    toolbar->setMovable(false);
    auto* brand=new QLabel;brand->setObjectName("studioBrand");brand->setAccessibleName("Lighting Studio");brand->setPixmap(studioLogo());toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    open->setIcon(studioToolIcon("open")); save->setIcon(studioToolIcon("save"));
    undo->setIcon(studioToolIcon("undo")); redo->setIcon(studioToolIcon("redo"));
    toolbar->addAction(open); toolbar->addAction(save);
    saveAs->setIcon(studioToolIcon("save-as")); saveAs->setToolTip("Save layout under a new name (Ctrl+Shift+S)"); toolbar->addAction(saveAs);
    auto* projectName=new QLabel("Untitled"); projectName->setObjectName("currentProjectName"); projectName->setMinimumWidth(80); projectName->setMaximumWidth(210); toolbar->addWidget(projectName)->setObjectName("projectNameAction");
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
        // Effects must keep its tools and layers visible; only Layout offers canvas fullscreen.
        if(canvas->context()==LightingCanvas::Context::Layout) connect(canvas,&LightingCanvas::fullscreenRequested,this,[this,canvas,layout] {
            if(auto* host=qobject_cast<QDialog*>(canvas->window());host && host->objectName()=="canvasFullscreen") {host->reject();return;}
            auto* host=new QDialog(this);host->setObjectName("canvasFullscreen");host->setAttribute(Qt::WA_DeleteOnClose);
            auto* full=new QVBoxLayout(host);
            const auto screenBounds=screen()->geometry(), available=screen()->availableGeometry();
            // Some Windows taskbars remain above fullscreen dialogs; keep controls inside the usable monitor area.
            full->setContentsMargins(std::max(0,available.left()-screenBounds.left()),std::max(8,available.top()-screenBounds.top()),
                std::max(0,screenBounds.right()-available.right()),std::max(8,screenBounds.bottom()-available.bottom()));
            auto* playback=new QPushButton(host);playback->setObjectName("fullscreenCanvasPlayback");
            playback->setCheckable(true);playback->setChecked(playing_->isChecked());updatePlaybackButton(playback,playing_->isChecked());
            auto* header=new QHBoxLayout;header->addWidget(playback);
            auto* outputState=new QLabel(findChild<QLabel*>("toolbarOutputStatus")->text());outputState->setObjectName("fullscreenOutputStatus");outputState->setProperty("statusBadge",true);
            outputState->setProperty("outputState",findChild<QLabel*>("toolbarOutputStatus")->property("outputState"));
            outputState->setTextFormat(Qt::RichText); // Traffic light, no link (owner request 2026-11-17).
            outputState->setAccessibleName("Device output status");header->addWidget(outputState);header->addStretch();full->addLayout(header);
            // Only the canvas receives spare height; a stretchable status label must not expand the header.
            full->addWidget(canvas,1);
            connect(playback,&QPushButton::toggled,playing_,&QPushButton::setChecked);
            connect(playing_,&QPushButton::toggled,playback,[playback](bool running){playback->setChecked(running);updatePlaybackButton(playback,running);});
            connect(host,&QDialog::finished,this,[canvas,layout] {layout->addWidget(canvas);canvas->show();canvas->fitToView();});
            host->showFullScreen();canvas->fitToView();
        });
        return wrapper;
    };
    tabs_->addTab(wrapCanvas(layoutCanvas_), "Layout");
    auto* effectWorkspace=new PreviewSplitter; effectWorkspace->setObjectName("effectWorkspace");
    hardwarePreview_=new LightingCanvas(document_,LightingCanvas::Context::Layout); hardwarePreview_->setObjectName("hardwareLivePreview"); hardwarePreview_->setPreviewOnly();
    auto* livePanel=new QWidget;livePanel->setObjectName("hardwarePreviewPanel");auto* liveLayout=new QVBoxLayout(livePanel);liveLayout->setContentsMargins(0,0,0,0);liveLayout->setSpacing(0);
    liveLayout->addWidget(hardwarePreview_);
    effectWorkspace->addWidget(wrapCanvas(effectsCanvas_)); effectWorkspace->addWidget(livePanel); effectWorkspace->setStretchFactor(0,3); effectWorkspace->setStretchFactor(1,2); effectWorkspace->setSizes({740,0});livePanel->hide();
    tabs_->addTab(effectWorkspace, "Effects");
    applicationPages_=new QStackedWidget; applicationPages_->setObjectName("applicationPages");
    applicationPages_->addWidget(tabs_); setCentralWidget(applicationPages_);
    // The Component Editor is a page, not a tab: replay the tab change with workspaceIndex() so every
    // tab handler hides docks, drawing tools and canvas commands while it is open and restores them on
    // any exit (owner review 2026-11-26: the editor looked different depending on where it was opened).
    connect(applicationPages_,&QStackedWidget::currentChanged,this,[this](int) {
        undoGroup_->setActiveStack(componentPage_ && applicationPages_->currentWidget()==componentPage_ ? componentPage_->undoStack() : document_.history());
        emit tabs_->currentChanged(workspaceIndex());
    });
    connect(this,&LightingWindow::outputRequested,this,[this] { findChild<QAction*>("workspaceHardware")->trigger(); });

    auto* dock = new QDockWidget("INSPECTOR", this); dock->setObjectName("propertiesDock");
    dock->setFeatures(QDockWidget::DockWidgetClosable);
    auto* contexts = new QStackedWidget;contexts->setObjectName("inspectorSurface");
    auto* sidebarDock=new QDockWidget("NAVIGATOR",this); sidebarDock->setObjectName("layersDock");
    sidebarDock->setFeatures(QDockWidget::DockWidgetClosable);
    sidebarDock->setMinimumWidth(220); sidebarDock->setMaximumWidth(330);
    auto* sidebarContexts=new QStackedWidget;sidebarContexts->setObjectName("navigatorSurface"); sidebarDock->setWidget(sidebarContexts);
    addDockWidget(Qt::LeftDockWidgetArea,sidebarDock);
    auto* componentPanel=new QWidget; auto* componentStack=new QVBoxLayout(componentPanel);
    componentStack->setContentsMargins(8,8,8,8); componentStack->setSpacing(6);
    sidebarContexts->addWidget(componentPanel);
    auto* layersPanel=new QWidget; auto* layerStack=new QVBoxLayout(layersPanel);
    layerStack->setContentsMargins(8,8,8,8); layerStack->setSpacing(6);
    sidebarContexts->addWidget(layersPanel);
    dock->setWidget(contexts);
    auto addPanel = [contexts](QWidget* panel) {
        auto* scroll = new QScrollArea;
        scroll->setWidgetResizable(true);
        // Dynamic inspector sections must grow the scroll content, never compress fields at high DPI.
        panel->layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);
        scroll->setWidget(panel);
        contexts->addWidget(scroll);
    };
    addDockWidget(Qt::RightDockWidgetArea, dock);
    dock->setMinimumWidth(300); dock->setMaximumWidth(440);
    auto* layoutPanel = new QWidget;
    auto* layoutControls = new QVBoxLayout(layoutPanel);
    layoutControls->setContentsMargins(8,8,8,8); layoutControls->setSpacing(6);
    addPanel(layoutPanel);
    auto* removeComponent=button(layoutControls, "Remove selected component", [this] {removeSelectedComponents();});
    removeComponent->setProperty("danger",true);
    removeComponent->setObjectName("removeComponent");
    auto* hardwarePage=new QWidget; hardwarePage->setObjectName("hardwarePage");
    auto* hardwareColumns=new QHBoxLayout(hardwarePage); hardwareColumns->setContentsMargins(28,24,28,24); hardwareColumns->setSpacing(28);
    auto* hardwareOptions=new QWidget; auto* hardwareControls=new QVBoxLayout(hardwareOptions); hardwareControls->setContentsMargins(0,0,0,0);
    auto* hardwareOutput=new QWidget; deviceOutput_=new QVBoxLayout(hardwareOutput); deviceOutput_->setContentsMargins(0,0,0,0);
    hardwareOptions->setMinimumWidth(320);hardwareOptions->setMaximumWidth(620);
    hardwareColumns->addWidget(hardwareOptions,2); hardwareColumns->addWidget(hardwareOutput,3);
    auto* hardwareEmpty=new QLabel("No compatible controller detected\n\nConnect a supported device and choose Refresh. Saved layouts remain editable offline; arranging components does not connect hardware.");
    hardwareEmpty->setObjectName("hardwareEmptyState");hardwareEmpty->setWordWrap(true);hardwareEmpty->setProperty("emptyState",true);
    deviceOutput_->addWidget(hardwareEmpty);deviceOutput_->addStretch();
    auto* hardwareTitle=new QLabel("Devices"); hardwareTitle->setProperty("heading",true);
    // The component library opens from the top of Devices, next to the title (workspace brief 2026-11-26).
    auto* titleRow=new QHBoxLayout; titleRow->setContentsMargins(0,0,0,0); titleRow->addWidget(hardwareTitle,1);
    hardwareControls->addLayout(titleRow);
    auto* hardwareHelp=new QLabel("Select a connector to add components. Use Layout to arrange them; Connector settings sets their LED order.");
    hardwareHelp->setWordWrap(true); hardwareControls->addWidget(hardwareHelp);
    // Full-window Component Editor; replaces the former "Custom components" dialog (owner critique 2026-11-25).
    auto* componentEditorButton=new QPushButton("Component Editor"); titleRow->addWidget(componentEditorButton);
    connect(componentEditorButton,&QPushButton::clicked,this,[this] { if (leaveComponentPage()) { componentPage()->openLibrary(); applicationPages_->setCurrentWidget(componentPage()); } });
    componentEditorButton->setObjectName("openComponentEditor");
    componentEditorButton->setToolTip("Create, edit, import, export and delete component definitions");
    auto* offline = new QComboBox;
    offline->setObjectName("offlineController");
    offline->setAccessibleName("Controller for offline planning");
    offline->addItem("Choose a controller for offline planning", QString());
    for (const auto& profile : document_.profiles())
        if (std::any_of(profile.channels.cbegin(), profile.channels.cend(), [](const auto& channel) {
            return channel.direct && !channelHasFixedComponent(channel);
        })) offline->addItem(profile.name, profile.id);
    hardwareControls->addWidget(offline);
    connect(offline, &QComboBox::currentIndexChanged, this, [this,offline](int) {
        offlineProfile_=offline->currentData().toString();
        selectedHeader_.clear();
        refresh();
    });
    const auto editDeviceLeds = [this] {
        const auto id=layoutCanvas_->selection();
        for (const auto& placement : document_.state().placements) if (placement.id==id) {
            if (placement.locked) { statusBar()->showMessage("Unlock the component in Layout before editing.",6000); return; }
            bool arrangementOnly=false;
            for (const auto& assignment:document_.state().assignments) if (assignment.placements.contains(id)) {
                arrangementOnly=channelHasFixedComponent(assignmentChannel(
                    assignmentProfile(document_.profiles(),assignment.profileId),assignment.channelId));
                break;
            }
            if (!leaveComponentPage()) return;
            componentPage()->openInstance(id,arrangementOnly);
            applicationPages_->setCurrentWidget(componentPage());
            return;
        }
    };
    // One entry point: LEDs of a placed part are edited from Layout only (workspace brief 2026-11-26).
    auto* editLayoutLeds=button(layoutControls,"Edit LEDs",editDeviceLeds);
    editLayoutLeds->setObjectName("editLayoutLeds");
    const auto updateLedActions=[this,editLayoutLeds] {
        bool placed=false;
        for (const auto& placement:document_.state().placements) if(placement.id==layoutCanvas_->selection()) placed=!placement.locked;
        editLayoutLeds->setEnabled(placed);
        editLayoutLeds->setToolTip("Place LEDs and edit their address order. Devices with fixed wiring support positioning.");
    };
    connect(layoutCanvas_,&LightingCanvas::selectionChanged,this,updateLedActions);
    connect(&document_,&LightingDocument::changed,this,updateLedActions);
    updateLedActions();
    auto* identify = button(hardwareControls, "Identify", [this] { flashComponent(layoutCanvas_->selection()); });
    identify->setObjectName("identifyDeviceComponent");
    identify->setToolTip("Highlight the selected LED addresses. If hardware output is active, those addresses flash.");
    auto* componentMenu = new QPushButton("Component actions");
    auto* actions = new QMenu(componentMenu);
    auto* duplicateComponentAction = actions->addAction("Duplicate");
    auto* removeComponentAction = actions->addAction("Remove");
    componentMenu->setMenu(actions);
    hardwareControls->addWidget(componentMenu);
    connect(duplicateComponentAction,&QAction::triggered,this,[this] {
        try { layoutCanvas_->select(document_.duplicateComponent(layoutCanvas_->selection())); }
        catch (const std::exception& error) { headerSummary_->setText(QString::fromUtf8(error.what())); }
    });
    removeComponentAction->setObjectName("removeDeviceComponent");
    connect(removeComponentAction,&QAction::triggered,this,[this] {
        for (const auto& placement:document_.state().placements) if (placement.id==layoutCanvas_->selection()) {
            if (placement.locked) { headerSummary_->setText("Unlock this component in Layout before removing it."); return; }
            break;
        }
        removeSelectedComponents();
    });
    auto* detachHardware = componentMenu;
    detachHardware->setObjectName("removeHardwareComponent");
    detachHardware->setEnabled(false);
    identify->setEnabled(false);
    connect(layoutCanvas_,&LightingCanvas::selectionChanged,identify,[this,identify] {
        const auto id=layoutCanvas_->selection();
        identify->setEnabled(std::any_of(document_.state().placements.cbegin(),document_.state().placements.cend(),
            [&](const auto& placement) { return placement.id==id; }));
    });
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
    tabs_->addTab(hardwareScroll,"Devices");
    components_=new QTreeWidget; components_->setObjectName("layoutComponents");
    components_->setHeaderLabels({"Connector / component","Status / manufacturer","LEDs"});
    auto* hardwareTree=new DeviceTree;
    hardwareTree->setObjectName("hardwareComponents");
    hardwareTree->setMinimumHeight(220);
    hardwareTree->setDragEnabled(true);
    hardwareTree->setAcceptDrops(true);
    hardwareTree->setDragDropMode(QAbstractItemView::DragDrop);
    hardwareTree->componentDropped=[this](const auto& id,const auto& target) {
        try { moveComponentToConnector(id,connectorAssignment(target)); }
        catch (const std::exception& error) { headerSummary_->setText(QString::fromUtf8(error.what())); }
    };
    // Both workspaces share the same inventory model and selection; editing never maintains a second hardware list.
    hardwareTree->setModel(components_->model()); hardwareTree->setSelectionModel(components_->selectionModel()); hardwareControls->insertWidget(2,hardwareTree,1);
    hardwareTree->header()->setStretchLastSection(false);
    hardwareTree->header()->setSectionResizeMode(0,QHeaderView::Stretch); hardwareTree->header()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
    hardwareTree->header()->setSectionResizeMode(2,QHeaderView::ResizeToContents);
    components_->setColumnHidden(1,true); // Keep the narrow Layout sidebar focused on names; Hardware exposes the manufacturer column.
    components_->setColumnHidden(2,true); // LED usage column: Hardware view only.
    components_->setSelectionMode(QAbstractItemView::ExtendedSelection);components_->setHeaderHidden(true); components_->setRootIsDecorated(true); componentStack->insertWidget(0,components_,1);
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
    connect(components_,&QTreeWidget::itemSelectionChanged,this,[this]{
        QSet<QUuid> ids;for(auto* item:components_->selectedItems()){const auto id=item->data(0,Qt::UserRole).toUuid();if(!id.isNull())ids.insert(id);}
        const auto primary=components_->currentItem() ? components_->currentItem()->data(0,Qt::UserRole).toUuid() : QUuid{};
        QMetaObject::invokeMethod(this,[this,ids,primary]{if(!ids.isEmpty() && layoutCanvas_->selection()==primary)layoutCanvas_->selectMany(ids,primary);},Qt::QueuedConnection);
    });
    auto* layoutTitle=new QLabel("Arrange your lighting"); layoutTitle->setProperty("heading",true); layoutControls->addWidget(layoutTitle);
    auto* layoutHint=new QLabel("Select a group or component. Drag its corners to resize, the round handle to rotate, or its LEDs to move.");
    layoutHint->setWordWrap(true); layoutHint->setProperty("muted",true); layoutControls->addWidget(layoutHint);
    auto* placementPrecision=new QWidget; auto* placementForm = new QFormLayout(placementPrecision);
    auto* placementSection=disclosure(layoutControls,"Position, size and rotation",placementPrecision); placementSection->setObjectName("placementSection");
    placementSection->findChild<QToolButton*>()->setChecked(true);
    const QStringList placementLabels{"Position X", "Position Y", "Scale X %", "Scale Y %", "Rotation"};
    for (int i = 0; i < placementLabels.size(); ++i) {
        auto* control = new ParameterSpinBox;
        control->setObjectName(QString("placement%1").arg(i));

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
        syncSelectionTrees();
        updatePreview();
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
    // One name for every connector; fixed devices still get their playback clock there (2026-11-26).
    auto* headerSettings=button(headerControls,"Connector settings...",[this] {
        if(selectedHeader_.isEmpty()) return;
        try {
            ChannelEditor editor(document_,selectedHeaderAssignment(),this);
            if(editor.exec()==QDialog::Accepted) document_.assignChannel(editor.assignment());
        } catch(const std::exception& error) { statusBar()->showMessage(QString::fromUtf8(error.what()),6000); }
    });
    headerSettings->setObjectName("headerSettings");
    auto* componentPreview = new QLabel;
    componentPreview->setObjectName("deviceComponentPreview");
    componentPreview->setAlignment(Qt::AlignCenter);
    componentPreview->setAccessibleName("Selected component and LED address preview");
    // The pixmap is rendered at the label's size (updatePreview), so the label must not size itself from it.
    componentPreview->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    componentPreview->setMinimumHeight(110);
    headerControls->addWidget(componentPreview,1);
    auto* deviceActions=new QWidget;
    deviceActions->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    auto* deviceActionRows=new QVBoxLayout(deviceActions);
    deviceActionRows->setContentsMargins(0,0,0,0);
    auto* componentActions=new QHBoxLayout;
    for (auto* control:{identify,detachHardware}) {
        hardwareControls->removeWidget(control);
        componentActions->addWidget(control);
    }
    // Keep actions in a fixed-height panel so changes in the details cannot move the mouse targets.
    deviceActionRows->addLayout(componentActions);
    headerControls->removeWidget(headerSettings);
    deviceActionRows->addWidget(headerSettings);
    deviceOutput_->insertWidget(0,deviceActions);
    // Preview and the output endpoint list share the spare height (1:2) instead of a trailing spacer.
    deviceOutput_->insertWidget(1,headerPanel,1);
    auto* moveComponentAction = actions->addAction("Move to connector...");
    connect(moveComponentAction,&QAction::triggered,this,[this] {
        const auto id = layoutCanvas_->selection();
        QStringList captions;
        QVector<ChannelAssignment> targets;
        for (int index=0; index<components_->topLevelItemCount(); ++index) {
            const auto data=components_->topLevelItem(index)->data(0,Qt::UserRole+1).toMap();
            if (data.isEmpty()) continue;
            const auto& profile=assignmentProfile(document_.profiles(),data.value("profileId").toString());
            const auto& channel=assignmentChannel(profile,data.value("channelId").toString());
            if (!channel.direct || channelHasFixedComponent(channel)) continue;
            const auto target=connectorAssignment(data);
            if (target.placements.contains(id)) continue;
            targets.append(target);
            captions.append(QString("%1 / %2 (%3 addresses available) ? %4")
                .arg(profile.name,channel.id).arg(document_.availableAddresses(target)).arg(target.endpoint));
        }
        if (targets.isEmpty()) { headerSummary_->setText("No other connector can accept a component."); return; }
        bool accepted=false;
        const auto caption=QInputDialog::getItem(this,"Move component","Destination",captions,0,false,&accepted);
        if (!accepted) return;
        try { moveComponentToConnector(id,targets.at(captions.indexOf(caption))); }
        catch (const std::exception& error) { headerSummary_->setText(QString::fromUtf8(error.what())); }
    });
    auto* refreshHardware=button(hardwareControls,"Scan hardware",[this] { scanHeaders(); }); refreshHardware->setObjectName("refreshHeaders");
    auto* hardwareActions=new QHBoxLayout;
    for(auto* control:{addHardware,refreshHardware}) { hardwareControls->removeWidget(control); hardwareActions->addWidget(control); }
    addHardware->setText("Add component");
    refreshHardware->setToolTip("Refresh connected hardware and verify supported RAM identity through PawnIO. Discovery does not change colors; configured canvas output resumes automatically.");
    hardwareControls->insertLayout(3,hardwareActions);
    scan_=std::move(scan);
    if(automaticDiscovery)QTimer::singleShot(0,this,[this] { scanHeaders(); });

    addToolBarBreak(Qt::TopToolBarArea);
    auto* previewToolbar = studioHeaderToolbar("Preview",this);addToolBar(previewToolbar); previewToolbar->setObjectName("previewToolbar");
    playing_ = new QPushButton; playing_->setCheckable(true); playing_->setObjectName("playPreview");
    updatePlaybackButton(playing_,false);
    previewTime_ = new QDoubleSpinBox(this); previewTime_->setObjectName("previewTime");
    previewTime_->setRange(0, 86400); previewTime_->setDecimals(2); previewTime_->setSuffix(" s");
    previewTime_->setKeyboardTracking(false); previewTime_->setAccessibleName("Preview time in seconds");
    previewToolbar->setMovable(false);
    previewToolbar->addWidget(playing_); previewTime_->hide(); // Keep the animation clock internal, without a numeric toolbar field.
    previewChannel_=new QComboBox; previewChannel_->setObjectName("previewChannel");
    previewChannel_->setMaximumWidth(230);
    previewChannel_->addItem("Preview: All outputs",QUuid{}); previewChannel_->setToolTip("Preview one output composition. This does not change effect destinations.");
    previewToolbar->addWidget(previewChannel_);
    previewToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    auto* fit=previewToolbar->addAction(studioToolIcon("fit"),"Fit canvas"); fit->setObjectName("fitCanvas"); fit->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    connect(fit,&QAction::triggered,this,[this] { (tabs_->currentIndex()==0 ? layoutCanvas_ : effectsCanvas_)->fitToView(); });
    for(const auto& zoom:QVector<QPair<QString,double>>{{"Zoom out",1/1.2},{"Zoom in",1.2}}) {
        auto* action=previewToolbar->addAction(studioToolIcon(zoom.second<1 ? "zoom-out" : "zoom-in"),zoom.first); action->setObjectName(zoom.second<1 ? "zoomOut" : "zoomIn"); action->setToolTip(zoom.first+" (mouse wheel also zooms)");
        connect(action,&QAction::triggered,this,[this,factor=zoom.second] { (tabs_->currentIndex()==0 ? layoutCanvas_ : effectsCanvas_)->zoomBy(factor); });
    }
    // Keep one-click Fit; group zoom steps so the view-mode selector fits the command row.
    auto* zoomButton=new QToolButton;zoomButton->setObjectName("zoomMenuButton");zoomButton->setIcon(studioToolIcon("zoom-in"));zoomButton->setAccessibleName("Zoom");zoomButton->setToolTip("Zoom in or out. The mouse wheel also zooms; Ctrl+0 fits the canvas.");zoomButton->setPopupMode(QToolButton::InstantPopup);
    auto* zoomMenu=new QMenu("Zoom",zoomButton);zoomButton->setMenu(zoomMenu);zoomMenu->addAction(fit);
    for(const auto& name:{"zoomOut","zoomIn"}){auto* action=findChild<QAction*>(name);previewToolbar->removeAction(action);zoomMenu->addAction(action);}
    previewToolbar->addWidget(zoomButton);
    auto* snap=new QAction("Snap to grid",this);addAction(snap); snap->setObjectName("snapToGrid");
    snap->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_G));snap->setShortcutContext(Qt::ApplicationShortcut);
    snap->setCheckable(true);snap->setChecked(SnapGrid::load().enabled);
    snap->setToolTip("Snap positions and resize handles to the world grid; rotation uses 15 degrees. Hold Alt to move freely. Ctrl+Shift+G toggles snapping, including fullscreen.");
    auto* gridSize=new QSpinBox;gridSize->setObjectName("gridSpacing");gridSize->setRange(1,100);gridSize->setSuffix({});
    gridSize->setValue(int(SnapGrid::load().spacing));gridSize->setKeyboardTracking(false);
    gridSize->setAccessibleName("Snap grid spacing");gridSize->setToolTip("Spatial grid spacing in canvas pixels. Palette snapping has its own percentage step.");

    auto updateGrid=[this,snap,gridSize] {
        SnapGrid{snap->isChecked(),double(gridSize->value())}.save();
        layoutCanvas_->gridChanged();effectsCanvas_->gridChanged();for(auto* bar:findChildren<QWidget*>("colorStopBar"))bar->update();update();
    };
    connect(snap,&QAction::toggled,this,updateGrid);connect(gridSize,&QSpinBox::valueChanged,this,updateGrid);
    auto* labels=new QAction(studioToolIcon("eye"),"Labels",this);addAction(labels);labels->setObjectName("showComponentLabels");labels->setCheckable(true);
    labels->setChecked(QSettings().value("editor/showComponentLabels",false).toBool());
    labels->setToolTip("Show component labels. Hold L in the editor to reveal them temporarily. Full names remain available on hover.");
    layoutCanvas_->setLabelsAction(labels);effectsCanvas_->setLabelsAction(labels);
    connect(labels,&QAction::toggled,this,[this,labels](bool checked) {
        labels->setIcon(studioToolIcon(checked ? "eye" : "eye-off"));QSettings().setValue("editor/showComponentLabels",checked);setLabelsHeld(labelsHeld_);
    });
    labels->setIcon(studioToolIcon(labels->isChecked() ? "eye" : "eye-off"));setLabelsHeld(false);
    auto* output=previewToolbar->addAction("Output: Stopped"); output->setObjectName("reviewOutput");
    output->setToolTip("Show connected hardware and output status; canvas output starts automatically");
    // Keep output state outside toolbar overflow and separate from the animation clock.
    previewToolbar->removeAction(output); output->setEnabled(false);
    auto* outputStatus=new QLabel("Devices: Offline");
    outputStatus->setTextFormat(Qt::RichText); outputStatus->setObjectName("toolbarOutputStatus");
    outputStatus->setProperty("statusBadge",true);
    outputStatus->setAccessibleName("Device output status");statusBar()->addPermanentWidget(outputStatus);
    auto* previewStatus=new QLabel("Preview: stopped");previewStatus->setObjectName("previewPlaybackStatus");statusBar()->insertPermanentWidget(0,previewStatus);
    connect(previewChannel_,&QComboBox::currentIndexChanged,this,&LightingWindow::updatePreview);
    connect(layoutCanvas_,&LightingCanvas::layoutPreviewChanged,this,[this] { emit previewFrameChanged(previewFrame_); });
    auto* timer = new QTimer(this); timer->setTimerType(Qt::PreciseTimer); timer->setInterval(16);
    connect(playing_, &QPushButton::toggled, this, [this, timer](bool play) {
        updatePlaybackButton(playing_,play);
        findChild<QLabel*>("previewPlaybackStatus")->setText(play ? "Preview: playing" : "Preview: stopped");
        if (play) { playbackStart_ = previewTime_->value(); previewClock_.restart(); timer->start(); }
        else timer->stop();
    });
    connect(timer, &QTimer::timeout, this, [this] {
        previewTime_->setValue(std::min(86400.0, playbackStart_ + previewClock_.elapsed()/1000.0));
        if (previewTime_->value() >= 86400) playing_->setChecked(false);
    });
    connect(previewTime_, &QDoubleSpinBox::editingFinished, this, [this] { playing_->setChecked(false); });
    // Minimized beside the service: skip the full render, catch up in changeEvent (owner report 2026-11-25).
    connect(previewTime_, &QDoubleSpinBox::valueChanged, this, [this] {
        if (!renderWhenMinimized_ && isMinimized()) { previewStale_ = true; return; }
        updatePreview();
    });
    auto* effectPanel = new QWidget;
    auto* effectControls = new QVBoxLayout(effectPanel);
    effectControls->setContentsMargins(8,8,8,8); effectControls->setSpacing(6);
    inspectorTitle_=new QLabel("Select a layer"); inspectorTitle_->setProperty("heading",true); inspectorTitle_->setWordWrap(true); effectControls->addWidget(inspectorTitle_);
    auto* editingContext=new QLabel("Editing: effect");editingContext->setObjectName("layerEditingContext");editingContext->setWordWrap(true);editingContext->setProperty("modeBadge",true);effectControls->addWidget(editingContext);
    inspectorTabs_=new QTabWidget; inspectorTabs_->setObjectName("layerInspectorTabs");
    effectControls->addWidget(inspectorTabs_);
    auto* appearance=new QWidget; auto* appearanceControls=new QVBoxLayout(appearance); appearanceControls->setContentsMargins(0,8,0,0); appearanceControls->setSpacing(8);
    auto* motion=new QWidget; auto* motionControls=new QVBoxLayout(motion); motionControls->setContentsMargins(0,8,0,0); motionControls->setSpacing(6);
    auto* maskProperties=new QWidget; auto* maskControls=new QVBoxLayout(maskProperties); maskControls->setContentsMargins(0,8,0,0); maskControls->setSpacing(6);
    maskControls->setSizeConstraint(QLayout::SetMinimumSize);
    {auto* explanation=new QLabel("A mask decides where this effect shows: white shows it, black hides it, gray shows it partly.");
     explanation->setObjectName("maskExplanation");explanation->setWordWrap(true);explanation->setProperty("secondary",true);maskControls->addWidget(explanation);}
    inspectorTabs_->addTab(appearance,"Source"); inspectorTabs_->addTab(motion,"Motion"); inspectorTabs_->addTab(maskProperties,"Mask");
    connect(inspectorTabs_,&QTabWidget::currentChanged,this,[this](int page) {
        if(page==2) {
            // Enter with a working brush rather than inheriting Fill/Move from source editing.
            effectsCanvas_->setTool(LightingCanvas::Tool::Pencil);
            findChild<QPushButton*>("pencilTool")->setChecked(true);
        }
        editingMask_=page==2; if(page!=2) { effectsCanvas_->setCropTool(false); findChild<QCheckBox*>("cropTool")->setChecked(false); }
        refreshProperties();
        // Each task opens at its primary controls; Motion's scroll offset can hide Source after a banner resize.
        for(auto* parent=inspectorTabs_->parentWidget();parent;parent=parent->parentWidget())
            if(auto* scroll=qobject_cast<QScrollArea*>(parent)){scroll->verticalScrollBar()->setValue(0);break;}

    });
    addPanel(effectPanel);
    button(layerStack, "New layer", [this] {
        if (document_.state().layers.size() >= 64) { statusBar()->showMessage("Document limit: 64 layers."); return; }
        editingMask_=false; inspectorTabs_->setCurrentIndex(0);
        selectedLayer_=document_.addPaintLayer(document_.creationParent(selectedLayer_));activeLayer_=document_.layerRoot(selectedLayer_);effectsCanvas_->setTool(LightingCanvas::Tool::Move);effectTarget_->setChecked(true); refresh();
    })->setObjectName("newLayer");
    button(layerStack, "Import image...", [this] {
        const auto file = QFileDialog::getOpenFileName(this, "Import image", QDir::homePath(), "Images (*.png *.jpg *.jpeg *.gif *.bmp *.webp)");
        if (file.isEmpty()) return;
        try { selectedLayer_ = document_.importImage(file,document_.creationParent(selectedLayer_)); editingMask_ = false; refresh(); }
        catch (const std::exception& error) { QMessageBox::warning(this, "Import failed", QString::fromUtf8(error.what())); }
    });
    auto* addMenu=new QMenu("Add layer",layersPanel);addMenu->setObjectName("addLayerMenu");
    // Not added to Edit; effects are added from the layer panel, the add-effect button and Tools > Add effect.
    auto* paintAction=addMenu->addAction("New paint layer");paintAction->setObjectName("newPaintLayer");connect(paintAction,&QAction::triggered,this,[this]{findChild<QPushButton*>("newLayer")->click();});
    auto* groupAction=addMenu->addAction(studioToolIcon("layers"),"New group");groupAction->setObjectName("newGroup");
    groupAction->setToolTip("Create an empty group. Effects added to the selected group share its mask, opacity and blend mode.");
    connect(groupAction,&QAction::triggered,this,[this]{
        if(document_.state().layers.size()>=64){statusBar()->showMessage("Document limit: 64 layers and objects.",6000);return;}
        selectedLayer_=document_.addEmptyLayer();activeLayer_=selectedLayer_;editingMask_=false;
        effectsCanvas_->setTool(LightingCanvas::Tool::Move);effectTarget_->setChecked(true);inspectorTabs_->setCurrentIndex(0);refresh();
    });
    auto* patterns=addMenu->addMenu("Gradients and patterns");auto* shapes=addMenu->addMenu("Shapes");auto* particlesMenu=addMenu->addMenu("Particles");auto* organic=addMenu->addMenu("Organic and noise");auto* scenes=addMenu->addMenu("Scenes");
    for(const auto& definition:effectDefinitions())if(definition.kind!=EffectKind::Paint && definition.kind!=EffectKind::Image && canCreateEffect(definition.kind)) {
        auto* target=definition.kind==EffectKind::Lightning ? organic : isParticle(definition.kind) ? particlesMenu : isShape(definition.kind) ? shapes : (definition.kind==EffectKind::Voronoi || definition.kind==EffectKind::Fireplace || definition.kind==EffectKind::Plasma || definition.kind==EffectKind::Aurora || definition.kind==EffectKind::LavaLamp) ? organic : patterns;
        auto* entry=target->addAction(definition.label);entry->setObjectName("create_"+definition.id);entry->setToolTip(definition.description);entry->setStatusTip(definition.description);
        connect(entry,&QAction::triggered,this,[this,kind=definition.kind] {
            if(document_.state().layers.size()>=64){statusBar()->showMessage("Document limit: 64 layers and objects.",6000);return;}
            // All effect families follow the same explicit group/sibling insertion rule.
            const auto parent=document_.creationParent(selectedLayer_);
            selectedLayer_=document_.addLayer(kind,parent);editingMask_=false;effectsCanvas_->setCropTool(false);
            findChild<QCheckBox*>("cropTool")->setChecked(false);effectsCanvas_->setTool(LightingCanvas::Tool::Move);
            effectTarget_->setChecked(true);inspectorTabs_->setCurrentIndex(0);refresh();
        });
    }
    auto* multizone=scenes->addAction("Multizone (four editable regions)");multizone->setObjectName("create_multizone");
    connect(multizone,&QAction::triggered,this,[this] {
        try {selectedLayer_=document_.addMultizoneLayer();editingMask_=false;effectsCanvas_->setTool(LightingCanvas::Tool::Move);inspectorTabs_->setCurrentIndex(0);refresh();}
        catch(const std::exception& error){statusBar()->showMessage(QString::fromUtf8(error.what()),6000);}
    });
    // Sorted by name inside each category; the definition order read as unsorted (owner 2026-11-17).
    // The same submenus serve the layer panel, the Tools menu and the add-effect button.
    const QList<QMenu*> effectCategories{patterns,shapes,particlesMenu,organic,scenes};
    for(auto* category:effectCategories) {
        auto entries=category->actions();entries.removeIf([](QAction* entry){return entry->isSeparator();});
        std::sort(entries.begin(),entries.end(),[](QAction* a,QAction* b){return QString::localeAwareCompare(a->text(),b->text())<0;});
        for(auto* entry:category->actions())category->removeAction(entry); // removeAction keeps the menu-owned actions alive; clear() would delete them.
        category->addActions(entries);
    }
    auto* imageAction=addMenu->addAction("Import image...");connect(imageAction,&QAction::triggered,this,[layersPanel]{for(auto* button:layersPanel->findChildren<QPushButton*>())if(button->text()=="Import image..."){button->click();break;}});
    auto* addButton=new QToolButton;addButton->setText("New layer");addButton->setObjectName("addLayer");connect(addButton,&QToolButton::clicked,this,[this]{findChild<QPushButton*>("newLayer")->click();});addButton->setToolTip("Create a transparent paint layer. Select a group to create it inside that group.");
    auto* groupButton=new QToolButton;groupButton->setDefaultAction(groupAction);groupButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);groupButton->setObjectName("addGroup");
    auto* newRow=new QWidget;auto* newLayout=new QHBoxLayout(newRow);newLayout->setContentsMargins(0,0,0,0);newLayout->setSpacing(6);newLayout->addWidget(addButton,1);newLayout->addWidget(groupButton,1);layerStack->insertWidget(0,newRow);
    findChild<QPushButton*>("newLayer")->hide();for(auto* button:layersPanel->findChildren<QPushButton*>())if(button->text()=="Import image...")button->hide();
    auto* tree = new LayerTree;
    layers_ = tree;
    layers_->setObjectName("effectLayers");
    layers_->setMinimumHeight(180);
    layers_->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    layers_->setHeaderLabels({"Layer / object", "Source", "Mask"});
    layers_->setRootIsDecorated(true);
    layers_->setItemDelegate(new LayerDelegate(layers_));
    layers_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    layers_->setSelectionBehavior(QAbstractItemView::SelectRows);
    layers_->setDragDropMode(QAbstractItemView::InternalMove);
    layers_->setDefaultDropAction(Qt::MoveAction);
    layers_->setIconSize({36, 26});layers_->setIndentation(14);
    layers_->header()->setStretchLastSection(false);
    layers_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    layers_->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    layers_->header()->setSectionResizeMode(2, QHeaderView::Fixed);
    layers_->setColumnWidth(1, 52); layers_->setColumnWidth(2, 44);
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
    connect(layers_,&QTreeWidget::itemSelectionChanged,this,[this]{
        QSet<QUuid> ids;for(auto* item:layers_->selectedItems())ids.insert(item->data(0,Qt::UserRole).toUuid());
        const auto primary=layers_->currentItem() ? layers_->currentItem()->data(0,Qt::UserRole).toUuid() : QUuid{};
        QMetaObject::invokeMethod(this,[this,ids,primary]{if(!ids.isEmpty() && effectsCanvas_->selection()==primary)effectsCanvas_->selectMany(ids,primary);},Qt::QueuedConnection);
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
        QVector<QUuid> order;QMap<QUuid,QUuid> parents;
        for(int i=layers_->topLevelItemCount()-1;i>=0;--i) {
            auto* root=layers_->topLevelItem(i);const auto id=root->data(0,Qt::UserRole).toUuid();order.append(id);
            for(int j=root->childCount()-1;j>=0;--j){auto* child=root->child(j);if(child->childCount()){statusBar()->showMessage("Nested layers are not supported.",6000);refresh();return;}const auto object=child->data(0,Qt::UserRole).toUuid();for(const auto& layer:document_.state().layers)if(layer.id==object && layer.container){statusBar()->showMessage("Layer containers cannot be nested.",6000);refresh();return;}order.append(object);parents[object]=id;}
        }
        if(order.size()!=document_.state().layers.size()){statusBar()->showMessage("Cannot reorder an incomplete layer tree.",6000);refresh();return;}
        document_.edit("Reorder layers", [&](LightingState& state) {
            auto before = state.layers;
            for (int i = 0; i < order.size(); ++i)
                for (auto layer : before) if (layer.id == order[i]) {layer.parentId=parents.value(layer.id);state.layers[i] = layer;}
        });
    };
    auto* orderRow=new QHBoxLayout;layerStack->addLayout(orderRow);
    for(const auto& entry:QVector<QPair<QString,int>>{{"Move up",-1},{"Move down",1}}) {
        auto* action=new QAction(entry.first,this);action->setObjectName(entry.second<0 ? "moveLayerUp" : "moveLayerDown");
        action->setShortcut(QKeySequence(Qt::CTRL | (entry.second<0 ? Qt::Key_PageUp : Qt::Key_PageDown)));addAction(action);
        auto* control=new QPushButton(entry.first);control->setObjectName(action->objectName()+"Button");control->setToolTip(action->text()+" ("+action->shortcut().toString()+")");orderRow->addWidget(control);
        connect(action,&QAction::triggered,this,[this,direction=entry.second] {moveSelectedLayer(direction);});
        connect(control,&QPushButton::clicked,action,&QAction::trigger);connect(action,&QAction::changed,control,[action,control]{control->setEnabled(action->isEnabled());});
    }
    button(layerStack,"Duplicate selection",[this]{effectsCanvas_->duplicateSelection();})->setObjectName("duplicateLayer");
    button(layerStack,"Delete selection",[this]{effectsCanvas_->deleteSelection();})->setObjectName("deleteLayer");
    auto* createActions=new QHBoxLayout; auto* manageActions=new QHBoxLayout;
    for(auto* control:layersPanel->findChildren<QPushButton*>(QString{},Qt::FindDirectChildrenOnly)) {

        if(control->text()=="Duplicate selection" || control->text()=="Delete selection") { layerStack->removeWidget(control); manageActions->addWidget(control); }
    }
    delete createActions; layerStack->addLayout(manageActions);
    auto* form = new QFormLayout;
    appearanceControls->addLayout(form);
    transparency_ = new QSlider(Qt::Horizontal);
    transparency_->setRange(0, 100);
    transparency_->setObjectName("layerTransparency");
    transparency_->setAccessibleName("Layer opacity percent");
    auto* transparencyValue = new QSpinBox;transparencyValue->setRange(0,100);transparencyValue->setValue(100);transparencyValue->setKeyboardTracking(false);transparencyValue->setAccessibleName("Layer opacity");
    transparencyValue->setObjectName("transparencyValue");
    auto* opacityRow=new QWidget;auto* opacityLayout=new QHBoxLayout(opacityRow);opacityLayout->setContentsMargins(0,0,0,0);opacityLayout->addWidget(transparency_,1);opacityLayout->addWidget(transparencyValue);form->addRow("Opacity",opacityRow);
    connect(transparencyValue,&QSpinBox::valueChanged,transparency_,&QSlider::setValue);
    connect(transparency_, &QSlider::valueChanged, this, [this, transparencyValue](int value) {
        {const QSignalBlocker block(transparencyValue);transparencyValue->setValue(value);}
        updateLayer("Layer transparency", [value](auto& layer) { layer.transparency = 100-value; });
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
    sourceColor_=new ColorEditor;sourceColor_->setObjectName("sourceColor");sourceColor_->setAccessibleName("Primary source color");appearanceControls->addWidget(sourceColor_);
    connect(sourceColor_,&ColorEditor::colorChanged,this,[this](QColor c){updateLayer("Source color",[=](auto& layer){layer.color=c;});});
    secondaryColor_=new ColorEditor; secondaryColor_->setObjectName("secondaryEffectColor"); appearanceControls->addWidget(secondaryColor_);
    connect(secondaryColor_,&ColorEditor::colorChanged,this,[this](QColor color) { updateLayer("Secondary color",[color](auto& layer) { layer.secondaryColor=color; }); });
    auto* paletteLabel=new QLabel("Particle colors");paletteLabel->setObjectName("particlePaletteLabel");appearanceControls->addWidget(paletteLabel);
    gradientColors_=new ColorListEditor(2,16); gradientColors_->setGradientMode(); gradientColors_->setObjectName("gradientColors"); appearanceControls->addWidget(gradientColors_);
    connect(gradientColors_,&ColorListEditor::stopsChanged,this,[this] {
        const auto colors=gradientColors_->colors(); const auto positions=gradientColors_->positions();
        updateLayer("Source color stops",[colors,positions](auto& layer) { layer.gradientColors=colors; layer.gradientPositions=positions; layer.color=colors.first(); layer.secondaryColor=colors.last(); });
    });
    connect(gradientColors_,&ColorListEditor::gestureStarted,this,[this] { document_.history()->beginMacro("Source color stop"); });
    connect(gradientColors_,&ColorListEditor::gestureFinished,this,[this] { document_.history()->endMacro(); });
    const auto dataDirectory=libraryPath.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
        : QFileInfo(libraryPath).absolutePath();
    const auto effectDirectory=dataDirectory+"/Custom effects";
    auto* savePreset=button(appearanceControls,"Save as custom effect",[this,effectDirectory] {
        try {
            if (!QDir().mkpath(effectDirectory)) throw std::runtime_error("Cannot create the custom effects library");
            const auto path=QFileDialog::getSaveFileName(this,"Save custom effect",effectDirectory+"/Untitled.effect.json","Custom effects (*.effect.json)");
            if (path.isEmpty()) return;
            document_.saveCustomEffect(selectedLayer_,path.endsWith(".effect.json",Qt::CaseInsensitive) ? path : path+".effect.json");
        } catch (const std::exception& error) { QMessageBox::warning(this,"Effect save failed",QString::fromUtf8(error.what())); }
    });
    savePreset->setObjectName("saveCustomEffect");
    auto* customEffects=button(layerStack,"Custom effects",[this,effectDirectory] {
        const auto path=QFileDialog::getOpenFileName(this,"Custom effects",effectDirectory,"Custom effects (*.effect.json)");
        if (path.isEmpty()) return;
        try {
            selectedLayer_=document_.insertCustomEffect(path);
            editingMask_=false;
            effectsCanvas_->setTool(LightingCanvas::Tool::Move);
            inspectorTabs_->setCurrentIndex(0);
            refresh();
        } catch (const std::exception& error) { QMessageBox::warning(this,"Effect insert failed",QString::fromUtf8(error.what())); }
    });
    customEffects->setObjectName("customEffects");
    // The effect library sits at the top beside New layer / Add effect, like the component library
    // at the top of Devices (workspace brief 2026-11-26); it used to hide below the layer list.
    layerStack->removeWidget(customEffects); layerStack->insertWidget(1,customEffects);
    auto* description=new QLabel;description->setObjectName("effectDescription");description->setWordWrap(true);description->setProperty("muted",true);appearanceControls->insertWidget(0,description);
    auto* intensity=new QWidget;auto* intensityForm=new QFormLayout(intensity);intensityForm->setContentsMargins(0,0,0,0);
    auto* intensityMode=new QComboBox;intensityMode->setObjectName("intensityMode");intensityMode->addItems({"Off","Pulse","Breathe","Combined"});intensityForm->addRow("Mode",intensityMode);
    connect(intensityMode,&QComboBox::currentIndexChanged,this,[this](int mode){updateMotion("Intensity modulation",[=](auto& layer){layer.pulse=(mode==1 || mode==3) ? (layer.pulse>0 ? layer.pulse : .5) : 0;layer.breathe=(mode==2 || mode==3) ? (layer.breathe>0 ? layer.breathe : .5) : 0;});});
    auto* hueGroup=new QWidget;auto* hueForm=new QFormLayout(hueGroup);hueForm->setContentsMargins(0,0,0,0);
    animationControls_ = new QWidget;
    auto* animationForm = new QFormLayout(animationControls_);
    animationForm->setContentsMargins(0, 0, 0, 0);
    const QStringList animationLabels{"Object rotation", "Pulse rate", "Hue rotation"};
    for (int i = 0; i < 3; ++i) {
        auto* value = new ParameterSpinBox;
        value->setObjectName("animationRate" + QString::number(i));
        value->setRange(i == 1 ? 0 : -360, i == 1 ? 10 : 360);
        value->setKeyboardTracking(false); animation_.append(value);
        value->setAccessibleName(animationLabels[i]);
        if(i==1)value->setProperty("slowRange",true);
        value->setDisplayUnit(i==1 ? 60 : 1,i==1 ? " /min" : " deg/s");
        (i==1 ? intensityForm : i==2 ? hueForm : animationForm)->addRow(animationLabels[i],sliderField(value));
        connect(value, &QDoubleSpinBox::valueChanged, this, [this,i](double number) {
            updateMotion("Animation rate",[=](auto& layer) { if(i==0)layer.spin=number;else if(i==1)layer.pulse=number;else layer.hueShift=number; });
        });
    }
    motionControls->addWidget(animationControls_);
    auto* applyMotion=new QPushButton("Apply motion to all visible objects");applyMotion->setObjectName("applyMotionToAll");motionControls->addWidget(applyMotion);
    connect(applyMotion,&QPushButton::clicked,this,[this]{
        const auto found=std::find_if(document_.state().layers.begin(),document_.state().layers.end(),[this](const auto& layer){return layer.id==selectedLayer_;});if(found==document_.state().layers.end())return;const auto source=*found;
        document_.edit("Copy motion to visible layers",[=](auto& state){for(auto& layer:state.layers)if(layer.visible && !layer.container && layer.id!=source.id){layer.spin=source.spin;layer.pulse=source.pulse;layer.breathe=source.breathe;layer.hueShift=source.hueShift;layer.driftX=source.driftX;layer.driftY=source.driftY;layer.zoom=source.zoom;layer.warp=source.warp;layer.warpY=source.warpY;layer.trails=source.trails;layer.cycleRate=source.cycleRate;layer.colorCycle=source.colorCycle;layer.colorCyclePositions=source.colorCyclePositions;layer.patternSpeed=source.patternSpeed;}});
    });
    disclosure(motionControls,"Intensity modulation",intensity);
    auto* particles=new QWidget; particles->setObjectName("particleParameters"); auto* particleForm=new QFormLayout(particles); particleForm->setContentsMargins(0,0,0,0);
    // Keep particle rate/count/size above palette details when artwork reduces inspector height.
    appearanceControls->insertWidget(1,particles);
    auto* patternControls=new QWidget; patternControls->setObjectName("patternParameters"); auto* patternForm=new QFormLayout(patternControls); patternForm->setContentsMargins(0,0,0,0);
    appearanceControls->insertWidget(1,patternControls);
    auto* advancedContent=new QWidget; auto* advancedForm=new QFormLayout(advancedContent); advancedForm->setContentsMargins(0,0,0,0);
    auto* chaseControls=new QWidget;
    chaseControls->setObjectName("chaseParameters");
    auto* chaseForm=new QFormLayout(chaseControls);
    appearanceControls->insertWidget(2,chaseControls);
    for(const auto& parameter:advancedParameters()) {
        auto* value=new ParameterSpinBox; value->setObjectName(parameter.id); value->setRange(parameter.minimum,parameter.maximum);
         value->setKeyboardTracking(false); advanced_.append(value);
        const auto id=parameter.id;value->setProperty("defaultValue",EffectLayer{}.*parameter.member);
        value->setDisplayUnit(id=="trails" ? 1000 : id=="breathe" || id=="cycleRate" || id=="patternSpeed" ? 60 : id=="particleSpeed" ? .3 : 1,
            id=="trails" ? " ms" : id=="breathe" || id=="cycleRate" || id=="particleSpeed" || id=="patternSpeed" ? " /min" : id=="driftX" || id=="driftY" ? " px/s" : id=="zoom" ? " %" : id=="warp" || id=="particleSize" ? " px" : "");
        if(parameter.id=="breathe" || parameter.id=="cycleRate")value->setProperty("slowRange",true);
        const bool primary=parameter.id=="particleCount" || parameter.id=="particleSpeed" || parameter.id=="particleSize";
        if (id.startsWith("chase")) {
            value->setDecimals(id=="chaseSpeed" ? 1 : 0);
            if (id=="chaseReverse") {
                auto* reverse=new QCheckBox("Reverse direction");
                reverse->setObjectName("chaseReverseToggle");
                reverse->setToolTip("Start at the last address and travel toward the first.");
                chaseForm->addRow(reverse);
                value->setParent(chaseControls);
                value->hide();
                connect(reverse,&QCheckBox::toggled,value,[value](bool on){value->setValue(on);});
                connect(value,&QDoubleSpinBox::valueChanged,reverse,[reverse](double on) {
                    const QSignalBlocker block(reverse);
                    reverse->setChecked(on!=0);
                });
            } else chaseForm->addRow(parameter.label,value);
        } else if(id.startsWith("pattern") || id=="fillOrder") {
            auto* label=new QLabel(parameter.label); value->setProperty("parameterLabel",QVariant::fromValue(static_cast<QObject*>(label)));
            if(id=="patternWidth" || id=="patternAmplitude" || id=="patternHeight" || id=="patternWarp" || id=="patternWavelength" || id=="patternFrequency" || id=="patternContrast") value->setDisplayUnit(100," %");
            QWidget* field=nullptr;
            if(id=="fillOrder" || id=="patternDistance") {
                auto* order=new QComboBox;order->setObjectName(id=="fillOrder" ? "patternFillOrder" : "patternDistanceMode");
                order->addItems(id=="fillOrder" ? QStringList{"Rows","Snake","Spiral"} : QStringList{"Euclidean (straight-line)","Manhattan (diamond)","Chebyshev (square)"});
                // Keep the shared parameter/history path while exposing a named order selector.
                value->setParent(patternControls);value->hide();
                connect(order,&QComboBox::currentIndexChanged,value,[value](int n){value->setValue(n);});
                connect(value,&QDoubleSpinBox::valueChanged,order,[order](double n){const QSignalBlocker block(order);order->setCurrentIndex(int(n));});
                field=order;
            } else if(id=="patternSeed") {
                field=new QWidget;auto* row=new QHBoxLayout(field);row->setContentsMargins(0,0,0,0);
                value->setDecimals(0);row->addWidget(value);auto* random=new QPushButton("Randomize");random->setObjectName("randomizePatternSeed");row->addWidget(random);
                connect(random,&QPushButton::clicked,value,[value]{value->setValue(QRandomGenerator::global()->bounded(1000001));});
            } else {
                if(id=="patternCells" || id=="patternSites") {value->setDecimals(0);value->setSingleStep(1);}
                field=sliderField(value);
            }
            value->setProperty("visibilityContainer",QVariant::fromValue(static_cast<QObject*>(field))); patternForm->addRow(label,field);
        }
        else if(parameter.id=="breathe") {auto* row=sliderField(value);intensityForm->addRow("Breathe rate",row);value->setProperty("visibilityContainer",QVariant::fromValue(static_cast<QObject*>(row)));}
        else if(parameter.id=="particleSeed") {auto* row=new QWidget;auto* layout=new QHBoxLayout(row);layout->setContentsMargins(0,0,0,0);layout->addWidget(value);auto* random=new QPushButton("Randomize");random->setObjectName("randomizeParticleSeed");layout->addWidget(random);connect(random,&QPushButton::clicked,this,[value]{value->setValue(QRandomGenerator::global()->bounded(1000001));});particleForm->addRow("Random pattern",row);value->setProperty("visibilityContainer",QVariant::fromValue(static_cast<QObject*>(row)));}
        else if(primary) {
            auto* field=sliderField(value); value->setProperty("visibilityContainer",QVariant::fromValue(static_cast<QObject*>(field)));
            auto* label=new QLabel(parameter.label); value->setProperty("parameterLabel",QVariant::fromValue(static_cast<QObject*>(label))); particleForm->addRow(label,field);
        } else if(parameter.id!="cycleRate") {
            auto* label=new QLabel(parameter.label); label->setWordWrap(true); value->setProperty("parameterLabel",QVariant::fromValue(static_cast<QObject*>(label)));
            auto* field=sliderField(value); value->setProperty("visibilityContainer",QVariant::fromValue(static_cast<QObject*>(field)));
            advancedForm->addRow(label,field);
        }
        connect(value,&QDoubleSpinBox::valueChanged,this,[this,id,member=parameter.member](double number) { updateMotion("Effect parameter",[=](auto& layer) { layer.*member=number; });if(id=="cycleRate" && number>0 && !editingMask_)playing_->setChecked(true); });
    }
    auto* details=disclosure(motionControls,"More animation settings",advancedContent); details->setObjectName("advancedMotion");
    colorCycle_=new ColorListEditor(2,16); colorCycle_->setObjectName("colorCycle");
    colorCycle_->setGradientMode();
    auto* cycleContent=new QWidget;cycleContent->setObjectName("colorAnimationControls");auto* cycleControls=new QVBoxLayout(cycleContent);cycleControls->setContentsMargins(0,0,0,0);cycleControls->setSpacing(6);cycleControls->addWidget(hueGroup);
    auto* cycleHint=new QLabel("Palette rate: 0 = off. Hue rotation is independent.");
    cycleHint->setWordWrap(true); cycleControls->addWidget(cycleHint);
    auto* cycleForm=new QFormLayout; cycleControls->addLayout(cycleForm);
    for(auto* value:advanced_) if(value->objectName()=="cycleRate") cycleForm->addRow("Palette rate",sliderField(value));
    cycleControls->addWidget(colorCycle_);
    disclosure(motionControls,"Color animation",cycleContent)->setObjectName("colorAnimationSection");
    connect(colorCycle_,&ColorListEditor::stopsChanged,this,[this] {
        const auto colors=colorCycle_->colors(); const auto positions=colorCycle_->positions();
        updateMotion("Cycle stops",[colors,positions](auto& layer) { layer.colorCycle=colors; layer.colorCyclePositions=positions; });
    });
    connect(colorCycle_,&ColorListEditor::gestureStarted,this,[this] { document_.history()->beginMacro("Cycle color"); });
    connect(colorCycle_,&ColorListEditor::gestureFinished,this,[this] { document_.history()->endMacro(); });
    motionControls->addStretch();
    auto* maskScope=new QLabel;maskScope->setObjectName("maskScopeSummary");maskScope->setWordWrap(true);maskControls->addWidget(maskScope);
    maskLinked_ = new QCheckBox("Follow effect movement"); maskLinked_->setObjectName("maskLinked");
    maskControls->addWidget(maskLinked_);
    connect(maskLinked_,&QCheckBox::toggled,this,[this](bool linked) {
        updateLayer("Link mask",[=](auto& layer) { layer.maskLinked = linked; });
    });
    auto* clipToFrame=new QCheckBox("Limit source to its frame");clipToFrame->setObjectName("clipToFrame");
    clipToFrame->setToolTip("Draw a pattern only inside its frame. Off: the pattern fills the canvas, so rotation never shows edges.");
    appearanceControls->addWidget(clipToFrame);
    connect(clipToFrame,&QCheckBox::toggled,this,[this](bool clip) { updateLayer("Limit to frame",[=](auto& layer) { layer.clipToFrame = clip; }); });
    cropControls_ = new QWidget; auto* cropForm = new QGridLayout(cropControls_);
    cropForm->setContentsMargins(0,0,0,0);
    const QStringList cropLabels{"Crop left", "Crop top", "Crop width", "Crop height"};
    for (int i=0;i<4;++i) {
        auto* value = new ParameterSpinBox; value->setObjectName("imageCrop"+QString::number(i));
        value->setRange(i<2 ? 0 : 0.001,1);  value->setKeyboardTracking(false);
        crop_.append(value);
        auto* label=new QLabel(cropLabels[i]); label->setWordWrap(true); value->setProperty("parameterLabel",QVariant::fromValue(static_cast<QObject*>(label)));
        cropForm->addWidget(label,i/2,(i%2)*2); cropForm->addWidget(value,i/2,(i%2)*2+1);
        connect(value,&QDoubleSpinBox::valueChanged,this,[this,i](double number) {
            updateLayer("Image crop",[=](auto& layer) {auto r=layer.crop;if(i==0)r.moveLeft(number);if(i==1)r.moveTop(number);if(i==2)r.setWidth(number);if(i==3)r.setHeight(number);layer.crop=r;});refreshProperties();
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
        auto* value = new ParameterSpinBox;
        value->setObjectName("effectGeometry" + QString::number(i));

        value->setKeyboardTracking(false);
        value->setRange(i < 2 ? -10000 : i == 4 ? -360 : 1, i == 4 ? 360 : 10000);
        geometry_.append(value);
        auto* label=new QLabel(geometryLabels[i]); label->setWordWrap(true);
        value->setProperty("parameterLabel",QVariant::fromValue(static_cast<QObject*>(label)));
        geometryForm->addWidget(label,i/2,(i%2)*2); geometryForm->addWidget(value,i/2,(i%2)*2+1);
        connect(value,&QDoubleSpinBox::valueChanged,this,[this,i](double number) {
            updateLayer("Effect geometry",[=](auto& layer) {if(i==0)layer.center.setX(number);if(i==1)layer.center.setY(number);if(i==2)layer.extent.setWidth(number);if(i==3)layer.extent.setHeight(number);if(i==4)layer.rotation=number;});
        });
    }
    auto* geometrySection=disclosure(appearanceControls,"Position and size (optional)",geometryControls_);
    geometrySection->setObjectName("geometrySection");
    geometrySection->findChild<QToolButton*>()->setChecked(false);
    repetitions_ = new ParameterSpinBox;
    repetitions_->setObjectName("effectRepetitions");
    repetitions_->setRange(1, 64);
    repetitions_->setDecimals(0);repetitions_->setSingleStep(1);repetitions_->setAccessibleName("Repetitions");
    repetitions_->setProperty("defaultValue",4);
    auto* repetitionControls=new QWidget;repetitionControls->setObjectName("effectRepetitionControls");
    auto* repetitionForm=new QFormLayout(repetitionControls);repetitionForm->setContentsMargins(0,0,0,0);
    repetitionForm->addRow("Repetitions",sliderField(repetitions_));appearanceControls->addWidget(repetitionControls);
    connect(repetitions_, &QDoubleSpinBox::valueChanged, this, [this](double value) {
        updateLayer("Effect repetitions", [value](auto& layer) { layer.repetitions = int(value); });
    });
    auto* shapeFillMode=new QComboBox;shapeFillMode->setObjectName("shapeFillMode");shapeFillMode->addItems({"Solid fill","Gradient fill"});
    shapeFillMode->setAccessibleName("Shape fill");appearanceControls->insertWidget(0,shapeFillMode);
    connect(shapeFillMode,&QComboBox::currentIndexChanged,this,[this](int index){updateLayer("Shape fill",[index](auto& layer){if(isShape(layer.effect))layer.gradientFill=index==1;});});
    auto* tools=new QToolBar("Drawing tools",this); tools->setObjectName("effectTools"); tools->setMovable(false);
    tools->setAllowedAreas(Qt::LeftToolBarArea); addToolBar(Qt::LeftToolBarArea,tools);
    auto* toolGroup=new QButtonGroup(this); toolGroup->setExclusive(true);toolGroup->setObjectName("drawingToolGroup");
    const QStringList toolNames{"Move", "Pencil", "Line", "Rectangle", "Ellipse", "Gradient", "Fill", "Eraser"};
    const QList<int> keys{Qt::Key_V,Qt::Key_B,Qt::Key_I,Qt::Key_U,Qt::Key_O,Qt::Key_G,Qt::Key_F,Qt::Key_E};
    for(int index=0;index<toolNames.size();++index) {
        auto* tool=new QPushButton; tool->setObjectName(index==0 ? "moveEffectTool" : toolNames[index].toLower()+"Tool"); tool->setFixedSize(32,32);
        tool->setIcon(studioToolIcon(toolNames[index].toLower())); tool->setCheckable(true); tool->setShortcut(QKeySequence(keys[index]));
        const QStringList descriptions{"Move the selected object. Drag corners to resize and the round handle to rotate.","Draw on the active paint object. Other sources receive a new paint object in the active layer.","Drag from the start to the end of a straight line.","Drag a rectangle. Use Filled shapes for a solid shape.","Drag an ellipse. Use Filled shapes for a solid shape.","Drag to set a gradient. Afterwards drag either arrow endpoint to change its length and direction.","Fill a connected area with the foreground color. On a procedural or image source, place that region on a paint object in the active layer.","Erase paint pixels. On a procedural effect, erase its mask so the effect stays editable."};
        tool->setToolTip(toolNames[index]+" ("+QChar(keys[index])+")\n"+descriptions[index]); tool->setAccessibleName(toolNames[index]); tools->addWidget(tool); toolGroup->addButton(tool,index);
        if(index==0) { effectTarget_=tool; tool->setChecked(true); }
        connect(tool,&QPushButton::clicked,this,[this,index] { layoutCanvas_->setTool(LightingCanvas::Tool::Move);if(tabs_->currentIndex()==0){layoutCanvas_->setFocus();refreshToolOptions();return;}effectsCanvas_->setCropTool(false); findChild<QCheckBox*>("cropTool")->setChecked(false); effectsCanvas_->setTool(static_cast<LightingCanvas::Tool>(index)); refreshProperties(); });
    }
    auto* hand=new QToolButton;hand->setObjectName("panCanvasTool");hand->setIcon(studioToolIcon("pan"));hand->setFixedSize(32,32);hand->setCheckable(true);toolGroup->addButton(hand,int(LightingCanvas::Tool::Pan));hand->setAccessibleName("Pan canvas");hand->setToolTip("Hand (H): drag to move the view without moving objects. Hold Space or drag with the middle mouse button from any tool.");
    connect(hand,&QToolButton::clicked,this,[this]{effectsCanvas_->setTool(LightingCanvas::Tool::Pan);layoutCanvas_->setTool(LightingCanvas::Tool::Pan);refreshProperties();});tools->addWidget(hand);
    auto* panAction=new QAction("Pan canvas",this);panAction->setObjectName("panCanvasAction");panAction->setShortcut(QKeySequence(Qt::Key_H));addAction(panAction);connect(panAction,&QAction::triggered,hand,&QToolButton::click);
    tools->addSeparator();
    auto* sources=new QToolButton;sources->setObjectName("sourcesTool");sources->setIcon(studioToolIcon("add"));sources->setAccessibleName("Add effect or image");
    sources->setToolTip("Add effect or image: search the library. A selected group receives the new effect; a selected child receives a sibling. Otherwise a separate layer is created.");sources->setPopupMode(QToolButton::InstantPopup);
    // A sparkle instead of a bare "+" with a dropdown arrow (owner report 2026-11-17). It stays
    // icon-sized: the vertical tool strip beside the canvas must not widen.
    sources->setText("Add effect");sources->setIcon(studioToolIcon("add-effect"));sources->setFixedSize(32,32);
    sources->setStyleSheet("QToolButton::menu-indicator{image:none;width:0px;}");
    auto* sourceMenu=new QMenu(sources);sourceMenu->setObjectName("effectLibraryMenu");sources->setMenu(sourceMenu);
    auto* generatorPopupBody=new QWidget;auto* generatorPopupLayout=new QVBoxLayout(generatorPopupBody);generatorPopupLayout->setContentsMargins(8,8,8,8);
    auto* search=new QLineEdit;search->setObjectName("effectLibrarySearch");search->setPlaceholderText("Search effects and images...");search->setAccessibleName("Search effect library");search->installEventFilter(this);generatorPopupLayout->addWidget(search);
    auto* results=new QListWidget;results->setObjectName("effectLibraryResults");results->setFixedSize(300,220);generatorPopupLayout->addWidget(results);
    results->setMouseTracking(true);
    auto* swatch=new QLabel(this,Qt::ToolTip | Qt::WindowDoesNotAcceptFocus);
    swatch->setObjectName("effectLibraryPreview");
    swatch->setAttribute(Qt::WA_ShowWithoutActivating);
    swatch->setAttribute(Qt::WA_TransparentForMouseEvents);
    swatch->setFixedSize(252,132);
    swatch->setStyleSheet(QString("background:%1; border:1px solid %2; border-radius:4px;")
        .arg(studioThemeColor("@panel").name(),studioThemeColor("@border").name()));
    swatch->setAlignment(Qt::AlignCenter);
    swatch->setAccessibleName("Animated preview of the highlighted effect");
    auto* previewEffect=new QTimer(sourceMenu);
    previewEffect->setInterval(180);
    auto* candidate=new LightingDocument(document_.installedCatalog(),sourceMenu,document_.profiles());
    const auto updateSwatch=[candidate,swatch] {
        try {
            const auto seconds=double(QDateTime::currentMSecsSinceEpoch()%60000)/1000;
            const auto frame=candidate->render(seconds);
            if (candidate->state().placements.isEmpty()) {
                swatch->setPixmap(QPixmap::fromImage(frame.scaled(240,110,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
                return;
            }
            auto image=frame.scaled(240,120,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
            QPainter painter(&image);
            painter.setRenderHint(QPainter::Antialiasing);
            const auto bounds=candidate->layoutBounds();
            const auto colors=candidate->previewComponentColors(frame,seconds);
            painter.setPen(QColor("#70839a"));
            for (const auto& placement:candidate->state().placements) {
                const auto& definition=candidate->catalog()[placement.componentId];
                const auto points=candidate->componentPoints(placement);
                const auto output=colors.value(placement.id);
                for (int i=0;i<points.size();++i) {
                    if (definition.nonPhysicalSlots.contains(definition.mapping.at(i))) continue;
                    painter.setBrush(output.value(definition.mapping.at(i),Qt::black));
                    painter.drawEllipse(QPointF((points[i].x()-bounds.left())*image.width()/bounds.width(),
                        (points[i].y()-bounds.top())*image.height()/bounds.height()),2.5,2.5);
                }
            }
            painter.end();
            swatch->setPixmap(QPixmap::fromImage(image));
        } catch (const std::exception& error) { swatch->setText(QString::fromUtf8(error.what())); }
    };
    const auto clearSwatch=[swatch,previewEffect] {
        previewEffect->stop();
        swatch->hide();
        swatch->clear();
    };
    const auto showSwatch=[this,candidate,swatch,previewEffect,clearSwatch,updateSwatch](QMenu* menu,const QRect& row,const QString& key) {
        if (!menu->isVisible() || !key.startsWith("create_")) { clearSwatch(); return; }
        try {
            candidate->edit("Preview",[&](LightingState& state) {
                state=document_.state();
                state.layers.clear();
                if(key!="create_multizone") state.layers={LightingDocument::effectDefaults(effectDefinition(key.mid(7)).kind)};
            });
            if (key=="create_multizone") candidate->addMultizoneLayer();
            candidate->history()->clear();
        } catch (const std::exception&) { clearSwatch(); return; }
        const QRect bounds=menu->screen()->availableGeometry();
        const QRect menuRect(menu->mapToGlobal(QPoint{}),menu->size());
        const int right=menuRect.right()+8;
        const int left=menuRect.left()-swatch->width()-8;
        const int x=right+swatch->width()<=bounds.right() ? right : left;
        swatch->move(qBound(bounds.left(),x,qMax(bounds.left(),bounds.right()-swatch->width()+1)),
            qBound(bounds.top(),menu->mapToGlobal(row.topLeft()).y(),qMax(bounds.top(),bounds.bottom()-swatch->height()+1)));
        updateSwatch();
        swatch->show();
        previewEffect->start();
    };
    connect(previewEffect,&QTimer::timeout,this,updateSwatch);
    connect(sourceMenu,&QMenu::aboutToHide,this,clearSwatch);
    for (auto* category:effectCategories) {
        connect(category,&QMenu::aboutToHide,this,clearSwatch);
        connect(category,&QMenu::hovered,this,[category,showSwatch](QAction* entry) {
            showSwatch(category,category->actionGeometry(entry),entry->isEnabled() ? entry->objectName() : QString{});
        });
    }
    connect(results,&QListWidget::itemEntered,results,[results](QListWidgetItem* item) {
        if (item->flags()&Qt::ItemIsEnabled) results->setCurrentItem(item);
    });
    connect(results,&QListWidget::currentItemChanged,this,[results,sourceMenu,showSwatch,clearSwatch](QListWidgetItem* item) {
        if (!item || !results->isVisible()) { clearSwatch(); return; }
        auto row=results->visualItemRect(item);
        row.moveTopLeft(results->viewport()->mapTo(sourceMenu,row.topLeft()));
        showSwatch(sourceMenu,row,item->data(Qt::UserRole).toString());
    });
    // Browsing uses the category submenus like the menu bar; the flat result list appears only while
    // searching. Bold headings inside one list were not clearer (owner review 2026-11-17).
    for(auto* category:effectCategories)for(auto* entry:category->actions()) {
        auto* item=new QListWidgetItem(entry->text(),results);item->setData(Qt::UserRole,entry->objectName());item->setToolTip(entry->toolTip());
    }
    imageAction->setObjectName("importImageAction");auto* imageItem=new QListWidgetItem("Import image...",results);imageItem->setData(Qt::UserRole,imageAction->objectName());
    auto* libraryAction=new QWidgetAction(sourceMenu);libraryAction->setDefaultWidget(generatorPopupBody);sourceMenu->addAction(libraryAction);
    auto* categorySeparator=sourceMenu->addSeparator();
    for(auto* category:effectCategories)sourceMenu->addMenu(category);
    auto* importSeparator=sourceMenu->addSeparator();sourceMenu->addAction(imageAction);
    const auto filterLibrary=[this,results,libraryAction,effectCategories,categorySeparator,importSeparator,imageAction](const QString& query){
        const auto text=query.trimmed();
        const bool searching=!text.isEmpty();
        results->setVisible(searching);
        for(auto* category:effectCategories)category->menuAction()->setVisible(!searching);
        for(auto* action:{categorySeparator,importSeparator,imageAction})action->setVisible(!searching);
        QListWidgetItem* first=nullptr;
        for(int i=0;i<results->count();++i){auto* item=results->item(i);
            auto* action=findChild<QAction*>(item->data(Qt::UserRole).toString());
            item->setHidden(!searching || !item->text().contains(text,Qt::CaseInsensitive));
            item->setFlags(action && action->isEnabled() ? Qt::ItemIsEnabled|Qt::ItemIsSelectable : Qt::NoItemFlags);
            if(action && !action->isEnabled())item->setToolTip("Unlock the selected layer or group before adding an effect.");
            if(!item->isHidden() && !first && (item->flags()&Qt::ItemIsEnabled))first=item;
        }
        results->setCurrentItem(first);
        libraryAction->setData(text); // An action change makes the open menu recompute its size.
    };
    connect(search,&QLineEdit::textChanged,results,filterLibrary);
    const auto activate=[this,sourceMenu](QListWidgetItem* item){if(!item || item->isHidden() || !(item->flags()&Qt::ItemIsEnabled) || item->data(Qt::UserRole).toString().isEmpty())return;sourceMenu->hide();if(auto* action=findChild<QAction*>(item->data(Qt::UserRole).toString()))action->trigger();};
    connect(results,&QListWidget::itemClicked,this,activate);connect(results,&QListWidget::itemActivated,this,activate);
    connect(search,&QLineEdit::returnPressed,this,[results,activate]{activate(results->currentItem());});
    connect(sourceMenu,&QMenu::aboutToShow,this,[search,filterLibrary]{search->clear();filterLibrary({});QTimer::singleShot(0,search,[search]{search->setFocus();});});
    // The category actions are shared with Tools. Restore them after a search closes.
    connect(sourceMenu,&QMenu::aboutToHide,this,[search]{search->clear();});
    auto* browseEffects=new QToolButton;
    browseEffects->setText("Add effect");
    browseEffects->setPopupMode(QToolButton::InstantPopup);
    browseEffects->setMenu(sourceMenu);
    browseEffects->setToolTip(sources->toolTip());
    newLayout->addWidget(browseEffects,1);
    tools->addWidget(sources);tools->addSeparator();
    for(auto* control:tools->findChildren<QAbstractButton*>()){control->setFocusPolicy(Qt::StrongFocus);control->setIconSize({20,20});}
    auto* cropTool=new QCheckBox; cropTool->setObjectName("cropTool"); cropTool->setParent(this); cropTool->hide();
    auto* cropButton=new QToolButton; cropButton->setIcon(studioToolIcon("crop")); cropButton->setCheckable(true); cropButton->setFixedSize(32,32); cropButton->setObjectName("cropImageTool");
    cropButton->setToolTip("Crop imported image (C)"); cropButton->setAccessibleName("Crop image"); cropButton->setShortcut(QKeySequence(Qt::Key_C)); tools->addWidget(cropButton);
    connect(cropButton,&QToolButton::toggled,cropTool,&QCheckBox::setChecked); connect(cropTool,&QCheckBox::toggled,cropButton,&QToolButton::setChecked);
    connect(cropTool,&QCheckBox::toggled,this,[this](bool crop) { if(crop) { editingMask_=false; effectsCanvas_->setTool(LightingCanvas::Tool::Move); } effectsCanvas_->setCropTool(crop); });
    connect(effectsCanvas_,&LightingCanvas::maskEditingRequested,this,[this] { inspectorTabs_->setCurrentIndex(2); });
    connect(effectsCanvas_,&LightingCanvas::editingError,this,[this](const QString& message) { statusBar()->showMessage(message,8000); });
    connect(effectsCanvas_,&LightingCanvas::selectionChanged,this,[this] {
        if(selectedLayer_!=effectsCanvas_->selection()) { selectedLayer_=effectsCanvas_->selection(); refresh(); }
        syncSelectionTrees();
    });
    auto* toolOptions=new QToolBar("Tool options",this); toolOptions->setObjectName("drawingOptions"); toolOptions->setMinimumHeight(34);toolOptions->setMovable(false); addToolBarBreak(); addToolBar(toolOptions);
    appearanceControls->removeWidget(color_); color_->setMaximumWidth(190); auto* colorSlot=new QWidget;auto* colorLayout=new QHBoxLayout(colorSlot);colorLayout->setContentsMargins(0,0,0,0);colorSlot->setFixedWidth(190);color_->setFixedWidth(190);auto colorPolicy=color_->sizePolicy();colorPolicy.setRetainSizeWhenHidden(true);color_->setSizePolicy(colorPolicy);colorLayout->addWidget(color_);
    toolOptions->addWidget(colorSlot)->setObjectName("foregroundColorAction");
    // Retain the slot when a tool option is hidden so the toolbar does not move the canvas.
    const auto addOption=[toolOptions](QWidget* option,const QString& name) {
        auto* slot=new QWidget;
        auto* layout=new QHBoxLayout(slot);
        layout->setContentsMargins(0,0,0,0);
        auto policy=option->sizePolicy();
        policy.setRetainSizeWhenHidden(true);
        option->setSizePolicy(policy);
        layout->addWidget(option);
        toolOptions->addWidget(slot)->setObjectName(name);
    };
    auto* shapeFill=new QCheckBox("Fill shape"); shapeFill->setObjectName("shapeFill"); shapeFill->setChecked(true);
    addOption(shapeFill,"shapeFillAction");
    shapeFill->setToolTip("Fill the rectangle or ellipse you draw. Turn off to draw its outline.");
    tools->hide(); toolOptions->hide();
    connect(tabs_,&QTabWidget::currentChanged,this,[this,tools](int index) { tools->setVisible(index<2);
        for(auto* action:tools->actions()){auto* widget=tools->widgetForAction(action);action->setVisible(index==1 || (widget && (widget->objectName()=="moveEffectTool" || widget->objectName()=="panCanvasTool")));}
        refreshToolOptions(); });
    maskEnabled_ = new QCheckBox("Apply mask to output");maskEnabled_->setObjectName("maskEnabled");
    maskEnabled_->setToolTip("Apply this mask to the composition and device output. Turning it off preserves the stored mask and any edits.");
    maskControls->insertWidget(1,maskEnabled_);
    connect(maskEnabled_, &QCheckBox::toggled, this, [this](bool enabled) {
        updateLayer("Toggle mask", [enabled](auto& layer) { layer.maskEnabled = enabled; });
    });
    auto* showMask = new QCheckBox("Mask only");showMask->setObjectName("maskPreview");
    showMask->setToolTip("Show mask coverage while editing its mask: white reveals, black hides, gray partially reveals. This preview does not change device output.");
    maskControls->insertWidget(2,showMask);
    connect(showMask, &QCheckBox::toggled, effectsCanvas_, &LightingCanvas::setShowMask);
    auto* brushControls=new QWidget;
    auto* brushGrid=new QGridLayout(brushControls);
    brushGrid->setContentsMargins(0,0,0,0);
    brushGrid->setSpacing(8);
    // Keep the brush rows together when the inspector scroll area gets shorter at high DPI.
    brushGrid->setSizeConstraint(QLayout::SetMinimumSize);
    brushGrid->setColumnStretch(1,1);
    maskControls->addWidget(brushControls);
    const auto spin=[brushGrid](const QString& name,int low,int high,int value) {
        auto* control=new ParameterSpinBox;
        control->setRange(low,high);
        control->setValue(value);
        control->setAccessibleName(name);
        auto* label=new QLabel(name);
        label->setBuddy(control);
        const int row=brushGrid->count()/2;
        brushGrid->addWidget(label,row,0);
        brushGrid->addWidget(sliderField(control),row,1);
        brushGrid->setRowMinimumHeight(row,32);
        return control;
    };
    auto* size=new ParameterSpinBox; size->setAccessibleName("Brush diameter"); size->setObjectName("brushDiameter"); size->setRange(1,320); size->setValue(6); size->setSuffix(" px");
    auto* stroke=sliderField(size); stroke->setObjectName("brushWidthField"); stroke->setMinimumWidth(180); stroke->setMaximumWidth(250);
    stroke->findChild<QSlider*>()->setObjectName("brushSizeSlider");
    auto* strokeLabel=new QLabel(" Width ");strokeLabel->setObjectName("brushWidthLabel");
    strokeLabel->setFixedWidth(strokeLabel->sizeHint().width());
    addOption(strokeLabel,"strokeLabelAction");
    addOption(stroke,"strokeSizeAction");
    const auto syncDrawing=[this,size,shapeFill] { effectsCanvas_->setDrawing(color_->color(),size->value(),shapeFill->isChecked()); };
    connect(color_,&ColorEditor::colorChanged,this,[syncDrawing] { syncDrawing(); }); connect(size,&QDoubleSpinBox::valueChanged,this,[syncDrawing] { syncDrawing(); }); connect(shapeFill,&QCheckBox::toggled,this,[syncDrawing] { syncDrawing(); });
    syncDrawing();
    auto* toolHint=new QLabel; toolHint->setFixedWidth(300); toolHint->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred); toolHint->setObjectName("toolHint"); toolHint->setProperty("muted",true); toolOptions->addWidget(toolHint);
    auto* hardness = spin("Hardness %", 0, 100, 30);
    hardness->setObjectName("maskBrushHardness");
    auto* brushOpacity = spin("Brush opacity %", 0, 100, 100);
    brushOpacity->setObjectName("maskBrushOpacity");
    auto* gray = spin("Paint visibility %", 0, 255, 0);gray->setObjectName("maskPaintValue");static_cast<ParameterSpinBox*>(gray)->setDisplayUnit(100./255," %");
    brushControls->setMinimumHeight(brushGrid->minimumSize().height());
    auto* presets=new QWidget;auto* presetRow=new QHBoxLayout(presets);presetRow->setContentsMargins(0,0,0,0);
    for(const auto& entry:QVector<QPair<QString,int>>{{"Hide",0},{"Reveal",255}}) {
        auto* preset=new QPushButton(entry.first);preset->setObjectName(entry.second?"maskRevealBrush":"maskHideBrush");presetRow->addWidget(preset);
        preset->setCheckable(true);
        connect(preset,&QPushButton::clicked,this,[this,gray,value=entry.second]{gray->setValue(value);findChild<QPushButton*>("pencilTool")->click();refreshToolOptions();});
    }
    maskControls->insertWidget(1,presets);
    auto* maskThumbnail=new QLabel;
    maskThumbnail->setObjectName("maskThumbnail");
    maskThumbnail->setAccessibleName("Mask thumbnail");
    maskThumbnail->setAlignment(Qt::AlignLeft);
    maskControls->insertWidget(0,maskThumbnail);
    auto* maskHint=new QLabel;maskHint->setObjectName("maskBrushSummary");
    maskHint->setWordWrap(true); maskControls->addWidget(maskHint);
    gray->setToolTip("Pencil paints visibility: 0% hides, 100% reveals, intermediate values partially reveal. Eraser always hides. Gradient paints black to white.");
    const auto syncBrush=[this,size,hardness,brushOpacity,gray] {
        effectsCanvas_->setBrush(size->value()/2.0,hardness->value()/100.0,brushOpacity->value()/100.0,gray->value());
    };
    for (auto* control : {size, hardness, brushOpacity, gray}) connect(control,&QDoubleSpinBox::valueChanged,this,syncBrush);
    connect(gray,&QDoubleSpinBox::valueChanged,this,[this]{refreshToolOptions();});
    syncBrush(); // The initial mask brush must match the displayed diameter before the first size edit.
    button(maskControls, "Reveal entire target", [this] { updateLayer("Reveal layer", [](auto& layer) { layer.mask.fill(255); }); })->setObjectName("maskRevealAll");
    button(maskControls, "Hide entire target", [this] { updateLayer("Hide layer with mask", [](auto& layer) { layer.mask.fill(0); }); })->setObjectName("maskHideAll");
    button(maskControls, "Invert", [this] { updateLayer("Invert mask", [](auto& layer) { layer.mask.invertPixels(); }); })->setObjectName("maskInvert");
    maskControls->addStretch(); appearanceControls->addStretch(); effectControls->addStretch();
    auto* layerHint=new QLabel("Source / Mask");layerHint->setToolTip("Drag to reorder; Ctrl+PageUp/PageDown also moves a layer. Double-click to rename. Source edits light; Mask edits visibility.");
    layerHint->setWordWrap(true); layerHint->setProperty("muted",true); layerStack->addWidget(layerHint);
    connect(tabs_, &QTabWidget::currentChanged,this,[this,sidebarContexts,contexts,labels](int index) {
        setLabelsHeld(false);
        labels->setEnabled(index!=2);labels->setVisible(index!=2);
        updateWorkspacePanels();
        if(index<2) { sidebarContexts->setCurrentIndex(index); contexts->setCurrentIndex(index); }
        refreshProperties();
    });
    connect(effectsCanvas_,&LightingCanvas::addRequested,this,[this] {
        for(auto* control:findChildren<QPushButton*>()) if(control->text()=="New layer") { control->click(); break; }
    });
    connect(layoutCanvas_,&LightingCanvas::editingError,this,[this](const QString& message){statusBar()->showMessage(message,8000);});
    connect(layoutCanvas_,&LightingCanvas::addRequested,this,[this] { addHeaderComponents(); });
    connect(&document_, &LightingDocument::changed, this, &LightingWindow::refresh);
    connect(document_.history(), &QUndoStack::cleanChanged, this, [this](bool clean) { setWindowModified(!clean); });
    setWindowTitle(QString("Lighting Studio [%1][*] — RGB Lighting Editor").arg(studioBuildId));
    statusBar()->showMessage("Lighting Studio - Create effects and arrange components; canvas output is automatic.");
    auto* viewMenu=menuBar()->addMenu("View"); viewMenu->setObjectName("workspaceViewMenu");
    auto* canvasMenu=viewMenu->addMenu("Canvas");canvasMenu->addAction(fit);canvasMenu->addAction(findChild<QAction*>("zoomOut"));canvasMenu->addAction(findChild<QAction*>("zoomIn"));
    viewMenu->addAction(labels);viewMenu->addAction(panAction);
    auto* panelsMenu=new QMenu("Panels",this);
    for(auto* panel:{sidebarDock,dock}) {
        auto* toggle=panel->toggleViewAction();toggle->setObjectName(panel==sidebarDock ? "toggleNavigator" : "toggleInspector");
        toggle->setText(panel==sidebarDock ? "Navigator" : "Inspector");
        toggle->setShortcut(QKeySequence(panel==sidebarDock ? "Ctrl+Shift+L" : "Ctrl+Shift+P"));viewMenu->addAction(toggle);panelsMenu->addAction(toggle);
        connect(panel,&QDockWidget::visibilityChanged,this,[this,panel,sidebarDock](bool visible) {
            // Parent-window hiding and workspace switches must not overwrite the user's panel choices.
            // Minimizing hides the docks too; closing while minimized saved both panels as hidden
            // (owner report 2026-11-24).
            if(adjustingPanels_ || !isVisible() || isMinimized())return;
            if(panel==sidebarDock)navigatorShown_=visible;
            else {inspectorShown_=visible;if(compactWorkspace_)inspectorBeforeCompact_=visible;}
        });
    }
    auto* panelsButton=new QToolButton;panelsButton->setText("Panels");panelsButton->setObjectName("workspacePanels");
    panelsButton->setPopupMode(QToolButton::InstantPopup);panelsButton->setMenu(panelsMenu);
    // Keyboard-only navigation: window shortcuts, not menu entries (listed under Help > Keyboard shortcuts).
    auto* nextPanel=new QAction("Focus next panel",this);nextPanel->setObjectName("focusNextPanel");nextPanel->setShortcut(QKeySequence(Qt::Key_F6));addAction(nextPanel);
    auto* previousPanel=new QAction("Focus previous panel",this);previousPanel->setObjectName("focusPreviousPanel");previousPanel->setShortcut(QKeySequence(Qt::SHIFT|Qt::Key_F6));addAction(previousPanel);
    connect(nextPanel,&QAction::triggered,this,[this]{cyclePanelFocus(false);});
    connect(previousPanel,&QAction::triggered,this,[this]{cyclePanelFocus(true);});
    connect(tabs_,&QTabWidget::currentChanged,this,[nextPanel,previousPanel](int index){nextPanel->setEnabled(index<2);previousPanel->setEnabled(index<2);});
    auto* toolsMenu=menuBar()->addMenu("Tools");toolsMenu->setObjectName("drawingToolsMenu");
    // Drawing tools only act on a canvas. The menu stays in place and is disabled in Devices and the
    // Component Editor (workspace brief 2026-11-26; replaces hiding it, owner review 2026-11-17).
    connect(tabs_,&QTabWidget::currentChanged,toolsMenu,[toolsMenu](int index){toolsMenu->menuAction()->setEnabled(index<2);});
    auto* toolActions=new QActionGroup(toolsMenu);toolActions->setExclusive(true);
    for(auto* control:toolGroup->buttons()) {
        auto* action=toolsMenu->addAction(control->icon(),control->accessibleName());
        action->setObjectName("choose_"+control->objectName());action->setCheckable(true);toolActions->addAction(action);action->setChecked(control->isChecked());
        if(!control->shortcut().isEmpty())action->setText(action->text()+"\t"+control->shortcut().toString());
        connect(action,&QAction::triggered,control,&QAbstractButton::click);
        connect(control,&QAbstractButton::toggled,action,&QAction::setChecked);
        // The menu mirrors the strip: Components shows only Move and Pan there, so only those are listed (owner review 2026-11-18).
        const auto sync=[this,control,action,tools]{action->setVisible(control->isVisibleTo(tools));action->setEnabled(tabs_->currentIndex()<2 && control->isEnabled());};
        connect(toolsMenu,&QMenu::aboutToShow,action,sync);
        connect(tabs_,&QTabWidget::currentChanged,action,[sync](int){sync();});
    }
    // Every tool of the strip has its menu entry, the add-effect button included (owner review 2026-11-17).
    toolsMenu->addSeparator();auto* addEffectMenu=toolsMenu->addMenu(studioToolIcon("add-effect"),"Add effect");addEffectMenu->setObjectName("toolsAddEffectMenu");
    for(auto* category:effectCategories)addEffectMenu->addMenu(category);
    addEffectMenu->addSeparator();addEffectMenu->addAction(imageAction);
    connect(toolsMenu,&QMenu::aboutToShow,addEffectMenu,[addEffectMenu,sources,tools]{addEffectMenu->menuAction()->setVisible(sources->isVisibleTo(tools));});
    auto* resetWorkspace=viewMenu->addAction("Reset workspace"); resetWorkspace->setObjectName("resetWorkspace");panelsMenu->addSeparator();panelsMenu->addAction(resetWorkspace);
    // The default follows the Windows animation setting; this is the only in-app way to change a stored
    // editor/reduceMotion value (restored after the R6 menu cleanup, CodeRabbit review 2026-11-15).
    auto* reduceMotion=viewMenu->addAction("Reduce UI motion");reduceMotion->setObjectName("reduceMotion");reduceMotion->setCheckable(true);reduceMotion->setChecked(!studioMotionEnabled());
    connect(reduceMotion,&QAction::toggled,this,[](bool on){QSettings().setValue("editor/reduceMotion",on);});
    connect(resetWorkspace,&QAction::triggered,this,[this,sidebarDock,dock] {
        navigatorShown_=inspectorShown_=inspectorBeforeCompact_=true;updateWorkspacePanels();resizeDocks({sidebarDock,dock},{280,340},Qt::Horizontal);
        layoutCanvas_->fitToView(); effectsCanvas_->fitToView();
    });
    auto* navigation=studioHeaderToolbar("Workspace",this);addToolBar(navigation); navigation->setObjectName("workspaceNavigation"); navigation->setMovable(false);
    insertToolBar(toolbar,navigation);
    navigation->setToolButtonStyle(Qt::ToolButtonTextOnly);
    auto* brandAction=navigation->addWidget(brand);
    auto* hardwareAction=new QAction("Devices",this);hardwareAction->setObjectName("workspaceHardware");hardwareAction->setCheckable(true);
    QList<QAction*> workspaceActions{hardwareAction};
    auto* workspaceMenu=viewMenu->addMenu("Workspace");workspaceMenu->addAction(hardwareAction);
    auto* previewMenu=menuBar()->addMenu("Preview");previewMenu->setObjectName("previewMenu");
    auto* togglePreview=previewMenu->addAction("Stop preview");togglePreview->setObjectName("togglePreviewPlayback");
    connect(togglePreview,&QAction::triggered,playing_,&QPushButton::click);
    connect(playing_,&QPushButton::toggled,togglePreview,[togglePreview](bool running){togglePreview->setText(running ? "Stop preview" : "Play preview");});
    auto* scopeMenu=previewMenu->addMenu("Preview output");scopeMenu->setObjectName("previewOutputMenu");
    connect(scopeMenu,&QMenu::aboutToShow,this,[this,scopeMenu]{
        scopeMenu->clear();
        for(int i=0;i<previewChannel_->count();++i){auto* action=scopeMenu->addAction(previewChannel_->itemText(i));action->setCheckable(true);action->setChecked(i==previewChannel_->currentIndex());connect(action,&QAction::triggered,previewChannel_,[this,i]{previewChannel_->setCurrentIndex(i);});}
    });
    const auto openEditorWorkspace=[this](int index) {
        if(!leaveComponentPage()) return;
        if(tabs_->currentIndex()==index) return;
        auto* before=tabs_->currentIndex()==0 ? layoutCanvas_ : effectsCanvas_;
        const bool preserveCamera=isVisible() && before->isVisible();
        const auto point=before==layoutCanvas_ ? document_.layoutBounds().topLeft() : QPointF();
        const auto anchor=before->mapToGlobal(before->viewportPoint(point).toPoint());
        tabs_->setCurrentIndex(index);auto* after=index==0 ? layoutCanvas_ : effectsCanvas_;
        // A hidden startup page has no valid screen anchor; let its first resize fit the canvas.
        if(preserveCamera)after->copyCamera(*before,anchor);else after->fitToView();after->setFocus();
        // Inspector rows can resize the dock after the tab signal; copy again once layout settles.
        if(preserveCamera)QTimer::singleShot(0,after,[this,before,after,index,anchor]{if(tabs_->currentIndex()==index)after->copyCamera(*before,anchor);});
    };
    for(int index=0;index<2;++index) {
        auto* action=new QAction(index==0 ? "Layout" : "Effects",this);
        workspaceActions.append(action);
        action->setObjectName(index==0 ? "workspaceLayout" : "workspaceEffects");
        action->setCheckable(true);
        workspaceMenu->addAction(action);
        connect(tabs_,&QTabWidget::currentChanged,action,[this,action,index](int) { action->setChecked(workspaceIndex()==index && applicationPages_->currentWidget()==tabs_); });
        connect(action,&QAction::triggered,this,[this,openEditorWorkspace,index,action]{openEditorWorkspace(index);action->setChecked(tabs_->currentIndex()==index && applicationPages_->currentWidget()==tabs_);});
    }
    connect(hardwareAction,&QAction::triggered,this,[this,hardwareAction]{if(leaveComponentPage())tabs_->setCurrentIndex(2);hardwareAction->setChecked(tabs_->currentIndex()==2 && applicationPages_->currentWidget()==tabs_);});
    auto* align=new QToolButton;align->setObjectName("alignSelection");align->setText("Align / Snap");align->setAccessibleName("Align and distribute selection");
    align->setToolTip("Align to selection bounds; distribute centers with at least three selected objects");align->setPopupMode(QToolButton::InstantPopup);
    auto* alignMenu=new QMenu(align);align->setMenu(alignMenu);previewToolbar->addWidget(align);
    alignMenu->addSection("Align selection");
    for(const auto& key:QStringList{"left","center","right","top","middle","bottom","distribute-horizontal","distribute-vertical"}) {
        auto* action=alignMenu->addAction(key.startsWith("distribute") ? QString(key).replace('-', ' ').replace("distribute","Distribute") : "Align "+key);action->setObjectName("align_"+key);
        connect(action,&QAction::triggered,this,[this,key]{(tabs_->currentIndex()==0 ? layoutCanvas_ : effectsCanvas_)->alignSelection(key);});
    }
    alignMenu->addSection("Snapping");alignMenu->addAction(snap);
    auto* snapSettings=new QWidget;auto* snapForm=new QFormLayout(snapSettings);snapForm->setContentsMargins(12,4,12,8);snapForm->addRow("Canvas grid",gridSize);
    auto* paletteStep=new QSpinBox;paletteStep->setObjectName("paletteSnapPercent");paletteStep->setRange(1,100);paletteStep->setValue(QSettings().value("editor/paletteSnapPercent",1).toInt());paletteStep->setKeyboardTracking(false);paletteStep->setAccessibleName("Color stop snap step");
    paletteStep->setToolTip("Color point spacing as a percentage of the color bar. 1 divides the bar into 100 steps. Used only while Snap to grid is enabled; hold Alt to bypass.");snapForm->addRow("Color stop step",paletteStep);
    connect(paletteStep,&QSpinBox::valueChanged,this,[this](int value){QSettings().setValue("editor/paletteSnapPercent",value);for(auto* bar:findChildren<QWidget*>("colorStopBar"))bar->update();});
    auto* snapAction=new QWidgetAction(alignMenu);snapAction->setDefaultWidget(snapSettings);alignMenu->addAction(snapAction);
    auto* guides=alignMenu->addAction("Smart guides");guides->setObjectName("smartGuides");guides->setCheckable(true);guides->setChecked(true);
    layoutCanvas_->setSmartGuides(true);effectsCanvas_->setSmartGuides(true);
    connect(guides,&QAction::toggled,this,[this](bool on){layoutCanvas_->setSmartGuides(on);effectsCanvas_->setSmartGuides(on);});
    auto* lock=editMenu->addAction("Lock position (cannot be moved)");lock->setObjectName("lockSelection");
    lock->setToolTip("Locked items cannot be moved, resized or rotated until unlocked. A padlock marks them on the canvas.");
    auto* unlock=editMenu->addAction("Unlock position");unlock->setObjectName("unlockSelection");
    connect(lock,&QAction::triggered,this,[this]{if(tabs_->currentIndex()!=2)(tabs_->currentIndex()==0 ? layoutCanvas_ : effectsCanvas_)->lockSelection(true);});
    connect(unlock,&QAction::triggered,this,[this]{if(tabs_->currentIndex()!=2)(tabs_->currentIndex()==0 ? layoutCanvas_ : effectsCanvas_)->lockSelection(false);});
    auto* duplicate=editMenu->addAction("Duplicate selection");duplicate->setObjectName("duplicateSelection");duplicate->setShortcut(QKeySequence("Ctrl+D"));
    connect(duplicate,&QAction::triggered,this,[this]{auto* focus=QApplication::focusWidget();if(qobject_cast<QLineEdit*>(focus) || qobject_cast<QAbstractSpinBox*>(focus) || qobject_cast<QTextEdit*>(focus) || qobject_cast<QPlainTextEdit*>(focus))return;if(tabs_->currentIndex()!=2)(tabs_->currentIndex()==0 ? layoutCanvas_ : effectsCanvas_)->duplicateSelection();});
    auto* remove=editMenu->addAction("Delete selection");remove->setObjectName("deleteSelection");
    // Canvas edits are offered only where they act: in Design with a selection (owner report and final review 2026-11-17).
    connect(tabs_,&QTabWidget::currentChanged,this,[this](int){refreshToolOptions();});
    connect(remove,&QAction::triggered,this,[this]{if(tabs_->currentIndex()<2)(tabs_->currentIndex()==0 ? layoutCanvas_ : effectsCanvas_)->deleteSelection();});
    auto* rename=new QAction("Rename",this);rename->setObjectName("renameLayer");connect(rename,&QAction::triggered,this,[this]{if(layers_->currentItem())layers_->editItem(layers_->currentItem(),0);});
    auto* visibility=new QAction("Show / hide",this);visibility->setObjectName("toggleLayerVisibility");connect(visibility,&QAction::triggered,this,[this]{
        const auto id=selectedLayer_;document_.edit("Toggle visibility",[id](auto& state){for(auto& layer:state.layers)if(layer.id==id)layer.visible=!layer.visible;});
    });
    auto* cropSelection=new QAction(studioToolIcon("crop"),"Crop image",this);cropSelection->setObjectName("cropSelectedImage");connect(cropSelection,&QAction::triggered,cropButton,&QToolButton::click);
    auto* flash=new QAction("Flash component (identify)",this);flash->setObjectName("flashComponent");
    flash->setToolTip("Lights only this component for 2 seconds. Hardware lights only while output is running; otherwise the Layout preview shows it.");
    connect(flash,&QAction::triggered,this,[this]{const auto ids=layoutCanvas_->selections();if(ids.size()==1)flashComponent(*ids.begin());});
    const auto populateContext=[this,duplicate,remove,rename,visibility,cropSelection,cropButton,lock,unlock,fit,groupAction,paintAction,flash](QMenu* menu,bool onObject){
        if(onObject){
            auto* canvas=tabs_->currentIndex()==0 ? layoutCanvas_ : effectsCanvas_;bool editable=!canvas->selections().isEmpty();if(tabs_->currentIndex()==1)for(const auto& layer:document_.state().layers)if(layer.id==selectedLayer_ && layer.container && !layer.locked)editable=true;
            duplicate->setEnabled(editable);remove->setEnabled(editable);
            menu->addAction(duplicate);menu->addAction(remove);
            if(tabs_->currentIndex()==1){menu->addAction(rename);menu->addAction(visibility);menu->addAction(findChild<QAction*>("moveLayerUp"));menu->addAction(findChild<QAction*>("moveLayerDown"));if(cropButton->isEnabled())menu->addAction(cropSelection);}
            if(tabs_->currentIndex()==0){const auto ids=layoutCanvas_->selections();flash->setEnabled(ids.size()==1 && document_.hasSoftwareChannel(*ids.begin()));menu->addAction(flash);}
            menu->addSeparator();menu->addAction(lock);menu->addAction(unlock);
        // Hardware is attached only in Devices: one entry point per function (owner review 2026-11-18).
        }else{menu->addAction(fit);if(tabs_->currentIndex()==1){menu->addSeparator();menu->addAction(paintAction);menu->addAction(groupAction);}}
    };
    connect(effectsCanvas_,&LightingCanvas::contextMenuRequested,this,populateContext);connect(layoutCanvas_,&LightingCanvas::contextMenuRequested,this,populateContext);
    layers_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(layers_,&QWidget::customContextMenuRequested,this,[this,populateContext](QPoint point){
        auto* item=layers_->itemAt(point);if(item && !item->isSelected())layers_->setCurrentItem(item);
        QMenu menu(layers_);menu.setObjectName("layerContextMenu");populateContext(&menu,item!=nullptr);menu.exec(layers_->viewport()->mapToGlobal(point));
    });
    components_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(components_,&QWidget::customContextMenuRequested,this,[this,populateContext](QPoint point){
        auto* item=components_->itemAt(point);if(item && !item->isSelected())components_->setCurrentItem(item);
        QMenu menu(components_);menu.setObjectName("componentContextMenu");populateContext(&menu,item!=nullptr);menu.exec(components_->viewport()->mapToGlobal(point));
    });
    alignMenu->setTitle("Align / Snap");viewMenu->addMenu(alignMenu);
    // View options control overlays within a workspace without changing the editing target.
    auto* layoutEffect=viewMenu->addAction("Show effect behind components");layoutEffect->setObjectName("showLayoutEffect");layoutEffect->setCheckable(true);layoutEffect->setChecked(true);
    connect(layoutEffect,&QAction::toggled,this,[this](bool on){layoutCanvas_->setEditorView(on,true);});
    auto* effectComponents=viewMenu->addAction("Show components over effect");effectComponents->setObjectName("showEffectComponents");effectComponents->setCheckable(true);effectComponents->setChecked(true);
    connect(effectComponents,&QAction::toggled,this,[this](bool on){effectsCanvas_->setEditorView(true,on);});
    effectsCanvas_->setEditorView(true,effectComponents->isChecked());
    auto* split=viewMenu->addAction("Show separate hardware preview");split->setObjectName("splitHardwarePreview");split->setCheckable(true);
    connect(split,&QAction::toggled,this,[this,effectWorkspace,livePanel](bool on){
        livePanel->setVisible(on && !document_.state().placements.isEmpty());
        if(on)effectWorkspace->setSizes({480,260});
    });
    connect(tabs_,&QTabWidget::currentChanged,this,[this,hardwareAction,previewToolbar,tools,layoutEffect,effectComponents,split](int index){
        hardwareAction->setChecked(index==2 && applicationPages_->currentWidget()==tabs_);previewToolbar->setVisible(true);tools->setVisible(index<2);
        layoutEffect->setVisible(index==0);
        effectComponents->setVisible(index==1);
        split->setEnabled(index==1); // menu bar stays the same in every area (workspace brief 2026-11-26)
    });
    // Direct workspace navigation shares the logo row to leave room for preview controls below.
    for(auto* action:workspaceActions) navigation->insertAction(brandAction,action);
    const auto firstEdit=previewToolbar->actions().first();
    previewToolbar->insertSeparator(firstEdit);
    // Open, Save, Save as, Undo and Redo act in every workspace (owner report 2026-11-17: saving
    // was impossible in Devices); only canvas commands hide there.
    const auto historyCommands=toolbar->actions();
    for(auto* action:toolbar->actions()){toolbar->removeAction(action);previewToolbar->insertAction(firstEdit,action);}
    toolbar->hide();
    auto* brandSpace=new QWidget(navigation);brandSpace->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    navigation->insertWidget(brandAction,brandSpace);
    // Keep panel recovery ahead of optional controls that can overflow.
    previewToolbar->insertWidget(firstEdit,panelsButton);
    previewToolbar->insertSeparator(firstEdit);
    previewToolbar->insertSeparator(fit);
    previewToolbar->insertSeparator(previewToolbar->actions().last());
    previewToolbar->setIconSize(QSize(22,22));
    previewToolbar->setStyleSheet("QToolBar { spacing: 6px; } QToolButton, QPushButton, QComboBox { min-height: 28px; max-height: 28px; padding: 4px 7px; } QTabBar::tab { min-height: 28px; max-height: 28px; padding: 4px 7px; } QComboBox { padding-right: 24px; } QToolButton[popupMode=\"2\"] { padding-right: 20px; }");
    const auto scopeHint=[this]{previewChannel_->setToolTip(previewChannel_->currentText()+"\nDisplay scope only; device output is unchanged. Other channels are dimmed. Global effects can look identical on multiple channels.");};
    connect(previewChannel_,&QComboBox::currentTextChanged,this,[scopeHint](const QString&){scopeHint();});scopeHint();
    cropButton->setToolTip("Crop image: drag a frame, move it or resize its corners. Enter or double-click applies; Escape cancels.");
    // Canvas commands stay in place and are disabled in Devices and the Component Editor, like the menus
    // (owner decision "greyed out", review 2026-11-26: the toolbar still hid them). History and the
    // playback button act everywhere and keep their own state.
    const auto editorCommands=previewToolbar->actions();
    connect(tabs_,&QTabWidget::currentChanged,this,[this,previewToolbar,editorCommands,historyCommands](int index){
        for(auto* action:editorCommands) {
            if(action->objectName()=="projectNameAction"){action->setVisible(false);continue;}
            auto* widget=previewToolbar->widgetForAction(action);
            if(historyCommands.contains(action) || widget==playing_)continue;
            action->setEnabled(index<2);if(widget)widget->setEnabled(index<2);
        }
    });
    tabs_->tabBar()->hide();
    // Devices has no canvas: entries that act on one stay in the menus but are disabled, so the menu
    // bar is the same in every area (workspace brief 2026-11-26; replaces hiding them, 2026-11-18).
    // Toolbar counterparts still leave the toolbar there.
    const QList<QAction*> canvasOnly{canvasMenu->menuAction(),alignMenu->menuAction(),previewMenu->menuAction(),
        panAction,resetWorkspace,findChild<QAction*>("toggleNavigator"),findChild<QAction*>("toggleInspector"),
        findChild<QAction*>("lockSelection"),findChild<QAction*>("unlockSelection"),findChild<QAction*>("duplicateSelection"),findChild<QAction*>("deleteSelection")};
    connect(tabs_,&QTabWidget::currentChanged,this,[this,canvasOnly](int index){
        for(auto* action:canvasOnly)action->setEnabled(index<2);
        // Selection-dependent entries (duplicate, delete, lock) must not stay enabled without a selection.
        if(index<2){refreshToolOptions();updateWorkspacePanels();}
    });
    tabs_->setCurrentIndex(1);
    tabs_->setCurrentIndex(document_.hasComponentLibrary() ? 2 : 0);
    menuBar()->removeAction(help->menuAction());menuBar()->addMenu(help);
    resizeDocks({sidebarDock,dock},{280,340},Qt::Horizontal);
    for(auto* field:findChildren<ColorEditor*>()) {
        connect(field,&ColorEditor::gestureStarted,&document_,&LightingDocument::beginColorPreview);
        connect(field,&ColorEditor::gestureFinished,&document_,&LightingDocument::finishColorPreview);
    }
    for(auto* slider:findChildren<QSlider*>())if(slider->property("numericField").isValid()) {
        connect(slider,&QSlider::sliderPressed,this,[this]{document_.history()->beginMacro("Parameter drag");});
        connect(slider,&QSlider::sliderReleased,this,[this]{document_.history()->endMacro();});
    }
    for(auto* field:findChildren<QAbstractSpinBox*>())styleStudioNumericField(field);
    for(auto* form:findChildren<QFormLayout*>()) {
        form->setVerticalSpacing(6);form->setHorizontalSpacing(8);form->setLabelAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        for(int row=0;row<form->rowCount();++row) {
            auto* labelItem=form->itemAt(row,QFormLayout::LabelRole);auto* fieldItem=form->itemAt(row,QFormLayout::FieldRole);
            auto* label=labelItem ? qobject_cast<QLabel*>(labelItem->widget()) : nullptr;auto* field=fieldItem ? fieldItem->widget() : nullptr;
            if(!label || !field)continue;
            auto* number=qobject_cast<QAbstractSpinBox*>(field);if(!number)number=field->findChild<QAbstractSpinBox*>();
            if(number){label->setBuddy(number);if(number->accessibleName().isEmpty())number->setAccessibleName(label->text());}
        }
    }
    QVector<QLabel*> motionLabels;int motionLabelWidth=0;
    for(auto* form:motion->findChildren<QFormLayout*>())for(int row=0;row<form->rowCount();++row) {
        auto* item=form->itemAt(row,QFormLayout::LabelRole);auto* label=item ? qobject_cast<QLabel*>(item->widget()) : nullptr;
        if(label){motionLabels.append(label);motionLabelWidth=std::max(motionLabelWidth,label->fontMetrics().horizontalAdvance(label->text()));}
    }
    for(auto* label:motionLabels)label->setMinimumWidth(motionLabelWidth+8);
    installParameterTooltips(this);
    refresh();
    qApp->installEventFilter(this);
    playing_->setChecked(true);
}

LightingWindow::~LightingWindow()
{
    qApp->removeEventFilter(this);
    // Finish hide/focus/gesture callbacks while document_ still exists, including a fullscreen canvas.
    hide();
    for(auto* canvas:{layoutCanvas_,effectsCanvas_,hardwarePreview_})canvas->hide();
}

void LightingWindow::setLabelsHeld(bool held)
{
    labelsHeld_=held;
    layoutCanvas_->setShowLabels(held || findChild<QAction*>("showComponentLabels")->isChecked());
    effectsCanvas_->setShowLabels(held || findChild<QAction*>("showComponentLabels")->isChecked());
}

QVariantMap LightingWindow::workspacePanelPreferences() const
{
    // Hardware/compact mode hides panels temporarily; persist the user's editing preference.
    return {{"navigator",navigatorShown_},{"inspector",compactWorkspace_ ? inspectorBeforeCompact_ : inspectorShown_}};
}
void LightingWindow::restoreWorkspacePanelPreferences(const QVariantMap& panels)
{
    if(panels.isEmpty())return;
    navigatorShown_=panels.value("navigator",true).toBool();
    inspectorBeforeCompact_=panels.value("inspector",true).toBool();
    inspectorShown_=!compactWorkspace_ && inspectorBeforeCompact_;
    updateWorkspacePanels();
}

int LightingWindow::workspaceIndex() const
{
    return applicationPages_ && applicationPages_->currentWidget()!=tabs_ ? 2 : tabs_->currentIndex();
}

void LightingWindow::updateWorkspacePanels()
{
    auto* navigator=findChild<QDockWidget*>("layersDock");auto* inspector=findChild<QDockWidget*>("propertiesDock");
    if(!navigator || !inspector)return;
    const bool editing=workspaceIndex()!=2;
    adjustingPanels_=true;navigator->setVisible(editing && navigatorShown_);inspector->setVisible(editing && inspectorShown_);adjustingPanels_=false;
    for(const auto* name:{"toggleNavigator","toggleInspector"})if(auto* action=findChild<QAction*>(name))action->setEnabled(editing);
}

void LightingWindow::paintEvent(QPaintEvent* event)
{
    QMainWindow::paintEvent(event);
    static const QPixmap background(":/branding/lighting-studio-background.png");
    QPainter painter(this);
    painter.fillRect(rect(),studioThemeColor("@surround"));
    if(!background.isNull() && width()>0 && height()>0) {
        // Crop scenery only; branding stays in the separate navigation logo.
        const qreal scale=std::max(qreal(width())/background.width(),qreal(height())/background.height());
        const QSizeF sourceSize(width()/scale,height()/scale);
        const QRectF source(QPointF((background.width()-sourceSize.width())/2,
                                   (background.height()-sourceSize.height())/2),sourceSize);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawPixmap(QRectF(rect()),background,source);
        painter.fillRect(rect(),QColor(4,12,22,205)); // calm, near-opaque surface over the photo (critique 2026-11-26)
    }
}

void LightingWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    if(!findChild<QDockWidget*>("propertiesDock"))return;
    const bool compact=width()<1180;
    if(compact==compactWorkspace_)return;
    compactWorkspace_=compact;
    // Collapse once on entering compact mode; explicit panel choices remain usable until the next size transition.
    if(compact) {inspectorBeforeCompact_=inspectorShown_;inspectorShown_=false;}
    else inspectorShown_=inspectorBeforeCompact_;
    updateWorkspacePanels();
}

void LightingWindow::cyclePanelFocus(bool backwards)
{
    if(tabs_->currentIndex()>1)return;
    auto* canvas=tabs_->currentIndex()==0 ? layoutCanvas_ : effectsCanvas_;
    auto* navigator=findChild<QDockWidget*>("layersDock");
    auto* inspector=findChild<QDockWidget*>("propertiesDock");
    QVector<QWidget*> panels;
    if(navigator->isVisible())panels.append(navigator);
    panels.append(canvas);
    if(inspector->isVisible())panels.append(inspector);
    const auto* focus=QApplication::focusWidget();
    int current=-1;
    for(int i=0;i<panels.size();++i)if(panels[i]==focus || panels[i]->isAncestorOf(focus)){current=i;break;}
    const int next=current<0 ? (backwards ? panels.size()-1 : 0) : (current+(backwards ? panels.size()-1 : 1))%panels.size();
    auto* target=panels[next];
    if(target==navigator)target=tabs_->currentIndex()==0 ? static_cast<QWidget*>(components_) : layers_;
    else if(target==inspector) {
        for(auto* field:inspector->findChildren<QWidget*>())if(field->isVisible() && field->isEnabled() && (field->focusPolicy()&Qt::TabFocus)) {target=field;break;}
    }
    target->setFocus(backwards ? Qt::BacktabFocusReason : Qt::TabFocusReason);
}

void LightingWindow::moveSelectedLayer(int direction)
{
    for(int i=0;i<document_.state().layers.size();++i)if(document_.state().layers[i].id==selectedLayer_) {
        int target=i-direction;
        while(target>=0 && target<document_.state().layers.size() && document_.state().layers[target].parentId!=document_.state().layers[i].parentId)target-=direction;
        if(target>=0 && target<document_.state().layers.size())document_.edit("Reorder layers",[i,target](auto& state){state.layers.swapItemsAt(i,target);});
        return;
    }
}

void LightingWindow::updateMotion(const QString& label,const std::function<void(EffectLayer&)>& operation)
{
    updateLayer(label,operation); // Bulk changes require the explicit Apply motion action.
}

void LightingWindow::updateLayer(const QString& label, const std::function<void(EffectLayer&)>& operation)
{
    if (selectedLayer_.isNull()) return;
    for(const auto& layer:document_.state().layers)if(layer.id==selectedLayer_) {
        bool locked=layer.locked;for(const auto& parent:document_.state().layers)if(parent.id==layer.parentId)locked|=parent.locked;
        if(locked){statusBar()->showMessage("Selection is locked. Unlock it before editing.",5000);return;}
    }
    try {
        document_.edit(label, [&](LightingState& state) {
            const auto id=label.startsWith("Layer channel") ? document_.layerRoot(selectedLayer_) : selectedLayer_;
            for (auto& layer : state.layers) if (layer.id == id) {if(layer.locked)throw std::invalid_argument("Selection is locked");operation(layer);}
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
        // The service also reports devices of experimental profiles this editor has not opted into.
        discovered_.removeIf([this](const ProfileCandidate& candidate) {
            return std::none_of(document_.profiles().cbegin(),document_.profiles().cend(),[&](const DeviceProfile& profile){return profile.id==candidate.profileId;});
        });
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
    return connectorAssignment(selectedHeader_);
}

ChannelAssignment LightingWindow::connectorAssignment(const QVariantMap& connector) const
{
    ChannelAssignment result;
    result.profileId=connector.value("profileId").toString();
    result.endpoint=connector.value("path").toString();
    result.channelId=connector.value("channelId").toString();
    for(const auto& assignment:document_.state().assignments)
        if(assignment.profileId==result.profileId && assignment.endpoint==result.endpoint && assignment.channelId==result.channelId) return assignment;
    const auto& channel=assignmentChannel(assignmentProfile(document_.profiles(),result.profileId),result.channelId);
    result.software=channel.direct;
    if(!channel.nativeModes.isEmpty()) result.native.id=channel.nativeModes.first();
    return result;
}

void LightingWindow::moveComponentToConnector(const QUuid& id, ChannelAssignment connector)
{
    if (connector.placements.contains(id)) return;
    const auto available=document_.availableAddresses(connector);
    for (const auto& placement:document_.state().placements) if (placement.id==id) {
        const int required=document_.catalog().value(placement.componentId).addresses();
        if (required>available)
            throw std::invalid_argument(QString("%1 has %2 available addresses. This component needs %3.")
                .arg(connector.channelId).arg(available).arg(required).toStdString());
        document_.moveComponent(id,connector);
        selectedHeader_={{"profileId",connector.profileId},{"path",connector.endpoint},{"channelId",connector.channelId}};
        layoutCanvas_->select(id);
        refreshHeader();
        return;
    }
}

void LightingWindow::refreshHeader()
{
    const bool selected=!selectedHeader_.isEmpty();
    auto* add = findChild<QPushButton*>("addHeaderComponents");
    add->setEnabled(false);
    findChild<QPushButton*>("headerSettings")->setEnabled(selected);
    if(!selected) {
        headerTitle_->setText("Select a connector");
        headerSummary_->setText("Scan for devices or choose a controller for offline planning.");
        return;
    }
    const auto assignment=selectedHeaderAssignment();
    const auto& profile=assignmentProfile(document_.profiles(),assignment.profileId);
    const auto& channel=assignmentChannel(profile,assignment.channelId);
    const auto available = document_.availableAddresses(assignment);
    const bool attachable = channel.direct && !channelHasFixedComponent(channel);
    add->setEnabled(attachable && available > 0);
    const int leds=document_.outputLedsUsed(assignment);
    if(auto* empty=findChild<QLabel*>("hardwareEmptyState"))empty->setText(QString("No compatible controller detected\n\nSaved selection: %1 / %2\n%3 components / %4 LED addresses configured.\n\nChoose Refresh to check connections. Layouts remain editable offline.").arg(profile.name,channel.id).arg(assignment.placements.size()).arg(leds));
    for(const auto& placement:document_.state().placements) if(placement.id==layoutCanvas_->selection()) {
        const auto& component=document_.catalog()[placement.componentId]; headerTitle_->setText(document_.componentName(placement));
        headerSummary_->setText(QString("%1 / %2 light points / %3 LED addresses\n%4 addresses available%5")
            .arg(assignment.channelId).arg(component.positions.size()).arg(component.addresses()).arg(available)
            .arg(attachable ? QString() : "\nThis connector has a built-in component.")); return;
    }
    headerTitle_->setText(assignment.channelId);
    // One row per attached component: its output range on this header (shared outputs repeat the range).
    const auto offsets=document_.componentLedOffsets();
    QStringList rows;
    for(const auto& id:assignment.placements) for(const auto& placement:document_.state().placements) if(placement.id==id) {
        const int count=document_.catalog()[placement.componentId].addresses();
        const int start=offsets.value(id,-1);
        rows.append(start<0 ? QString("%1: %2 LEDs").arg(document_.componentName(placement)).arg(count)
                            : QString("%1: LEDs %2-%3").arg(document_.componentName(placement)).arg(start+1).arg(start+count));
    }
    headerSummary_->setText(QString("%1\n%2 components / %3 of %4 LEDs\n%5").arg(profile.name).arg(assignment.placements.size()).arg(leds).arg(channel.maxLeds)
        .arg(attachable ? QString("%1 addresses available").arg(available) : channel.direct ? "This connector has a built-in component." : "Native lighting only")
        + (rows.isEmpty() ? QString() : "\n" + rows.join("\n")));
}

void LightingWindow::addHeaderComponents()
{
    QVector<HeaderChoice> choices;
    for(int i=0;i<components_->topLevelItemCount();++i) {
        const auto data=components_->topLevelItem(i)->data(0,Qt::UserRole+1).toMap(); if(data.isEmpty()) continue;
        const auto& profile=assignmentProfile(document_.profiles(),data.value("profileId").toString());
        if(profile.driverId=="lighting.peripherals" || profile.driverId=="lighting.auxiliary") continue; // Dedicated devices are detected, never attached to ARGB headers.
        const auto& channel=assignmentChannel(profile,data.value("channelId").toString());
        if(!channel.direct || channelHasFixedComponent(channel)) continue; // Integrated output, not an attachable fan header.
        // Shared outputs count once, matching the header summary (CodeRabbit review 2026-11-15).
        int used=0;
        for(const auto& assignment:document_.state().assignments) if(assignment.profileId==profile.id && assignment.endpoint==data.value("path") && assignment.channelId==channel.id)
            used=document_.outputLedsUsed(assignment);
        choices.append({profile.name+" / "+channel.id,data,channel.maxLeds-used,channel.fixedLeds ? channel.fixedLeds-used : 0});
    }
    auto products = document_.catalog();
    try {
        for (const auto& definition : document_.customComponents()) products.insert(definition.id, definition);
    } catch (const std::exception& error) { headerSummary_->setText(QString::fromUtf8(error.what())); return; }
    HardwarePicker picker(products,choices,selectedHeader_,this);
    if(picker.exec()!=QDialog::Accepted) return;
    selectedHeader_=picker.header(); const auto assignment=selectedHeaderAssignment();
    try {
        auto component=picker.component(); const int count=picker.quantity();
        if (document_.hasComponentLibrary() && !document_.catalog().contains(component.id)) component.id=document_.registerComponent(component);
        document_.edit("Attach hardware",[&](LightingState& state) {
            if(!state.components.contains(component.id) && state.components.size()>=256) throw std::invalid_argument("Component definition limit reached");
            if(!state.components.contains(component.id)) state.components.insert(component.id,component);
            auto updated=assignment;
            QRectF bounds; for(const auto& point:component.positions) bounds=bounds.united(QRectF(point,QSizeF(.001,.001)));
            const double scale=3.0/std::max(3.0,std::max(bounds.width(),bounds.height()));
            const int nextAddress=chainStarts(updated).isEmpty() ? 0 : document_.outputLedsUsed(updated)+1;
            for(int index=0;index<count;++index) {
                ComponentPlacement placement; placement.componentId=component.id; const int position=state.placements.size();
                placement.scale={scale,scale}; placement.origin={50.0+(position%7)*80-bounds.left()*20*scale,60.0+(position/7)*80-bounds.top()*20*scale};
                state.placements.append(placement); updated.placements.append(placement.id);
                if (nextAddress) updated.starts[placement.id]=nextAddress+index*component.addresses();
            }
            bool replaced=false; for(auto& existing:state.assignments) if(existing.id==updated.id) { existing=updated; replaced=true; }
            if(!replaced) state.assignments.append(updated);
        });
    } catch(const std::exception& error) { statusBar()->showMessage(QString::fromUtf8(error.what()),8000); }
}

ComponentEditorPage* LightingWindow::componentPage()
{
    // Built lazily: the built-in product list reads installed component metadata from disk.
    if (!componentPage_) {
        componentPage_=new ComponentEditorPage(document_,hardwareProducts(document_.installedCatalog()),applicationPages_);
        undoGroup_->addStack(componentPage_->undoStack());
        applicationPages_->addWidget(componentPage_);
        connect(componentPage_,&ComponentEditorPage::closeRequested,this,[this] { applicationPages_->setCurrentWidget(tabs_); });
    }
    return componentPage_;
}

bool LightingWindow::leaveComponentPage()
{
    if (!componentPage_ || applicationPages_->currentWidget()!=componentPage_) return true;
    if (!componentPage_->confirmLeave()) return false;
    applicationPages_->setCurrentWidget(tabs_);
    return true;
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
        placementControls_[i]->setEnabled(selected && !selected->locked);
        if (selected) {
            const double values[]{selected->origin.x(), selected->origin.y(),
                selected->scale.width() * 100, selected->scale.height() * 100, selected->rotation};
            placementControls_[i]->setValue(values[i]);
        }
    }
}

void LightingWindow::syncSelectionTrees()
{
    if(!layers_ || !components_)return;
    // Mirror all selected members without sending tree-selection edits back into the canvas.
    for(const auto& entry:{qMakePair(layers_,effectsCanvas_),qMakePair(components_,layoutCanvas_)}) {
        const QSignalBlocker block(entry.first);const auto ids=entry.second->selections();
        for(QTreeWidgetItemIterator it(entry.first);*it;++it) {
            const auto id=(*it)->data(0,Qt::UserRole).toUuid();
            (*it)->setSelected(!id.isNull() && (ids.contains(id) || id==entry.second->selection()));
        }
    }
}

void LightingWindow::refresh()
{
    // Devices the last scan did not find stay in the project, but the canvases hide them by default
    // (Settings > Disconnected devices; owner request 2026-11-24). Without any scan result (offline
    // planning, discovery off) everything shows. The match is exact, like the Devices list: a device on
    // another USB port has another path and counts as disconnected at its old one.
    QSet<QUuid> disconnected;
    if(!discovered_.isEmpty()) for(const auto& assignment:document_.state().assignments)
        if(std::none_of(discovered_.cbegin(),discovered_.cend(),[&](const auto& candidate){return candidate.profileId==assignment.profileId && candidate.device.path==assignment.endpoint;}))
            for(const auto& id:assignment.placements) disconnected.insert(id);
    const bool dim=QSettings().value("editor/disconnectedDevices","hide").toString()=="dim";
    layoutCanvas_->setDisconnectedPlacements(disconnected,dim);effectsCanvas_->setDisconnectedPlacements(disconnected,dim);
    const bool needsController=discovered_.isEmpty() && offlineProfile_.isEmpty() && document_.state().assignments.isEmpty();
    findChild<QLabel*>("hardwareEmptyState")->setVisible(needsController);
    findChild<QComboBox*>("offlineController")->setVisible(discovered_.isEmpty());
    hardwarePreview_->setVisible(!document_.state().placements.isEmpty()); // Give the space back to authoring when there is no hardware to preview.
    findChild<QWidget*>("hardwarePreviewPanel")->setVisible(!document_.state().placements.isEmpty() && findChild<QAction*>("splitHardwarePreview") && findChild<QAction*>("splitHardwarePreview")->isChecked());
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
        if (discovered_.isEmpty() && !offlineProfile_.isEmpty()) {
            const auto& profile = assignmentProfile(document_.profiles(), offlineProfile_);
            for (const auto& channel : profile.channels) if (channel.direct && !channelHasFixedComponent(channel))
                append(profile.id, "offline:" + profile.id, channel.id);
        }
        // Saved chains stay editable while disconnected; discovery never rewrites their exact endpoint.
        for(const auto& assignment:document_.state().assignments) append(assignment.profileId,assignment.endpoint,assignment.channelId);
        QSet<QUuid> assigned;
        // LEDs column: header usage "used / max" (shared outputs count once) and each component's output range.
        const auto ledOffsets=document_.componentLedOffsets();
        for(const auto& data:headers) {
            bool online=false;
            for(const auto& candidate:discovered_) if(candidate.profileId==data["profileId"] && candidate.device.path==data["path"]) online=true;
            const auto& profile=assignmentProfile(document_.profiles(),data["profileId"].toString());
            auto* group=new QTreeWidgetItem(components_,{profile.name+" / "+data["channelId"].toString()});
            group->setData(0,Qt::UserRole+1,data); group->setExpanded(true);
            group->setToolTip(0,profile.name+"\n"+data["path"].toString());
            const auto& spec = assignmentChannel(profile, data["channelId"].toString());
            group->setText(1, online ? "Connected" : "Offline");
            group->setText(2, QString("0 / %1").arg(spec.fixedLeds ? spec.fixedLeds : spec.maxLeds));
            if(data==selectedHeader_) components_->setCurrentItem(group);
            for(const auto& assignment:document_.state().assignments)
                if(assignment.profileId==data["profileId"] && assignment.endpoint==data["path"] && assignment.channelId==data["channelId"]) {
                    group->setData(0,Qt::UserRole,assignment.id);
                    group->setText(2,QString("%1 / %2").arg(document_.outputLedsUsed(assignment)).arg(spec.fixedLeds ? spec.fixedLeds : spec.maxLeds));
                    for(int i=0;i<assignment.placements.size();++i)
                        for(const auto& placement:document_.state().placements) if(placement.id==assignment.placements[i]) {
                            assigned.insert(placement.id);
                            const bool standalone=(profile.driverId=="lighting.peripherals" || profile.driverId=="lighting.auxiliary") && assignment.placements.size()==1;
                            auto* child=standalone ? group : new QTreeWidgetItem(group);
                            child->setText(0,document_.componentName(placement));
                            child->setData(0,Qt::UserRole,placement.id);
                            child->setText(1,standalone ? (online ? "Connected" : "Offline") : hardwareManufacturer(document_.catalog()[placement.componentId]));
                            if (!standalone) child->setFlags(child->flags()|Qt::ItemIsDragEnabled);
                            const int count=document_.catalog()[placement.componentId].addresses(), start=ledOffsets.value(placement.id,-1);
                            child->setText(2,standalone || start<0 ? QString::number(count) : QString("%1-%2").arg(start+1).arg(start+count));
                            if(placement.id==layoutCanvas_->selection()) components_->setCurrentItem(child);
                        }
                }
        }
        QTreeWidgetItem* unassigned=nullptr;
        for(const auto& placement:document_.state().placements) if(!assigned.contains(placement.id)) {
            if(!unassigned) { unassigned=new QTreeWidgetItem(components_,{"Unassigned components"}); unassigned->setExpanded(true); }
            auto* child=new QTreeWidgetItem(unassigned,{document_.componentName(placement)});
            child->setData(0,Qt::UserRole,placement.id);
            child->setFlags(child->flags()|Qt::ItemIsDragEnabled);
            child->setText(1,hardwareManufacturer(document_.catalog()[placement.componentId]));
            if(placement.id==layoutCanvas_->selection()) components_->setCurrentItem(child);
        }
        if(!headers.contains(selectedHeader_)) selectedHeader_.clear();
        if(selectedHeader_.isEmpty() && !headers.isEmpty()) { selectedHeader_=headers.first(); components_->setCurrentItem(components_->topLevelItem(0)); }
    }
    findChild<QTreeView*>("hardwareComponents")->expandAll();
    refreshHeader();
    {
        const QSignalBlocker block(previewChannel_); const auto before=previewChannel_->currentData();
        previewChannel_->clear(); previewChannel_->addItem("Preview: All outputs",QUuid{});
        for(const auto& assignment:document_.state().assignments) if(assignment.software)
            previewChannel_->addItem("Preview: "+assignment.channelId+" / "+assignment.endpoint,assignment.id);
        const int index=previewChannel_->findData(before); previewChannel_->setCurrentIndex(index<0 ? 0 : index);
    }
    QSignalBlocker blockTree(layers_);
    layers_->clear();
    const EffectLayer* selected = nullptr;
    QMap<QUuid,QTreeWidgetItem*> items;
    for (auto it = document_.state().layers.crbegin(); it != document_.state().layers.crend(); ++it) {
        auto* item = new QTreeWidgetItem;
        items.insert(it->id,item);
        if(it->parentId.isNull())layers_->addTopLevelItem(item);
        item->setText(0, it->name);
        item->setData(0,Qt::UserRole+1,it->container ? "Group" : effectDefinition(it->effect).label);
        item->setToolTip(0,(it->locked ? QString("Locked\n") : QString{})+it->name+(it->container ? " - Group: shared mask, opacity and blend for its children" : " - Effect: "+effectDefinition(it->effect).label)+"\nClick the eye or press Space to show/hide this layer.");
        item->setData(0,Qt::AccessibleDescriptionRole,"Effect: "+effectDefinition(it->effect).label);
        item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsDragEnabled);
        if(!it->parentId.isNull())item->setFlags(item->flags() & ~Qt::ItemIsDropEnabled);
        item->setCheckState(0, it->visible ? Qt::Checked : Qt::Unchecked);
        item->setData(0, Qt::UserRole, it->id);
        item->setIcon(1, QIcon(QPixmap::fromImage(renderEffect(*it).scaled(40, 25, Qt::KeepAspectRatio, Qt::SmoothTransformation))));
        item->setIcon(2, QIcon(QPixmap::fromImage(it->mask.scaled(40, 25))));
        item->setToolTip(1, "Select effect: " + effectDefinition(it->effect).label); item->setToolTip(2, "Select grayscale mask");
        if (it->id == selectedLayer_) selected = &*it;
    }
    for(auto it=document_.state().layers.crbegin();it!=document_.state().layers.crend();++it)if(!it->parentId.isNull()) {
        if(auto* parent=items.value(it->parentId)){parent->addChild(items[it->id]);parent->setExpanded(true);}
    }
    if(selected)layers_->setCurrentItem(items.value(selectedLayer_),editingMask_ ? 2 : 1);
    if (!selected && (!selectedLayer_.isNull() || activeLayer_.isNull())) {
        selectedLayer_=document_.state().layers.isEmpty() ? QUuid{} : document_.state().layers.back().id;
        activeLayer_=document_.layerRoot(selectedLayer_);
        if(!selectedLayer_.isNull()) layers_->setCurrentItem(layers_->topLevelItem(0),editingMask_ ? 2 : 1);
    }
    refreshProperties();
    syncSelectionTrees();
    updatePreview();
}

void LightingWindow::installOutputPanel(QWidget* panel)
{
    deviceOutput_->insertWidget(deviceOutput_->count()-1,panel,2);
}

namespace {
// Compact status instead of a second link into Devices (owner request 2026-11-17): green = active,
// gray = configured but not active, red = errors. Other states (portable mode) stay as text.
QString outputTrafficLight(const QString& state)
{
    // "faulted" counts channels with an error; service-wide errors are in "errors" only.
    const auto match=QRegularExpression("^(\\d+) active / (\\d+) configured / (\\d+) errors(?: / (\\d+) faulted)?$").match(state);
    if(!match.hasMatch())return "Devices: "+state.toHtmlEscaped();
    const int active=match.captured(1).toInt(),configured=match.captured(2).toInt(),errors=match.captured(3).toInt();
    const int faulted=match.captured(4).isEmpty() ? errors : match.captured(4).toInt();
    const auto dot=[](const char* color,int count){return QString("<span style=\"color:%1\">&#9679;</span> %2").arg(color).arg(count);};
    return dot("#3fb950",active)+"&nbsp;&nbsp;"+dot("#8b949e",std::max(0,configured-active-faulted))+"&nbsp;&nbsp;"+dot("#f85149",errors);
}
}

void LightingWindow::setOutputState(const QString& state)
{
    auto* action=findChild<QAction*>("reviewOutput"); action->setText("Output: "+state);
    // Shown once: the status pill and its tooltip. It was repeated in the tooltip and as a Devices line
    // (owner review 2026-11-26: "redundant"); the service panel keeps its own per-output list.
    const auto explanation=state=="Running" ? QString("Canvas edits and playback update the lights live.") :
        state=="Stopped" ? QString("No colors are sent. Resume output in Devices.") : QString();
    const auto summary="Output: "+state+(explanation.isEmpty() ? QString() : "\n"+explanation);
    action->setToolTip(summary);
    auto labels=findChildren<QLabel*>("fullscreenOutputStatus");labels.prepend(findChild<QLabel*>("toolbarOutputStatus"));
    for(auto* label:labels) {
        label->setText(outputTrafficLight(state));label->setToolTip(summary);label->setProperty("outputState",state.contains(" errors") ? "faulted" : state.contains(" active") ? (state.startsWith("0 active") ? "stopped" : "running") : state.toLower());
        label->style()->unpolish(label);label->style()->polish(label);
    }
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
    layoutCanvas_->setPreviewFrame(displayed,seconds,channel); effectsCanvas_->setPreviewFrame(displayed,localTime,channel,seconds); hardwarePreview_->setPreviewFrame(previewFrame_,seconds,channel);
    if (tabs_->currentIndex()==2) {
        auto* thumbnail=findChild<QLabel*>("deviceComponentPreview");
        bool shown=false;
        for (const auto& placement : document_.state().placements) if (placement.id==layoutCanvas_->selection()) {
            const auto& definition=document_.catalog()[placement.componentId];
            const auto colors=document_.previewComponentColors(previewFrame_,seconds).value(placement.id);
            // shortcut: re-rendered on the next preview tick only, so a resize while paused keeps the old size.
            const QSize area=thumbnail->size().expandedTo({176,110});
            const qreal ratio=thumbnail->devicePixelRatioF();
            QPixmap image(area*ratio);
            image.setDevicePixelRatio(ratio);
            image.fill(Qt::transparent);
            QPainter painter(&image);
            painter.setRenderHint(QPainter::Antialiasing);
            const auto photo=hardwarePreviewPhoto(definition);
            auto bounds=hardwareBodyBounds(definition);
            if (!photo.isNull()) {
                // Keep the photo's aspect ratio: grow the LED body frame instead of stretching the photo
                // into it (owner report 2026-11-25: 240 mm AiO photo squeezed into a square LED frame).
                auto size=QSizeF(photo.size()); size.scale(bounds.size(),Qt::KeepAspectRatioByExpanding);
                bounds=QRectF(bounds.center()-QPointF(size.width()/2,size.height()/2),size);
            }
            const double scale=qMin((area.width()-16.0)/qMax(.01,bounds.width()),(area.height()-16.0)/qMax(.01,bounds.height()));
            const double dot=qMax(3.5,3.5*qMin(area.width()/176.0,area.height()/110.0));
            painter.translate(area.width()/2.0,area.height()/2.0);
            painter.scale(scale,scale);
            painter.translate(-bounds.center());
            if (photo.isNull()) drawHardwareComponentArtwork(painter,definition);
            else {
                painter.setRenderHint(QPainter::SmoothPixmapTransform);
                painter.drawPixmap(bounds,photo,photo.rect());
            }
            painter.setPen(Qt::NoPen);
            for (int i=0;i<definition.positions.size();++i) {
                const int address=definition.mapping.at(i);
                if (definition.nonPhysicalSlots.contains(address)) continue;
                painter.setBrush(colors.value(address,Qt::black));
                painter.drawEllipse(definition.positions[i],dot/scale,dot/scale);
            }
            painter.end();
            thumbnail->setPixmap(image);
            shown=true;
            break;
        }
        if (!shown) thumbnail->clear();
    }
    emit previewFrameChanged(previewFrame_);
}

void LightingWindow::flashComponent(const QUuid& placement)
{
    // Hardware receives the flash only while output runs (OutputPanel::dispatch); the preview always shows it.
    document_.setIdentifyPlacement(placement);
    const auto offsets=document_.componentLedOffsets();
    QStringList shared;
    for (const auto& connector:document_.state().assignments)
        if (connector.software && connector.placements.contains(placement)) {
            const auto& instances=document_.state().placements;
            const auto selected=std::find_if(instances.cbegin(),instances.cend(),
                [&](const auto& instance){return instance.id==placement;});
            if (selected==instances.cend()) break;
            const auto start=offsets.value(placement);
            const auto end=start+document_.catalog().value(selected->componentId).addresses();
            for (const auto& instance:instances) {
                if (instance.id==placement || !connector.placements.contains(instance.id)) continue;
                const auto otherStart=offsets.value(instance.id);
                const auto otherEnd=otherStart+document_.catalog().value(instance.componentId).addresses();
                if (otherStart<end && start<otherEnd) shared.append(document_.componentName(instance));
            }
            break;
        }
    if (!shared.isEmpty()) statusBar()->showMessage("These LED addresses are also displayed by "+shared.join(", "),2000);
    updatePreview();
    emit identifyRequested(placement);
    const auto generation = ++identifyGeneration_;
    QTimer::singleShot(2000, this, [this, generation] {
        if (generation != identifyGeneration_) return;
        document_.setIdentifyPlacement({});
        updatePreview();
    });
}

void LightingWindow::refreshToolOptions()
{
    auto* options=findChild<QToolBar*>("drawingOptions"); if(!options) return;
    const EffectLayer* selected=nullptr;
    for(const auto& layer:document_.state().layers) if(layer.id==selectedLayer_) selected=&layer;
    const auto tool=tabs_->currentIndex()==0 ? layoutCanvas_->tool() : effectsCanvas_->tool();
    if(auto* button=findChild<QButtonGroup*>("drawingToolGroup")->button(int(tool)))button->setChecked(true);
    const bool panning=tool==LightingCanvas::Tool::Pan;
    const bool hasSelection=!(tabs_->currentIndex()==0 ? layoutCanvas_ : effectsCanvas_)->selections().isEmpty() || (tabs_->currentIndex()==1 && selected && selected->container && !selected->locked);
    for(const auto& name:{"duplicateSelection","deleteSelection","lockSelection"})if(auto* action=findChild<QAction*>(name))action->setEnabled(tabs_->currentIndex()<2 && hasSelection);
    // A locked object is not an editable selection, but it is exactly what Unlock acts on (CodeRabbit review 2026-11-18).
    if(auto* unlock=findChild<QAction*>("unlockSelection"))unlock->setEnabled(tabs_->currentIndex()<2 &&
        (hasSelection || (tabs_->currentIndex()==0 ? layoutCanvas_ : effectsCanvas_)->selectionHasLocked()));
    const int selectionCount=(tabs_->currentIndex()==0 ? layoutCanvas_ : effectsCanvas_)->selections().size();
    for(auto* action:findChildren<QAction*>())if(action->objectName().startsWith("align_"))
        action->setEnabled(selectionCount>=(action->objectName().contains("distribute") ? 3 : 2));
    const bool editing=tabs_->currentIndex()==1;
    const bool drawing=editing && !panning && tool!=LightingCanvas::Tool::Move && tool!=LightingCanvas::Tool::Eraser && (tool!=LightingCanvas::Tool::Gradient || !selected || !isGradient(selected->effect));
    const bool color=!editingMask_ && drawing;
    const bool shape=editing && !panning && (tool==LightingCanvas::Tool::Rectangle || tool==LightingCanvas::Tool::Ellipse);
    const bool brush=tool==LightingCanvas::Tool::Pencil || tool==LightingCanvas::Tool::Eraser;
    const bool stroke=editing && !panning && (brush || tool==LightingCanvas::Tool::Line || shape);
    color_->setVisible(color);color_->setEnabled(color);
    for(const auto* name:{"foregroundColorAction","shapeFillAction","strokeLabelAction","strokeSizeAction"})
        findChild<QAction*>(name)->setVisible(editing);
    findChild<QCheckBox*>("shapeFill")->setVisible(shape);
    auto* strokeLabel=findChild<QLabel*>("brushWidthLabel");
    strokeLabel->setText(brush ? " Size " : " Width ");
    strokeLabel->setVisible(stroke);
    findChild<QWidget*>("brushWidthField")->setVisible(stroke);
    // Pan is a real tool: keep help and brush visibility aligned; bounds-check future tool additions.
    const QStringList hints{"Move: select and drag an object", "Pencil: draw on the canvas", "Line (I): drag from start to end", "Rectangle: drag to draw", "Ellipse: drag to draw", "Gradient (G): drag to create; existing grips edit the selected gradient", "Fill: click a connected area", "Eraser: remove pixels or hide the source mask", "Pan (H): drag to move the canvas; Space temporarily pans from any tool"};
    auto* toolHint=findChild<QLabel*>("toolHint");
    const QString hint=editing || panning ? hints.value(int(tool),"Choose a tool") : "Drag to move · Shift adds · hold L for labels";
    toolHint->setText(toolHint->fontMetrics().elidedText(hint,Qt::ElideRight,toolHint->width()-8));toolHint->setToolTip(hint);
    if(auto* summary=findChild<QLabel*>("maskBrushSummary")) {
        const double visibility=findChild<QDoubleSpinBox*>("maskPaintValue")->value();
        const int percent=qRound(visibility*100./255);
        const bool pencil=editing && editingMask_ && !panning && tool==LightingCanvas::Tool::Pencil;
        findChild<QPushButton*>("maskHideBrush")->setChecked(pencil && visibility==0);
        findChild<QPushButton*>("maskRevealBrush")->setChecked(pencil && visibility==255);
        // Mask coverage and freehand strength are consumed by different tools.
        findChild<QDoubleSpinBox*>("maskBrushHardness")->parentWidget()->setEnabled(brush && !panning);
        findChild<QDoubleSpinBox*>("maskBrushOpacity")->parentWidget()->setEnabled(brush && !panning);
        findChild<QDoubleSpinBox*>("maskPaintValue")->parentWidget()->setEnabled(!panning &&
            (tool==LightingCanvas::Tool::Pencil || tool==LightingCanvas::Tool::Line || shape || tool==LightingCanvas::Tool::Fill));
        summary->setText(tool==LightingCanvas::Tool::Eraser ? "Eraser hides coverage (black, 0%), regardless of Paint visibility." :
            tool==LightingCanvas::Tool::Gradient ? "Gradient paints from hidden (black) to revealed (white)." :
            tool==LightingCanvas::Tool::Move || tool==LightingCanvas::Tool::Pan ? "Move and Pan do not paint mask coverage. Choose a drawing tool to edit visibility." :
            QString("%1 paints %2% visibility. White reveals; black hides. Pencil brush opacity controls stroke strength, not layer opacity.")
                .arg(tool==LightingCanvas::Tool::Fill ? "Fill" : tool==LightingCanvas::Tool::Pencil ? "Pencil" : "This tool").arg(percent));
    }
    // Photoshop-style options keep a stable row so switching tools never shifts the canvas under the pointer.
    options->setVisible(workspaceIndex()<2);
}

void LightingWindow::refreshProperties()
{
    QSignalBlocker blockTransparency(transparency_), blockBlend(blend_), blockMask(maskEnabled_), blockRepeat(repetitions_);
    const EffectLayer* selected = nullptr;
    for (const auto& layer : document_.state().layers) if (layer.id == selectedLayer_) selected = &layer;
    {
        const QSignalBlocker blockAll(allChannels_),blockTargets(channelTargets_);
        allChannels_->setEnabled(selected!=nullptr); const EffectLayer* routing=selected;for(const auto& parent:document_.state().layers)if(selected && parent.id==selected->parentId)routing=&parent;
        allChannels_->setChecked(routing && routing->allChannels);
        channelTargets_->clear(); channelTargets_->setEnabled(routing && !routing->allChannels);
        channelTargets_->setVisible(routing && !routing->allChannels);
        allChannels_->setVisible(selected && (!document_.state().assignments.isEmpty() || !selected->allChannels));
        for(const auto& assignment:document_.state().assignments) if(assignment.software) {
            auto* item=new QListWidgetItem(assignment.channelId+" / "+assignment.endpoint,channelTargets_);
            item->setData(Qt::UserRole,assignment.id); item->setFlags(item->flags()|Qt::ItemIsUserCheckable);
            item->setCheckState(selected && routing->channelTargets.contains(assignment.id) ? Qt::Checked : Qt::Unchecked);
        }
    }
    if(!selectedLayer_.isNull())activeLayer_=document_.layerRoot(selectedLayer_);
    if(!activeLayer_.isNull() && document_.layerRoot(activeLayer_).isNull())activeLayer_={};
    effectsCanvas_->setActiveLayer(activeLayer_);
    effectsCanvas_->select(selectedLayer_);
    effectsCanvas_->setMaskTarget(editingMask_);

    for (QWidget* widget : std::initializer_list<QWidget*>{transparency_, blend_, color_, gradientColors_, maskEnabled_})
        widget->setEnabled(selected != nullptr);
    // Editing a procedural source with Eraser selects its mask; that must never lock preview playback.
    color_->setEnabled(tabs_->currentIndex()==1);
    playing_->setEnabled(true);
    findChild<QPushButton*>("saveCustomEffect")->setEnabled(selected!=nullptr);
    findChild<QWidget*>("chaseParameters")->setVisible(selected && !editingMask_ && selected->effect==EffectKind::Chase);
    auto* thumbnail=findChild<QLabel*>("maskThumbnail");
    if (selected) {
        thumbnail->setPixmap(QPixmap::fromImage(selected->mask.scaled(112,70,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
        thumbnail->setAccessibleName(editingMask_ ? "Mask thumbnail, editing" : "Mask thumbnail");
    } else thumbnail->clear();
    inspectorTabs_->setEnabled(selected!=nullptr);inspectorTabs_->setTabEnabled(1,selected && !selected->container);
    inspectorTitle_->setText(selected ? (selected->container ? selected->name : effectDefinition(selected->effect).label) : "Select an object");
    findChild<QLabel*>("layerEditingContext")->setText(!selected ? "No layer selected" :
        (editingMask_ ? "Editing: mask visibility - " : selected->container ? "Editing group: " : "Editing: effect source - ")+selected->name);
    if(auto* summary=findChild<QLabel*>("maskScopeSummary")) {
        QString scope="Select a layer or effect to edit its mask.";
        if(selected) {
            bool children=false,parent=false;QString path=selected->name;
            for(const auto& item:document_.state().layers) {
                children|=item.parentId==selected->id;
                if(item.id==selected->parentId){parent=true;path=item.name+" > "+path;}
            }
            scope="Mask target: "+path+"\n"+(selected->container || children ?
                "Affects the combined layer, including its child effects." : parent ?
                "Affects only this effect; its parent layer mask also applies." : "Affects only this effect.");
            scope+=selected->maskEnabled ? "\nApplied to output. Source pixels are preserved." :
                "\nBypassed: mask edits are saved but do not change output. Enable Apply mask to output to use them.";
            maskLinked_->setToolTip(selected->container || children ?
                "Follow this layer's own movement and animation, not the motion of its children. This does not link to another layer." :
                "Follow this effect when moved, resized, rotated or animated. Off keeps the mask in canvas coordinates. This does not link to another layer.");
        }
        summary->setText(scope);
    }
    bool insertionLocked=selected && selected->locked;
    if(selected)for(const auto& parent:document_.state().layers)if(parent.id==selected->parentId)insertionLocked|=parent.locked;
    for(auto* action:findChildren<QAction*>())if(action->objectName().startsWith("create_") || action->objectName()=="newPaintLayer" || action->objectName()=="importImageAction")action->setEnabled(!insertionLocked);
    findChild<QPushButton*>("newLayer")->setEnabled(!insertionLocked);findChild<QToolButton*>("addLayer")->setEnabled(!insertionLocked);
    const int selectedIndex=selected ? int(selected-document_.state().layers.constData()) : -1;
    findChild<QAction*>("moveLayerUp")->setEnabled(tabs_->currentIndex()==1 && selectedIndex>=0 && selectedIndex+1<document_.state().layers.size());
    findChild<QAction*>("moveLayerDown")->setEnabled(tabs_->currentIndex()==1 && selectedIndex>0);
    animationControls_->setVisible(selected && !editingMask_);
    findChild<QWidget*>("particleParameters")->setVisible(selected && isParticle(selected->effect));
    findChild<QPushButton*>("resetCrop")->setVisible(selected && !editingMask_ && selected->effect==EffectKind::Image);
    findChild<QWidget*>("cropSection")->setVisible(selected && !editingMask_ && selected->effect == EffectKind::Image);
    maskLinked_->setEnabled(selected != nullptr);
    auto* cropTool=findChild<QCheckBox*>("cropTool");
    const bool canCrop=tabs_->currentIndex()==1 && selected && !insertionLocked && !editingMask_ && selected->effect==EffectKind::Image && !selected->image.isNull();
    if(!canCrop && cropTool->isChecked())cropTool->setChecked(false);
    cropTool->setEnabled(canCrop);
    findChild<QToolButton*>("cropImageTool")->setEnabled(cropTool->isEnabled());
    findChild<QWidget*>("patternParameters")->setVisible(selected && !editingMask_ && isPattern(selected->effect));
    findChild<QLabel*>("effectDescription")->setVisible(selected && !editingMask_);
    sourceColor_->setVisible(!editingMask_ && selected && selected->effect!=EffectKind::Paint && selected->effect!=EffectKind::Image && !isGradient(selected->effect) && !isParticle(selected->effect));
    gradientColors_->setVisible(selected && !editingMask_ && (isGradient(selected->effect) || isParticle(selected->effect) || selected->gradientFill || effectsCanvas_->tool()==LightingCanvas::Tool::Gradient));
    auto* shapeFillMode=findChild<QComboBox*>("shapeFillMode");
    shapeFillMode->setVisible(selected && !editingMask_ && isShape(selected->effect));
    {const QSignalBlocker block(shapeFillMode);shapeFillMode->setCurrentIndex(selected && selected->gradientFill ? 1 : 0);}
    imageFit_->setVisible(selected && !editingMask_ && selected->effect == EffectKind::Image);
    secondaryColor_->hide();
    findChild<QLabel*>("particlePaletteLabel")->setVisible(selected && !editingMask_ && isParticle(selected->effect));
    findChild<QWidget*>("geometrySection")->setVisible(selected && !selected->container && !editingMask_ && selected->effect != EffectKind::StaticColor && selected->effect != EffectKind::Chase);
    findChild<QWidget*>("effectRepetitionControls")->setVisible(selected && !editingMask_ && (selected->effect==EffectKind::Stripes || selected->effect==EffectKind::Waves || selected->effect==EffectKind::Pinwheel || selected->effect==EffectKind::Spiral || selected->effect==EffectKind::SineWave || selected->effect==EffectKind::Dna || (selected->effect==EffectKind::Radar || selected->effect==EffectKind::Ripples || selected->effect==EffectKind::Kaleidoscope)));
    if (selected) {
        findChild<QLabel*>("effectDescription")->setText(effectDefinition(selected->effect).description);
        transparency_->setValue(100-selected->transparency);
        {auto* value=findChild<QSpinBox*>("transparencyValue");const QSignalBlocker block(value);value->setValue(100-selected->transparency);}
        transparency_->setToolTip(QString::number(100-selected->transparency) + "% opaque. 100% is fully visible; 0% is hidden. The mask additionally controls local coverage.");
        blend_->setCurrentIndex(static_cast<int>(selected->blend));
        const double rates[]{selected->spin, selected->pulse, selected->hueShift};
        for (int i = 0; i < 3; ++i) {
            const QSignalBlocker block(animation_[i]);
            setEffectControlRange(animation_[i],i==1 ? 0 : -90,i==1 ? 1 : 90,rates[i]);
        }
        {const QSignalBlocker block(findChild<QComboBox*>("intensityMode"));findChild<QComboBox*>("intensityMode")->setCurrentIndex(selected->pulse>0 ? (selected->breathe>0 ? 3 : 1) : selected->breathe>0 ? 2 : 0);}
        animation_[0]->setEnabled(selected->effect != EffectKind::StaticColor);
        animation_[2]->setEnabled(selected->effect!=EffectKind::Image && selected->effect!=EffectKind::Paint);
        const QSignalBlocker blockLink(maskLinked_), blockCycle(colorCycle_);
        maskLinked_->setChecked(selected->maskLinked);
        {auto* clip=findChild<QCheckBox*>("clipToFrame");const QSignalBlocker block(clip);clip->setChecked(selected->clipToFrame);clip->setVisible(isGradient(selected->effect));}
        colorCycle_->setStops(selected->colorCycle,selected->colorCyclePositions);
        const auto stops=selected->gradientColors.isEmpty() ? QVector<QColor>{selected->color,selected->secondaryColor} : selected->gradientColors;
        gradientColors_->setStops(stops,selected->gradientPositions); effectsCanvas_->setGradientStops(stops,selected->gradientPositions);
        colorCycle_->setEnabled(selected->effect != EffectKind::Image);
        const auto& parameters = advancedParameters();
        for (int i=0;i<advanced_.size();++i) {
            const QSignalBlocker block(advanced_[i]);
            const auto range=practicalEffectRange(parameters[i],selected->effect);
            setEffectControlRange(advanced_[i],range.first,range.second,selected->*(parameters[i].member));
            const bool particle = parameters[i].id.startsWith("particle");
            const auto id=parameters[i].id;
            const auto& defaults=effectDefinition(selected->effect);
            if(id=="patternSpeed") {
                const bool radar=selected->effect==EffectKind::Radar,voronoi=selected->effect==EffectKind::Voronoi,fire=selected->effect==EffectKind::Fireplace;
                const QString label=radar ? "Rotation speed" : voronoi ? "Cell motion speed" : fire ? "Flame motion speed" : "Pattern speed";
                const QString help=radar ? "Revolutions per minute. Positive: clockwise; negative: counterclockwise; zero: still." : voronoi ? "Cell movement cycles per minute. Negative reverses movement; zero freezes cells." : fire ? "Flame motion cycles per minute. Negative reverses the flow; zero freezes flames." : "Pattern cycles per minute. Negative reverses motion; zero freezes it.";
                advanced_[i]->setToolTip(help);
                if(auto* field=qobject_cast<QWidget*>(advanced_[i]->property("visibilityContainer").value<QObject*>())) {field->setToolTip(help);for(auto* slider:field->findChildren<QSlider*>())slider->setToolTip(help);}
                if(auto* labelWidget=qobject_cast<QLabel*>(advanced_[i]->property("parameterLabel").value<QObject*>())) {labelWidget->setText(label);labelWidget->setToolTip(help);}
            }
            advanced_[i]->setProperty("defaultValue",id=="patternSpeed" ? defaults.patternSpeed : id=="particleCount" ? defaults.particleCount : id=="patternSites" ? defaults.patternSites : EffectLayer{}.*(parameters[i].member));
            if(id=="particleCount" || id=="particleSpeed" || id=="particleSize") {
                const QString label=selected->effect==EffectKind::Lightning ? (id=="particleCount" ? "Bolt count" : id=="particleSpeed" ? "Flash rate" : "Bolt width") : parameters[i].label;
                if(auto* widget=qobject_cast<QLabel*>(advanced_[i]->property("parameterLabel").value<QObject*>()))widget->setText(label);
            }
            if(id=="patternDetail" || id=="patternSites") {
                const QString text=id=="patternSites" ? (selected->effect==EffectKind::LavaLamp ? "Blob count" : "Sites") : (selected->effect==EffectKind::Aurora ? "Texture scale" : "Flame detail");
                const QString help=id=="patternSites" ? (selected->effect==EffectKind::LavaLamp ? "Number of merging blobs. Higher counts fill more of the source." : "Number of moving Voronoi sites. More sites produce smaller cells.") : (selected->effect==EffectKind::Aurora ? "Noise texture scale along the light curtain. Higher values create finer folds." : "Flame texture density, 1 to 16. Higher values create more, smaller tongues; this does not increase height.");
                advanced_[i]->setToolTip(help);
                if(auto* label=qobject_cast<QLabel*>(advanced_[i]->property("parameterLabel").value<QObject*>())){label->setText(text);label->setToolTip(help);}
                if(auto* field=qobject_cast<QWidget*>(advanced_[i]->property("visibilityContainer").value<QObject*>())){field->setToolTip(help);for(auto* slider:field->findChildren<QSlider*>())slider->setToolTip(help);}
            }
            bool show = !particle || isParticle(selected->effect);
            if(id.startsWith("pattern") || id=="fillOrder") {
                const bool wave=selected->effect==EffectKind::SineWave || selected->effect==EffectKind::Dna;
                const bool voronoi=selected->effect==EffectKind::Voronoi,fire=selected->effect==EffectKind::Fireplace;
                const bool aurora=selected->effect==EffectKind::Aurora,lava=selected->effect==EffectKind::LavaLamp,checker=selected->effect==EffectKind::Checkerboard;
                show=isPattern(selected->effect) && (id=="patternSpeed" || (id=="patternWidth" && (wave || aurora || selected->effect==EffectKind::Visor || selected->effect==EffectKind::Radar)) ||
                    (id=="patternAmplitude" && (wave || aurora)) || (id=="patternWavelength" && selected->effect==EffectKind::Dna) || ((id=="patternFrequency" || id=="patternContrast" || id=="patternWarp") && (selected->effect==EffectKind::Plasma || selected->effect==EffectKind::Kaleidoscope)) || ((id=="patternCells" && checker) || ((id=="patternCells" || id=="fillOrder") && selected->effect==EffectKind::PixelFill)) ||
                    (id=="patternSeed" && (voronoi || fire || aurora || lava)) || ((id=="patternSites" && lava) || ((id=="patternSites" || id=="patternDistance") && voronoi)) ||
                    ((id=="patternDetail" && aurora) || ((id=="patternDetail" || id=="patternHeight") && fire)));
            }
            // Proxy fields feed the shared parameter/history path but have no layout slot.
            advanced_[i]->setVisible(show && id!="fillOrder" && id!="patternDistance" && id!="chaseReverse");
            if(auto* field=qobject_cast<QWidget*>(advanced_[i]->property("visibilityContainer").value<QObject*>())) field->setVisible(show);
            if(auto* label=qobject_cast<QWidget*>(advanced_[i]->property("parameterLabel").value<QObject*>())) label->setVisible(show);
            advanced_[i]->setEnabled((!particle || isParticle(selected->effect)) &&
                (parameters[i].id != "cycleRate" || selected->effect != EffectKind::Image));
        }
        {auto* order=findChild<QComboBox*>("patternFillOrder");const QSignalBlocker block(order);order->setCurrentIndex(int(selected->fillOrder));}
        {auto* metric=findChild<QComboBox*>("patternDistanceMode");const QSignalBlocker block(metric);metric->setCurrentIndex(int(selected->patternDistance));}
        {auto* reverse=findChild<QCheckBox*>("chaseReverseToggle");const QSignalBlocker block(reverse);reverse->setChecked(selected->chaseReverse!=0);}
        const double crop[]{selected->crop.x(),selected->crop.y(),selected->crop.width(),selected->crop.height()};
        for (int i=0;i<4;++i) { const QSignalBlocker block(crop_[i]);
            static_cast<ParameterSpinBox*>(crop_[i])->setDisplayUnit(i%2==0 ? std::max(1,selected->image.width()) : std::max(1,selected->image.height())," px");crop_[i]->setValue(crop[i]); }
        const QSignalBlocker blockFit(imageFit_);
        imageFit_->setCurrentIndex(selected->imageFill ? 1 : 0);
        const bool brush=effectsCanvas_->tool()!=LightingCanvas::Tool::Move || selected->effect==EffectKind::Paint;
        color_->setColor(brush ? foreground_ : selected->color);
        effectsCanvas_->setDrawing(brush ? foreground_ : selected->color,findChild<QDoubleSpinBox*>("brushDiameter")->value(),findChild<QCheckBox*>("shapeFill")->isChecked());
        maskEnabled_->setChecked(selected->maskEnabled);
        sourceColor_->setColor(selected->color);secondaryColor_->setColor(selected->secondaryColor);
        // Repetitions share the persisted 1..64 contract across gradients and native patterns.
        const int maximumRepeats=64;
        setEffectControlRange(repetitions_,1,maximumRepeats,selected->repetitions);
        const QVector<double> values{selected->center.x(), selected->center.y(), selected->extent.width(), selected->extent.height(), selected->rotation};
        for (int i = 0; i < geometry_.size(); ++i) {
            const QSignalBlocker block(geometry_[i]);
            geometry_[i]->setValue(values[i]);
        }
        geometry_[3]->setEnabled(selected->effect != EffectKind::LinearGradient &&
            selected->effect != EffectKind::Stripes && selected->effect != EffectKind::Waves);
    }
    if(selected){animation_[1]->setEnabled(selected->pulse>0);for(auto* field:advanced_)if(field->objectName()=="breathe")field->setEnabled(selected->breathe>0);}
    // Effect controls show plain numbers; scale conversion remains in the existing numeric contract.
    for(auto* field:inspectorTabs_->findChildren<QDoubleSpinBox*>()) field->setSuffix({});
    for(auto* field:inspectorTabs_->findChildren<QSpinBox*>()) field->setSuffix({});
    syncSliderFields(this);
    refreshToolOptions();
}

void LightingWindow::closeEvent(QCloseEvent* event)
{
    if (componentPage_ && applicationPages_->currentWidget()==componentPage_ && !componentPage_->confirmLeave()) { event->ignore(); return; }
    if (!confirmDiscard()) { event->ignore(); return; }
    emit closing();
    event->accept();
}

void LightingWindow::changeEvent(QEvent* event)
{
    QMainWindow::changeEvent(event);
    if (event->type()==QEvent::WindowStateChange && previewStale_ && !isMinimized()) { previewStale_=false; updatePreview(); }
}

void LightingWindow::loadProject(const QString& path, bool recovered, const QString& original, const QJsonArray& playbackDefinitions)
{
    emit projectLoading();
    document_.load(path,playbackDefinitions); fileName_ = recovered ? original : path;
    refreshProjectName();
    if (recovered) document_.history()->resetClean();
    findChild<QAction*>("workspaceEffects")->trigger();
}

QString withProjectSuffix(const QString& path)
{
    if (path.endsWith(".lighting.json", Qt::CaseInsensitive)) return path;
    return (path.endsWith(".json", Qt::CaseInsensitive) ? path.chopped(5) : path) + ".lighting.json";
}

bool LightingWindow::saveDocument()
{
    auto path = fileName_;
    if (path.isEmpty()) {
        // A bare file name resolved to the install folder, which is read-only without admin rights.
        const auto chosen = QFileDialog::getSaveFileName(this, "Save lighting document",
            QDir::homePath() + "/Untitled.lighting.json", "Lighting documents (*.lighting.json)");
        if (chosen.isEmpty()) return false;
        path = withProjectSuffix(chosen);
        // The dialog confirmed overwriting only the name as typed, not the suffixed one.
        if (path != chosen && QFileInfo::exists(path) && QMessageBox::question(this, "Replace file",
                QFileInfo(path).fileName() + " already exists. Replace it?") != QMessageBox::Yes) return false;
    }
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
    setWindowTitle(name+QString("[*] — Lighting Studio [%1]").arg(studioBuildId));
}

void LightingWindow::removeSelectedComponents()
{
    const auto selected=layoutCanvas_->selection();QSet<QUuid> ids=layoutCanvas_->selections();
    for(const auto& placement:document_.state().placements) if(placement.id==selected) ids.insert(selected);
    for(const auto& assignment:document_.state().assignments) if(assignment.id==selected)
        for(const auto& id:assignment.placements) ids.insert(id);
    for(const auto& item:document_.state().placements)if(item.locked)ids.remove(item.id);
    if(ids.isEmpty()) return;
    try {
        document_.removeComponents(ids);
        layoutCanvas_->select({});
    } catch(const std::exception& error) {statusBar()->showMessage("Cannot remove component: "+QString::fromUtf8(error.what()),6000);}
}
bool LightingWindow::eventFilter(QObject* watched,QEvent* event)
{
    if(event->type()==QEvent::KeyPress && watched==findChild<QLineEdit*>("effectLibrarySearch") && static_cast<QKeyEvent*>(event)->key()==Qt::Key_Down) {
        auto* results=findChild<QListWidget*>("effectLibraryResults");
        if(results->isVisible()) results->setFocus();
        else {
            auto* menu=findChild<QMenu*>("effectLibraryMenu");
            for (auto* action:menu->actions()) if(action->isVisible() && action->menu()) { menu->setActiveAction(action); break; }
            menu->setFocus();
        }
        return true;
    }

    auto* widget=qobject_cast<QWidget*>(watched);
    const auto typing=[](QWidget* target) {
        for(auto* parent=target;parent;parent=parent->parentWidget())
            if(qobject_cast<QLineEdit*>(parent) || qobject_cast<QAbstractSpinBox*>(parent) ||
               qobject_cast<QTextEdit*>(parent) || qobject_cast<QPlainTextEdit*>(parent))return true;
        return false;
    };
    // QWidget ancestry stops at top-level windows; the reparented fullscreen canvas is also an owned view.
    const bool owned=widget && (widget->window()==this || widget->window()==layoutCanvas_->window());
    if(labelsHeld_ && (event->type()==QEvent::ApplicationDeactivate ||
       (owned && (event->type()==QEvent::WindowDeactivate || (event->type()==QEvent::FocusIn && typing(widget)) ||
                  (event->type()==QEvent::Hide && (widget==this || widget==layoutCanvas_))))))setLabelsHeld(false);
    if(event->type()==QEvent::KeyRelease && static_cast<QKeyEvent*>(event)->key()==Qt::Key_L && labelsHeld_) {
        if(!static_cast<QKeyEvent*>(event)->isAutoRepeat())setLabelsHeld(false);
        return true;
    }
    if(owned && (widget->window()==layoutCanvas_->window() || widget->window()==effectsCanvas_->window()) && tabs_->currentIndex()!=2 && (layoutCanvas_->isVisible() || effectsCanvas_->isVisible()) && event->type()==QEvent::KeyPress) {
        const auto* key=static_cast<QKeyEvent*>(event);
        // L is a momentary view override, never a text shortcut or a change to the saved eye preference.
        if(key->key()==Qt::Key_L && key->modifiers()==Qt::NoModifier && !typing(widget) && !typing(QApplication::focusWidget())) {
            if(!key->isAutoRepeat())setLabelsHeld(true);
            return true;
        }
    }
    // The application-wide reveal filter must not broaden the existing Delete scope.
    if(event->type()==QEvent::KeyPress && (watched==layoutCanvas_ || watched==effectsCanvas_ || watched==layers_ || watched==components_ ||
        watched==findChild<QTreeView*>("hardwareComponents"))) {
        const auto* key=static_cast<QKeyEvent*>(event);
        if(key->key()==Qt::Key_Delete && !(key->modifiers() & ~Qt::KeypadModifier)) {
            // Text/spin editors own Delete while typing; never turn a field edit into object deletion.
            const auto* focus=QApplication::focusWidget();
            if(qobject_cast<const QLineEdit*>(focus) || qobject_cast<const QAbstractSpinBox*>(focus)) return QMainWindow::eventFilter(watched,event);
            if(!key->isAutoRepeat()) {
                if(watched==effectsCanvas_ || watched==layers_) effectsCanvas_->deleteSelection();
                else removeSelectedComponents();
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(watched,event);
}
