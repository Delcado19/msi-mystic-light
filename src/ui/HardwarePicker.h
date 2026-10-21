#pragma once
#include "device/Component.h"
#include "ui/DeviceArtwork.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QIcon>
#include <QPixmap>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

inline QIcon hardwareIcon(const Component& component) {
    QPixmap image(80,64);image.fill(Qt::transparent);QPainter p(&image);p.setRenderHint(QPainter::Antialiasing);
    drawDeviceArtwork(p,QRectF(10,4,60,56),component);
    return QIcon(image);
}

// A channel-scoped product choice: search first, then add one or more identical
// products atomically. Capacity is checked again by configureComponents().
class HardwarePicker : public QDialog {
public:
    bool customRequested=false;
    HardwarePicker(const QString& channel,std::vector<Component> products,int remaining,QWidget* parent=nullptr)
        :QDialog(parent),products_(std::move(products)) {
        setObjectName("hardwarePicker");setWindowTitle("Add component to "+channel);resize(760,590);
        auto* root=new QVBoxLayout(this);
        auto* heading=new QLabel("Add to "+channel+QString(" / %1 addresses available").arg(remaining),this);root->addWidget(heading);
        auto* search=new QLineEdit(this);search->setObjectName("hardwareSearch");search->setPlaceholderText("Search fans, strips, brands or your own definitions");search->setClearButtonEnabled(true);root->addWidget(search);
        auto* list=new QListWidget(this);list->setObjectName("hardwareProducts");list->setViewMode(QListView::IconMode);list->setResizeMode(QListView::Adjust);list->setMovement(QListView::Static);list->setWordWrap(true);list->setGridSize(QSize(210,135));list->setIconSize(QSize(80,64));root->addWidget(list,1);
        for(const auto& c:products_){auto* item=new QListWidgetItem(hardwareIcon(c),QString::fromStdString(c.brand.empty()?c.displayName:c.brand+" / "+c.displayName),list);item->setToolTip(QString::fromStdString(c.productName)+QString(" / %1 addresses, %2 physical LEDs").arg(c.ledCount).arg(c.ledCount*c.physicalLedsPerAddress));}
        auto* details=new QLabel(this);details->setWordWrap(true);root->addWidget(details);
        auto* row=new QHBoxLayout;root->addLayout(row);row->addWidget(new QLabel("Quantity",this));
        quantity_=new QSpinBox(this);quantity_->setObjectName("hardwareQuantity");quantity_->setRange(1,120);row->addWidget(quantity_);row->addStretch();
        auto* custom=new QPushButton("Create custom fan / strip",this);custom->setObjectName("pickerCreateCustom");row->addWidget(custom);
        auto* buttons=new QDialogButtonBox(QDialogButtonBox::Cancel,this);auto* add=buttons->addButton("Add to "+channel,QDialogButtonBox::AcceptRole);add->setObjectName("confirmAddHardware");root->addWidget(buttons);
        const auto updateChoice=[this,list,details,add,remaining]{
            selected_=list->currentRow();const bool selected=selected_>=0 && !list->item(selected_)->isHidden();
            const int count=selected?products_[selected_].ledCount:0;const int maximum=count?remaining/count:0;
            quantity_->setMaximum(qMax(1,maximum));quantity_->setEnabled(maximum>0);add->setEnabled(maximum>0);
            details->setText(selected?QString::fromStdString(products_[selected_].productName)+QString(" / %1 addresses per component / %2 physical LEDs").arg(count).arg(count*products_[selected_].physicalLedsPerAddress)+(maximum?QString():QString(" / does not fit this channel")):QString("Choose a component to add."));
        };
        connect(list,&QListWidget::currentRowChanged,this,[updateChoice]{updateChoice();});
        connect(search,&QLineEdit::textChanged,this,[list,updateChoice](const QString& query){int first=-1;for(int i=0;i<list->count();++i){const bool match=(list->item(i)->text()+" "+list->item(i)->toolTip()).contains(query.trimmed(),Qt::CaseInsensitive);list->item(i)->setHidden(!match);if(match && first<0)first=i;}list->setCurrentRow(first);updateChoice();});
        connect(custom,&QPushButton::clicked,this,[this]{customRequested=true;reject();});
        connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
        connect(add,&QPushButton::clicked,this,[this,add]{if(add->isEnabled())accept();});
        list->setCurrentRow(products_.empty()?-1:0);updateChoice();search->setFocus();
    }
    Component component() const{return products_.at(selected_);}
    int quantity() const{return quantity_->value();}
private:
    std::vector<Component> products_;int selected_=-1;QSpinBox* quantity_=nullptr;
};
