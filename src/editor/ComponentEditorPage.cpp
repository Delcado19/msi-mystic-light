#include "ComponentEditorPage.h"
#include "ComponentCanvas.h"
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
    counts_ = new QLabel; counts_->setObjectName("componentStatusLeds"); form->addRow(counts_);
    identity_ = new QLabel; identity_->setObjectName("componentIdentity");
    identity_->setTextInteractionFlags(Qt::TextSelectableByMouse); identity_->setWordWrap(true);
    form->addRow("ID", identity_);
    column->addStretch();
    properties->setWidget(panel);
    body->addWidget(properties);

    auto* status = new QHBoxLayout; root->addLayout(status);
    message_ = new QLabel; message_->setObjectName("componentMessage"); message_->setWordWrap(true);
    status->addWidget(message_, 1);

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
    entry_ = Entry::None; instance_ = {}; arrangementOnly_ = false;
    listColumn_->show(); canvas_->setLocked(false);
    draft_ = saved_ = {}; undo_.clear();
    refreshList(selectId);
    if (selectId.isEmpty()) showDraft();
}

void ComponentEditorPage::openInstance(const QUuid&, bool) {}

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
    title_->setText(!open ? "Component Editor" : (draft_.name.isEmpty() ? "Untitled component" : draft_.name) + (isDirty() ? "  •" : ""));
    save_->setText(entry_ == Entry::BuiltIn ? "Save as copy" : entry_ == Entry::Instance ? "Apply" : "Save");
    save_->setEnabled(entry_ == Entry::BuiltIn || entry_ == Entry::New || (open && isDirty()));
    delete_->setEnabled(entry_ == Entry::Library || entry_ == Entry::Copy);
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
        else return false;
        saved_ = draft_;
        refreshList(entryId_);
        showDraft();
        setMessage("Saved " + hardwareName(definition) + ".");
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
