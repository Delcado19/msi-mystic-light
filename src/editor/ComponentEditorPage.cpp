#include "ComponentEditorPage.h"
#include "ComponentCanvas.h"
#include "StudioWidgets.h"
#include <QAbstractButton>
#include <QButtonGroup>
#include <QGraphicsEllipseItem>
#include <QGridLayout>
#include <QShortcut>
#include <QSpinBox>
#include "HardwareIdentity.h"
#include "LightingDocument.h"
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSaveFile>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QUndoCommand>
#include <QVBoxLayout>
#include <stdexcept>

namespace {
const QStringList categories{"Fans", "Strips", "Mice", "Keyboards", "AiO", "Pumps", "RAM", "Graphics cards", "Motherboards", "Other"};

// One undo step = the whole draft before and after a finished gesture.
class DraftCommand : public QUndoCommand {
public:
    DraftCommand(const QString& label, ComponentDraft before, ComponentDraft after, std::function<void(const ComponentDraft&)> apply)
        : QUndoCommand(label), before_(std::move(before)), after_(std::move(after)), apply_(std::move(apply)) {}
    void redo() override { apply_(after_); }
    void undo() override { apply_(before_); }
private:
    ComponentDraft before_, after_;
    std::function<void(const ComponentDraft&)> apply_;
};

bool deviceCopy(const QString& id) { return id.startsWith("instance.") || id.startsWith("restored."); }

QPushButton* barButton(QHBoxLayout* bar, const QString& text, const QString& name)
{
    auto* button = new QPushButton(text);
    button->setObjectName(name);
    bar->addWidget(button);
    return button;
}
}

ComponentEditorPage::ComponentEditorPage(LightingDocument& document, const ComponentCatalog& builtIn, QWidget* parent)
    : QWidget(parent), document_(document), builtIn_(builtIn)
{
    setObjectName("componentEditorPage");
    confirm = [this](const QString& text) {
        return QMessageBox::question(this, "Component Editor", text) == QMessageBox::Yes;
    };
    askUnsaved = [this](const QString& text) {
        return QMessageBox::question(this, "Component Editor", text,
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    };
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 10, 12, 8); root->setSpacing(8);

    auto* bar = new QHBoxLayout; root->addLayout(bar);
    auto* back = barButton(bar, "Back", "componentBack");
    back->setToolTip("Return to the workspace");
    title_ = new QLabel; title_->setObjectName("componentTitle"); title_->setProperty("heading", true);
    bar->addSpacing(12); bar->addWidget(title_, 1);
    new_ = barButton(bar, "New", "componentNew");
    import_ = barButton(bar, "Import", "componentImport");
    export_ = barButton(bar, "Export", "componentExport");
    delete_ = barButton(bar, "Delete", "componentDelete");
    save_ = barButton(bar, "Save", "componentSave");
    save_->setProperty("primary", true);

    auto* body = new QHBoxLayout; root->addLayout(body, 1); body->setSpacing(8);
    listColumn_ = new QWidget; listColumn_->setFixedWidth(260);
    auto* listLayout = new QVBoxLayout(listColumn_); listLayout->setContentsMargins(0, 0, 0, 0);
    auto* listTitle = new QLabel("COMPONENTS"); listTitle->setProperty("section", true); listLayout->addWidget(listTitle);
    search_ = new QLineEdit; search_->setObjectName("componentSearch"); search_->setPlaceholderText("Search components");
    search_->setClearButtonEnabled(true); listLayout->addWidget(search_);
    list_ = new QTreeWidget; list_->setObjectName("componentList"); list_->setHeaderHidden(true);
    list_->setAccessibleName("Components"); listLayout->addWidget(list_, 1);
    body->addWidget(listColumn_);

    canvas_ = new ComponentCanvas;
    auto* rail = new QWidget; rail->setObjectName("componentToolRail"); rail->setFixedWidth(44);
    body->addWidget(rail);
    body->addWidget(canvas_, 1);

    auto* properties = new QScrollArea; properties->setObjectName("componentProperties");
    properties->setWidgetResizable(true); properties->setFixedWidth(320);
    auto* panel = new QWidget; auto* column = new QVBoxLayout(panel);
    column->setContentsMargins(0, 0, 8, 0); column->setSpacing(6);
    auto* device = section(column, "DEVICE", true);
    auto* form = new QFormLayout(device);
    name_ = new QLineEdit; name_->setObjectName("componentName"); form->addRow("Name", name_);
    manufacturer_ = new QLineEdit; manufacturer_->setObjectName("componentManufacturer");
    manufacturer_->setPlaceholderText("Generic"); form->addRow("Manufacturer", manufacturer_);
    category_ = new QComboBox; category_->setObjectName("componentCategory");
    category_->addItem("Automatic", QString());
    for (const auto& category : categories) category_->addItem(category, category);
    form->addRow("Category", category_);
    identity_ = new QLabel; identity_->setObjectName("componentIdentity");
    identity_->setTextInteractionFlags(Qt::TextSelectableByMouse); identity_->setWordWrap(true);
    form->addRow("ID", identity_);
    buildShapesAndWiring(column);
    column->addStretch();
    properties->setWidget(panel);
    body->addWidget(properties);

    auto* status = new QHBoxLayout; root->addLayout(status);
    message_ = new QLabel; message_->setObjectName("componentMessage"); message_->setWordWrap(true);
    counts_ = new QLabel; counts_->setObjectName("componentStatusLeds");
    cursor_ = new QLabel("grid --, --"); cursor_->setObjectName("componentStatusGrid");
    zoom_ = new QLabel; zoom_->setObjectName("componentStatusZoom");
    hint_ = new QLabel; hint_->setObjectName("componentStatusHint"); hint_->setProperty("secondary", true);
    for (auto* label : {counts_, cursor_, zoom_}) { status->addWidget(label); status->addSpacing(16); }
    status->addWidget(message_, 1);
    status->addWidget(hint_);
    buildToolRail(rail);
    buildShortcuts();
    connect(canvas_, &ComponentCanvas::cursorMoved, this, [this](std::optional<QPointF> point) {
        cursor_->setText(point ? QString("grid %1, %2").arg(point->x(), 0, 'f', 1).arg(point->y(), 0, 'f', 1) : QString("grid --, --"));
    });
    connect(canvas_, &ComponentCanvas::zoomChanged, this, [this](double zoom) { zoom_->setText(QString("zoom %1%").arg(qRound(zoom * 100))); });
    connect(canvas_, &ComponentCanvas::wireOrderFinished, this, [this](const QVector<int>& order) {
        applyEdit(withWireOrder(draft_, order), "Set wire order");
        tools_->button(0)->setChecked(true);
        setMessage("Wire order applied.");
    });

    connect(back, &QPushButton::clicked, this, [this] { if (confirmLeave()) emit closeRequested(); });
    connect(new_, &QPushButton::clicked, this, [this] {
        if (!confirmLeave()) return;
        ComponentDraft fresh; fresh.name = "New component"; fresh.category = "Other";
        draft_ = saved_ = fresh; entry_ = Entry::New; entryId_.clear(); undo_.clear();
        { const QSignalBlocker block(list_); list_->setCurrentItem(nullptr); }
        showDraft(); canvas_->fit();
        setMessage("Add LEDs with a shape or by clicking the canvas, then Save.");
    });
    connect(import_, &QPushButton::clicked, this, [this] {
        const auto file = QFileDialog::getOpenFileName(this, "Import component definition", QDir::homePath(), "Component definitions (*.json)");
        if (file.isEmpty()) return;
        try { document_.importComponent(file); refreshList(entryId_); setMessage("Imported " + QFileInfo(file).fileName() + "."); }
        catch (const std::exception& error) { setMessage(QString::fromUtf8(error.what())); }
    });
    connect(export_, &QPushButton::clicked, this, [this] {
        try {
            const auto definition = draft_.toDefinition();
            const auto path = QFileDialog::getSaveFileName(this, "Export component definition", QDir::homePath(), "Component definitions (*.json)");
            if (path.isEmpty()) return;
            const auto bytes = QJsonDocument(definition.toJson()).toJson();
            QSaveFile file(path);
            if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
                throw std::runtime_error("Could not export component definition");
            setMessage("Exported " + QFileInfo(path).fileName() + ".");
        } catch (const std::exception& error) { setMessage(QString::fromUtf8(error.what())); }
    });
    connect(delete_, &QPushButton::clicked, this, [this] { removeCurrent(); });
    connect(save_, &QPushButton::clicked, this, [this] { save(); });
    connect(search_, &QLineEdit::textChanged, this, [this](const QString& text) {
        for (int group = 0; group < list_->topLevelItemCount(); ++group)
            for (int i = 0; i < list_->topLevelItem(group)->childCount(); ++i) {
                auto* item = list_->topLevelItem(group)->child(i);
                item->setHidden(!item->text(0).contains(text, Qt::CaseInsensitive));
            }
    });
    connect(list_, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem* current, QTreeWidgetItem* previous) {
        if (switching_ || !current || current->data(0, Qt::UserRole).toString().isEmpty()) return;
        const auto revert = [&] { switching_ = true; list_->setCurrentItem(previous); switching_ = false; };
        if (!confirmLeave()) { revert(); return; }
        load(current->data(0, Qt::UserRole).toString(), Entry(current->data(0, Qt::UserRole + 1).toInt()));
    });
    connect(name_, &QLineEdit::editingFinished, this, [this] {
        if (name_->text() == draft_.name) return;
        auto after = draft_; after.name = name_->text(); applyEdit(after, "Rename");
    });
    connect(manufacturer_, &QLineEdit::editingFinished, this, [this] {
        if (manufacturer_->text() == draft_.manufacturer) return;
        auto after = draft_; after.manufacturer = manufacturer_->text(); applyEdit(after, "Change manufacturer");
    });
    connect(category_, &QComboBox::currentIndexChanged, this, [this] {
        const auto value = category_->currentData().toString();
        if (value == draft_.category) return;
        auto after = draft_; after.category = value; applyEdit(after, "Change category");
    });
    connect(canvas_, &ComponentCanvas::draftEdited, this, [this](const ComponentDraft& after, const QString& label) { applyEdit(after, label); });
    showDraft();
}

QWidget* ComponentEditorPage::section(QVBoxLayout* column, const QString& title, bool open)
{
    auto* header = new QToolButton; header->setText(title); header->setCheckable(true); header->setChecked(open);
    header->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); header->setArrowType(open ? Qt::DownArrow : Qt::RightArrow);
    header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); header->setProperty("section", true);
    auto* body = new QWidget; body->setVisible(open);
    connect(header, &QToolButton::toggled, body, [header, body](bool on) {
        body->setVisible(on); header->setArrowType(on ? Qt::DownArrow : Qt::RightArrow);
    });
    column->addWidget(header); column->addWidget(body);
    return body;
}

void ComponentEditorPage::openLibrary(const QString& selectId)
{
    entry_ = Entry::None; entryId_.clear(); instance_ = {}; arrangementOnly_ = false;
    listColumn_->show(); canvas_->setLocked(false);
    draft_ = saved_ = {}; undo_.clear();
    refreshList(selectId);
    if (selectId.isEmpty()) showDraft();
}

void ComponentEditorPage::openInstance(const QUuid& placement, bool arrangementOnly)
{
    for (const auto& item : document_.state().placements) if (item.id == placement) {
        instance_ = placement; arrangementOnly_ = arrangementOnly;
        entry_ = Entry::Instance; entryId_ = item.componentId;
        draft_ = saved_ = ComponentDraft::fromDefinition(document_.catalog().value(item.componentId));
        undo_.clear();
        listColumn_->hide();
        showDraft(); canvas_->fit();
        setMessage(arrangementOnly
            ? "This connector has a fixed LED count: you can move, mirror and rotate its LEDs."
            : "Apply changes only this component in the layout. Use Back to return without changes.");
        return;
    }
    setMessage("Select a component first.");
}

void ComponentEditorPage::refreshList(const QString& selectId)
{
    switching_ = true;
    list_->clear();
    const auto custom = document_.customComponents();
    QSet<QString> used;
    for (const auto& placement : document_.state().placements) used.insert(placement.componentId);
    auto* mine = new QTreeWidgetItem(list_, {"My components"});
    auto* copies = new QTreeWidgetItem(list_);
    auto* builtIn = new QTreeWidgetItem(list_, {"Built-in"});
    QTreeWidgetItem* selected = nullptr;
    const auto add = [&](QTreeWidgetItem* group, const ComponentDefinition& definition, Entry kind) {
        auto text = hardwareName(definition);
        if (kind == Entry::Copy) {
            QStringList users;
            for (const auto& placement : document_.state().placements)
                if (placement.componentId == definition.id) users.append(document_.componentName(placement));
            text += users.isEmpty() ? "  · unused" : "  · used by " + users.join(", ");
        } else if (used.contains(definition.id)) text += "  ●";
        auto* item = new QTreeWidgetItem(group, {text});
        item->setData(0, Qt::UserRole, definition.id);
        item->setData(0, Qt::UserRole + 1, int(kind));
        item->setToolTip(0, definition.id + (used.contains(definition.id) ? "\nUsed in the open layout" : ""));
        if (definition.id == selectId) selected = item;
    };
    for (const auto& definition : custom) add(deviceCopy(definition.id) ? copies : mine, definition, deviceCopy(definition.id) ? Entry::Copy : Entry::Library);
    for (const auto& definition : builtIn_) if (!custom.contains(definition.id)) add(builtIn, definition, Entry::BuiltIn);
    copies->setText(0, QString("Device copies (%1)").arg(copies->childCount()));
    for (auto* group : {mine, copies, builtIn}) {
        group->setFlags(Qt::ItemIsEnabled);
        group->sortChildren(0, Qt::AscendingOrder);
    }
    mine->setExpanded(true); builtIn->setExpanded(true); copies->setExpanded(false);
    if (selected) { selected->parent()->setExpanded(true); list_->setCurrentItem(selected); }
    switching_ = false;
    if (!search_->text().isEmpty()) emit search_->textChanged(search_->text());
    if (selected && (entryId_ != selectId || entry_ == Entry::None)) load(selectId, Entry(selected->data(0, Qt::UserRole + 1).toInt()));
}

void ComponentEditorPage::load(const QString& id, Entry kind)
{
    const auto definition = kind == Entry::BuiltIn ? builtIn_.value(id) : document_.customComponents().value(id);
    draft_ = saved_ = ComponentDraft::fromDefinition(definition);
    entry_ = kind; entryId_ = id;
    undo_.clear();
    showDraft(); canvas_->fit();
    setMessage(kind == Entry::BuiltIn ? "Built-in components are read-only. Save as copy to keep your changes." : QString());
}

void ComponentEditorPage::applyEdit(const ComponentDraft& after, const QString& label)
{
    if (after == draft_ || entry_ == Entry::None) { showDraft(); return; }
    undo_.push(new DraftCommand(label, draft_, after, [this](const ComponentDraft& draft) { draft_ = draft; showDraft(); }));
}

void ComponentEditorPage::showDraft()
{
    canvas_->setDraft(draft_);
    const bool open = entry_ != Entry::None;
    {
        const QSignalBlocker a(name_), b(manufacturer_), c(category_);
        name_->setText(draft_.name);
        manufacturer_->setText(draft_.manufacturer);
        const int index = category_->findData(draft_.category);
        category_->setCurrentIndex(index >= 0 ? index : 0);
    }
    for (auto* field : std::initializer_list<QWidget*>{name_, manufacturer_, category_}) field->setEnabled(open && entry_ != Entry::Instance);
    const int addresses = int(QSet<int>(draft_.mapping.cbegin(), draft_.mapping.cend()).size());
    counts_->setText(QString("%1 LEDs · %2 addresses").arg(draft_.positions.size()).arg(addresses));
    identity_->setText(draft_.id.isEmpty() ? "(assigned on save)" : draft_.id);
    const auto name = draft_.name.isEmpty() ? QString("Untitled component") : hardwareName(draft_.toDefinitionUnchecked());
    title_->setText(!open ? "Component Editor" : (entry_ == Entry::Instance ? "Editing " : QString()) + name + (isDirty() ? "  •" : ""));
    save_->setText(entry_ == Entry::BuiltIn ? "Save as copy" : entry_ == Entry::Instance ? "Apply" : "Save");
    save_->setEnabled(entry_ == Entry::BuiltIn || entry_ == Entry::New || (open && isDirty()));
    delete_->setEnabled(entry_ == Entry::Library || entry_ == Entry::Copy);
    // Protocol slots (e.g. keyboards) and fixed connectors keep LED count and wire order.
    const bool countLocked = !open || draftLocksCount(draft_) || arrangementOnly_;
    for (auto* widget : countEditing_) widget->setEnabled(!countLocked);
    for (auto* widget : wiringEditing_) widget->setEnabled(!countLocked && !draft_.positions.isEmpty());
    canvas_->setLocked(countLocked);
    if (countLocked && tools_->checkedId() == 1) tools_->button(0)->setChecked(true);
    address_->setMaximum(qMax(1, addresses));
    zoom_->setText(QString("zoom %1%").arg(qRound(canvas_->zoom() * 100)));
    updateHint();
    export_->setEnabled(open && !draft_.positions.isEmpty());
}

bool ComponentEditorPage::save()
{
    try {
        auto definition = draft_.toDefinition();
        if (entry_ == Entry::BuiltIn || entry_ == Entry::New) {
            entryId_ = document_.saveComponentCopy(definition);
            draft_.id = entryId_; entry_ = Entry::Library;
        } else if (entry_ == Entry::Library || entry_ == Entry::Copy) document_.updateCustomComponent(definition);
        else if (entry_ == Entry::Instance) {
            // Same rules as the former per-instance dialog: fixed connectors keep their definition and
            // only move LEDs; other placements get a variant without touching sibling placements.
            if (arrangementOnly_) document_.setComponentGeometry(instance_, definition.positions);
            else if (definition.toJson() != document_.catalog().value(entryId_).toJson()) document_.applyComponentVariant(instance_, definition);
            for (const auto& placement : document_.state().placements) if (placement.id == instance_) entryId_ = placement.componentId;
            draft_ = ComponentDraft::fromDefinition(document_.catalog().value(entryId_));
        } else return false;
        saved_ = draft_;
        if (entry_ != Entry::Instance) refreshList(entryId_);
        showDraft();
        setMessage((entry_ == Entry::Instance ? "Applied " : "Saved ") + hardwareName(definition) + ".");
        return true;
    } catch (const std::exception& error) {
        setMessage(QString::fromUtf8(error.what()));
        return false;
    }
}

void ComponentEditorPage::removeCurrent()
{
    if (entry_ != Entry::Library && entry_ != Entry::Copy) return;
    try {
        // removeCustomComponent explains which placements block the deletion.
        if (!document_.componentUsers(entryId_).isEmpty()) { document_.removeCustomComponent(entryId_); return; }
        if (!confirm(QString("Delete \"%1\"? This cannot be undone. Layouts that still use it will report it as missing.").arg(draft_.name))) return;
        document_.removeCustomComponent(entryId_);
        const auto name = draft_.name;
        entry_ = Entry::None; entryId_.clear(); draft_ = saved_ = {}; undo_.clear();
        refreshList({});
        showDraft();
        setMessage("Deleted " + name + ".");
    } catch (const std::exception& error) { setMessage(QString::fromUtf8(error.what())); }
}

bool ComponentEditorPage::confirmLeave()
{
    if (!isDirty()) return true;
    switch (askUnsaved(QString("Save changes to \"%1\"?").arg(draft_.name))) {
    case QMessageBox::Save: return save();
    case QMessageBox::Discard: draft_ = saved_; undo_.clear(); showDraft(); return true;
    default: return false;
    }
}

void ComponentEditorPage::setMessage(const QString& text) { message_->setText(text); }

void ComponentEditorPage::buildToolRail(QWidget* rail)
{
    auto* layout = new QVBoxLayout(rail); layout->setContentsMargins(4, 0, 4, 0); layout->setSpacing(4);
    tools_ = new QButtonGroup(this); tools_->setExclusive(true);
    const auto tool = [&](const QString& name, const QString& icon, const QString& tip, bool checkable = false) {
        auto* button = new QToolButton; button->setObjectName(name);
        button->setIcon(studioToolIcon(icon)); button->setIconSize({20, 20}); button->setFixedSize(36, 36);
        button->setToolTip(tip); button->setAccessibleName(tip.section(" (", 0, 0)); button->setCheckable(checkable);
        layout->addWidget(button, 0, Qt::AlignHCenter);
        return button;
    };
    const auto separator = [&] { auto* line = new QFrame; line->setFrameShape(QFrame::HLine); layout->addWidget(line); };
    auto* select = tool("componentToolSelect", "select", "Edit LEDs (V): click to add, drag to move, right-click to delete", true);
    wireTool_ = tool("componentToolWire", "wire", "Wire order (W): click LEDs in data order", true);
    tools_->addButton(select, 0); tools_->addButton(wireTool_, 1); select->setChecked(true);
    connect(tools_, &QButtonGroup::idToggled, this, [this](int id, bool on) {
        if (!on) return;
        canvas_->setTool(id == 1 ? ComponentCanvas::Tool::Wire : ComponentCanvas::Tool::Select);
        updateHint();
    });
    separator();
    auto* duplicate = tool("componentDuplicate", "duplicate", "Duplicate selection (D)");
    auto* mirrorH = tool("componentMirrorH", "mirror-h", "Mirror horizontally (M)");
    auto* mirrorV = tool("componentMirrorV", "mirror-v", "Mirror vertically (Shift+M)");
    auto* rotate = tool("componentRotate", "rotate", "Rotate 90 degrees clockwise (R)");
    auto* remove = tool("componentDeleteLeds", "delete", "Delete selected LEDs (Del)");
    separator();
    grid_ = tool("componentGridToggle", "grid", "Show grid (G)", true); grid_->setChecked(true);
    path_ = tool("componentPathToggle", "path", "Show wire path (P)", true); path_->setChecked(true);
    separator();
    auto* zoomIn = tool("componentZoomIn", "zoom-in", "Zoom in (+)");
    auto* zoomOut = tool("componentZoomOut", "zoom-out", "Zoom out (-)");
    auto* fit = tool("componentFit", "fit", "Fit LEDs (0)");
    layout->addStretch();
    help_ = tool("componentHelp", "help", "Help and shortcuts (?)");
    countEditing_ << duplicate << remove << wireTool_;
    connect(duplicate, &QToolButton::clicked, this, [this] {
        editSelection([](const ComponentDraft& d, const QSet<int>& rows) { return rows.isEmpty() ? d : withDuplicated(d, rows, {1, 1}); }, "Duplicate LEDs");
    });
    connect(mirrorH, &QToolButton::clicked, this, [this] {
        editSelection([](const ComponentDraft& d, const QSet<int>& rows) { return withMirror(d, rows, Qt::Horizontal); }, "Mirror horizontally");
    });
    connect(mirrorV, &QToolButton::clicked, this, [this] {
        editSelection([](const ComponentDraft& d, const QSet<int>& rows) { return withMirror(d, rows, Qt::Vertical); }, "Mirror vertically");
    });
    connect(rotate, &QToolButton::clicked, this, [this] {
        editSelection([](const ComponentDraft& d, const QSet<int>& rows) { return withRotation90(d, rows); }, "Rotate");
    });
    connect(remove, &QToolButton::clicked, this, [this] {
        const auto rows = rowsForDelete();
        if (rows.isEmpty()) return;
        editSelection([rows](const ComponentDraft& d, const QSet<int>&) { return withRemoved(d, rows); }, "Delete LEDs");
    });
    connect(grid_, &QToolButton::toggled, canvas_, &ComponentCanvas::setGridVisible);
    connect(path_, &QToolButton::toggled, canvas_, &ComponentCanvas::setPathVisible);
    connect(zoomIn, &QToolButton::clicked, canvas_, [this] { canvas_->zoomBy(1.25); });
    connect(zoomOut, &QToolButton::clicked, canvas_, [this] { canvas_->zoomBy(0.8); });
    connect(fit, &QToolButton::clicked, canvas_, &ComponentCanvas::fit);

    auto* overlay = new QFrame(this); overlay->setObjectName("componentHelpOverlay");
    overlay->setProperty("popup", true); overlay->setAutoFillBackground(true); overlay->setFrameShape(QFrame::StyledPanel);
    helpOverlay_ = overlay;
    auto* helpLayout = new QVBoxLayout(overlay); helpLayout->setContentsMargins(20, 16, 20, 16);
    auto* text = new QLabel(
        "<h3>Help &amp; shortcuts</h3>"
        "<p>A component is a set of LEDs with positions and a data (wire) order. Add LEDs with a shape or by "
        "clicking the canvas, set the order with the Wire tool or a preset, then Save.</p>"
        "<table cellspacing=\"6\">"
        "<tr><td><b>Click</b> empty grid</td><td>add LED</td><td><b>Right-click</b> LED</td><td>delete it</td></tr>"
        "<tr><td><b>Drag</b> LED</td><td>move selection</td><td><b>Drag</b> empty grid</td><td>box select</td></tr>"
        "<tr><td><b>Shift+click/drag</b></td><td>add to selection</td><td><b>Middle-drag</b> / Space+drag</td><td>pan</td></tr>"
        "<tr><td><b>Scroll</b></td><td>zoom at cursor</td><td><b>Alt</b> while dragging</td><td>ignore the grid</td></tr>"
        "<tr><td><b>V</b></td><td>edit tool</td><td><b>W</b></td><td>wire order tool</td></tr>"
        "<tr><td><b>R</b></td><td>rotate 90 degrees clockwise</td><td><b>D</b></td><td>duplicate selection</td></tr>"
        "<tr><td><b>M</b> / <b>Shift+M</b></td><td>mirror H / V</td><td><b>Del</b> / Backspace</td><td>delete selection or LED under cursor</td></tr>"
        "<tr><td><b>Ctrl+Z</b></td><td>undo</td><td><b>Ctrl+Y</b> / Ctrl+Shift+Z</td><td>redo</td></tr>"
        "<tr><td><b>Ctrl+A</b></td><td>select all</td><td><b>Esc</b></td><td>leave wire mode / clear selection</td></tr>"
        "<tr><td><b>G</b> / <b>P</b></td><td>grid / wire path</td><td><b>+</b> / <b>-</b> / <b>0</b></td><td>zoom in / out / fit</td></tr>"
        "</table>");
    text->setWordWrap(true); text->setTextFormat(Qt::RichText); helpLayout->addWidget(text);
    auto* close = new QPushButton("Got it"); close->setProperty("primary", true);
    helpLayout->addWidget(close, 0, Qt::AlignRight);
    overlay->hide();
    connect(close, &QPushButton::clicked, overlay, &QWidget::hide);
    connect(help_, &QToolButton::clicked, this, [this] {
        helpOverlay_->setVisible(!helpOverlay_->isVisible());
        helpOverlay_->resize(qMin(720, width() - 80), helpOverlay_->sizeHint().height());
        helpOverlay_->move((width() - helpOverlay_->width()) / 2, (height() - helpOverlay_->height()) / 2);
        helpOverlay_->raise();
    });
}

void ComponentEditorPage::buildShapesAndWiring(QVBoxLayout* column)
{
    auto* shapes = section(column, "ADD SHAPES", true);
    auto* shapeLayout = new QGridLayout(shapes); shapeLayout->setContentsMargins(0, 0, 0, 0);
    const QList<QPair<QString, DraftShape>> shapeButtons{{"Matrix", DraftShape::Matrix}, {"Strip", DraftShape::Strip},
        {"Ring", DraftShape::Ring}, {"Fan", DraftShape::Fan}, {"L-Shape", DraftShape::LShape}, {"U-Shape", DraftShape::UShape}};
    for (int i = 0; i < shapeButtons.size(); ++i) {
        const auto label = shapeButtons[i].first;
        const auto shape = shapeButtons[i].second;
        auto* button = new QPushButton(label); button->setObjectName("componentShape" + QString(label).remove('-'));
        button->setToolTip("Replace all LEDs with a " + label.toLower() + " of the chosen LED count");
        shapeLayout->addWidget(button, i / 3, i % 3);
        countEditing_ << button;
        connect(button, &QPushButton::clicked, this, [this, shape, label] {
            try { applyEdit(withShape(draft_, shape, shapeCount_->value()), "Add " + label); }
            catch (const std::exception& error) { setMessage(QString::fromUtf8(error.what())); }
        });
    }
    shapeCount_ = new QSpinBox; shapeCount_->setObjectName("componentShapeCount"); shapeCount_->setRange(1, 1024);
    shapeCount_->setValue(12); shapeCount_->setAccessibleName("LEDs in the new shape");
    auto* countRow = new QHBoxLayout; countRow->addWidget(new QLabel("LEDs")); countRow->addWidget(shapeCount_, 1);
    shapeLayout->addLayout(countRow, 2, 0, 1, 3);
    countEditing_ << shapeCount_;

    auto* wiring = section(column, "WIRING", true);
    auto* wiringLayout = new QVBoxLayout(wiring); wiringLayout->setContentsMargins(0, 0, 0, 0);
    auto* info = new QLabel("The LED order must match the device's data line. Use the Wire tool (W) to click "
        "LEDs in order, or apply a preset.");
    info->setWordWrap(true); info->setProperty("secondary", true); wiringLayout->addWidget(info);
    const QList<QPair<QString, WiringPreset>> presets{{"Rows: left to right", WiringPreset::RowsLeftToRight},
        {"Rows: serpentine", WiringPreset::RowsSerpentine}, {"Columns: top to bottom", WiringPreset::ColumnsTopToBottom},
        {"Clockwise around center", WiringPreset::Clockwise}};
    const QStringList presetNames{"RowsLeftToRight", "RowsSerpentine", "ColumnsTopToBottom", "Clockwise"};
    for (int i = 0; i < presets.size(); ++i) {
        auto* button = new QPushButton(presets[i].first); button->setObjectName("componentPreset" + presetNames[i]);
        wiringLayout->addWidget(button); wiringEditing_ << button;
        const auto preset = presets[i].second;
        const auto label = presets[i].first;
        connect(button, &QPushButton::clicked, this, [this, preset, label] { applyEdit(withPreset(draft_, preset), "Wiring: " + label); });
    }
    auto* shareRow = new QHBoxLayout; wiringLayout->addLayout(shareRow);
    shareRow->addWidget(new QLabel("Address"));
    address_ = new QSpinBox; address_->setObjectName("componentAddress"); address_->setRange(1, 1);
    address_->setAccessibleName("LED address for the selected LEDs"); shareRow->addWidget(address_, 1);
    auto* assign = new QPushButton("Assign"); assign->setObjectName("componentAssignAddress");
    assign->setToolTip("Give the selected LEDs this address so they share one color");
    shareRow->addWidget(assign);
    wiringEditing_ << address_ << assign;
    connect(assign, &QPushButton::clicked, this, [this] {
        editSelection([this](const ComponentDraft& d, const QSet<int>& rows) {
            return rows.isEmpty() ? d : withSharedAddress(d, rows, address_->value() - 1);
        }, "Share address");
    });
    connect(address_, &QSpinBox::valueChanged, this, [this](int value) {
        // Stepping through addresses highlights every LED of that address for checking.
        if (!address_->hasFocus()) return;
        QSet<int> rows;
        for (int row = 0; row < draft_.mapping.size(); ++row) if (draft_.mapping[row] == value - 1) rows.insert(row);
        canvas_->selectRows(rows);
    });

    auto* danger = section(column, "DANGER ZONE", false);
    auto* dangerLayout = new QVBoxLayout(danger); dangerLayout->setContentsMargins(0, 0, 0, 0);
    auto* clear = new QPushButton("Clear all LEDs"); clear->setObjectName("componentClearAll"); clear->setProperty("danger", true);
    dangerLayout->addWidget(clear); countEditing_ << clear;
    connect(clear, &QPushButton::clicked, this, [this] {
        if (draft_.positions.isEmpty()) return;
        editSelection([](const ComponentDraft& d, const QSet<int>&) {
            QSet<int> all; for (int row = 0; row < d.positions.size(); ++row) all.insert(row);
            return withRemoved(d, all);
        }, "Clear all LEDs");
    });
    auto* removeDefinition = new QPushButton("Delete component"); removeDefinition->setObjectName("componentDangerDelete");
    removeDefinition->setProperty("danger", true); dangerLayout->addWidget(removeDefinition);
    connect(removeDefinition, &QPushButton::clicked, this, [this] { removeCurrent(); });
}

void ComponentEditorPage::buildShortcuts()
{
    const auto bind = [this](const QKeySequence& keys, std::function<void()> action) {
        auto* shortcut = new QShortcut(keys, this);
        shortcut->setContext(Qt::WidgetWithChildrenShortcut);
        connect(shortcut, &QShortcut::activated, this, std::move(action));
    };
    const auto click = [this](const char* name) {
        return [this, name] { if (auto* b = findChild<QAbstractButton*>(name); b && b->isEnabled()) b->click(); };
    };
    bind(QKeySequence(Qt::Key_V), [this] { tools_->button(0)->setChecked(true); });
    bind(QKeySequence(Qt::Key_W), [this] { if (wireTool_->isEnabled()) wireTool_->setChecked(true); });
    bind(QKeySequence(Qt::Key_R), click("componentRotate"));
    bind(QKeySequence(Qt::Key_D), click("componentDuplicate"));
    bind(QKeySequence(Qt::Key_M), click("componentMirrorH"));
    bind(QKeySequence(Qt::SHIFT | Qt::Key_M), click("componentMirrorV"));
    bind(QKeySequence(Qt::Key_Delete), click("componentDeleteLeds"));
    bind(QKeySequence(Qt::Key_Backspace), click("componentDeleteLeds"));
    bind(QKeySequence::Undo, [this] { undo_.undo(); });
    bind(QKeySequence(Qt::CTRL | Qt::Key_Y), [this] { undo_.redo(); });
    bind(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z), [this] { undo_.redo(); });
    bind(QKeySequence(Qt::CTRL | Qt::Key_A), [this] {
        QSet<int> all; for (int row = 0; row < draft_.positions.size(); ++row) all.insert(row);
        canvas_->selectRows(all);
    });
    bind(QKeySequence(Qt::Key_Escape), [this] {
        if (helpOverlay_->isVisible()) helpOverlay_->hide();
        else if (tools_->checkedId() == 1) tools_->button(0)->setChecked(true);
        else canvas_->selectRows({});
    });
    bind(QKeySequence(Qt::Key_G), [this] { grid_->toggle(); });
    bind(QKeySequence(Qt::Key_P), [this] { path_->toggle(); });
    bind(QKeySequence(Qt::Key_Plus), [this] { canvas_->zoomBy(1.25); });
    bind(QKeySequence(Qt::Key_Equal), [this] { canvas_->zoomBy(1.25); });
    bind(QKeySequence(Qt::Key_Minus), [this] { canvas_->zoomBy(0.8); });
    bind(QKeySequence(Qt::Key_0), [this] { canvas_->fit(); });
    bind(QKeySequence(Qt::Key_Question), [this] { help_->click(); });
}

void ComponentEditorPage::editSelection(const std::function<ComponentDraft(const ComponentDraft&, const QSet<int>&)>& operation, const QString& label)
{
    try { applyEdit(operation(draft_, canvas_->selectedRows()), label); }
    catch (const std::exception& error) { setMessage(QString::fromUtf8(error.what())); }
}

QSet<int> ComponentEditorPage::rowsForDelete() const
{
    auto rows = canvas_->selectedRows();
    if (!rows.isEmpty()) return rows;
    // Without a selection, Delete removes the LED under the mouse pointer.
    for (auto* item : canvas_->items(canvas_->viewport()->mapFromGlobal(QCursor::pos()))) {
        auto* led = item->parentItem() ? item->parentItem() : item;
        if (led->type() == QGraphicsEllipseItem::Type) return {led->data(0).toInt()};
    }
    return {};
}

void ComponentEditorPage::updateHint()
{
    hint_->setText(tools_->checkedId() == 1
        ? "click LEDs in data order · Esc = cancel · M-drag = pan · scroll = zoom"
        : "click = add · drag LED = move · drag empty = box select · R-click = delete · M-drag = pan · scroll = zoom");
}
