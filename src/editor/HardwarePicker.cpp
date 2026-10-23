#include "HardwarePicker.h"
#include <QComboBox>
#include <QCryptographicHash>
#include <QDialogButtonBox>
#include <QDirIterator>
#include <QFile>
#include <QFormLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QVersionNumber>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
ComponentDefinition generic(const QString& id,const QString& name,int count,bool fan)
{
    ComponentDefinition c; c.id=id; c.name=name;
    for(int i=0;i<count;++i) {
        const double angle=6.283185307179586*i/count;
        c.positions.append(fan ? QPointF(1.3+1.3*std::cos(angle),1.3+1.3*std::sin(angle)) : QPointF(i*.4,0)); c.mapping.append(i);
    }
    return c;
}
bool isFan(const ComponentDefinition& c) { return (c.id+" "+c.name).contains("fan",Qt::CaseInsensitive) || c.name.contains("M25"); }
QIcon productIcon(const ComponentDefinition& c)
{
    QPixmap pixmap(96,76); pixmap.fill(Qt::transparent); QPainter p(&pixmap); p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor("#788498"),2)); p.setBrush(QColor("#252c38"));
    if(isFan(c)) { p.drawRoundedRect(QRectF(16,6,64,64),8,8); p.drawEllipse(QRectF(22,12,52,52)); p.setBrush(QColor("#52657c")); p.drawEllipse(QPointF(48,38),10,10); }
    else { p.drawRoundedRect(QRectF(8,25,80,22),5,5); }
    p.setPen(Qt::NoPen);
    for(int i=0;i<8;++i) { p.setBrush(QColor::fromHsv(i*40,150,240)); p.drawEllipse(isFan(c) ? QPointF(48+25*std::cos(i*.7854),38+25*std::sin(i*.7854)) : QPointF(14+i*9.5,36),3,3); }
    return QIcon(pixmap);
}
void installedProducts(ComponentCatalog& products)
{
    QDir engine(qEnvironmentVariable("LOCALAPPDATA")+"/VortxEngine");
    auto versions=engine.entryList({"app-*"},QDir::Dirs|QDir::NoDotAndDotDot);
    std::sort(versions.begin(),versions.end(),[](const QString& a,const QString& b) { return QVersionNumber::fromString(a.mid(4))>QVersionNumber::fromString(b.mid(4)); });
    for(const auto& version:versions) {
        const auto directory=engine.filePath(version+"/Signal-x64/Components"); if(!QDir(directory).exists()) continue;
        QDirIterator files(directory,{"*.json"},QDir::Files,QDirIterator::Subdirectories);
        while(files.hasNext() && products.size()<2000) {
            QFile file(files.next()); if(file.size()>128*1024 || !file.open(QIODevice::ReadOnly)) continue;
            const auto bytes=file.read(128*1024+1); if(bytes.size()>128*1024 || file.error()!=QFileDevice::NoError) continue; const auto json=QJsonDocument::fromJson(bytes).object();
            const auto product=json.value("ProductName").toString().trimmed(), brand=json.value("Brand").toString().trimmed();
            if(product.isEmpty()) continue;
            const auto displayName=(product.startsWith(brand,Qt::CaseInsensitive) ? product : brand+" "+product).trimmed().left(160);
            const auto count=json.value("LedCount"); const double number=count.toDouble(-1);
            if(number<1 || number>120 || std::floor(number)!=number) continue;
            const auto type=json.value("Type").toString().toLower();
            if(type.contains("ram") || type.contains("gpu") || type.contains("motherboard")) continue;
            auto coordinates=json.value("LedCoordinates").toArray(), mapping=json.value("LedMapping").toArray();
            if(coordinates.size()<number || mapping.size()<number) continue;
            while(coordinates.size()>number) coordinates.removeLast(); while(mapping.size()>number) mapping.removeLast();
            try {
                // Import only validated geometry/mapping metadata. No upstream images, scripts or drivers are copied.
                auto c=ComponentDefinition::fromJson({{"schemaVersion",1},{"id","hardware."+QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex().left(16))},
                    {"name",displayName}, {"ledCount",number},{"positions",coordinates},{"mapping",mapping}});
                double left=1e100,top=1e100,right=-1e100,bottom=-1e100;
                for(const auto& point:c.positions) {
                    if(std::abs(point.x())>100000 || std::abs(point.y())>100000) throw std::invalid_argument("Invalid product coordinates");
                    left=std::min(left,point.x()); top=std::min(top,point.y()); right=std::max(right,point.x()); bottom=std::max(bottom,point.y());
                }
                const double scale=3.0/std::max(1.0,std::max(right-left,bottom-top));
                for(auto& point:c.positions) point=(point-QPointF(left,top))*scale;
                products.insert(c.id,c);
            } catch(const std::exception&) { /* Malformed local metadata is not a usable hardware definition. */ }
        }
        break;
    }
}
}
HardwarePicker::HardwarePicker(const ComponentCatalog& catalog,const QVector<HeaderChoice>& headers,const QVariantMap& selected,QWidget* parent)
    :QDialog(parent),products_(catalog),headers_(headers)
{
    setObjectName("hardwarePicker"); setWindowTitle("Add hardware"); resize(760,640);
    for(int count:{6,8,12,16}) { auto c=generic(QString("generic.fan.%1").arg(count),QString("Generic fan / %1 addresses").arg(count),count,true); if(!products_.contains(c.id)) products_.insert(c.id,c); }
    for(int count:{8,30,60}) { auto c=generic(QString("generic.strip.%1").arg(count),QString("Generic strip / %1 LEDs").arg(count),count,false); if(!products_.contains(c.id)) products_.insert(c.id,c); }
    // Restore the original user's installed M25 models: six addresses for paired physical LEDs.
    for(const auto& model:{"Phanteks M25 Gen2 120 White","Phanteks M25 Gen2 120 Reverse White"}) {
        auto c=generic(QString(model).contains("Reverse") ? "phanteks.m25.reverse" : "phanteks.m25",model,6,true); products_.insert(c.id,c);
    }
    installedProducts(products_);
    auto* root=new QVBoxLayout(this); root->setContentsMargins(20,20,20,20); root->setSpacing(12);
    auto* heading=new QLabel("Choose your hardware"); heading->setProperty("heading",true); root->addWidget(heading);
    header_=new QComboBox; header_->setObjectName("hardwareHeader"); header_->setPlaceholderText("No supported header detected - connect your controller and refresh headers"); root->addWidget(header_);
    for(const auto& choice:headers_) { header_->addItem(choice.name,choice.identity); if(choice.identity==selected) header_->setCurrentIndex(header_->count()-1); }
    auto* search=new QLineEdit; search->setObjectName("hardwareSearch"); search->setPlaceholderText("Search fans, strips, brands or custom hardware"); search->setClearButtonEnabled(true); root->addWidget(search);
    auto* category=new QComboBox; category->setObjectName("hardwareCategory"); category->addItems({"All hardware","Fans","Strips / other"}); root->addWidget(category);
    productsList_=new QListWidget; productsList_->setObjectName("hardwareProducts"); productsList_->setViewMode(QListView::IconMode);
    productsList_->setResizeMode(QListView::Adjust); productsList_->setMovement(QListView::Static); productsList_->setWordWrap(true);
    productsList_->setGridSize({210,134}); productsList_->setIconSize({96,76}); root->addWidget(productsList_,1);
    auto productIds=products_.keys();
    const auto priority=[](const QString& id) { return id.startsWith("phanteks.") ? 0 : id.startsWith("generic.") ? 3 : id.startsWith("hardware.") ? 2 : 1; };
    std::stable_sort(productIds.begin(),productIds.end(),[&](const QString& a,const QString& b) { return priority(a)<priority(b); });
    for(const auto& id:productIds) { const auto& c=products_[id]; auto* item=new QListWidgetItem(productIcon(c),c.name,productsList_); item->setData(Qt::UserRole,c.id); item->setToolTip(QString("%1 / %2 addressable LEDs").arg(c.name).arg(c.positions.size())); }
    auto* details=new QLabel; details->setWordWrap(true); details->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum); details->setMinimumHeight(details->fontMetrics().lineSpacing()*2); root->addWidget(details);
    auto* form=new QFormLayout; root->addLayout(form);
    quantity_=new QSpinBox; quantity_->setObjectName("hardwareQuantity"); quantity_->setRange(1,64); form->addRow("Quantity",quantity_);
    leds_=new QSpinBox; leds_->setObjectName("hardwareLedCount"); leds_->setRange(1,120); form->addRow("LEDs per component",leds_);
    leds_->setToolTip("Generic geometry only: set the actual addressable LED count. Product mappings retain their verified count.");
    auto* buttons=new QDialogButtonBox(QDialogButtonBox::Cancel); auto* add=buttons->addButton("Add to header",QDialogButtonBox::AcceptRole); add->setObjectName("confirmAddHardware"); root->addWidget(buttons);
    const auto update=[this,details,add] {
        const auto* item=productsList_->currentItem(); const bool selected=item && !item->isHidden();
        const auto c=selected ? products_[item->data(Qt::UserRole).toString()] : ComponentDefinition{};
        const bool generic=c.id.startsWith("generic."); leds_->setEnabled(generic);
        const int count=generic ? leds_->value() : int(c.positions.size()), index=header_->currentIndex();
        const int remaining=index>=0 ? headers_[index].remaining : 0;
        add->setEnabled(selected && count*quantity_->value()<=remaining && (index<0 || headers_[index].required==0 || count*quantity_->value()==headers_[index].required));
        details->setText(selected ? QString("%1 / %2 LEDs in this chain / %3 addresses available").arg(c.name).arg(count*quantity_->value()).arg(remaining) : "Choose a product to add to this header.");
    };
    connect(productsList_,&QListWidget::currentItemChanged,this,[this,update](QListWidgetItem* item) {
        if(item) { const QSignalBlocker block(leds_); leds_->setValue(int(products_[item->data(Qt::UserRole).toString()].positions.size())); } update();
    });
    connect(header_,&QComboBox::currentIndexChanged,this,[update] { update(); });
    connect(quantity_,&QSpinBox::valueChanged,this,[update] { update(); }); connect(leds_,&QSpinBox::valueChanged,this,[update] { update(); });
    const auto filter=[this,search,category,update] {
        QListWidgetItem* first=nullptr;
        for(int i=0;i<productsList_->count();++i) { auto* item=productsList_->item(i); const auto c=products_[item->data(Qt::UserRole).toString()];
            const bool visible=c.name.contains(search->text(),Qt::CaseInsensitive) && (category->currentIndex()==0 || (category->currentIndex()==1)==isFan(c));
            item->setHidden(!visible); if(visible && !first) first=item;
        }
        if(!productsList_->currentItem() || productsList_->currentItem()->isHidden()) productsList_->setCurrentItem(first); update();
    };
    connect(search,&QLineEdit::textChanged,this,[filter] { filter(); }); connect(category,&QComboBox::currentIndexChanged,this,[filter] { filter(); });
    connect(buttons,&QDialogButtonBox::accepted,this,&QDialog::accept); connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
    if(productsList_->count()) productsList_->setCurrentRow(0); update();
}
ComponentDefinition HardwarePicker::component() const
{
    const auto* item=productsList_->currentItem(); if(!item) throw std::invalid_argument("Choose hardware first");
    auto c=products_[item->data(Qt::UserRole).toString()];
    if(c.id.startsWith("generic.") && c.positions.size()!=leds_->value()) c=generic(QString("generic.%1.%2").arg(isFan(c) ? "fan" : "strip").arg(leds_->value()),QString("Generic %1 / %2 LEDs").arg(isFan(c) ? "fan" : "strip").arg(leds_->value()),leds_->value(),isFan(c));
    return ComponentDefinition::fromJson(c.toJson());
}
int HardwarePicker::quantity() const { return quantity_->value(); }
QVariantMap HardwarePicker::header() const { return header_->currentData().toMap(); }
