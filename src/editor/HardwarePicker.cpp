#include "HardwarePicker.h"
#include "HardwareIdentity.h"
#include "HardwareArtwork.h"
#include "StudioWidgets.h"
#include "ComponentDraft.h"
#include <QButtonGroup>
#include <QToolButton>
#include <QComboBox>
#include <QCryptographicHash>
#include <QDialogButtonBox>
#include <QDirIterator>
#include <QFile>
#include <QFormLayout>
#include <QStyle>
#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QFrame>
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
QIcon productIcon(const ComponentDefinition& c)
{
    QPixmap pixmap(112,88);pixmap.fill(Qt::transparent);QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    const auto category=hardwareArtworkCategory(c);
    const auto photo=hardwarePreviewPhoto(c);
    if (!photo.isNull()) {
        const auto thumbnail=photo.scaled(104,80,Qt::KeepAspectRatio,Qt::SmoothTransformation);
        painter.drawPixmap((112-thumbnail.width())/2,(88-thumbnail.height())/2,thumbnail);
        return QIcon(pixmap);
    }
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
                // LEDs on an identical coordinate sit behind each other (SWAFAN and Freezer 36 rings, CoreLiquid
                // LED 4/5, Vertagear SL5000): keep positions for sampling, separate them by layer. Computed after
                // the D30/strip templates, which make every position distinct.
                // More than 16 LEDs on one point exceed the 0..15 layer range: keep the product without layers
                // instead of dropping it in the catch below (CodeRabbit PR #10).
                const auto layers=overlapLayers(c.positions);
                if(std::all_of(layers.cbegin(),layers.cend(),[](int layer){return layer<=15;})) c.layers=layers;
                products.insert(c.id,ComponentDefinition::fromJson(c.toJson()));
            } catch(const std::exception&) { /* Malformed local metadata is not a usable hardware definition. */ }
        }
        break;
    }
}
}
ComponentCatalog hardwareProducts(const ComponentCatalog& catalog)
{
    // Built-in definitions are shipped files in components/ only (plan 2026-11-27 Task 4); the code adds
    // just the validated imports of locally installed vendor metadata ("hardware.*").
    auto products=catalog;
    installedProducts(products);
    for(auto it=products.begin();it!=products.end();) {
        if(hardwareCategory(*it)=="Fans" && it->name.contains("Reverse",Qt::CaseInsensitive)) it=products.erase(it); else ++it;
    }
    return products;
}
namespace {
constexpr int sectionRole = Qt::UserRole + 1; // "section" header rows, "create" row
QListWidgetItem* sectionRow(QListWidget* list, const QString& text) // list may be null: insert it yourself
{
    auto* item = new QListWidgetItem(text, list);
    item->setFlags(Qt::NoItemFlags); item->setData(sectionRole, "section");
    auto font = item->font(); font.setBold(true); item->setFont(font);
    return item;
}
}
HardwarePicker::HardwarePicker(const ComponentCatalog& builtIn,const ComponentCatalog& user,const QVector<HeaderChoice>& headers,const QVariantMap& selected,QWidget* parent)
    :QDialog(parent),headers_(headers)
{
    setObjectName("hardwarePicker"); setWindowTitle("Attach component"); resize(720,680);
    products_=hardwareProducts(builtIn);
    for(const auto& definition:user) { products_.insert(definition.id,definition); user_.insert(definition.id); }
    // Per-placement copies (instance.*, restored.*) are variants of placed parts, not products to add;
    // they showed one AiO twice in Add hardware (owner finding 2026-11-26). They stay in the Component Editor.
    for(auto it=products_.begin();it!=products_.end();) {
        if(it.key().startsWith("instance.") || it.key().startsWith("restored.")) it=products_.erase(it); else ++it;
    }
    auto* root=new QVBoxLayout(this); root->setContentsMargins(20,20,20,20); root->setSpacing(12);
    // Header row: which header, left; what is left on it, right (owner design 2026-11-27).
    auto* headerRow=new QHBoxLayout; root->addLayout(headerRow);
    header_=new QComboBox; header_->setObjectName("hardwareHeader"); header_->setAccessibleName("Header");
    header_->setPlaceholderText("No supported header detected - connect your controller and refresh headers");
    remaining_=new QLabel; remaining_->setObjectName("attachRemaining"); remaining_->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    headerRow->addWidget(header_,1); headerRow->addWidget(remaining_);
    for(const auto& choice:headers_) {
        header_->addItem(choice.kind.isEmpty() ? choice.name : choice.name+"  ·  "+choice.kind,choice.identity);
        if(auto* model=qobject_cast<QStandardItemModel*>(header_->model()); model && !choice.supported) {
            auto* item=model->item(header_->count()-1); item->setFlags(item->flags() & ~Qt::ItemIsEnabled); item->setToolTip(choice.reason);
        }
        if(choice.identity==selected && choice.supported) header_->setCurrentIndex(header_->count()-1);
    }
    if(header_->currentIndex()<0) for(int i=0;i<headers_.size();++i) if(headers_[i].supported) { header_->setCurrentIndex(i); break; }
    // Segmented search: type and manufacturer are their own criteria, combined with the text.
    auto* searchBar=new QFrame; searchBar->setObjectName("attachSearchBar"); searchBar->setFrameShape(QFrame::StyledPanel);
    auto* searchLayout=new QHBoxLayout(searchBar); searchLayout->setContentsMargins(4,4,4,4); searchLayout->setSpacing(4);
    category_=new QComboBox; category_->setObjectName("hardwareCategory"); category_->setAccessibleName("Component type");
    category_->addItems({"All types","Fans","Strips","Mice","Keyboards","AiO","Pumps","RAM","Graphics cards","Motherboards","Other"});
    manufacturer_=new QComboBox; manufacturer_->setObjectName("hardwareManufacturer"); manufacturer_->setAccessibleName("Manufacturer"); manufacturer_->addItem("All manufacturers");
    QStringList brands; for(const auto& c:products_) if(!brands.contains(hardwareManufacturer(c))) brands.append(hardwareManufacturer(c));
    brands.sort(Qt::CaseInsensitive); manufacturer_->addItems(brands);
    search_=new QLineEdit; search_->setObjectName("hardwareSearch"); search_->setPlaceholderText("Search components"); search_->setClearButtonEnabled(true);
    search_->addAction(studioToolIcon("search"),QLineEdit::LeadingPosition);
    searchLayout->addWidget(category_); searchLayout->addWidget(manufacturer_); searchLayout->addWidget(search_,1);
    root->addWidget(searchBar);
    productsList_=new QListWidget; productsList_->setObjectName("hardwareProducts"); productsList_->setIconSize({64,48});
    productsList_->setUniformItemSizes(false); productsList_->setSpacing(2); root->addWidget(productsList_,1);
    // My components first, then Built-in; each sorted by manufacturer, then name.
    QStringList mine, shipped;
    for(const auto& id:products_.keys()) (user_.contains(id) ? mine : shipped).append(id);
    const auto byMaker=[this](const QString& a,const QString& b) {
        const auto& x=products_[a]; const auto& y=products_[b];
        const int maker=QString::compare(hardwareManufacturer(x),hardwareManufacturer(y),Qt::CaseInsensitive);
        return maker!=0 ? maker<0 : QString::compare(hardwareName(x),hardwareName(y),Qt::CaseInsensitive)<0;
    };
    std::stable_sort(mine.begin(),mine.end(),byMaker); std::stable_sort(shipped.begin(),shipped.end(),byMaker);
    QSet<QByteArray> models;
    const auto addSection=[&](const QString& title,const QStringList& ids) {
        if(ids.isEmpty()) return;
        sectionRow(productsList_,title);
        for(const auto& id:ids) { const auto& c=products_[id];
            auto identity=c.toJson();identity.remove("id");identity.insert("name",hardwareName(c).toCaseFolded());
            const auto key=QJsonDocument(identity).toJson(QJsonDocument::Compact);
            // Merge cosmetic variants only when LED geometry and wire mapping also match; saved IDs remain valid.
            if(models.contains(key)) continue;
            models.insert(key);
            auto* item=new QListWidgetItem(productIcon(c),hardwareName(c),productsList_); item->setData(Qt::UserRole,c.id);
            item->setToolTip(QString("%1 · %2 addresses").arg(c.name).arg(c.addresses()));
        }
    };
    addSection("My components",mine); addSection("Built-in",shipped);
    // Empty result: offer to create it, the search text becomes the name (Task 7 opens the form).
    create_=new QListWidgetItem(productsList_); create_->setData(sectionRole,"create"); create_->setHidden(true);
    newRow_=new QListWidgetItem("+ New component",productsList_); newRow_->setData(sectionRole,"create");
    // Quick-create form (Option A, owner 2026-11-27): type, manufacturer, name, addresses; the editor is
    // one click away for anything more complex.
    quickForm_=new QFrame; quickForm_->setObjectName("quickForm"); static_cast<QFrame*>(quickForm_)->setFrameShape(QFrame::StyledPanel);
    auto* quickLayout=new QFormLayout(quickForm_);
    auto* types=new QWidget; types->setObjectName("quickType"); auto* typeRow=new QHBoxLayout(types); typeRow->setContentsMargins(0,0,0,0);
    auto* typeGroup=new QButtonGroup(types);
    for(const auto* type:{"Fans","Strips","Ring","RAM","Other"}) {
        auto* tile=new QToolButton; tile->setText(type); tile->setObjectName(QString("quickType_%1").arg(type)); tile->setCheckable(true);
        tile->setChecked(quickType_==type); typeGroup->addButton(tile); typeRow->addWidget(tile);
        connect(tile,&QToolButton::clicked,this,[this,type] { quickType_=type; });
    }
    typeRow->addStretch(1); quickLayout->addRow("Type",types);
    quickManufacturer_=new QComboBox; quickManufacturer_->setObjectName("quickManufacturer"); quickManufacturer_->setEditable(true);
    quickManufacturer_->addItems(brands); quickManufacturer_->setCurrentText(QString()); quickLayout->addRow("Manufacturer",quickManufacturer_);
    quickName_=new QLineEdit; quickName_->setObjectName("quickName"); quickLayout->addRow("Name",quickName_);
    quickAddresses_=new QSpinBox; quickAddresses_->setObjectName("quickAddresses"); quickAddresses_->setRange(1,1024); quickAddresses_->setValue(12);
    quickLayout->addRow("Addresses",quickAddresses_);
    quickError_=new QLabel; quickError_->setObjectName("quickError"); quickError_->setWordWrap(true); quickLayout->addRow(quickError_);
    auto* quickButtons=new QHBoxLayout;
    auto* editor=new QPushButton("Open in editor…"); editor->setObjectName("quickOpenEditor");
    auto* createAttach=new QPushButton("Create && attach"); createAttach->setObjectName("quickCreateAttach"); createAttach->setProperty("primary",true);
    quickButtons->addWidget(editor); quickButtons->addStretch(1); quickButtons->addWidget(createAttach); quickLayout->addRow(quickButtons);
    root->addWidget(quickForm_); quickForm_->hide();
    connect(createAttach,&QPushButton::clicked,this,[this] { quickCreate(); });
    connect(editor,&QPushButton::clicked,this,[this] { done(OpenEditor); });
    // Selected component: name left, its addresses right.
    auto* selectionRow=new QHBoxLayout; root->addLayout(selectionRow);
    selectionName_=new QLabel; selectionName_->setObjectName("attachSelectionName");
    selectionAddresses_=new QLabel; selectionAddresses_->setObjectName("attachSelectionAddresses"); selectionAddresses_->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    selectionRow->addWidget(selectionName_,1); selectionRow->addWidget(selectionAddresses_);
    auto* form=new QFormLayout; root->addLayout(form);
    quantity_=new QSpinBox; quantity_->setObjectName("hardwareQuantity"); quantity_->setRange(1,64); form->addRow("Quantity",quantity_);
    auto* buttons=new QDialogButtonBox(QDialogButtonBox::Cancel);
    attach_=buttons->addButton("Attach",QDialogButtonBox::AcceptRole); attach_->setObjectName("confirmAddHardware"); root->addWidget(buttons);
    connect(productsList_,&QListWidget::currentItemChanged,this,[this](QListWidgetItem* item) {
        if(item==create_) showQuickForm(search_->text().trimmed());
        else if(item==newRow_) showQuickForm(QString());
        else quickForm_->hide();
        update();
    });
    connect(header_,&QComboBox::currentIndexChanged,this,[this] { update(); });
    connect(quantity_,&QSpinBox::valueChanged,this,[this] { update(); });
    connect(search_,&QLineEdit::textChanged,this,[this] { filter(); });
    connect(category_,&QComboBox::currentIndexChanged,this,[this] { filter(); });
    connect(manufacturer_,&QComboBox::currentIndexChanged,this,[this] { filter(); });
    connect(buttons,&QDialogButtonBox::accepted,this,&QDialog::accept); connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
    update();
}
void HardwarePicker::update()
{
    const auto* item=productsList_->currentItem();
    const bool selected=item && !item->isHidden() && !item->data(Qt::UserRole).toString().isEmpty();
    const auto c=selected ? products_[item->data(Qt::UserRole).toString()] : ComponentDefinition{};
    const int index=header_->currentIndex();
    const int remaining=index>=0 ? headers_[index].remaining : 0, needed=selected ? c.addresses()*quantity_->value() : 0;
    const int required=index>=0 ? headers_[index].required : 0;
    selectionName_->setText(selected ? hardwareName(c) : QString("Choose a component"));
    selectionAddresses_->setText(selected ? QString("%1 addresses").arg(c.addresses()) : QString());
    if(index<0) remaining_->clear();
    else if(!selected) remaining_->setText(QString("%1 addresses remaining").arg(remaining));
    else if(needed>remaining) remaining_->setText(QString("%1 addresses remaining · exceeds by %2").arg(remaining).arg(needed-remaining));
    else if(required>0 && needed!=required) remaining_->setText(QString("needs exactly %1 addresses").arg(required));
    else remaining_->setText(QString("%1 → %2 addresses remaining").arg(remaining).arg(remaining-needed));
    attach_->setEnabled(selected && index>=0 && headers_[index].supported && needed<=remaining && (required==0 || needed==required));
}
void HardwarePicker::filter()
{
    QListWidgetItem* first=nullptr; QListWidgetItem* section=nullptr; bool sectionUsed=false;
    for(int i=0;i<productsList_->count();++i) { auto* item=productsList_->item(i);
        const auto role=item->data(sectionRole).toString();
        if(role=="create") continue;
        if(role=="section") { if(section) section->setHidden(!sectionUsed); section=item; sectionUsed=false; continue; }
        const auto c=products_[item->data(Qt::UserRole).toString()];
        const bool visible=(hardwareName(c).contains(search_->text(),Qt::CaseInsensitive) || c.name.contains(search_->text(),Qt::CaseInsensitive))
            && (category_->currentIndex()==0 || category_->currentText()==hardwareCategory(c))
            && (manufacturer_->currentIndex()==0 || manufacturer_->currentText()==hardwareManufacturer(c));
        item->setHidden(!visible); if(visible && !first) first=item; sectionUsed=sectionUsed || visible;
    }
    if(section) section->setHidden(!sectionUsed);
    const auto text=search_->text().trimmed();
    create_->setText(QString("+ Create \"%1\"").arg(text)); create_->setHidden(first || text.isEmpty());
    newRow_->setHidden(!create_->isHidden());
    if(!productsList_->currentItem() || productsList_->currentItem()->isHidden() || productsList_->currentItem()->data(Qt::UserRole).toString().isEmpty()) productsList_->setCurrentItem(first);
    update();
}
void HardwarePicker::showQuickForm(const QString& name)
{
    quickName_->setText(name); quickError_->clear(); quickForm_->show(); quickName_->setFocus();
}
void HardwarePicker::quickCreate()
{
    try {
        if(quickName_->text().trimmed().isEmpty()) throw std::invalid_argument("Enter a name.");
        if(!saveComponent) throw std::invalid_argument("Saving components is not available here.");
        // Same shapes the Component Editor's shape buttons use: fans and rings as a circle, the rest as a line.
        ComponentDraft draft; draft.name=quickName_->text().trimmed(); draft.manufacturer=quickManufacturer_->currentText().trimmed();
        const auto shape=quickType_=="Fans" ? DraftShape::Fan : quickType_=="Ring" ? DraftShape::Ring : DraftShape::Strip;
        draft=withShape(draft,shape,quickAddresses_->value());
        draft.category=quickType_=="Ring" ? QString("Other") : quickType_;
        draft.id="custom.new"; // saveComponent assigns the real ID
        auto definition=draft.toDefinition();
        definition.id=saveComponent(definition);
        products_.insert(definition.id,definition); user_.insert(definition.id);
        // Into "My components": after its header, which is created on top when it did not exist yet.
        int row=0;
        if(productsList_->count()==0 || productsList_->item(0)->text()!="My components") productsList_->insertItem(0,sectionRow(nullptr,"My components"));
        row=1;
        auto* item=new QListWidgetItem(productIcon(definition),hardwareName(definition)); item->setData(Qt::UserRole,definition.id);
        item->setToolTip(QString("%1 · %2 addresses").arg(definition.name).arg(definition.addresses()));
        productsList_->insertItem(row,item); productsList_->item(0)->setHidden(false);
        quickForm_->hide(); quantity_->setValue(1); productsList_->setCurrentItem(item);
        if(attach_->isEnabled()) accept();
        // quickForm_ (and quickError_ in it) is hidden now: report on the visible selection line (CodeRabbit PR #10).
        else selectionAddresses_->setText(selectionAddresses_->text()+" · Saved to your components, but it does not fit this header.");
    } catch(const std::exception& error) { quickError_->setText(QString::fromUtf8(error.what())); quickForm_->show(); }
}
QString HardwarePicker::editorName() const { return quickName_->text().trimmed(); }
void HardwarePicker::selectComponent(const QString& id)
{
    for(int i=0;i<productsList_->count();++i) if(productsList_->item(i)->data(Qt::UserRole).toString()==id) { productsList_->setCurrentRow(i); return; }
}
ComponentDefinition HardwarePicker::component() const
{
    const auto* item=productsList_->currentItem(); if(!item) throw std::invalid_argument("Choose hardware first");
    if(item->data(Qt::UserRole).toString().isEmpty()) throw std::invalid_argument("Choose hardware first");
    return ComponentDefinition::fromJson(products_[item->data(Qt::UserRole).toString()].toJson());
}
int HardwarePicker::quantity() const { return quantity_->value(); }
QVariantMap HardwarePicker::header() const { return header_->currentData().toMap(); }
