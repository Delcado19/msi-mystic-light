#pragma once
#include "device/Component.h"
#include "ui/DeviceArtwork.h"
#include <QDialog>
#include <QSignalBlocker>
#include <QComboBox>
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
    if(deviceArtworkKind(component)=="strip"){QSvgRenderer renderer(QStringLiteral(":/studio/devices/strip-card.svg"));renderer.render(&p,QRectF(2,2,76,60));}
    else drawDeviceArtwork(p,QRectF(2,2,76,60),component);
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
        auto* category=new QComboBox(this);category->setObjectName("hardwareCategory");category->addItem("All hardware");
        for(const auto& c:products_){const auto name=hardwareCategory(c);if(category->findText(name)<0)category->addItem(name);}root->addWidget(category);
        auto* list=new QListWidget(this);list->setObjectName("hardwareProducts");list->setViewMode(QListView::IconMode);list->setResizeMode(QListView::Adjust);list->setMovement(QListView::Static);list->setWordWrap(true);list->setGridSize(QSize(210,135));list->setIconSize(QSize(80,64));root->addWidget(list,1);
        for(const auto& c:products_){auto* item=new QListWidgetItem(hardwareIcon(c),compactHardwareName(c),list);item->setToolTip(QString::fromStdString(c.productName)+" / "+hardwareLightingDescription(c));}
        count_=new QSpinBox(this);count_->setObjectName("hardwareVerifiedCount");count_->setRange(0,120);count_->setSpecialValueText("Enter verified address count");root->addWidget(count_);
        auto* details=new QLabel(this);details->setWordWrap(true);root->addWidget(details);
        auto* row=new QHBoxLayout;root->addLayout(row);row->addWidget(new QLabel("Quantity",this));
        quantity_=new QSpinBox(this);quantity_->setObjectName("hardwareQuantity");quantity_->setRange(1,120);row->addWidget(quantity_);row->addStretch();
        auto* custom=new QPushButton("Create custom hardware",this);custom->setObjectName("pickerCreateCustom");row->addWidget(custom);
        auto* buttons=new QDialogButtonBox(QDialogButtonBox::Cancel,this);auto* add=buttons->addButton("Add to "+channel,QDialogButtonBox::AcceptRole);add->setObjectName("confirmAddHardware");root->addWidget(buttons);
        const auto updateChoice=[this,list,details,add,remaining]{
            selected_=list->currentRow();const bool selected=selected_>=0 && !list->item(selected_)->isHidden();
            const bool needsCount=selected && products_[selected_].type.find("count required")!=std::string::npos;
            count_->setVisible(needsCount);
            const bool dedicated=selected && (hardwareCategory(products_[selected_])=="RAM" || (hardwareCategory(products_[selected_])=="Heatsinks / GPU" || hardwareCategory(products_[selected_])=="Graphics cards") || hardwareCategory(products_[selected_])=="Motherboards");
            const int count=selected?(needsCount?count_->value():products_[selected_].ledCount):0;const int maximum=count?remaining/count:0;
            quantity_->setMaximum(qMax(1,maximum));quantity_->setEnabled(maximum>0);add->setEnabled(maximum>0 && !dedicated);
            details->setText(selected?QString::fromStdString(products_[selected_].productName)+" / "+(dedicated?hardwareLightingDescription(products_[selected_]):needsCount?QString("%1 addresses / physical LED count unverified").arg(count):hardwareLightingDescription(products_[selected_]))+(dedicated?QString(" / dedicated controller required; cannot connect to an ARGB header"):needsCount && count==0?QString(" / LED count not verified; enter the actual address count"):maximum?QString():QString(" / does not fit this channel")):QString("Choose a component to add."));
        };
        connect(count_,&QSpinBox::valueChanged,this,[updateChoice]{updateChoice();});
        connect(list,&QListWidget::currentRowChanged,this,[this,updateChoice]{const QSignalBlocker blocker(count_);count_->setValue(0);updateChoice();});
        const auto filter=[this,list,updateChoice,search,category]{const auto query=search->text();int first=-1;for(int i=0;i<list->count();++i){const bool match=(category->currentIndex()==0 || hardwareCategory(products_[i])==category->currentText()) && (list->item(i)->text()+" "+list->item(i)->toolTip()).contains(query.trimmed(),Qt::CaseInsensitive);list->item(i)->setHidden(!match);if(match && first<0)first=i;}list->setCurrentRow(first);updateChoice();};
        connect(search,&QLineEdit::textChanged,this,filter);connect(category,&QComboBox::currentTextChanged,this,filter);
        connect(custom,&QPushButton::clicked,this,[this]{customRequested=true;reject();});
        connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
        connect(add,&QPushButton::clicked,this,[this,add]{if(add->isEnabled())accept();});
        list->setCurrentRow(products_.empty()?-1:0);updateChoice();search->setFocus();
    }
    Component component() const{auto c=products_.at(selected_);if(c.type.find("count required")!=std::string::npos){auto configured=Component::makeGenericPlaceholder(count_->value());configured.type=c.type.substr(0,c.type.find(" ("));configured.brand=c.brand;configured.productName=c.productName;configured.displayName=c.displayName;configured.arrange(configured.type=="AIO"?"Circle":"Line");c=std::move(configured);}return c;}
    int quantity() const{return quantity_->value();}
private:
    std::vector<Component> products_;int selected_=-1;QSpinBox* quantity_=nullptr;QSpinBox* count_=nullptr;
};
