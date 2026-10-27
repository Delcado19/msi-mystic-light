#include "HardwarePicker.h"
#include "HardwareIdentity.h"
#include "HardwareArtwork.h"
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
#include <QSet>
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
bool isFan(const ComponentDefinition& c) { return hardwareCategory(c)=="Fans"; }
QIcon productIcon(const ComponentDefinition& c)
{
    QPixmap pixmap(112,88);pixmap.fill(Qt::transparent);QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    const auto category=hardwareArtworkCategory(c);
    static const QMap<QString,QSizeF> sizes{{"Fans",{72,72}},{"Pumps",{72,72}},
        {"Strips",{100,14}},{"RAM",{100,30}},{"Graphics cards",{100,46}},
        {"Keyboards",{100,43}},{"Mice",{44,72}},{"AiO",{100,57}},{"Motherboards",{58,72}},{"Cables",{100,24}}};
    const auto size=sizes.value(category,{88,66});
    const QRectF bounds(QPointF((112-size.width())/2,(88-size.height())/2),size);
    if(category=="Strips") {
        const auto body=hardwareBodyBounds(c);
        const double scale=std::min(bounds.width()/body.width(),bounds.height()/body.height());
        painter.translate(bounds.center());painter.scale(scale,scale);painter.translate(-body.center());
        drawHardwareComponentArtwork(painter,c);
    } else drawHardwareArtwork(painter,category,bounds);
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
            if(product.contains("Reverse",Qt::CaseInsensitive)) continue;
            const auto displayName=(product.startsWith(brand,Qt::CaseInsensitive) || (brand=="Phanteks" && product.startsWith("Phantek ",Qt::CaseInsensitive)) ? product : brand+" "+product).trimmed().left(160);
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
                c.manufacturer=brand.isEmpty() ? "Generic" : brand;
                c.category=type.contains("fan") ? "Fans" : type.contains("strip") ? "Strips" : type.contains("keyboard") ? "Keyboards" : type.contains("mouse") ? "Mice" : type.contains("aio") ? "AiO" : type.contains("pump") ? "Pumps" : hardwareCategory(c);
                // The local D30 metadata is a narrow strip diagram; use a fan footprint, preserving addresses and permutation.
                if(product.contains("D30-120",Qt::CaseInsensitive)) {
                    const auto ring=generic(c.id,c.name,int(c.positions.size()),true); c.positions=ring.positions;
                }
                // Multi-row Strimer cables retain their physical rows; only ordinary strips use a straight template.
                if(c.category=="Strips" && hardwareArtworkCategory(c)!="Cables") {
                    for(int i=0;i<c.positions.size();++i) c.positions[i]={3.0*i/std::max(1,int(c.positions.size())-1),0};
                }
                products.insert(c.id,ComponentDefinition::fromJson(c.toJson()));
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
    { auto c=generic("phanteks.m25","Phanteks M25 Gen2 120 White",6,true); products_.insert(c.id,c); }
    installedProducts(products_);
    for(auto it=products_.begin();it!=products_.end();) {
        if(hardwareCategory(*it)=="Fans" && it->name.contains("Reverse",Qt::CaseInsensitive)) it=products_.erase(it); else ++it;
    }
    auto* root=new QVBoxLayout(this); root->setContentsMargins(20,20,20,20); root->setSpacing(12);
    auto* heading=new QLabel("Choose your hardware"); heading->setProperty("heading",true); root->addWidget(heading);
    header_=new QComboBox; header_->setObjectName("hardwareHeader"); header_->setPlaceholderText("No supported header detected - connect your controller and refresh headers"); root->addWidget(header_);
    for(const auto& choice:headers_) { header_->addItem(choice.name,choice.identity); if(choice.identity==selected) header_->setCurrentIndex(header_->count()-1); }
    auto* search=new QLineEdit; search->setObjectName("hardwareSearch"); search->setPlaceholderText("Search fans, strips, brands or custom hardware"); search->setClearButtonEnabled(true); root->addWidget(search);
    auto* category=new QComboBox; category->setObjectName("hardwareCategory"); category->addItems({"All hardware","Fans","Strips","Mice","Keyboards","AiO","Pumps","RAM","Graphics cards","Motherboards","Other"}); root->addWidget(category);
    auto* manufacturer=new QComboBox; manufacturer->setObjectName("hardwareManufacturer"); manufacturer->addItem("All manufacturers");
    QStringList brands; for(const auto& c:products_) if(!brands.contains(hardwareManufacturer(c))) brands.append(hardwareManufacturer(c));
    brands.sort(Qt::CaseInsensitive); manufacturer->addItems(brands); root->addWidget(manufacturer);
    productsList_=new QListWidget; productsList_->setObjectName("hardwareProducts"); productsList_->setViewMode(QListView::IconMode);
    productsList_->setResizeMode(QListView::Adjust); productsList_->setMovement(QListView::Static); productsList_->setWordWrap(true);
    productsList_->setGridSize({210,134}); productsList_->setIconSize({96,76}); root->addWidget(productsList_,1);
    auto productIds=products_.keys();
    const auto priority=[](const QString& id) { return id.startsWith("phanteks.") ? 0 : id.startsWith("generic.") ? 3 : id.startsWith("hardware.") ? 2 : 1; };
    std::stable_sort(productIds.begin(),productIds.end(),[&](const QString& a,const QString& b) { return priority(a)<priority(b); });
    QSet<QByteArray> models;
    for(const auto& id:productIds) { const auto& c=products_[id];
        auto identity=c.toJson();identity.remove("id");identity.insert("name",hardwareName(c).toCaseFolded());
        const auto key=QJsonDocument(identity).toJson(QJsonDocument::Compact);
        // Merge cosmetic variants only when LED geometry and wire mapping also match; saved IDs remain valid.
        if(models.contains(key)) continue;
        models.insert(key);
        auto* item=new QListWidgetItem(productIcon(c),hardwareName(c),productsList_); item->setData(Qt::UserRole,c.id); item->setToolTip(QString("%1 / %2 addressable LEDs").arg(c.name).arg(c.positions.size())); }
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
    const auto filter=[this,search,category,manufacturer,update] {
        QListWidgetItem* first=nullptr;
        for(int i=0;i<productsList_->count();++i) { auto* item=productsList_->item(i); const auto c=products_[item->data(Qt::UserRole).toString()];
            const bool visible=hardwareName(c).contains(search->text(),Qt::CaseInsensitive) && (category->currentIndex()==0 || category->currentText()==hardwareCategory(c)) && (manufacturer->currentIndex()==0 || manufacturer->currentText()==hardwareManufacturer(c));
            item->setHidden(!visible); if(visible && !first) first=item;
        }
        if(!productsList_->currentItem() || productsList_->currentItem()->isHidden()) productsList_->setCurrentItem(first); update();
    };
    connect(search,&QLineEdit::textChanged,this,[filter] { filter(); }); connect(category,&QComboBox::currentIndexChanged,this,[filter] { filter(); }); connect(manufacturer,&QComboBox::currentIndexChanged,this,[filter] { filter(); });
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
