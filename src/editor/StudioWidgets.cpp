#include "StudioWidgets.h"
#include "LightingDocument.h"
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>
#include <cmath>
#include <algorithm>

void applyStudioStyle(QWidget* window)
{
    auto font=window->font(); font.setPointSizeF(std::max(10.0,font.pointSizeF())); window->setFont(font);
    window->setStyleSheet(R"(
        QWidget { color: #e8eaf0; background: #24262d; }
        QMainWindow, QDialog { background: #1b1d23; }
        QMenuBar, QToolBar, QStatusBar { background: #202229; spacing: 8px; }
        QMenuBar { padding: 4px; }
        QMenuBar::item:selected, QMenu::item:selected { background: #35436a; }
        QToolBar { border: 0; padding: 6px; }
        QDockWidget { font-weight: 600; }
        QDockWidget::title { padding: 12px; background: #202229; }
        QTabWidget::pane { border: 0; }
        QTabBar::tab { padding: 10px 18px; color: #b6bdcd; background: #202229; border-bottom: 2px solid transparent; }
        QTabBar::tab:selected { color: #f4f6ff; border-bottom-color: #8eaaff; background: #2c303b; }
        QScrollArea { border: 0; }
        QPushButton, QToolButton { background: #323640; border: 1px solid #454b59; border-radius: 6px; padding: 7px 10px; }
        QPushButton:hover, QToolButton:hover { background: #41495a; }
        QPushButton:pressed, QToolButton:pressed, QToolButton:checked, QPushButton:checked { background: #3c4c76; border-color: #8eaaff; }
        QPushButton:disabled, QToolButton:disabled { color: #8b92a1; background: #292c33; border-color: #343842; }
        QPushButton[primary="true"] { background: #425f9f; border-color: #829de4; color: white; font-weight: 600; }
        QComboBox, QLineEdit, QSpinBox, QDoubleSpinBox { border: 1px solid #454b59; border-radius: 5px; background: #1d2027; padding: 6px; min-height: 18px; }
        QComboBox { padding-right: 20px; }
        QAbstractSpinBox::up-button, QAbstractSpinBox::down-button { width: 0; }
        QTreeWidget, QListWidget { background: #202229; border: 0; outline: 0; }
        QTreeWidget::item, QListWidget::item { padding: 8px 4px; border-radius: 5px; }
        QTreeWidget::item:selected, QListWidget::item:selected { background: #344667; color: white; }
        QHeaderView::section { background: #24262d; color: #b6bdcd; border: 0; padding: 6px; }
        QSlider::groove:horizontal { background: #454c5e; height: 4px; border-radius: 2px; }
        QSlider::sub-page:horizontal { background: #91aaff; border-radius: 2px; }
        QSlider::handle:horizontal { background: #dce5ff; width: 12px; margin: -5px 0; border-radius: 6px; }
        QSlider { min-height: 24px; }
        QCheckBox { spacing: 8px; padding: 4px 0; }
        QCheckBox::indicator { width: 15px; height: 15px; border: 1px solid #717b91; border-radius: 3px; background: #1d2027; }
        QCheckBox::indicator:checked { background: #91aaff; border-color: #c1d0ff; }
        QLabel[muted="true"] { color: #b6bdcd; }
        QLabel[heading="true"] { font-size: 16px; font-weight: 600; padding: 6px 0; }
        QToolButton[disclosure="true"] { text-align: left; background: transparent; border: 0; border-top: 1px solid #3a3e49; border-radius: 0; padding: 12px 0; }
        QScrollBar:vertical { width: 10px; background: #202229; }
        QScrollBar::handle:vertical { background: #596174; min-height: 24px; border-radius: 4px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QWidget:focus { border-color: #b7c9ff; }
        QToolTip { color: #edf1ff; background: #343d50; border: 1px solid #7788ad; padding: 6px; }
    )");
}

QWidget* sliderField(QDoubleSpinBox* value)
{
    auto* row=new QWidget; auto* layout=new QHBoxLayout(row); layout->setContentsMargins(0,0,0,0);
    auto* slider=new QSlider(Qt::Horizontal); slider->setRange(0,1000);
    slider->setAccessibleName(value->accessibleName().isEmpty() ? value->objectName() : value->accessibleName());
    slider->setProperty("numericField",QVariant::fromValue(static_cast<QObject*>(value)));
    value->setButtonSymbols(QAbstractSpinBox::NoButtons); value->setFixedWidth(72);
    layout->addWidget(slider,1); layout->addWidget(value);
    QObject::connect(slider,&QSlider::valueChanged,value,[value](int n) { value->setValue(value->minimum()+(value->maximum()-value->minimum())*n/1000.0); });
    QObject::connect(value,&QDoubleSpinBox::valueChanged,slider,[value,slider](double n) {
        const QSignalBlocker block(slider); slider->setValue(qRound(1000*(n-value->minimum())/(value->maximum()-value->minimum())));
    });
    return row;
}
void syncSliderFields(QWidget* root)
{
    // Undo/document refresh blocks numeric signals; synchronize handles without creating another edit.
    for(auto* slider:root->findChildren<QSlider*>()) if(auto* value=qobject_cast<QDoubleSpinBox*>(slider->property("numericField").value<QObject*>())) {
        const QSignalBlocker block(slider); slider->setEnabled(value->isEnabled());
        slider->setValue(qRound(1000*(value->value()-value->minimum())/(value->maximum()-value->minimum())));
    }
}
QWidget* disclosure(QVBoxLayout* layout,const QString& title,QWidget* content)
{
    auto* box=new QWidget; auto* stack=new QVBoxLayout(box); stack->setContentsMargins(0,0,0,0); stack->setSpacing(0);
    auto* toggle=new QToolButton; toggle->setText(title); toggle->setProperty("disclosure",true);
    toggle->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); toggle->setArrowType(Qt::RightArrow); toggle->setCheckable(true);
    toggle->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed); stack->addWidget(toggle); stack->addWidget(content); content->hide();
    QObject::connect(toggle,&QToolButton::toggled,content,[toggle,content](bool open) { content->setVisible(open); toggle->setArrowType(open ? Qt::DownArrow : Qt::RightArrow); });
    layout->addWidget(box); return box;
}
std::optional<EffectKind> chooseEffect(QWidget* parent)
{
    QDialog dialog(parent); dialog.setObjectName("effectGallery"); dialog.setWindowTitle("Add an effect layer"); dialog.resize(720,530);
    auto* layout=new QVBoxLayout(&dialog); layout->setContentsMargins(24,20,24,20); layout->setSpacing(12);
    auto* title=new QLabel("Choose an effect"); title->setProperty("heading",true); layout->addWidget(title);
    auto* subtitle=new QLabel("Each effect becomes a layer with its own editable mask."); subtitle->setProperty("muted",true); layout->addWidget(subtitle);
    auto* search=new QLineEdit; search->setPlaceholderText("Search effects..."); search->setObjectName("effectSearch"); layout->addWidget(search);
    auto* list=new QListWidget; list->setObjectName("effectGalleryItems"); list->setViewMode(QListView::IconMode);
    list->setMovement(QListView::Static); list->setResizeMode(QListView::Adjust); list->setIconSize({112,70}); list->setGridSize({150,116});
    for(const auto& definition:effectDefinitions()) {
        if(definition.kind==EffectKind::Image) continue; // Images use Import, so choosing an effect never creates an empty image layer.
        EffectLayer layer; layer.effect=definition.kind; layer.color=QColor("#91aaff"); layer.secondaryColor=QColor("#ff91ba");
        // Gallery and canvas use the same source renderer, not approximate marketing thumbnails.
        QPixmap preview(112,70); preview.fill(QColor("#171b25")); QPainter painter(&preview);
        painter.drawImage(preview.rect(),renderEffect(layer,1)); painter.end();
        auto* item=new QListWidgetItem(QIcon(preview),definition.label,list); item->setData(Qt::UserRole,static_cast<int>(definition.kind));
        item->setToolTip("Add "+definition.label+" with a white mask"); item->setTextAlignment(Qt::AlignCenter);
    }
    layout->addWidget(list,1); auto* buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText("Add layer"); layout->addWidget(buttons);
    QObject::connect(search,&QLineEdit::textChanged,list,[list,buttons](const QString& text) {
        QListWidgetItem* first=nullptr;
        for(int i=0;i<list->count();++i) {
            auto* item=list->item(i); item->setHidden(!item->text().contains(text,Qt::CaseInsensitive));
            if(!item->isHidden() && !first) first=item;
        }
        list->setCurrentItem(first); buttons->button(QDialogButtonBox::Ok)->setEnabled(first!=nullptr);
    });
    QObject::connect(list,&QListWidget::itemDoubleClicked,&dialog,&QDialog::accept);
    QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);
    QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    list->setCurrentRow(0); search->setFocus();
    if(dialog.exec()!=QDialog::Accepted || !list->currentItem() || list->currentItem()->isHidden()) return {};
    return static_cast<EffectKind>(list->currentItem()->data(Qt::UserRole).toInt());
}

QIcon studioToolIcon(const QString& name)
{
    QPixmap pixmap(24,24); pixmap.fill(Qt::transparent); QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing); painter.setPen(QPen(QColor("#e0e8fc"),1.7,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    if(name=="move") {
        painter.drawLine(12,3,12,21); painter.drawLine(3,12,21,12);
        for(int i=0;i<4;++i) { painter.save(); painter.translate(12,12); painter.rotate(90*i); painter.drawLine(0,-9,-3,-6); painter.drawLine(0,-9,3,-6); painter.restore(); }
    } else if(name=="brush") {
        painter.drawLine(9,15,18,4); painter.drawLine(12,17,21,6); painter.drawLine(18,4,21,6);
        painter.drawEllipse(QRectF(4,15,7,6)); painter.drawLine(4,18,2,22);
    } else {
        painter.drawLine(6,2,6,18); painter.drawLine(6,18,22,18); painter.drawLine(2,6,18,6); painter.drawLine(18,6,18,22);
    }
    return QIcon(pixmap);
}
