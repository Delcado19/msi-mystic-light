#include "editor/LightingWindow.h"
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
EffectLayer layer(QColor color)
{
    EffectLayer result; result.color = color;
    result.mask = LightingDocument::whiteMask(); return result;
}
}

class EditorTest : public QObject {
    Q_OBJECT
private slots:
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
};

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
        if (definition.kind < EffectKind::Circle || definition.kind == EffectKind::Image) continue;
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
    QCOMPARE(root.value("schemaVersion").toInt(), 4);
    // Published schema 2 has explicit effects but no image fields or Image type.
    auto version2 = root; QJsonArray version2Layers;
    for (const auto& item : root.value("layers").toArray()) {
        auto value = item.toObject();
        if (value.value("effect").toObject().value("type").toString() == "image") continue;
        value.remove("animation"); value.remove("imagePng"); value.remove("imageFill"); version2Layers.append(value);
    }
    version2.insert("schemaVersion", 2); version2.insert("layers", version2Layers);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write(QJsonDocument(version2).toJson()); file.close();
    loaded.load(path); QCOMPARE(loaded.render(), document.render());
    auto version3 = root; QJsonArray version3Layers;
    for (const auto& item : root.value("layers").toArray()) {
        auto value = item.toObject(); value.remove("animation"); version3Layers.append(value);
    }
    version3.insert("schemaVersion", 3); version3.insert("layers", version3Layers);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write(QJsonDocument(version3).toJson()); file.close(); loaded.load(path);
    QCOMPARE(loaded.render(), document.render());
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
    auto legacy = layers[0].toObject(); legacy.remove("animation"); legacy.remove("effect"); legacy.remove("imagePng"); legacy.remove("imageFill");
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
    for (const auto& format : {"JPEG", "BMP"}) {
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

QTEST_MAIN(EditorTest)
#include "test_editor.moc"
