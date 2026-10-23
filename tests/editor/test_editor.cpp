#include "editor/LightingWindow.h"
#include "editor/ComponentEditor.h"
#include "editor/ChannelEditor.h"
#include <QListWidget>
#include <QTimer>
#include <QTableWidget>
#include <QDialogButtonBox>
#include <QLabel>
#include <QApplication>
#include <QDir>
#include <QComboBox>
#include <QCheckBox>
#include <QHeaderView>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QSlider>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QImageReader>
#include <QElapsedTimer>
#include <QLineEdit>
#include <QtTest>
#include <stdexcept>

namespace {
ComponentCatalog catalog()
{
    ComponentDefinition strip;
    strip.id = "test.strip"; strip.name = "Test strip";
    strip.positions = {{0, 0}, {1, 0}, {2, 0}};
    strip.mapping = {2, 1, 0};
    return {{strip.id, strip}};
}
DeviceProfile channelProfile()
{
    DeviceProfile profile; profile.id="test.device"; profile.driverId="test.driver"; profile.name="Test device";
    ChannelProfile direct; direct.id="Direct"; direct.maxLeds=6; direct.direct=true;
    direct.nativeModes={"steady","breathing"}; profile.channels.append(direct);
    auto native=direct; native.id="Native"; native.direct=false; native.maxLeds=1; native.fixedLeds=1;
    profile.channels.append(native);
    auto fixed=direct; fixed.id="Fixed"; fixed.fixedLeds=6; profile.channels.append(fixed);
    return profile;
}
EffectLayer layer(QColor color)
{
    EffectLayer result; result.color = color;
    result.mask = LightingDocument::whiteMask(); return result;
}
}

class EditorTest : public QObject {
    Q_OBJECT
private slots:
    void channelAssignments();
    void channelControls();
    void componentSampling();
    void componentAuthoring();
    void componentEditor();
    void placementTransforms();
    void rgbaAndMask();
    void blending();
    void softPaintingAndGradient();
    void independentMasksAndUndo();
    void invalidEditsAreAtomic();
    void separateEditingContexts();
    void explicitDiscovery();
    void documentPersistence();
    void layerPanelOperations();
    void gradientAndShapeRendering();
    void effectPersistenceAndLegacy();
    void effectControls();
    void imageImportAndCrop();
    void animationPreview();
    void advancedEffects();
    void canvasTransformsAndCrop();
    void advancedControls();
};

void EditorTest::channelAssignments()
{
    const auto profile=channelProfile();
    auto otherProfile=profile; otherProfile.id="other.device";
    LightingDocument document(catalog(), nullptr, {profile,otherProfile});
    const auto first=document.addComponent("test.strip"), second=document.addComponent("test.strip");
    document.edit("Position", [](LightingState& state) { state.placements[1].origin={200,80}; });
    ChannelAssignment direct; direct.endpoint="fake://one"; direct.profileId=profile.id; direct.channelId="Direct";
    direct.software=true; direct.placements={second,first}; document.assignChannel(direct);
    ChannelAssignment native; native.endpoint=direct.endpoint; native.profileId=profile.id; native.channelId="Native";
    native.native.id="breathing"; native.native.colors={{1,2,3},{200,100,50}};
    native.native.brightness=3; native.native.speed=2; native.native.inward=true; document.assignChannel(native);
    QImage frame(LightingDocument::canvasSize(),QImage::Format_ARGB32); frame.fill(Qt::transparent);
    frame.setPixelColor(80,80,Qt::red); frame.setPixelColor(100,80,Qt::green); frame.setPixelColor(120,80,Qt::blue);
    frame.setPixelColor(240,80,QColor(200,100,50,128));
    auto samples=document.sampleChannels(frame);
    QCOMPARE(samples.size(),1); QVERIFY(!samples.contains(native.id));
    QCOMPARE(samples[direct.id].size(),6); QCOMPARE(samples[direct.id][0],QColor(100,50,25));
    QCOMPARE(samples[direct.id][3],QColor(Qt::blue)); QCOMPARE(samples[direct.id][5],QColor(Qt::red));
    auto reversed=direct; reversed.placements={first,second}; document.assignChannel(reversed);
    QCOMPARE(document.sampleChannels(frame)[direct.id][0],QColor(Qt::blue));
    document.history()->undo(); QCOMPARE(document.sampleChannels(frame),samples);
    const int before=document.history()->index();
    auto reject=[&](ChannelAssignment invalid) {
        QVERIFY_EXCEPTION_THROWN(document.assignChannel(invalid),std::invalid_argument);
        QCOMPARE(document.history()->index(),before); QCOMPARE(document.sampleChannels(frame),samples);
    };
    auto invalid=direct; invalid.placements={first,first}; reject(invalid);
    invalid=direct; invalid.placements={QUuid::createUuid()}; reject(invalid);
    invalid=direct; invalid.placements.clear(); reject(invalid);
    invalid=direct; invalid.channelId="Native"; reject(invalid);
    invalid=direct; invalid.id=QUuid::createUuid(); reject(invalid);
    invalid=direct; invalid.id=QUuid::createUuid(); invalid.channelId="Fixed"; reject(invalid);
    invalid=direct; invalid.profileId=otherProfile.id; reject(invalid);
    invalid=native; invalid.native.id="unsupported"; reject(invalid);
    invalid=native; invalid.native.speed=3; reject(invalid);
    invalid=native; invalid.native.colors.clear(); reject(invalid);
    invalid=direct; invalid.endpoint.clear(); reject(invalid);
    QVERIFY_EXCEPTION_THROWN(document.edit("Delete assigned instance",[&](LightingState& state) {
        state.placements.removeIf([&](const auto& item){return item.id==first;});
    }),std::invalid_argument);
    auto expanded=catalog().first(); expanded.positions.append({3,0}); expanded.mapping.append(3);
    QVERIFY_EXCEPTION_THROWN(document.defineComponent(expanded,true),std::invalid_argument);
    QCOMPARE(document.catalog()[expanded.id].positions.size(),3);
    QVERIFY_EXCEPTION_THROWN(document.sampleChannels(QImage()),std::invalid_argument);

    QTemporaryDir directory; const auto path=directory.filePath("channels.lighting.json"); document.save(path);
    LightingDocument loaded({},nullptr,{profile}); loaded.load(path);
    QCOMPARE(loaded.sampleChannels(frame),samples);
    const auto effect=loaded.state().assignments[1].native;
    QCOMPARE(effect.id,QString("breathing")); QCOMPARE(effect.colors.size(),2);
    QCOMPARE(effect.colors[1].r,quint8(200)); QCOMPARE(effect.brightness,3); QCOMPARE(effect.speed,2); QVERIFY(effect.inward);
    LightingDocument unavailable(catalog()); unavailable.addLayer(); const int unchanged=unavailable.history()->index();
    QVERIFY_EXCEPTION_THROWN(unavailable.load(path),std::invalid_argument); QCOMPARE(unavailable.history()->index(),unchanged);
    QFile file(path); QVERIFY(file.open(QIODevice::ReadOnly));
    const auto original=QJsonDocument::fromJson(file.readAll()).object(); file.close();
    auto write=[&](QJsonObject root) { QVERIFY(file.open(QIODevice::WriteOnly)); file.write(QJsonDocument(root).toJson()); file.close(); };
    auto root=original; auto assignments=root.value("assignments").toArray();
    auto malformed=assignments[0].toObject(); malformed.insert("software","true"); assignments[0]=malformed;
    root.insert("assignments",assignments); write(root);
    loaded.addLayer(); const int history=loaded.history()->index();
    QVERIFY_EXCEPTION_THROWN(loaded.load(path),std::invalid_argument); QCOMPARE(loaded.history()->index(),history);
    root=original; root.insert("schemaVersion",7); write(root);
    QVERIFY_EXCEPTION_THROWN(loaded.load(path),std::invalid_argument);
    root.remove("assignments"); write(root); loaded.load(path); QVERIFY(loaded.state().assignments.isEmpty());

    LightingDocument fixed(catalog(),nullptr,{profile}); const auto one=fixed.addComponent("test.strip");
    auto configured=direct; configured.channelId="Fixed"; configured.placements={one};
    QVERIFY_EXCEPTION_THROWN(fixed.assignChannel(configured),std::invalid_argument);
    const auto two=fixed.addComponent("test.strip"); configured.placements.append(two); fixed.assignChannel(configured);
    QCOMPARE(fixed.sampleChannels(frame)[configured.id].size(),6);
    auto bad=channelAssignmentJson(native); auto badNative=bad.value("native").toObject();
    badNative.insert("colors",QJsonArray{QJsonArray{1.5,0,0}}); bad.insert("native",badNative);
    QVERIFY_EXCEPTION_THROWN(parseChannelAssignment(bad),std::invalid_argument);
}

void EditorTest::channelControls()
{
    const auto profile=channelProfile();
    LightingWindow window(catalog(), [profile] {
        ProfileCandidate candidate; candidate.profileId=profile.id; candidate.device.path="fake://one";
        return QVector<ProfileCandidate>{candidate};
    },nullptr,{profile});
    const auto first=window.document().addComponent("test.strip"), second=window.document().addComponent("test.strip");
    ChannelAssignment initial; initial.endpoint="fake://one"; initial.profileId=profile.id; initial.channelId="Direct";
    ChannelEditor editor(window.document(),initial);
    editor.show(); QVERIFY(QTest::qWaitForWindowExposed(&editor));
    auto* mode=editor.findChild<QComboBox*>("channelMode"); mode->setCurrentIndex(mode->findData(true));
    auto* append=editor.findChild<QPushButton*>("appendInstance"); append->click(); append->click();
    auto* buttons=editor.findChild<QDialogButtonBox*>(); buttons->button(QDialogButtonBox::Ok)->click();
    QVERIFY(!editor.findChild<QLabel*>("channelError")->text().isEmpty()); QCOMPARE(editor.result(),int(QDialog::Rejected));
    editor.findChild<QPushButton*>("removeInstance")->click();
    editor.findChild<QComboBox*>("availableInstances")->setCurrentIndex(1); append->click();
    editor.findChild<QPushButton*>("instanceUp")->click();
    QCOMPARE(editor.assignment().placements,QVector<QUuid>({second,first}));
    QVERIFY(editor.grab().save(QString(TEST_ARTIFACT_DIR)+"/editor-channel.png"));
    buttons->button(QDialogButtonBox::Ok)->click(); QCOMPARE(editor.result(),int(QDialog::Accepted));
    window.document().assignChannel(editor.assignment());
    auto* tree=window.findChild<QTreeWidget*>("channelAssignments"); QCOMPARE(tree->topLevelItemCount(),1);
    QCOMPARE(tree->topLevelItem(0)->text(1),QString("Software"));
    window.document().history()->undo(); QCOMPARE(tree->topLevelItemCount(),0);
    window.document().history()->redo(); QCOMPARE(tree->topLevelItemCount(),1);
    for(auto* button:window.findChildren<QPushButton*>()) if(button->text()=="Scan supported devices") button->click();
    window.findChild<QComboBox*>("profileCandidates")->setCurrentIndex(1);
    QCOMPARE(window.findChild<QComboBox*>("profileChannels")->count(),3);
    bool modalSeen=false, keptSoftware=false;
    QTimer::singleShot(0,[&] {
        auto* dialog=dynamic_cast<ChannelEditor*>(QApplication::activeModalWidget());
        modalSeen=dialog!=nullptr;
        if(dialog) {
            keptSoftware=dialog->assignment().software;
            dialog->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Ok)->click();
        }
    });
    for(auto* button:window.findChildren<QPushButton*>()) if(button->text()=="Configure channel") button->click();
    QVERIFY(modalSeen); QVERIFY(keptSoftware);
    QCOMPARE(window.document().state().assignments.size(),1);
    window.document().history()->setClean();
    auto native=initial; native.channelId="Native";
    ChannelEditor nativeEditor(window.document(),native);
    QCOMPARE(nativeEditor.findChild<QComboBox*>("channelMode")->count(),1);
    nativeEditor.findChild<QComboBox*>("nativeMode")->setCurrentText("breathing");
    nativeEditor.findChild<QLineEdit*>("nativeColors")->setText("#112233, #445566");
    QCOMPARE(nativeEditor.assignment().native.colors.size(),2);
    nativeEditor.findChild<QLineEdit*>("nativeColors")->setText("#80112233");
    QVERIFY_EXCEPTION_THROWN(nativeEditor.assignment(),std::invalid_argument);
}

void EditorTest::componentAuthoring()
{
    LightingDocument document(catalog());
    auto definition = catalog().first(); definition.id = "custom.strip"; definition.name = "Custom strip";
    document.defineComponent(definition);
    const auto placement = document.addComponent(definition.id);
    auto changed = definition; changed.positions[1] = {4, 2}; changed.mapping = {0, 2, 1};
    document.defineComponent(changed, true);
    QCOMPARE(document.componentPoints(document.state().placements[0])[1], QPointF(160, 120));
    document.history()->undo(); QCOMPARE(document.catalog()[definition.id].positions[1], QPointF(1, 0));
    document.history()->redo();
    const int before = document.history()->index();
    QVERIFY_EXCEPTION_THROWN(document.defineComponent(definition), std::invalid_argument);
    auto invalid = changed; invalid.mapping = {0, 0, 2};
    QVERIFY_EXCEPTION_THROWN(document.defineComponent(invalid, true), std::invalid_argument);
    QCOMPARE(document.history()->index(), before);
    QTemporaryDir directory;
    const auto file = directory.filePath("portable.lighting.json"); document.save(file);
    LightingDocument loaded({}); loaded.load(file);
    QCOMPARE(loaded.catalog()[definition.id].positions, changed.positions);
    QCOMPARE(loaded.state().placements[0].id, placement);
    auto installed = catalog(); installed[definition.id] = definition;
    LightingDocument differentInstallation(installed); differentInstallation.load(file);
    QCOMPARE(differentInstallation.catalog()[definition.id].positions, changed.positions);

    const auto importedFile = directory.filePath("component.json");
    QFile input(importedFile); QVERIFY(input.open(QIODevice::WriteOnly));
    auto imported = definition; imported.id = "imported.strip";
    input.write(QJsonDocument(imported.toJson()).toJson()); input.close();
    loaded.importComponent(importedFile); QVERIFY(QFile::remove(importedFile));
    QVERIFY(loaded.catalog().contains(imported.id));
    loaded.history()->undo(); QVERIFY(!loaded.catalog().contains(imported.id));
    loaded.history()->redo(); QVERIFY(loaded.catalog().contains(imported.id));
    loaded.save(file); LightingDocument roundTrip({}); roundTrip.load(file);
    QVERIFY(roundTrip.catalog().contains(imported.id));
    QVERIFY_EXCEPTION_THROWN(loaded.importComponent(importedFile), std::invalid_argument);
    QFile project(file); QVERIFY(project.open(QIODevice::ReadOnly));
    auto root = QJsonDocument::fromJson(project.readAll()).object(); project.close();
    auto components = root.value("components").toArray(); components.append(components[0]);
    root.insert("components", components);
    QVERIFY(project.open(QIODevice::WriteOnly)); project.write(QJsonDocument(root).toJson()); project.close();
    const auto history = loaded.history()->index();
    QVERIFY_EXCEPTION_THROWN(loaded.load(file), std::invalid_argument);
    QCOMPARE(loaded.history()->index(), history); QVERIFY(loaded.catalog().contains(imported.id));

    // Schema 6 projects still resolve their definitions from the original installed catalog.
    root.remove("components"); root.remove("assignments"); root.insert("schemaVersion", 6); root.insert("placements", QJsonArray{});
    QVERIFY(project.open(QIODevice::WriteOnly)); project.write(QJsonDocument(root).toJson()); project.close();
    loaded.load(file); QVERIFY(loaded.catalog().isEmpty());
}

void EditorTest::componentEditor()
{
    ComponentEditor editor(catalog().first(), true);
    editor.show(); QVERIFY(QTest::qWaitForWindowExposed(&editor));
    auto* table = editor.findChild<QTableWidget*>("componentLeds");
    QVERIFY(editor.findChild<QLineEdit*>("componentId")->isReadOnly());
    table->item(1, 0)->setText("4.25");
    QCOMPARE(editor.component().positions[1], QPointF(4.25, 0));
    table->item(1, 2)->setText("2");
    QVERIFY_EXCEPTION_THROWN(editor.component(), std::invalid_argument);
    auto* buttons = editor.findChild<QDialogButtonBox*>();
    buttons->button(QDialogButtonBox::Ok)->click();
    QVERIFY(editor.grab().save(QString(TEST_ARTIFACT_DIR) + "/editor-component.png"));
    QVERIFY(!editor.findChild<QLabel*>("componentError")->text().isEmpty());
    QCOMPARE(editor.result(), int(QDialog::Rejected));
    table->item(1, 2)->setText("1");
    editor.findChild<QSpinBox*>("componentCount")->setValue(4);
    QCOMPARE(editor.component().mapping, QVector<int>({2, 1, 0, 3}));
    table->setCurrentCell(1, 0); table->editItem(table->item(1, 0));
    QLineEdit* cell = nullptr;
    for (auto* candidate : table->findChildren<QLineEdit*>()) if (candidate->isVisible()) cell = candidate;
    QVERIFY(cell); cell->setText("6.5");
    QTest::mouseClick(buttons->button(QDialogButtonBox::Ok), Qt::LeftButton);
    QCOMPARE(editor.result(), int(QDialog::Accepted));
    QCOMPARE(editor.component().positions[1].x(), 6.5);

    LightingWindow window({}, [] { return QVector<ProfileCandidate>{}; });
    auto* choices = window.findChild<QComboBox*>("componentCatalog");
    auto* add = window.findChild<QPushButton*>("addComponent");
    QVERIFY(!add->isEnabled());
    window.document().defineComponent(catalog().first());
    QCOMPARE(choices->count(), 1); QVERIFY(add->isEnabled()); add->click();
    QCOMPARE(window.document().state().placements.size(), 1);
    window.document().history()->undo(); window.document().history()->undo();
    QCOMPARE(choices->count(), 0); QVERIFY(!add->isEnabled());
    window.document().history()->redo(); QCOMPARE(choices->count(), 1);
    window.document().history()->setClean();
}

void EditorTest::placementTransforms()
{
    LightingDocument document(catalog());
    const auto id = document.addComponent("test.strip");
    document.edit("Transform", [](LightingState& state) {
        auto& placement = state.placements[0];
        placement.origin = {100, 100}; placement.scale = {2, 0.5}; placement.rotation = 90;
    });
    auto points = document.componentPoints(document.state().placements[0]);
    QVERIFY(QLineF(points[1], QPointF(100, 140)).length() < 0.001);
    QImage frame(LightingDocument::canvasSize(), QImage::Format_RGB32); frame.fill(Qt::black);
    frame.setPixelColor(100, 140, Qt::red);
    QCOMPARE(document.sampleComponents(frame)[id][1], QColor(Qt::red));
    QTemporaryDir directory;
    const auto file = directory.filePath("transform.lighting.json"); document.save(file);
    LightingDocument loaded(catalog()); loaded.load(file);
    QCOMPARE(loaded.state().placements[0].scale, QSizeF(2, 0.5));
    QCOMPARE(loaded.state().placements[0].rotation, 90.0);
    const auto index = document.history()->index();
    QVERIFY_EXCEPTION_THROWN(document.edit("Invalid scale", [](LightingState& state) {
        state.placements[0].scale.setWidth(0);
    }), std::invalid_argument);
    QCOMPARE(document.history()->index(), index);
    document.history()->undo();
    QCOMPARE(document.state().placements[0].scale, QSizeF(1, 1));
    document.history()->redo();
    QCOMPARE(document.state().placements[0].scale, QSizeF(2, 0.5));

    QFile input(file); QVERIFY(input.open(QIODevice::ReadOnly));
    auto root = QJsonDocument::fromJson(input.readAll()).object(); input.close();
    root.remove("components"); root.remove("assignments");
    auto placements = root.value("placements").toArray();
    auto placement = placements[0].toObject();
    for (int version = 1; version <= 5; ++version) {
        root.insert("schemaVersion", version);
        QVERIFY(input.open(QIODevice::WriteOnly)); input.write(QJsonDocument(root).toJson()); input.close();
        QVERIFY_EXCEPTION_THROWN(loaded.load(file), std::invalid_argument);
    }
    placement.remove("scaleX"); placement.remove("scaleY"); placement.remove("rotation");
    placements[0] = placement; root.insert("placements", placements);
    // Empty layers keep this geometry-only fixture valid for every legacy schema.
    for (int version = 1; version <= 5; ++version) {
        root.insert("schemaVersion", version);
        QVERIFY(input.open(QIODevice::WriteOnly)); input.write(QJsonDocument(root).toJson()); input.close();
        loaded.load(file);
        QCOMPARE(loaded.state().placements[0].scale, QSizeF(1, 1));
        QCOMPARE(loaded.state().placements[0].rotation, 0.0);
    }
    LightingWindow window(catalog(), [] { return QVector<ProfileCandidate>{}; });
    const auto selected = window.document().addComponent("test.strip");
    auto* canvas = window.findChild<LightingCanvas*>("layoutCanvas"); canvas->select(selected);
    auto* scale = window.findChild<QDoubleSpinBox*>("placement2");
    QVERIFY(scale->isEnabled()); scale->setValue(150);
    QCOMPARE(window.document().state().placements[0].scale.width(), 1.5);
    window.document().history()->undo(); QCOMPARE(scale->value(), 100.0);
    canvas->select({}); QVERIFY(!scale->isEnabled());
}

void EditorTest::componentSampling()
{
    LightingDocument document(catalog());
    const auto first = document.addComponent("test.strip");
    const auto second = document.addComponent("test.strip");
    document.edit("Position components", [](LightingState& state) {
        state.placements[0].origin = {0.5, 0};
        state.placements[1].origin = {-20, 0};
    });
    QImage frame(LightingDocument::canvasSize(), QImage::Format_ARGB32);
    frame.fill(Qt::transparent);
    frame.setPixelColor(0, 0, QColor(200, 100, 50, 128));
    frame.setPixelColor(20, 0, Qt::green);
    frame.setPixelColor(40, 0, Qt::blue);
    const auto sampled = document.sampleComponents(frame);
    QCOMPARE(sampled.size(), 2);
    QCOMPARE(sampled[first][0], QColor(Qt::blue));
    QCOMPARE(sampled[first][1], QColor(Qt::green));
    QCOMPARE(sampled[first][2], QColor(100, 50, 25));
    QCOMPARE(sampled[second][2], QColor(Qt::black));
    QCOMPARE(sampled[second][1], QColor(100, 50, 25));
    QCOMPARE(document.componentPoints(document.state().placements[0])[1], QPointF(20.5, 0));
    QVERIFY_EXCEPTION_THROWN(document.sampleComponents(QImage()), std::invalid_argument);
    auto invalidCatalog = catalog(); invalidCatalog["test.strip"].mapping = {0, 0, 2};
    LightingDocument invalid(invalidCatalog); invalid.addComponent("test.strip");
    QVERIFY_EXCEPTION_THROWN(invalid.sampleComponents(frame), std::invalid_argument);
    invalidCatalog["test.strip"].mapping = {0};
    LightingDocument mismatched(invalidCatalog); mismatched.addComponent("test.strip");
    QVERIFY_EXCEPTION_THROWN(mismatched.sampleComponents(frame), std::invalid_argument);
    document.edit("Off canvas", [](LightingState& state) {
        state.placements[0].origin = {640, 400};
        state.placements[1].origin = {1e100, -1e100};
    });
    for (const auto& colors : document.sampleComponents(frame))
        for (const auto& color : colors) QCOMPARE(color, QColor(Qt::black));
    document.history()->undo();
    QCOMPARE(document.sampleComponents(frame)[first], sampled[first]);
}

void EditorTest::rgbaAndMask()
{
    LightingState state;
    QCOMPARE(LightingDocument::render(state).pixelColor(10, 10).alpha(), 0);
    auto red = layer(QColor(255, 0, 0, 128));
    red.transparency = 50;
    red.mask.fill(128);
    state.layers = {red};
    auto pixel = LightingDocument::render(state).pixelColor(10, 10);
    QVERIFY(std::abs(pixel.alpha() - 32) <= 1);
    QCOMPARE(pixel.red(), 255);
    state.layers[0].mask.fill(0);
    QCOMPARE(LightingDocument::render(state).pixelColor(10, 10).alpha(), 0);
    state.layers[0].maskEnabled = false;
    QVERIFY(std::abs(LightingDocument::render(state).pixelColor(10, 10).alpha() - 64) <= 1);
    state.layers[0].visible = false;
    QCOMPARE(LightingDocument::render(state).pixelColor(10, 10).alpha(), 0);
}

void EditorTest::blending()
{
    const QVector<LayerBlend> modes{LayerBlend::Normal, LayerBlend::Multiply, LayerBlend::Screen, LayerBlend::Overlay};
    const QVector<int> expected{128, 64, 192, 128};
    for (int i = 0; i < modes.size(); ++i) {
        auto gray = layer(QColor(128, 128, 128)); gray.blend = modes[i];
        LightingState state; state.layers = {gray};
        // Transparent backdrop must not turn Multiply into opaque black.
        QCOMPARE(LightingDocument::render(state).pixelColor(5, 5), gray.color);
        state.layers = {layer(QColor(128, 128, 128)), gray};
        QVERIFY(std::abs(LightingDocument::render(state).pixelColor(5, 5).red() - expected[i]) <= 1);
    }
    LightingState state; state.layers = {layer(Qt::blue), layer(Qt::red)};
    QCOMPARE(LightingDocument::render(state).pixelColor(5, 5), QColor(Qt::red));
    state.layers[1].mask.fill(128);
    const auto mixed = LightingDocument::render(state).pixelColor(5, 5);
    QVERIFY(std::abs(mixed.red() - 128) <= 1);
    QVERIFY(std::abs(mixed.blue() - 127) <= 1);
}

void EditorTest::softPaintingAndGradient()
{
    auto mask = LightingDocument::whiteMask();
    LightingDocument::paintMask(mask, {100.5, 100.5}, {150.5, 100.5}, 20, 0, 1, 0);
    QCOMPARE(mask.constScanLine(100)[125], uchar(0));
    QVERIFY(mask.constScanLine(110)[125] > 100 && mask.constScanLine(110)[125] < 150);
    QCOMPARE(mask.constScanLine(130)[125], uchar(255));
    LightingDocument::gradientMask(mask, {0.5, 0.5}, {255.5, 0.5});
    QCOMPARE(mask.constScanLine(0)[0], uchar(0));
    QCOMPARE(mask.constScanLine(0)[128], uchar(128));
    QCOMPARE(mask.constScanLine(0)[255], uchar(255));
    auto original = mask;
    LightingDocument::gradientMask(mask, {10, 10}, {10, 10});
    QCOMPARE(mask, original);
    QVERIFY_EXCEPTION_THROWN(LightingDocument::paintMask(mask, {}, {}, 0, 0, 1, 0), std::invalid_argument);
    LightingDocument::paintMask(mask, {-10, -10}, {2, 2}, 10, 1, 0.5, 255);
    QVERIFY(mask.constScanLine(2)[2] > original.constScanLine(2)[2]);
}

void EditorTest::independentMasksAndUndo()
{
    LightingDocument document(catalog());
    const auto id = document.addLayer();
    document.edit("Duplicate", [&](LightingState& state) {
        auto copy = state.layers[0]; copy.id = QUuid::createUuid(); state.layers.append(copy);
    });
    document.edit("Paint copied mask", [](LightingState& state) { state.layers[1].mask.fill(0); });
    QCOMPARE(document.state().layers[0].id, id);
    QCOMPARE(document.state().layers[0].mask.constScanLine(0)[0], uchar(255));
    QCOMPARE(document.state().layers[1].mask.constScanLine(0)[0], uchar(0));
    document.history()->undo();
    QCOMPARE(document.state().layers[1].mask.constScanLine(0)[0], uchar(255));
    document.history()->redo();
    QCOMPARE(document.state().layers[1].mask.constScanLine(0)[0], uchar(0));
}

void EditorTest::invalidEditsAreAtomic()
{
    LightingDocument document(catalog()); document.addLayer();
    const auto count = document.history()->count();
    QVERIFY_EXCEPTION_THROWN(document.edit("Invalid", [](LightingState& state) { state.layers[0].transparency = 101; }), std::invalid_argument);
    QVERIFY_EXCEPTION_THROWN(document.addComponent("missing"), std::invalid_argument);
    QVERIFY_EXCEPTION_THROWN(document.edit("Duplicate identity", [](LightingState& state) { state.layers.append(state.layers[0]); }), std::invalid_argument);
    QCOMPARE(document.history()->count(), count);
    QCOMPARE(document.state().layers[0].transparency, 0);
    QCOMPARE(document.state().placements.size(), 0);
}

void EditorTest::separateEditingContexts()
{
    LightingWindow window(catalog(), [] { return QVector<ProfileCandidate>{}; });
    const auto placement = window.document().addComponent("test.strip");
    const auto effect = window.document().addLayer();
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto* tabs = window.findChild<QTabWidget*>("lightingViews");
    auto* layout = window.findChild<LightingCanvas*>("layoutCanvas");
    auto* effects = window.findChild<LightingCanvas*>("effectsCanvas");
    QCOMPARE(tabs->count(), 2);
    QVERIFY(layout->isVisible()); QVERIFY(!effects->isVisible());
    layout->select(placement);
    QTest::keyClick(layout, Qt::Key_Right);
    QCOMPARE(window.document().state().placements[0].origin, QPointF(81, 80));
    QCOMPARE(window.document().state().layers[0].mask.constScanLine(200)[320], uchar(255));
    QDir().mkpath(TEST_ARTIFACT_DIR);
    QVERIFY(window.grab().save(QString(TEST_ARTIFACT_DIR) + "/editor-layout.png"));
    tabs->setCurrentIndex(1);
    QVERIFY(!layout->isVisible()); QVERIFY(effects->isVisible());
    auto* tree = window.findChild<QTreeWidget*>("effectLayers");
    auto* item = tree->topLevelItem(0);
    const auto row = tree->visualItemRect(item);
    QTest::mouseClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier,
        QPoint(tree->header()->sectionPosition(2) + 15, row.center().y()));
    QCOMPARE(effects->selection(), effect);
    const auto center = effects->rect().center();
    QTest::mousePress(effects, Qt::LeftButton, Qt::NoModifier, center);
    QTest::mouseRelease(effects, Qt::LeftButton, Qt::NoModifier, center);
    QVERIFY(window.document().state().layers[0].mask.constScanLine(200)[320] < 20);
    QCOMPARE(window.document().state().placements[0].origin, QPointF(81, 80));
    const auto point = layout->canvasPoint({layout->width() / 2.0, layout->height() / 2.0});
    tabs->setCurrentIndex(0); tabs->setCurrentIndex(1);
    QCOMPARE(layout->canvasPoint({layout->width() / 2.0, layout->height() / 2.0}), point);
    auto* slider = window.findChild<QSlider*>("layerTransparency");
    slider->setValue(50);
    QCOMPARE(window.document().state().layers[0].transparency, 50);
    window.document().history()->undo();
    QCOMPARE(slider->value(), 0);
    QVERIFY(window.grab().save(QString(TEST_ARTIFACT_DIR) + "/editor-effects.png"));
    window.document().history()->setClean();
    window.close();
}

void EditorTest::explicitDiscovery()
{
    int scans = 0;
    LightingWindow window(catalog(), [&] {
        ++scans;
        ProfileCandidate candidate; candidate.profileId = "test.board"; candidate.device.path = "exact-test-path";
        return QVector<ProfileCandidate>{candidate, candidate};
    });
    QCOMPARE(scans, 0);
    for (auto* button : window.findChildren<QPushButton*>())
        if (button->text() == "Scan supported devices") button->click();
    QCOMPARE(scans, 1);
    auto* candidates = window.findChild<QComboBox*>("profileCandidates");
    QCOMPARE(candidates->count(), 3);
    QCOMPARE(candidates->currentIndex(), 0);
    candidates->setCurrentIndex(2);
    QCOMPARE(candidates->currentData().toMap().value("path").toString(), QString("exact-test-path"));
    QCOMPARE(candidates->currentData().toMap().value("profileId").toString(), QString("test.board"));
    QCOMPARE(scans, 1);
}

void EditorTest::documentPersistence()
{
    QTemporaryDir directory;
    const auto path = directory.filePath("project.lighting.json");
    LightingDocument document(catalog());
    const auto id = document.addLayer();
    document.addComponent("test.strip");
    document.edit("Gradient", [](LightingState& state) {
        state.layers[0].transparency = 35;
        state.layers[0].blend = LayerBlend::Overlay;
        LightingDocument::gradientMask(state.layers[0].mask, {0, 0}, {640, 0});
    });
    const auto expected = document.render();
    document.save(path);
    QVERIFY(document.history()->isClean());
    LightingDocument loaded(catalog()); loaded.load(path);
    QCOMPARE(loaded.render(), expected);
    QCOMPARE(loaded.state().layers[0].id, id);
    QCOMPARE(loaded.state().placements[0].id, document.state().placements[0].id);
    QCOMPARE(loaded.state().layers[0].mask, document.state().layers[0].mask);
    QVERIFY(loaded.history()->isClean());
    QCOMPARE(loaded.history()->count(), 0);

    QFile file(path); QVERIFY(file.open(QIODevice::ReadOnly));
    auto json = QJsonDocument::fromJson(file.readAll()).object(); file.close();
    auto placements = json.value("placements").toArray();
    auto invalid = placements[0].toObject(); invalid.insert("componentId", "missing");
    placements[0] = invalid; json.insert("placements", placements);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate)); file.write(QJsonDocument(json).toJson()); file.close();
    QVERIFY_EXCEPTION_THROWN(loaded.load(path), std::invalid_argument);
    QCOMPARE(loaded.render(), expected);
    QVERIFY(loaded.history()->isClean());
    loaded.addLayer();
    QVERIFY_EXCEPTION_THROWN(loaded.save(directory.filePath("missing/project.json")), std::runtime_error);
    QVERIFY(!loaded.history()->isClean());

    json.insert("placements", QJsonArray{});
    auto layers = json.value("layers").toArray();
    auto badLayer = layers[0].toObject(); badLayer.insert("maskPng", "bad-base64!");
    layers[0] = badLayer; json.insert("layers", layers);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate)); file.write(QJsonDocument(json).toJson()); file.close();
    const auto before = loaded.history()->count();
    QVERIFY_EXCEPTION_THROWN(loaded.load(path), std::invalid_argument);
    QCOMPARE(loaded.history()->count(), before);
}

void EditorTest::layerPanelOperations()
{
    LightingWindow window(catalog(), [] { return QVector<ProfileCandidate>{}; });
    auto click = [&](const QString& text) {
        for (auto* button : window.findChildren<QPushButton*>())
            if (button->text() == text) { button->click(); return; }
        QFAIL("Missing layer action");
    };
    click("New layer");
    auto* tree = window.findChild<QTreeWidget*>("effectLayers");
    tree->topLevelItem(0)->setText(0, "Red wash");
    QTRY_COMPARE(window.document().state().layers[0].name, QString("Red wash"));
    tree->topLevelItem(0)->setCheckState(0, Qt::Unchecked);
    QTRY_VERIFY(!window.document().state().layers[0].visible);
    click("Duplicate layer");
    QCOMPARE(window.document().state().layers.size(), 2);
    QCOMPARE(window.document().state().layers[1].name, QString("Red wash copy"));
    QVERIFY(window.document().state().layers[1].id != window.document().state().layers[0].id);
    click("Fill mask black");
    QCOMPARE(window.document().state().layers[0].mask.constScanLine(0)[0], uchar(255));
    QCOMPARE(window.document().state().layers[1].mask.constScanLine(0)[0], uchar(0));
    click("Delete layer");
    QCOMPARE(window.document().state().layers.size(), 1);
    window.document().history()->undo();
    QCOMPARE(window.document().state().layers.size(), 2);
    QCOMPARE(tree->topLevelItemCount(), 2);
    window.document().history()->setClean();
}

void EditorTest::gradientAndShapeRendering()
{
    auto effect = layer(Qt::red);
    effect.secondaryColor = Qt::blue;
    effect.effect = EffectKind::LinearGradient;
    effect.extent = {640, 400};
    auto image = renderEffect(effect);
    QVERIFY(image.pixelColor(0, 200).red() > 253);
    QVERIFY(image.pixelColor(639, 200).blue() > 253);
    QVERIFY(std::abs(image.pixelColor(320, 200).red() - 128) <= 1);
    effect.rotation = 90;
    image = renderEffect(effect);
    QCOMPARE(image.pixelColor(10, 200), image.pixelColor(600, 200));
    QVERIFY(image.pixelColor(320, 0).red() > image.pixelColor(320, 399).red());
    effect.rotation = 0;
    effect.effect = EffectKind::RadialGradient;
    image = renderEffect(effect);
    QVERIFY(image.pixelColor(320, 200).red() > 253);
    QVERIFY(image.pixelColor(639, 399).blue() > 253);
    effect.effect = EffectKind::ConicalGradient;
    image = renderEffect(effect);
    QVERIFY(image.pixelColor(600, 200).red() > 253);
    QVERIFY(std::abs(image.pixelColor(320, 380).blue() - 64) <= 1);
    effect.effect = EffectKind::Stripes;
    effect.repetitions = 4;
    image = renderEffect(effect);
    QCOMPARE(image.pixelColor(10, 200), QColor(Qt::red));
    QCOMPARE(image.pixelColor(90, 200), QColor(Qt::blue));
    effect.effect = EffectKind::Waves;
    image = renderEffect(effect);
    QVERIFY(image.pixelColor(0, 200).red() > 253);
    QVERIFY(image.pixelColor(80, 200).blue() > 253);
    effect.secondaryColor = QColor(0, 0, 255, 0);
    effect.effect = EffectKind::LinearGradient;
    image = renderEffect(effect);
    const auto faded = image.pixelColor(320, 200);
    QVERIFY(std::abs(faded.alpha() - 127) <= 1);
    QCOMPARE(faded.red(), 255); QCOMPARE(faded.blue(), 0);

    for (const auto& definition : effectDefinitions()) {
        if (definition.kind < EffectKind::Circle || definition.kind == EffectKind::Image || isParticle(definition.kind)) continue;
        effect.effect = definition.kind;
        effect.extent = {100, 60}; effect.center = {100, 100};
        image = renderEffect(effect);
        QCOMPARE(image.pixelColor(0, 0).alpha(), 0);
        int visible = 0;
        for (int y = 0; y < image.height(); ++y)
            for (int x = 0; x < image.width(); ++x) visible += image.pixelColor(x, y).alpha() > 0;
        QVERIFY2(visible > 100, qPrintable(definition.label));
    }
    effect.effect = EffectKind::Circle;
    image = renderEffect(effect);
    QCOMPARE(image.pixelColor(140, 100).alpha(), 255);
    effect.rotation = 90;
    QCOMPARE(renderEffect(effect).pixelColor(140, 100).alpha(), 0);
    effect.rotation = 0; effect.effect = EffectKind::Ring;
    QCOMPARE(renderEffect(effect).pixelColor(100, 100).alpha(), 0);
    LightingState state; state.layers = {effect}; state.layers[0].mask.fill(0);
    QCOMPARE(LightingDocument::render(state).pixelColor(140, 100).alpha(), 0);
    effect.extent.setWidth(0);
    QVERIFY_EXCEPTION_THROWN(renderEffect(effect), std::invalid_argument);
}

void EditorTest::effectPersistenceAndLegacy()
{
    QTemporaryDir directory;
    const auto path = directory.filePath("effects.json");
    LightingDocument document(catalog());
    for (const auto& definition : effectDefinitions()) document.addLayer(definition.kind);
    document.edit("Set gradient colors", [](LightingState& state) {
        state.layers[1].color = Qt::red; state.layers[1].secondaryColor = Qt::blue;
        for (auto& layer : state.layers) if (isParticle(layer.effect)) layer.visible = false;
        state.layers[1].rotation = 35; state.layers[1].center = {200, 100};
    });
    document.save(path);
    LightingDocument loaded(catalog()); loaded.load(path);
    QCOMPARE(loaded.render(), document.render());
    for (int i = 0; i < document.state().layers.size(); ++i) {
        QCOMPARE(loaded.state().layers[i].effect, document.state().layers[i].effect);
        QCOMPARE(loaded.state().layers[i].extent, document.state().layers[i].extent);
        QCOMPARE(loaded.state().layers[i].center, document.state().layers[i].center);
        QCOMPARE(loaded.state().layers[i].rotation, document.state().layers[i].rotation);
        QCOMPARE(loaded.state().layers[i].secondaryColor, document.state().layers[i].secondaryColor);
        QCOMPARE(loaded.state().layers[i].repetitions, document.state().layers[i].repetitions);
    }
    QFile file(path); QVERIFY(file.open(QIODevice::ReadOnly));
    auto root = QJsonDocument::fromJson(file.readAll()).object(); file.close();
    QCOMPARE(root.value("schemaVersion").toInt(), 8);
    // Published schema 2 has explicit effects but no image fields or Image type.
    auto version2 = root; version2.remove("components"); version2.remove("assignments"); QJsonArray version2Layers;
    for (const auto& item : root.value("layers").toArray()) {
        auto value = item.toObject();
        if (value.value("effect").toObject().value("type").toString() == "image" || isParticle(effectDefinition(value.value("effect").toObject().value("type").toString()).kind)) continue;
        value.remove("advanced"); value.remove("animation"); value.remove("imagePng"); value.remove("imageFill"); version2Layers.append(value);
    }
    version2.insert("schemaVersion", 2); version2.insert("layers", version2Layers);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write(QJsonDocument(version2).toJson()); file.close();
    loaded.load(path); QCOMPARE(loaded.render(), document.render());
    auto version3 = root; version3.remove("components"); version3.remove("assignments"); QJsonArray version3Layers;
    for (const auto& item : root.value("layers").toArray()) {
        auto value = item.toObject(); if (isParticle(effectDefinition(value.value("effect").toObject().value("type").toString()).kind)) continue;
        value.remove("advanced"); value.remove("animation"); version3Layers.append(value);
    }
    version3.insert("schemaVersion", 3); version3.insert("layers", version3Layers);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write(QJsonDocument(version3).toJson()); file.close(); loaded.load(path);
    QCOMPARE(loaded.render(), document.render());
    auto version4=root; version4.remove("components"); version4.remove("assignments"); QJsonArray version4Layers;
    for (const auto& item : root.value("layers").toArray()) {
        auto value=item.toObject(); if (isParticle(effectDefinition(value.value("effect").toObject().value("type").toString()).kind)) continue;
        value.remove("advanced"); version4Layers.append(value);
    }
    version4.insert("schemaVersion",4); version4.insert("layers",version4Layers);
    QVERIFY(file.open(QIODevice::WriteOnly|QIODevice::Truncate));
    file.write(QJsonDocument(version4).toJson()); file.close(); loaded.load(path);
    QCOMPARE(loaded.render(),document.render());
    document.save(path);
    loaded.load(path);
    auto layers = root.value("layers").toArray();
    auto json = layers[1].toObject(); auto parameters = json.value("effect").toObject();
    parameters.insert("type", "future-unsupported-effect"); json.insert("effect", parameters);
    layers[1] = json; root.insert("layers", layers);
    auto write = [&] {
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate)); file.write(QJsonDocument(root).toJson()); file.close();
    };
    write();
    const auto before = loaded.render();
    QVERIFY_EXCEPTION_THROWN(loaded.load(path), std::invalid_argument);
    QCOMPARE(loaded.render(), before);
    parameters.insert("type", "linear"); parameters.insert("width", -1);
    json.insert("effect", parameters); layers[1] = json; root.insert("layers", layers); write();
    QVERIFY_EXCEPTION_THROWN(loaded.load(path), std::invalid_argument);
    QCOMPARE(loaded.render(), before);

    // The previous published editor wrote schema 1 with implicit Static Color.
    auto legacy = layers[0].toObject(); legacy.remove("advanced"); legacy.remove("animation"); legacy.remove("effect"); legacy.remove("imagePng"); legacy.remove("imageFill");
    root.remove("components"); root.remove("assignments");
    root.insert("schemaVersion", 1); root.insert("layers", QJsonArray{legacy}); write();
    loaded.load(path);
    QCOMPARE(loaded.state().layers.size(), 1);
    QCOMPARE(loaded.state().layers[0].effect, EffectKind::StaticColor);
    QCOMPARE(loaded.render().pixelColor(10, 10), QColor(Qt::white));
}

void EditorTest::effectControls()
{
    LightingWindow window(catalog(), [] { return QVector<ProfileCandidate>{}; });
    auto* type = window.findChild<QComboBox*>("newEffectType");
    type->setCurrentIndex(type->findData(static_cast<int>(EffectKind::Star)));
    for (auto* button : window.findChildren<QPushButton*>()) if (button->text() == "New layer") button->click();
    QCOMPARE(window.document().state().layers[0].effect, EffectKind::Star);
    auto* width = window.findChild<QDoubleSpinBox*>("effectGeometry2"); width->setValue(150);
    QCOMPARE(window.document().state().layers[0].extent.width(), 150.0);
    window.document().history()->undo(); QCOMPARE(width->value(), 240.0);
    auto* selected = window.findChild<QComboBox*>("selectedEffectType");
    selected->setCurrentIndex(selected->findData(static_cast<int>(EffectKind::RadialGradient)));
    QCOMPARE(window.document().state().layers[0].name, QString("Radial Gradient"));
    QCOMPARE(window.document().state().layers[0].effect, EffectKind::RadialGradient);
    QCOMPARE(window.document().state().layers[0].mask.constScanLine(0)[0], uchar(255));
    window.document().edit("Preview colors", [](LightingState& state) {
        state.layers[0].color = QColor("#ffb347"); state.layers[0].secondaryColor = QColor("#242c78");
        state.layers[0].extent = {640, 400};
    });
    type->setCurrentIndex(type->findData(static_cast<int>(EffectKind::Star)));
    for (auto* button : window.findChildren<QPushButton*>()) if (button->text() == "New layer") button->click();
    window.document().edit("Preview shape", [](LightingState& state) {
        state.layers[1].color = QColor("#ff409e"); state.layers[1].rotation = 20;
        state.layers[1].transparency = 30;
    });
    window.findChild<QTabWidget*>("lightingViews")->setCurrentIndex(1);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QVERIFY(window.grab().save(QString(TEST_ARTIFACT_DIR) + "/editor-gradient-shape.png"));
    window.document().history()->setClean();
}

void EditorTest::imageImportAndCrop()
{
    QTemporaryDir directory;
    const auto source = directory.filePath("source.png"), project = directory.filePath("image.json");
    QImage pixels(200, 100, QImage::Format_ARGB32); pixels.fill(QColor(255, 0, 0, 128));
    QVERIFY(pixels.save(source));
    LightingDocument document(catalog());
    const auto id = document.importImage(source);
    QCOMPARE(document.state().layers[0].id, id);
    QCOMPARE(document.render().pixelColor(320, 10).alpha(), 0);
    QCOMPARE(document.render().pixelColor(320, 200).alpha(), 128);
    document.edit("Crop", [](auto& state) { state.layers[0].imageFill = true; });
    QCOMPARE(document.render().pixelColor(320, 10).alpha(), 128);
    document.save(project);
    QVERIFY(QFile::remove(source));
    LightingDocument loaded(catalog()); loaded.load(project);
    QCOMPARE(loaded.render(), document.render());
    document.history()->undo();
    QCOMPARE(document.render().pixelColor(320, 10).alpha(), 0);
    const auto count = document.history()->count();
    QVERIFY_EXCEPTION_THROWN(document.importImage(source), std::invalid_argument);
    QCOMPARE(document.history()->count(), count);
    QImage huge(4097, 1, QImage::Format_RGB32); huge.fill(Qt::white); QVERIFY(huge.save(source));
    QVERIFY_EXCEPTION_THROWN(document.importImage(source), std::invalid_argument);
    QCOMPARE(document.state().layers.size(), 1);
    for (const auto& format : {"JPEG", "BMP", "WEBP"}) {
        const auto path = directory.filePath(QString("fixture.") + format);
        QVERIFY(pixels.save(path, format));
        document.importImage(path);
    }
    const auto gif = directory.filePath("fixture.gif");
    QFile gifFile(gif); QVERIFY(gifFile.open(QIODevice::WriteOnly));
    gifFile.write(QByteArray::fromBase64("R0lGODlhAQABAIAAAAAAAP///ywAAAAAAQABAAACAUwAOw==")); gifFile.close();
    document.importImage(gif);
    QCOMPARE(document.state().layers.last().image.size(), QSize(1,1));
    QFile broken(project); QVERIFY(broken.open(QIODevice::ReadOnly));
    auto root = QJsonDocument::fromJson(broken.readAll()).object(); broken.close();
    auto layers = root.value("layers").toArray(); auto entry = layers[0].toObject();
    entry.insert("imagePng", "broken"); layers[0] = entry; root.insert("layers", layers);
    QVERIFY(broken.open(QIODevice::WriteOnly | QIODevice::Truncate));
    broken.write(QJsonDocument(root).toJson()); broken.close();
    const auto before = loaded.render();
    QVERIFY_EXCEPTION_THROWN(loaded.load(project), std::invalid_argument);
    QCOMPARE(loaded.render(), before);
    LightingWindow window(catalog(), [] { return QVector<ProfileCandidate>{}; });
    window.document().importImage(directory.filePath("fixture.BMP"));
    auto* tree = window.findChild<QTreeWidget*>("effectLayers");
    tree->setCurrentItem(tree->topLevelItem(0));
    auto* fit = window.findChild<QComboBox*>("imageFit"); fit->setCurrentIndex(1);
    QVERIFY(window.document().state().layers[0].imageFill);
    window.document().history()->undo(); QCOMPARE(fit->currentIndex(), 0);
    window.findChild<QTabWidget*>("lightingViews")->setCurrentIndex(1);
    window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window));
    QVERIFY(window.grab().save(QString(TEST_ARTIFACT_DIR) + "/editor-image.png"));
    window.document().history()->setClean();
}

void EditorTest::animationPreview()
{
    auto effect = layer(Qt::red); effect.hueShift = 120; effect.pulse = 1;
    QCOMPARE(renderEffect(effect, 0).pixelColor(100,100), QColor(Qt::red));
    QCOMPARE(renderEffect(effect, 0.5).pixelColor(100,100).alpha(), 0);
    QVERIFY(renderEffect(effect, 1).pixelColor(100,100).green() > 253);
    effect.hueShift = 0; effect.pulse = 0; effect.effect = EffectKind::Circle;
    effect.center = {100,100}; effect.extent = {100,40}; effect.spin = 90;
    QCOMPARE(renderEffect(effect, 0).pixelColor(140,100).alpha(), 255);
    QCOMPARE(renderEffect(effect, 1).pixelColor(140,100).alpha(), 0);
    QVERIFY_EXCEPTION_THROWN(renderEffect(effect, -1), std::invalid_argument);
    QTemporaryDir dir; const auto path = dir.filePath("motion.json");
    LightingWindow window(catalog(), [] { return QVector<ProfileCandidate>{}; });
    window.document().addLayer();
    auto* tree = window.findChild<QTreeWidget*>("effectLayers"); tree->setCurrentItem(tree->topLevelItem(0));
    auto* spin = window.findChild<QDoubleSpinBox*>("animationRate1"); spin->setValue(1);
    QCOMPARE(window.document().state().layers[0].pulse, 1.0);
    window.document().save(path);
    LightingDocument loaded(catalog()); loaded.load(path);
    QCOMPARE(loaded.state().layers[0].pulse, 1.0);
    QCOMPARE(loaded.render(0.5).pixelColor(320,200).alpha(), 0);
    const auto count = window.document().history()->count();
    auto* time = window.findChild<QDoubleSpinBox*>("previewTime"); time->setValue(0.5);
    auto* play = window.findChild<QCheckBox*>("playPreview"); play->setChecked(true);
    QTRY_VERIFY(time->value() > 0.5); play->setChecked(false);
    QCOMPARE(window.document().history()->count(), count); QVERIFY(window.document().history()->isClean());
    play->setChecked(true);
    for (auto* button : window.findChildren<QPushButton*>()) if (button->text() == "Edit mask") button->click();
    QVERIFY(!play->isChecked());
    window.document().edit("Mask", [](auto& state) { state.layers[0].mask.fill(0); });
    QCOMPARE(window.document().render(0.25).pixelColor(320,200).alpha(), 0);
    window.document().history()->undo();
    window.document().edit("Invalid rate", [](auto& state) { state.layers[0].spin = 90; });
    QVERIFY_EXCEPTION_THROWN(window.document().edit("Invalid rate", [](auto& state) { state.layers[0].pulse = -1; }), std::invalid_argument);
    window.document().history()->undo(); window.document().history()->setClean();
}

void EditorTest::advancedEffects()
{
    auto effect = layer(Qt::red); effect.effect = EffectKind::Circle;
    effect.center = {100,100}; effect.extent = {40,40}; effect.driftX=100;
    QCOMPARE(renderEffect(effect,1).pixelColor(200,100), QColor(Qt::red));
    QCOMPARE(renderEffect(effect,1).pixelColor(100,100).alpha(),0);
    effect.driftX=0; effect.zoom=50;
    QCOMPARE(renderEffect(effect,0.25).pixelColor(125,100).alpha(),255);
    effect.zoom=0; effect.breathe=1;
    QVERIFY(renderEffect(effect,0.5).pixelColor(100,100).alpha()<10);
    effect.breathe=0; effect.cycleRate=1; effect.colorCycle={Qt::red,Qt::blue};
    QCOMPARE(renderEffect(effect,0.5).pixelColor(100,100),QColor(Qt::blue));
    effect.cycleRate=0; effect.warp=20;
    QVERIFY(renderEffect(effect,0.25)!=renderEffect(effect,0)); effect.warp=0;
    effect.driftX=100; effect.trails=1;
    QVERIFY(renderEffect(effect,1).pixelColor(150,100).alpha()>0);
    for (auto kind : {EffectKind::Rain,EffectKind::Orbit,EffectKind::Fireflies,EffectKind::Burst}) {
        effect.effect=kind; effect.driftX=0; effect.trails=0; effect.particleCount=400;
        effect.extent={640,400}; effect.center={320,200};
        const auto first=renderEffect(effect,1);
        QCOMPARE(first,renderEffect(effect,1)); QVERIFY(first!=renderEffect(effect,2));
        effect.particleSeed+=1; QVERIFY(first!=renderEffect(effect,1));
    }
    effect.trails=1;
    LightingState benchmarkState; benchmarkState.layers={effect};
    QElapsedTimer benchmark; benchmark.start();
    for (int i=0;i<8;++i) LightingDocument::render(benchmarkState,1+i/30.0);
    qInfo() << "400 particles with trails, composed mean frame ms:" << benchmark.elapsed()/8.0;
    QTemporaryDir dir; LightingDocument document(catalog()); document.addLayer();
    document.edit("Advanced",[&](auto& state) { effect.id=state.layers[0].id; state.layers[0]=effect; });
    const auto path=dir.filePath("advanced.json"); document.save(path);
    LightingDocument loaded(catalog()); loaded.load(path); QCOMPARE(loaded.render(1),document.render(1));
    QFile file(path); QVERIFY(file.open(QIODevice::ReadOnly));
    auto root=QJsonDocument::fromJson(file.readAll()).object(); file.close();
    auto layers=root.value("layers").toArray(); auto entry=layers[0].toObject(); auto advanced=entry.value("advanced").toObject();
    advanced.insert("particleCount",401); entry.insert("advanced",advanced); layers[0]=entry; root.insert("layers",layers);
    QVERIFY(file.open(QIODevice::WriteOnly|QIODevice::Truncate)); file.write(QJsonDocument(root).toJson()); file.close();
    const auto before=loaded.render(1); QVERIFY_EXCEPTION_THROWN(loaded.load(path),std::invalid_argument);
    QCOMPARE(loaded.render(1),before);
    QVERIFY_EXCEPTION_THROWN(document.edit("Invalid",[](auto& state) { state.layers[0].particleCount=401; }),std::invalid_argument);
    document.edit("Linked",[](auto& state) {
        auto& layer=state.layers[0]; layer.effect=EffectKind::StaticColor; layer.maskLinked=true;
        layer.mask.fill(0); layer.mask.scanLine(100)[100]=255;
    });
    document.edit("Move",[](auto& state) { state.layers[0].center += QPointF(20,0); });
    QCOMPARE(document.state().layers[0].mask.constScanLine(100)[120],uchar(255));
    document.history()->undo(); QCOMPARE(document.state().layers[0].mask.constScanLine(100)[100],uchar(255));
    document.edit("Linked drift",[](auto& state) { state.layers[0].driftX=20; });
    QCOMPARE(document.render(1).pixelColor(120,100).alpha(),255);
    document.edit("Linked warp",[](auto& state) { state.layers[0].warp=20; });
    QCOMPARE(document.render(1.25).pixelColor(145,100).alpha(),255);
}

void EditorTest::canvasTransformsAndCrop()
{
    LightingWindow window(catalog(),[] { return QVector<ProfileCandidate>{}; });
    window.document().addLayer(EffectKind::Circle);
    auto* tree=window.findChild<QTreeWidget*>("effectLayers"); tree->setCurrentItem(tree->topLevelItem(0));
    window.findChild<QTabWidget*>("lightingViews")->setCurrentIndex(1);
    window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto* canvas=window.findChild<LightingCanvas*>("effectsCanvas");
    const auto center=canvas->rect().center();
    const auto count=window.document().history()->count();
    QTest::mousePress(canvas,Qt::LeftButton,Qt::NoModifier,center);
    QTest::mouseMove(canvas,center+QPoint(30,10)); QTest::mouseRelease(canvas,Qt::LeftButton,Qt::NoModifier,center+QPoint(30,10));
    QCOMPARE(window.document().state().layers[0].center,QPointF(350,210));
    QCOMPARE(window.document().history()->count(),count+1); window.document().history()->undo();
    QTest::mousePress(canvas,Qt::LeftButton,Qt::NoModifier,center);
    QTest::mouseMove(canvas,center+QPoint(50,0)); QTest::keyClick(canvas,Qt::Key_Escape);
    QTest::mouseRelease(canvas,Qt::LeftButton,Qt::NoModifier,center+QPoint(50,0));
    QCOMPARE(window.document().state().layers[0].center,QPointF(320,200));
    const auto rotate=center+QPoint(0,-144);
    QTest::mousePress(canvas,Qt::LeftButton,Qt::NoModifier,rotate);
    QTest::mouseMove(canvas,center+QPoint(144,0)); QTest::mouseRelease(canvas,Qt::LeftButton,Qt::NoModifier,center+QPoint(144,0));
    QVERIFY(std::abs(window.document().state().layers[0].rotation-90)<1); window.document().history()->undo();
    const auto corner=center+QPoint(120,120);
    QTest::mousePress(canvas,Qt::LeftButton,Qt::NoModifier,corner);
    QTest::mouseMove(canvas,corner+QPoint(30,30)); QTest::mouseRelease(canvas,Qt::LeftButton,Qt::NoModifier,corner+QPoint(30,30));
    QVERIFY(window.document().state().layers[0].extent.width()>290); window.document().history()->undo();
    QTemporaryDir dir; const auto imagePath=dir.filePath("crop.png");
    QImage pixels(200,100,QImage::Format_ARGB32); pixels.fill(Qt::red);
    for (int y=0;y<100;++y) for (int x=100;x<200;++x) pixels.setPixelColor(x,y,Qt::blue);
    QVERIFY(pixels.save(imagePath)); window.document().importImage(imagePath);
    tree->setCurrentItem(tree->topLevelItem(0));
    window.findChild<QCheckBox*>("cropTool")->setChecked(true);
    QTest::mousePress(canvas,Qt::LeftButton,Qt::NoModifier,center+QPoint(5,-80));
    QTest::mouseMove(canvas,center+QPoint(200,80));
    QTest::mouseRelease(canvas,Qt::LeftButton,Qt::NoModifier,center+QPoint(200,80));
    QVERIFY(window.document().state().layers.last().crop.width()<0.5);
    QCOMPARE(window.document().render().pixelColor(320,200),QColor(Qt::blue));
    window.document().history()->undo(); QCOMPARE(window.document().state().layers.last().crop,QRectF(0,0,1,1));
    auto* cropWidth=window.findChild<QDoubleSpinBox*>("imageCrop2");
    cropWidth->setValue(0.5); auto* cropLeft=window.findChild<QDoubleSpinBox*>("imageCrop0"); cropLeft->setValue(0.8);
    QCOMPARE(window.document().state().layers.last().crop.x(),0.0); // Rejected without escaping a Qt slot.
    auto* colors=window.findChild<QLineEdit*>("colorCycle"); colors->setText("not-a-color");
    QMetaObject::invokeMethod(colors,"editingFinished",Qt::DirectConnection);
    QCOMPARE(window.document().state().layers.last().colorCycle.size(),3);
    window.document().history()->setClean();
    QVERIFY(window.grab().save(QString(TEST_ARTIFACT_DIR)+"/editor-transform.png"));
}

void EditorTest::advancedControls()
{
    LightingWindow window(catalog(),[] { return QVector<ProfileCandidate>{}; });
    auto* types=window.findChild<QComboBox*>("newEffectType");
    types->setCurrentIndex(types->findData(static_cast<int>(EffectKind::Orbit)));
    for (auto* button : window.findChildren<QPushButton*>()) if (button->text()=="New layer") button->click();
    window.findChild<QDoubleSpinBox*>("particleCount")->setValue(400);
    QCOMPARE(window.document().state().layers[0].particleCount,400.0);
    QVERIFY(!window.findChild<QCheckBox*>("cropTool")->isEnabled());
    window.findChild<QDoubleSpinBox*>("particleSeed")->setValue(42);
    QCOMPARE(window.document().state().layers[0].particleSeed,42.0);
    window.document().history()->undo(); QCOMPARE(window.findChild<QDoubleSpinBox*>("particleSeed")->value(),1.0);
    window.findChild<QCheckBox*>("maskLinked")->setChecked(true);
    QVERIFY(window.document().state().layers[0].maskLinked);
    window.findChild<QDoubleSpinBox*>("trails")->setValue(0.5);
    QCOMPARE(window.document().state().layers[0].trails,0.5);
    window.document().edit("Preview appearance",[](auto& state) {
        auto& layer=state.layers[0]; layer.color=QColor("#ffc857"); layer.secondaryColor=QColor("#e652a0");
        layer.center={320,200}; layer.extent={540,340}; layer.particleSize=2;
    });
    window.findChild<QDoubleSpinBox*>("previewTime")->setValue(1.25);
    window.findChild<QTabWidget*>("lightingViews")->setCurrentIndex(1);
    window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window));
    QVERIFY(window.grab().save(QString(TEST_ARTIFACT_DIR)+"/editor-particles.png"));
    window.document().history()->setClean();
}

QTEST_MAIN(EditorTest)
#include "test_editor.moc"
