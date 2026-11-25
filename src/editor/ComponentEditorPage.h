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
    // Dialog hooks; tests replace them to answer without modal message boxes.
    std::function<bool(const QString&)> confirm;
    std::function<QMessageBox::StandardButton(const QString&)> askUnsaved;
signals:
    void closeRequested();
private:
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
    QLabel *title_ = nullptr, *message_ = nullptr, *counts_ = nullptr, *identity_ = nullptr;
    QPushButton *new_ = nullptr, *import_ = nullptr, *export_ = nullptr, *delete_ = nullptr, *save_ = nullptr;
    QButtonGroup* tools_ = nullptr;
    QToolButton *wireTool_ = nullptr, *grid_ = nullptr, *path_ = nullptr, *help_ = nullptr;
    QSpinBox *shapeCount_ = nullptr, *address_ = nullptr;
    QLabel *cursor_ = nullptr, *zoom_ = nullptr, *hint_ = nullptr;
    QWidget* helpOverlay_ = nullptr;
    QList<QWidget*> countEditing_, wiringEditing_; // disabled for protocol-locked or fixed instances
};
