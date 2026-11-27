#pragma once
#include "ComponentDraft.h"
#include <QMessageBox>
#include <QUndoStack>
#include <QUuid>
#include <QWidget>
#include <functional>

class LightingDocument;
class ComponentCanvas;
class QButtonGroup;
class QComboBox;
class QKeyEvent;
class QSpinBox;
class QToolButton;
class QLabel;
class QLineEdit;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;
class QVBoxLayout;

// Full-window Component Editor (replaces the former "Custom components" dialog and Component Lab).
// Library mode edits definitions in the user's component library; instance mode edits one placed
// component and applies the result to it.
class ComponentEditorPage : public QWidget {
    Q_OBJECT
public:
    ComponentEditorPage(LightingDocument& document, const ComponentCatalog& builtIn, QWidget* parent = nullptr);
    void openLibrary(const QString& selectId = {});
    void openInstance(const QUuid& placement, bool arrangementOnly);
    // Save/Discard/Cancel when there are unsaved edits; true when leaving the current draft is allowed.
    bool confirmLeave();
    bool isDirty() const { return !(draft_ == saved_); }
    const ComponentDraft& draft() const { return draft_; }
    QUndoStack* undoStack() { return &undo_; } // joins the window's undo group while the page is shown
    // Dialog hooks; tests replace them to answer without modal message boxes.
    std::function<bool(const QString&)> confirm;
    std::function<QMessageBox::StandardButton(const QString&)> askUnsaved;
    // User component folder (plan 2026-11-27 Task 5): the window asks for it before the first save.
    std::function<bool()> ensureLibrary;
    enum class DuplicateChoice { Cancel, Replace, Copy };
    std::function<DuplicateChoice(const QString& name)> askDuplicate;
    std::function<QString()> pickImportFile;
    void reloadLibrary() { refreshList(entryId_); }
signals:
    void closeRequested();
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
private:
    struct KeyBinding { std::function<bool(const QKeyEvent*)> matches; std::function<void()> action; };
    enum class Entry { None, New, Library, Copy, BuiltIn, Instance };
    QWidget* section(QVBoxLayout* column, const QString& title, bool open);
    void refreshList(const QString& selectId);
    void load(const QString& id, Entry kind);
    void applyEdit(const ComponentDraft& after, const QString& label);
    void showDraft();
    bool save();
    void removeCurrent();
    void setMessage(const QString& text);
    void buildToolRail(QWidget* rail);
    void buildShapesAndWiring(QVBoxLayout* column);
    void buildShortcuts();
    void editSelection(const std::function<ComponentDraft(const ComponentDraft&, const QSet<int>&)>& operation, const QString& label);
    QSet<int> rowsForDelete() const;
    void updateHint();
    // Display-only depth layers (owner request 2026-11-25): view state, not undo state.
    void setActiveLayer(int layer);
    void moveSelectionToLayer(int layer);
    void updateLayerBox();
    LightingDocument& document_;
    ComponentCatalog builtIn_;
    ComponentDraft draft_, saved_;
    Entry entry_ = Entry::None;
    QString entryId_;
    QUuid instance_;
    bool arrangementOnly_ = false, switching_ = false;
    QUndoStack undo_;
    ComponentCanvas* canvas_ = nullptr;
    QTreeWidget* list_ = nullptr;
    QWidget* listColumn_ = nullptr;
    QLineEdit *search_ = nullptr, *name_ = nullptr, *manufacturer_ = nullptr;
    QComboBox* category_ = nullptr;
    QLabel *title_ = nullptr, *message_ = nullptr, *counts_ = nullptr;
    QLineEdit* identity_ = nullptr;
    void addFromFile();
    QPushButton *new_ = nullptr, *import_ = nullptr, *export_ = nullptr, *delete_ = nullptr, *save_ = nullptr, *dangerDelete_ = nullptr;
    QButtonGroup* tools_ = nullptr;
    QToolButton *wireTool_ = nullptr, *grid_ = nullptr, *path_ = nullptr, *help_ = nullptr;
    QToolButton *layerView_ = nullptr, *layerBack_ = nullptr, *layerFront_ = nullptr;
    QLabel* layerLabel_ = nullptr;
    QSpinBox* moveLayer_ = nullptr;
    QSpinBox *shapeCount_ = nullptr, *address_ = nullptr;
    QLabel *cursor_ = nullptr, *zoom_ = nullptr, *hint_ = nullptr;
    QWidget* helpOverlay_ = nullptr;
    QVector<KeyBinding> bindings_;
    QList<QWidget*> countEditing_, wiringEditing_; // disabled for protocol-locked or fixed instances
    bool countLocked() const { return draftLocksCount(draft_) || arrangementOnly_; }
    // Built-in catalog parts are view-only; "Make a copy" creates an editable one (owner review 2026-11-26).
    bool readOnly() const { return entry_ == Entry::BuiltIn; }
    QList<QWidget*> arrangeEditing_; // mirror, rotate, layer moves: allowed on fixed counts, not on built-ins
    QList<QPushButton*> shapeButtons_; // add a shape, or rearrange the existing LEDs when the count is locked
    QLabel* wiringInfo_ = nullptr;
    QPushButton* assign_ = nullptr;
    void syncAddressField();
};
