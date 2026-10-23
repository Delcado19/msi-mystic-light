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
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <stdexcept>

ChannelEditor::ChannelEditor(const LightingDocument& document, ChannelAssignment initial, QWidget* parent)
    : QDialog(parent), initial_(std::move(initial))
{
    const auto& profile = assignmentProfile(document.profiles(), initial_.profileId);
    const auto& channel = assignmentChannel(profile, initial_.channelId);
    setWindowTitle("Configure channel - " + channel.id); resize(560, 620);
    applyStudioStyle(this);
    auto* layout = new QVBoxLayout(this);
    auto* identity = new QLabel(profile.name + " / " + channel.id + "\n" + initial_.endpoint);
    identity->setWordWrap(true); layout->addWidget(identity);
    layout->addWidget(new QLabel(QString("Capacity: %1 LEDs; fixed count: %2. No device output.").arg(channel.maxLeds).arg(channel.fixedLeds)));
    auto* form = new QFormLayout; layout->addLayout(form);
    auto* mode = new QComboBox; mode->setObjectName("channelMode");
    if (!channel.nativeModes.isEmpty()) mode->addItem("Native firmware", false);
    if (channel.direct) mode->addItem("Software canvas", true);
    mode->setCurrentIndex(mode->findData(initial_.software)); form->addRow("Output mode", mode);
    auto* rate=new QDoubleSpinBox; rate->setObjectName("channelRate"); rate->setRange(0,10); rate->setDecimals(3); rate->setValue(initial_.timeScale);
    auto* offset=new QDoubleSpinBox; offset->setObjectName("channelOffset"); offset->setRange(0,86400); offset->setDecimals(3); offset->setValue(initial_.timeOffset);
    auto* clock=new QWidget; auto* clockForm=new QFormLayout(clock);
    rate->setAccessibleName("Channel clock rate"); clockForm->addRow("Playback speed",sliderField(rate)); clockForm->addRow("Offset (seconds)",offset);
    auto* clockSection=disclosure(layout,"Channel playback clock",clock);
    auto clockEnabled=[mode,rate,offset] { rate->setEnabled(mode->currentData().toBool()); offset->setEnabled(mode->currentData().toBool()); };
    connect(mode,&QComboBox::currentIndexChanged,this,clockEnabled); clockEnabled();
    auto* native = new QWidget; auto* nativeForm = new QFormLayout(native); layout->addWidget(native);
    auto* effect = new QComboBox; effect->setObjectName("nativeMode"); effect->addItems(channel.nativeModes);
    effect->setCurrentIndex(qMax(0, effect->findText(initial_.native.id))); nativeForm->addRow("Firmware effect", effect);
    auto* nativeColors=new ColorListEditor(1,4); nativeColors->setObjectName("nativeColors");
    QVector<QColor> colors; for(const auto& color:initial_.native.colors) colors.append(QColor(color.r,color.g,color.b));
    nativeColors->setColors(colors); nativeForm->addRow("Colors",nativeColors);
    auto* brightness = new QSpinBox; brightness->setObjectName("nativeBrightness"); brightness->setRange(0,5);
    brightness->setValue(initial_.native.brightness); nativeForm->addRow("Brightness", brightness);
    auto* speed = new QSpinBox; speed->setObjectName("nativeSpeed"); speed->setRange(0,2);
    speed->setValue(initial_.native.speed); nativeForm->addRow("Speed", speed);
    auto* inward = new QCheckBox("Inward direction"); inward->setObjectName("nativeInward"); inward->setChecked(initial_.native.inward);
    nativeForm->addRow(inward);
    connect(mode, &QComboBox::currentIndexChanged, this, [mode,native] { native->setEnabled(!mode->currentData().toBool()); });
    native->setEnabled(!mode->currentData().toBool());
    native->setVisible(!mode->currentData().toBool());
    auto* chainCaption=new QLabel("Component chain: first item is nearest the controller."); layout->addWidget(chainCaption);
    auto* chain = new QListWidget; chain->setObjectName("channelChain");
    chain->setDragDropMode(QAbstractItemView::InternalMove); layout->addWidget(chain);
    auto label = [&document](const QUuid& id) {
        for (const auto& placement : document.state().placements) if (placement.id == id)
            return document.catalog()[placement.componentId].name + " [" + id.toString(QUuid::WithoutBraces).left(8) + "]";
        return QString("Missing instance ") + id.toString();
    };
    for (const auto& id : initial_.placements) {
        auto* item = new QListWidgetItem(label(id), chain); item->setData(Qt::UserRole,id);
    }
    auto* available = new QComboBox; available->setObjectName("availableInstances");
    for (const auto& placement : document.state().placements) available->addItem(label(placement.id), placement.id);
    layout->addWidget(available);
    auto* actions = new QHBoxLayout; layout->addLayout(actions);
    auto action = [&](const QString& text, const QString& name, auto callback) {
        auto* button = new QPushButton(text); button->setObjectName(name); actions->addWidget(button);
        connect(button, &QPushButton::clicked, this, callback);
    };
    action("Append", "appendInstance", [chain,available] {
        if (available->currentIndex() < 0) return;
        auto* item = new QListWidgetItem(available->currentText(), chain); item->setData(Qt::UserRole,available->currentData());
        chain->setCurrentItem(item);
    });
    action("Remove", "removeInstance", [chain] { delete chain->takeItem(chain->currentRow()); });
    action("Up", "instanceUp", [chain] {
        const int row=chain->currentRow(); if(row<=0) return;
        auto* item=chain->takeItem(row); chain->insertItem(row-1,item); chain->setCurrentItem(item);
    });
    action("Down", "instanceDown", [chain] {
        const int row=chain->currentRow(); if(row<0 || row>=chain->count()-1) return;
        auto* item=chain->takeItem(row); chain->insertItem(row+1,item); chain->setCurrentItem(item);
    });
    auto visibility=[mode,native,chain,available,actions,clock,clockSection,chainCaption] {
        const bool software=mode->currentData().toBool(); native->setVisible(!software);
        chain->setVisible(software); available->setVisible(software); clock->setEnabled(software);
        clockSection->setVisible(software); chainCaption->setVisible(software);
        for(int i=0;i<actions->count();++i) if(auto* widget=actions->itemAt(i)->widget()) widget->setVisible(software);
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
    result.timeScale=findChild<QDoubleSpinBox*>("channelRate")->value();
    result.timeOffset=findChild<QDoubleSpinBox*>("channelOffset")->value();
    result.native.id=findChild<QComboBox*>("nativeMode")->currentText();
    result.native.brightness=findChild<QSpinBox*>("nativeBrightness")->value();
    result.native.speed=findChild<QSpinBox*>("nativeSpeed")->value();
    result.native.inward=findChild<QCheckBox*>("nativeInward")->isChecked();
    result.native.colors.clear();
    for (const auto& color : findChild<ColorListEditor*>("nativeColors")->colors()) {
        if(!color.isValid() || color.alpha()!=255) throw std::invalid_argument("Native colors must be opaque RGB colors");
        result.native.colors.append({quint8(color.red()),quint8(color.green()),quint8(color.blue())});
    }
    result.placements.clear(); const auto* chain=findChild<QListWidget*>("channelChain");
    for(int row=0;row<chain->count();++row) result.placements.append(chain->item(row)->data(Qt::UserRole).toUuid());
    return result;
}
