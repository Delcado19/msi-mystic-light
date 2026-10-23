#include "editor/LightingWindow.h"
#include <QApplication>
#include <QDir>
#include <QComboBox>
#include <QHeaderView>
#include <QPushButton>
#include <QSlider>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
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
    click("New layer · Static Color");
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

QTEST_MAIN(EditorTest)
#include "test_editor.moc"
