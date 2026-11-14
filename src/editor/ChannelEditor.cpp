#include "ParameterSpinBox.h"
#include "ChannelEditor.h"
#include "StudioWidgets.h"
#include <QToolButton>
#include <QPixmap>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QRadioButton>
#include <QAbstractItemModel>
#include <algorithm>
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <stdexcept>

ChannelEditor::ChannelEditor(const LightingDocument& document, ChannelAssignment initial, QWidget* parent)
    : QDialog(parent), initial_(std::move(initial))
{
    const auto& profile = assignmentProfile(document.profiles(), initial_.profileId);
    const auto& channel = assignmentChannel(profile, initial_.channelId);
    fixedComponent_=channelHasFixedComponent(channel);
    setWindowTitle(fixedComponent_ ? "Device settings - "+profile.name : "Configure channel - "+channel.id); resize(fixedComponent_ ? 560 : 860, fixedComponent_ ? 400 : 680);
    applyStudioStyle(this);
    auto* layout = new QVBoxLayout(this);
    auto* identity = new QLabel(profile.name + " / " + channel.id + "\n" + initial_.endpoint);
    identity->setWordWrap(true); layout->addWidget(identity);
    layout->addWidget(new QLabel(fixedComponent_ ? "Standalone device: its component assignment is fixed." : QString("Capacity: %1 LEDs; fixed count: %2. No device output.").arg(channel.maxLeds).arg(channel.fixedLeds)));
    auto* form = new QFormLayout; layout->addLayout(form);
    auto* mode = new QComboBox; mode->setObjectName("channelMode");
    if (!channel.nativeModes.isEmpty()) mode->addItem("Native firmware", false);
    if (channel.direct) mode->addItem("Software canvas", true);
    mode->setCurrentIndex(mode->findData(initial_.software)); form->addRow("Output mode", mode);
    auto* rate=new ParameterSpinBox; rate->setObjectName("channelRate"); rate->setRange(0,10); rate->setDisplayUnit(100," %"); rate->setValue(initial_.timeScale);
    auto* offset=new ParameterSpinBox; offset->setObjectName("channelOffset"); offset->setRange(0,86400); offset->setDisplayUnit(1000," ms"); offset->setValue(initial_.timeOffset);
    rate->setProperty("initialValue",rate->value());offset->setProperty("initialValue",offset->value());
    auto* clock=new QWidget; auto* clockForm=new QFormLayout(clock);
    rate->setAccessibleName("Channel clock rate"); clockForm->addRow("Playback speed",sliderField(rate)); clockForm->addRow("Time offset",offset);
    auto* clockSection=disclosure(layout,fixedComponent_ ? "Device playback clock" : "Channel playback clock",clock);
    auto clockEnabled=[mode,rate,offset] { rate->setEnabled(mode->currentData().toBool()); offset->setEnabled(mode->currentData().toBool()); };
    connect(mode,&QComboBox::currentIndexChanged,this,clockEnabled); clockEnabled();
    auto* native = new QWidget; auto* nativeForm = new QFormLayout(native); layout->addWidget(native);
    auto* effect = new QComboBox; effect->setObjectName("nativeMode"); effect->addItems(channel.nativeModes);
    effect->setCurrentIndex(qMax(0, effect->findText(initial_.native.id))); nativeForm->addRow("Firmware effect", effect);
    auto* nativeColors=new ColorListEditor(1,4); nativeColors->setObjectName("nativeColors");
    QVector<QColor> colors; for(const auto& color:initial_.native.colors) colors.append(QColor(color.r,color.g,color.b));
    nativeColors->setColors(colors); nativeForm->addRow("Colors",nativeColors);
    auto* brightness = new QSpinBox; brightness->setObjectName("nativeBrightness"); brightness->setRange(0,5);
    brightness->setSpecialValueText("Level 0 / 5");brightness->setPrefix("Level ");brightness->setSuffix(" / 5");brightness->setValue(initial_.native.brightness); nativeForm->addRow("Brightness", brightness);
    auto* speed = new QSpinBox; speed->setObjectName("nativeSpeed"); speed->setRange(0,2);
    speed->setPrefix("Hardware level ");speed->setSuffix(" / 2");speed->setToolTip("Three firmware speed levels. Their physical ordering depends on the selected device and effect.");speed->setValue(initial_.native.speed); nativeForm->addRow("Speed", speed);
    auto* inward = new QCheckBox("Inward direction"); inward->setObjectName("nativeInward"); inward->setChecked(initial_.native.inward);
    nativeForm->addRow(inward);
    connect(mode, &QComboBox::currentIndexChanged, this, [mode,native] { native->setEnabled(!mode->currentData().toBool()); });
    native->setEnabled(!mode->currentData().toBool());
    native->setVisible(!mode->currentData().toBool());
    // Written for someone who has never configured LED addresses: say what is plugged in where,
    // show the consequence of every setting next to it, and never rely on hidden controls.
    auto* chainCaption=new QLabel("Devices on this connector, in cable order. The first one is plugged into the motherboard.");
    chainCaption->setWordWrap(true); layout->addWidget(chainCaption);
    auto* wiring=new QWidget; auto* wiringLayout=new QVBoxLayout(wiring); wiringLayout->setContentsMargins(0,0,0,0);
    wiringLayout->addWidget(new QLabel("How are these devices plugged in?"));
    auto* serial=new QRadioButton("One after another: each device has its own LEDs"); serial->setObjectName("wiringSerial");
    auto* splitter=new QRadioButton("Splitter cable: all devices show the same colors, starting at LED 1"); splitter->setObjectName("wiringSplitter");
    // Both stay unchecked for a mixed setup, so a single click can never silently overwrite custom starts.
    serial->setAutoExclusive(false); splitter->setAutoExclusive(false);
    auto* custom=new QLabel("Custom: each device starts at the LED number set below."); custom->setObjectName("wiringCustom"); custom->setProperty("muted",true);
    for(auto* widget:std::initializer_list<QWidget*>{serial,splitter,custom}) wiringLayout->addWidget(widget);
    layout->addWidget(wiring);
    auto* chain = new QListWidget; chain->setObjectName("channelChain");
    chain->setDragDropMode(fixedComponent_ ? QAbstractItemView::NoDragDrop : QAbstractItemView::InternalMove); chain->setEnabled(!fixedComponent_); layout->addWidget(chain);
    auto* summary=new QLabel; summary->setObjectName("chainSummary"); summary->setWordWrap(true); layout->addWidget(summary);
    auto label = [&document](const QUuid& id) {
        for (const auto& placement : document.state().placements) if (placement.id == id)
            return document.componentName(placement);
        return QString("Missing instance ") + id.toString();
    };
    auto ledCount = [&document](const QUuid& id) {
        for (const auto& placement : document.state().placements) if (placement.id == id)
            return int(document.catalog()[placement.componentId].mapping.size());
        return 0;
    };
    // Item roles: UserRole = placement id, +1 = explicit one-based start (0 = Auto), +2 = name, +3 = LED count.
    auto addRow = [chain,label,ledCount](const QUuid& id, int start) {
        auto* item = new QListWidgetItem(chain); item->setData(Qt::UserRole,id); item->setData(Qt::UserRole+1,start);
        item->setData(Qt::UserRole+2,label(id)); item->setData(Qt::UserRole+3,ledCount(id)); item->setToolTip(label(id));
        return item;
    };
    for (const auto& id : initial_.placements) addRow(id, initial_.starts.value(id,0));
    const auto maximum=channel.maxLeds; const auto header=channel.id;
    auto updateTexts=[chain,summary,serial,splitter,custom,maximum,header] {
        QVector<int> counts,starts; QStringList names; bool explicitStart=false;
        for(int row=0;row<chain->count();++row) {
            counts.append(qMax(1,chain->item(row)->data(Qt::UserRole+3).toInt())); starts.append(chain->item(row)->data(Qt::UserRole+1).toInt());
            names.append(chain->item(row)->data(Qt::UserRole+2).toString()); explicitStart=explicitStart || starts.last();
        }
        const bool allAuto=std::all_of(starts.cbegin(),starts.cend(),[](int start){return start==0;});
        const bool allOne=starts.size()>1 && std::all_of(starts.cbegin(),starts.cend(),[](int start){return start==1;});
        serial->setChecked(allAuto); splitter->setChecked(!allAuto && allOne); custom->setVisible(!allAuto && !allOne);
        if(counts.isEmpty()) { summary->setText("No devices yet. Add one below."); return; }
        const auto offsets=chainOffsets(counts,explicitStart ? starts : QVector<int>{});
        const auto sharing=chainSharing(counts,explicitStart ? starts : QVector<int>{});
        int used=0; bool shared=false;
        for(int row=0;row<chain->count();++row) {
            used=qMax(used,offsets[row]+counts[row]);
            auto note=sharedColorNote(sharing[row],row,names);
            bool supplies=false; for(int other=0;other<sharing.size();++other) if(other!=row) for(const auto& run:sharing[other]) supplies=supplies || run.source==row;
            shared=shared || supplies;
            const auto range=QString("LED %1-%2").arg(offsets[row]+1).arg(offsets[row]+counts[row]);
            if(note.isEmpty()) note=supplies ? range+": decides the shared colors" : range;
            if(auto* widget=chain->itemWidget(chain->item(row))) if(auto* result=widget->findChild<QLabel*>("chainResult")) result->setText(note);
        }
        if(used>maximum) summary->setText(QString("Too many LEDs: these devices need LED 1-%1, but %2 has only %3.").arg(used).arg(header).arg(maximum));
        else summary->setText(QString("Uses LED 1-%1 of %2 on %3.").arg(used).arg(maximum).arg(header)+
            (shared ? " Devices with the same LED numbers always light the same; the device higher in the list decides those colors." : QString()));
    };
    auto detach=[chain](QListWidgetItem* item) {
        if(auto* widget=item ? chain->itemWidget(item) : nullptr) { widget->hide(); chain->removeItemWidget(item); }
    };
    auto rebuildRows=[this,chain,maximum,updateTexts,detach] {
        for(int row=0;row<chain->count();++row) detach(chain->item(row));
        for(int row=0;row<chain->count();++row) {
            auto* item=chain->item(row);
            auto* widget=new QWidget; auto* line=new QHBoxLayout(widget); line->setContentsMargins(6,3,6,3);
            auto* name=new QLabel(item->data(Qt::UserRole+2).toString()); name->setMinimumWidth(150); line->addWidget(name,1);
            auto* count=new QLabel(QString("%1 LEDs").arg(item->data(Qt::UserRole+3).toInt())); count->setProperty("muted",true); line->addWidget(count);
            line->addSpacing(8); line->addWidget(new QLabel("Starts at LED"));
            auto* down=new QToolButton; down->setObjectName("chainStartDown"); down->setText("-"); down->setAccessibleName("Start one LED earlier");
            auto* start=new QSpinBox; start->setObjectName("chainStart"); start->setRange(0,maximum); start->setSpecialValueText("Auto");
            start->setToolTip("Auto: directly after the device above. A number: this device starts at that LED, so devices with the same start show the same colors.");
            start->setAccessibleName("Starts at LED for "+item->data(Qt::UserRole+2).toString());
            styleStudioNumericField(start); start->setValue(item->data(Qt::UserRole+1).toInt());
            auto* up=new QToolButton; up->setObjectName("chainStartUp"); up->setText("+"); up->setAccessibleName("Start one LED later");
            for(auto* button:{down,up}) button->setFixedWidth(26);
            line->addWidget(down); line->addWidget(start); line->addWidget(up);
            auto* result=new QLabel; result->setObjectName("chainResult"); result->setProperty("muted",true); result->setMinimumWidth(220); line->addWidget(result);
            connect(down,&QToolButton::clicked,start,&QSpinBox::stepDown); connect(up,&QToolButton::clicked,start,&QSpinBox::stepUp);
            connect(start,&QSpinBox::valueChanged,this,[item,updateTexts](int value) { item->setData(Qt::UserRole+1,value); updateTexts(); });
            // Size after the studio style is applied, otherwise the styled buttons are clipped.
            item->setText({}); chain->setItemWidget(item,widget); widget->ensurePolished(); item->setSizeHint(QSize(widget->sizeHint().width(),qMax(widget->sizeHint().height(),40)));
        }
        updateTexts();
    };
    rebuildRows();
    // Dragging a row discards its embedded widget; recreate the rows from their stored values.
    connect(chain->model(),&QAbstractItemModel::rowsMoved,this,[this,rebuildRows] { QMetaObject::invokeMethod(this,rebuildRows,Qt::QueuedConnection); });
    auto setAll=[chain](int value) {
        for(int row=0;row<chain->count();++row) if(auto* widget=chain->itemWidget(chain->item(row))) widget->findChild<QSpinBox*>("chainStart")->setValue(value);
    };
    connect(serial,&QRadioButton::clicked,this,[setAll,updateTexts] { setAll(0); updateTexts(); });
    connect(splitter,&QRadioButton::clicked,this,[setAll,updateTexts] { setAll(1); updateTexts(); });
    auto* available = new QComboBox; available->setObjectName("availableInstances");
    available->setEnabled(!fixedComponent_);
    for (const auto& placement : document.state().placements) available->addItem(label(placement.id), placement.id);
    layout->addWidget(available);
    auto* actions = new QHBoxLayout; layout->addLayout(actions);
    auto action = [&](const QString& text, const QString& name, auto callback) {
        auto* button = new QPushButton(text); button->setObjectName(name); actions->addWidget(button);
        button->setEnabled(!fixedComponent_);
        connect(button, &QPushButton::clicked, this, callback);
    };
    action("Add device", "appendInstance", [chain,available,addRow,rebuildRows] {
        if (available->currentIndex() < 0) return;
        chain->setCurrentItem(addRow(available->currentData().toUuid(),0)); rebuildRows();
    });
    action("Remove", "removeInstance", [chain,rebuildRows,detach] { detach(chain->currentItem()); delete chain->takeItem(chain->currentRow()); rebuildRows(); });
    action("Move up", "instanceUp", [chain,rebuildRows,detach] {
        const int row=chain->currentRow(); if(row<=0) return;
        detach(chain->item(row)); auto* item=chain->takeItem(row); chain->insertItem(row-1,item); chain->setCurrentItem(item); rebuildRows();
    });
    action("Move down", "instanceDown", [chain,rebuildRows,detach] {
        const int row=chain->currentRow(); if(row<0 || row>=chain->count()-1) return;
        detach(chain->item(row)); auto* item=chain->takeItem(row); chain->insertItem(row+1,item); chain->setCurrentItem(item); rebuildRows();
    });
    auto visibility=[this,mode,native,chain,available,actions,clock,clockSection,chainCaption,wiring,summary] {
        const bool software=mode->currentData().toBool(); native->setVisible(!software);
        const bool editableChain=software && !fixedComponent_;
        chain->setVisible(editableChain); wiring->setVisible(editableChain); summary->setVisible(editableChain); available->setVisible(editableChain); clock->setEnabled(software);
        clockSection->setVisible(software); chainCaption->setVisible(editableChain);
        for(int i=0;i<actions->count();++i) if(auto* widget=actions->itemAt(i)->widget()) widget->setVisible(editableChain);
    };
    connect(mode,&QComboBox::currentIndexChanged,this,visibility); visibility(); syncSliderFields(this);
    auto* error = new QLabel; error->setObjectName("channelError"); error->setWordWrap(true); layout->addWidget(error);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel); layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, [this,&document,error] {
        try {
            auto state = document.state(); const auto candidate = assignment();
            state.assignments.removeIf([&](const auto& item) { return item.id == candidate.id; });
            state.assignments.append(candidate); validateChannelAssignments(state, document.profiles()); accept();
        } catch(const std::exception& failure) { error->setText(QString::fromUtf8(failure.what())); }
    });
    installParameterTooltips(this);
}
ChannelAssignment ChannelEditor::assignment() const
{
    auto result = initial_;
    const auto* mode = findChild<QComboBox*>("channelMode");
    if(mode->currentIndex()<0) throw std::invalid_argument("Choose a supported channel mode");
    result.software=mode->currentData().toBool();
    const auto* rate=findChild<QDoubleSpinBox*>("channelRate");result.timeScale=rate->value()==rate->property("initialValue").toDouble() ? initial_.timeScale : rate->value();
    const auto* offset=findChild<QDoubleSpinBox*>("channelOffset");result.timeOffset=offset->value()==offset->property("initialValue").toDouble() ? initial_.timeOffset : offset->value();
    result.native.id=findChild<QComboBox*>("nativeMode")->currentText();
    result.native.brightness=findChild<QSpinBox*>("nativeBrightness")->value();
    result.native.speed=findChild<QSpinBox*>("nativeSpeed")->value();
    result.native.inward=findChild<QCheckBox*>("nativeInward")->isChecked();
    result.native.colors.clear();
    for (const auto& color : findChild<ColorListEditor*>("nativeColors")->colors()) {
        if(!color.isValid() || color.alpha()!=255) throw std::invalid_argument("Native colors must be opaque RGB colors");
        result.native.colors.append({quint8(color.red()),quint8(color.green()),quint8(color.blue())});
    }
    // Preserve the device identity even if a hidden widget is modified programmatically.
    if(!fixedComponent_) {
        result.placements.clear(); const auto* chain=findChild<QListWidget*>("channelChain");
        result.starts.clear();
        for(int row=0;row<chain->count();++row) {
            result.placements.append(chain->item(row)->data(Qt::UserRole).toUuid());
            if(const int start=chain->item(row)->data(Qt::UserRole+1).toInt()) result.starts.insert(result.placements.last(),start);
        }
    }
    return result;
}
