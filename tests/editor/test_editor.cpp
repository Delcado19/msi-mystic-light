#include "editor/ParameterSpinBox.h"
#include <QDialogButtonBox>
#include <QMenu>
#include <QColorDialog>
#include <QVariantAnimation>
#include "editor/LightingWindow.h"
#include "editor/ComponentEditor.h"
#include "editor/SnapGrid.h"
#include "editor/HardwareIdentity.h"
#include "editor/HardwareArtwork.h"
#include "editor/ChannelEditor.h"
#include "editor/AppSession.h"
#include "editor/StudioWidgets.h"
#include "editor/HardwarePicker.h"
#include "editor/LegacyHardware.h"
#include <QStackedWidget>
#include <QToolButton>
#include <QToolBar>
#include <QListWidget>
#include <QTimer>
#include <QTableWidget>
#include <QDialogButtonBox>
#include <QMenu>
#include <QColorDialog>
#include <QVariantAnimation>
#include <QLabel>
#include <QApplication>
#include <QDir>
#include <QComboBox>
#include <QCheckBox>
#include <QHeaderView>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFontMetrics>
#include <QJsonArray>
#include <QImageReader>
#include <QSvgRenderer>
#include <QElapsedTimer>
#include <QLineEdit>
#include <QLineF>
#include <QGraphicsView>
#include <QScrollBar>
#include <QSplitter>
#include <QSplitterHandle>
#include <QDockWidget>
#include <QStyleOptionSlider>
#include <QStyleOptionSlider>
#include <limits>
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsScene>
#include <QtTest>
#include <QScopeGuard>
#include <stdexcept>

namespace {
QString artifactDirectory()
{
    // Offscreen correctness runs must never overwrite native screenshots intended for documentation.
    const auto platform=QGuiApplication::platformName();
    const auto directory=QString(TEST_ARTIFACT_DIR)+"/"+platform;
    if(!QDir().mkpath(directory))qFatal("Cannot create screenshot artifact directory");
    QFile metadata(directory+"/rendering.json");
    if(!metadata.open(QIODevice::WriteOnly))qFatal("Cannot write screenshot rendering metadata");
    const QFontMetrics metrics(QApplication::font());
    metadata.write(QJsonDocument(QJsonObject{{"platform",platform},
        {"latinGlyphsAvailable",metrics.inFont(QChar('A')) && metrics.inFont(QChar('a')) && metrics.inFont(QChar('0'))}}).toJson());
    return directory;
}
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
void addEffect(LightingWindow& window,EffectKind kind=EffectKind::StaticColor)
{
    window.findChild<QAction*>("workspaceEffects")->trigger();
    window.findChild<QPushButton*>("playPreview")->setChecked(false);
    if(isParticle(kind) || (kind>=EffectKind::Circle && kind<=EffectKind::Moon)) {
        auto* action=window.findChild<QAction*>("create_"+effectDefinition(kind).id); QVERIFY(action); action->trigger(); return;
    }
    const auto id=window.document().addLayer(kind);
    auto* tree=window.findChild<QTreeWidget*>("effectLayers");
    for(int i=0;i<tree->topLevelItemCount();++i) if(tree->topLevelItem(i)->data(0,Qt::UserRole).toUuid()==id)
        tree->setCurrentItem(tree->topLevelItem(i));
}
EffectLayer layer(QColor color)
{
    EffectLayer result; result.color = color;
    result.mask = LightingDocument::whiteMask(); return result;
}
}

class EditorTest : public QObject {
    Q_OBJECT
    QTemporaryDir settingsDirectory_;
private slots:
    void initTestCase() {
        QVERIFY(settingsDirectory_.isValid());
        QCoreApplication::setOrganizationName("LightingStudioTests");
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,settingsDirectory_.path());
    }
    void init() { SnapGrid{false,10}.save(); QSettings().setValue("editor/paletteSnapPercent",1);QSettings().setValue("editor/showComponentLabels",false); } // Legacy gesture tests exercise free placement; grid tests enable it explicitly.
    void eraserPlayback_data();
    void eraserPlayback();
    void layerEyesAndStroke();
    void initialMaskBrushSize();
    void maskPlaybackAndInventory();
    void stableMinimapAnchor();
    void detailedHardwareArtwork();
    void layoutPresentationFixes();
    void denseHardwareArtwork();
    void gamingWorkspace();
    void layerModeAndOrder();
    void fixedComponentLabels();
    void labelRevealControls();
    void hardwarePreviewProjection();
    void deviceArtworkAlignment();
    void singleZoneGpuArtwork();
    void modelLabelsAndFullsizeKeyboard();
    void dedicatedDeviceIsNotAChain();
    void fixedDeviceSettings_data();
    void fixedDeviceSettings();
    void fixedDeviceRejectsChain();
    void scrollbarAppearance();
    void effectPreviewDivider();
    void sharedCanvasExtent();
    void deleteSelections();
    void protectedGroupLabels();
    void effectsOverflowNavigation();
    void anchoredResize_data();
    void anchoredResize();
    void edgeScrolling_data();
    void edgeScrolling();
    void workspaceNavigation();
    void hardwareIdentityAndSave();
    void snapGridBoundaries();
    void snapLayoutGroup();
    void snapLayoutPositions_data();
    void snapLayoutPositions();
    void snapEffectAndGradient();
    void snapLedArrangement();
    void snapPaletteStops();
    void snapControls();
    void drawingAndAnimationRegressions();
    void restoredLayoutAndDrawing();
    void legacyHardware();
    void periodicEffectsAndGradientStops();
    void globalColorControls();
    void integerDisplayRoundTrip();
    void liveColorPicker();
    void auditWorkspace();
    void objectLayersAndGestures();
    void routedCompositionCache();
    void drawingTools();
    void hardwareLibrary();
    void visualWorkspace();
    void channelScopesAndClocks();
    void sessionRecovery();
    void channelAssignments();
    void channelControls();
    void componentSampling();
    void componentAuthoring();
    void deviceLedArrangement();
    void componentEditor();
    void keyboardLedPresentation();
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
    void triangleAfterCrop();
    void expandingLayoutAndFullscreen();
    void canvasScrollbars();
    void invalidCanvasZoom_data();
    void invalidCanvasZoom();
    void canvasZoomLimits();
    void invalidLayoutGeometry();
    void invalidDraftSampling_data();
    void invalidDraftSampling();
    void scrollbarMouseInteraction();
    void scrollbarExactFit_data();
    void scrollbarExactFit();
    void advancedControls();
};

void EditorTest::periodicEffectsAndGradientStops()
{
    auto effect=layer(Qt::red); effect.effect=EffectKind::Circle; effect.center={638,200}; effect.extent={40,40};
    const auto base=renderEffect(effect); QCOMPARE(base.pixelColor(2,200),QColor(Qt::red)); QCOMPARE(base.pixelColor(638,200),QColor(Qt::red));
    effect.driftX=320; QCOMPARE(renderEffect(effect,2),base); effect.driftX=-320; QCOMPARE(renderEffect(effect,2),base);
    effect.driftX=20; effect.maskLinked=true; effect.mask.fill(0); effect.mask.scanLine(200)[638]=255;
    QCOMPARE(renderMask(effect,1).constScanLine(200)[18],uchar(255));
    auto gradient=layer(Qt::red); gradient.effect=EffectKind::LinearGradient; gradient.extent={640,400};
    gradient.gradientColors={Qt::red,Qt::green,Qt::blue}; const auto frame=renderEffect(gradient);
    QVERIFY(frame.pixelColor(320,200).green()>253); QVERIFY(frame.pixelColor(1,200).red()>250); QVERIFY(frame.pixelColor(638,200).blue()>250);
    gradient.rotation=-30; gradient.driftX=114;
    for(const double time:{0.,.25,1.,144.87}) {
        const auto moving=renderEffect(gradient,time);
        int worst=0;
        for(int y=0;y<400;++y) for(int x=1;x<640;++x) {
            const auto a=moving.pixelColor(x-1,y),b=moving.pixelColor(x,y);
            worst=std::max({worst,std::abs(a.red()-b.red()),std::abs(a.green()-b.green()),std::abs(a.blue()-b.blue())});
        }
        QVERIFY2(worst<=4,"Travelling diagonal gradient must not contain a canvas-edge color discontinuity");
        const auto a=moving.pixelColor(250,150), b=moving.pixelColor(300,237);
        QVERIFY(std::abs(a.red()-b.red())<=2 && std::abs(a.blue()-b.blue())<=2);
    }
    const auto direction=renderEffect(gradient,1);
    const auto earlier=renderEffect(gradient,0);
    QCOMPARE(direction.pixelColor(300,200),earlier.pixelColor(186,200));
    LightingDocument document(catalog()); document.addLayer(EffectKind::LinearGradient);
    document.edit("Three stops",[&](auto& state) { state.layers[0].gradientColors=gradient.gradientColors; });
    QTemporaryDir dir; const auto file=dir.filePath("gradient.json"); document.save(file); LightingDocument loaded(catalog()); loaded.load(file);
    QCOMPARE(loaded.state().layers[0].gradientColors,gradient.gradientColors); QCOMPARE(loaded.render(),document.render());
    QVERIFY_EXCEPTION_THROWN(document.edit("Invalid stops",[](auto& state) { state.layers[0].gradientColors={Qt::red}; }),std::invalid_argument);
}
void EditorTest::hardwareLibrary()
{
    QTemporaryDir directory; const auto original=qgetenv("LOCALAPPDATA");
    const auto restore=qScopeGuard([&] { qputenv("LOCALAPPDATA",original); }); qputenv("LOCALAPPDATA",directory.path().toUtf8());
    const auto folder=directory.filePath("VortxEngine/app-2.0.0/Signal-x64/Components"); QVERIFY(QDir().mkpath(folder));
    const QJsonObject source{{"ProductName","Fixture fan"},{"Brand","Test brand"},{"Type","Fan"},{"LedCount",3},
        {"LedCoordinates",QJsonArray{QJsonArray{100,200},QJsonArray{300,200},QJsonArray{200,400}}},{"LedMapping",QJsonArray{2,0,1}}};
    auto write=[&](const QString& name,const QJsonObject& data) { QFile file(folder+"/"+name+".json"); QVERIFY(file.open(QIODevice::WriteOnly)); file.write(QJsonDocument(data).toJson()); };
    write("valid",source);
    auto strip=source;strip["ProductName"]="Bent fixture strip";strip["Type"]="Strip";write("strip",strip);
    auto cable=source;cable["ProductName"]="Lian Li GPU Strimer Plus fixture";cable["Type"]="Cable";write("cable",cable);
    auto d30=source;d30["ProductName"]="Phantek D30-120";d30["Brand"]="Phanteks";write("d30",d30);
    auto reverse=source;reverse["ProductName"]="Fixture fan Reverse";write("reverse",reverse);
    auto invalid=source; invalid.insert("ProductName","Malformed mapping"); invalid.insert("LedMapping",QJsonArray{2,2,0}); write("invalid",invalid);
    auto dedicated=source; dedicated.insert("ProductName","Dedicated RAM"); dedicated.insert("Type","RAM"); write("dedicated",dedicated);
    HardwarePicker picker(catalog(),{{"Header",{{"channelId","Direct"}},12}},{{"channelId","Direct"}});
    auto* list=picker.findChild<QListWidget*>("hardwareProducts");
    picker.findChild<QLineEdit*>("hardwareSearch")->setText("Fixture fan"); QVERIFY(list->currentItem());
    const auto component=picker.component(); QCOMPARE(component.mapping,QVector<int>({2,0,1})); QCOMPARE(component.positions.size(),3);
    for(const auto& point:component.positions) QVERIFY(point.x()>=0 && point.x()<=3 && point.y()>=0 && point.y()<=3);
    QVERIFY(!picker.findChild<QSpinBox*>("hardwareLedCount")->isEnabled());
    picker.findChild<QSpinBox*>("hardwareQuantity")->setValue(5); QVERIFY(!picker.findChild<QPushButton*>("confirmAddHardware")->isEnabled());
    picker.findChild<QLineEdit*>("hardwareSearch")->setText("Malformed mapping"); QVERIFY(!list->currentItem());
    picker.findChild<QLineEdit*>("hardwareSearch")->setText("Dedicated RAM"); QVERIFY(!list->currentItem());
    auto* brand=picker.findChild<QComboBox*>("hardwareManufacturer");brand->setCurrentText("Test brand");
    picker.findChild<QLineEdit*>("hardwareSearch")->setText("Bent fixture strip");QVERIFY(list->currentItem());
    const auto straight=picker.component();QCOMPARE(straight.mapping,QVector<int>({2,0,1}));
    QCOMPARE(straight.positions,QVector<QPointF>({{0,0},{1.5,0},{3,0}}));QCOMPARE(straight.manufacturer,QString("Test brand"));
    picker.findChild<QLineEdit*>("hardwareSearch")->setText("Strimer Plus fixture");QVERIFY(list->currentItem());
    const auto rows=picker.component();QCOMPARE(hardwareArtworkCategory(rows),QString("Cables"));
    QCOMPARE(rows.positions,QVector<QPointF>({{0,0},{3,0},{1.5,3}}));QCOMPARE(rows.mapping,QVector<int>({2,0,1}));
    auto* category=picker.findChild<QComboBox*>("hardwareCategory");category->setCurrentText("Fans");QVERIFY(!list->currentItem());
    category->setCurrentText("All hardware");brand->setCurrentText("Phanteks");
    picker.findChild<QLineEdit*>("hardwareSearch")->setText("D30-120");QVERIFY(list->currentItem());
    QVERIFY(!list->currentItem()->text().contains("Phanteks Phanteks"));
    const auto round=picker.component();QCOMPARE(round.mapping,QVector<int>({2,0,1}));
    for(const auto& point:round.positions) QVERIFY(qAbs(QLineF(point,QPointF(1.3,1.3)).length()-1.3)<.0001);
    brand->setCurrentText("All manufacturers");picker.findChild<QLineEdit*>("hardwareSearch")->setText("Reverse");QVERIFY(!list->currentItem());
}

void EditorTest::drawingAndAnimationRegressions()
{
    LightingDocument document(catalog()); const auto id=document.addPaintLayer();
    LightingCanvas canvas(document,LightingCanvas::Context::Effects); canvas.resize(800,600); canvas.show(); QVERIFY(QTest::qWaitForWindowExposed(&canvas)); canvas.select(id);
    auto drag=[&](LightingCanvas::Tool tool,QPointF a,QPointF b) { canvas.setTool(tool); QTest::mousePress(&canvas,Qt::LeftButton,Qt::NoModifier,canvas.viewportPoint(a).toPoint()); QTest::mouseMove(&canvas,canvas.viewportPoint(b).toPoint()); QTest::mouseRelease(&canvas,Qt::LeftButton,Qt::NoModifier,canvas.viewportPoint(b).toPoint()); };
    canvas.setDrawing(QColor("#ff5a00"),6,true); drag(LightingCanvas::Tool::Ellipse,{120,100},{280,260});
    const auto before=document.state().layers[0].image;
    canvas.setDrawing(Qt::cyan,6,true); drag(LightingCanvas::Tool::Fill,{200,180},{200,180});
    const auto filled=document.state().layers[0].image;
    for(int y=0;y<400;++y) for(int x=0;x<640;++x) {
        QCOMPARE(qAlpha(filled.pixel(x,y)),qAlpha(before.pixel(x,y)));
        if(qAlpha(before.pixel(x,y))>0) QCOMPARE(qRed(filled.pixel(x,y)),0);
    }
    canvas.setGradientStops({Qt::red,Qt::blue},{0,1}); drag(LightingCanvas::Tool::Gradient,{120,180},{280,180});
    QCOMPARE(document.state().layers.size(),1); QCOMPARE(document.state().layers[0].effect,EffectKind::Paint);
    const auto graded=document.state().layers[0].image;
    for(int y=0;y<400;++y) for(int x=0;x<640;++x) QCOMPARE(qAlpha(graded.pixel(x,y)),qAlpha(before.pixel(x,y)));
    QVERIFY(graded.pixelColor(150,180).red()>graded.pixelColor(250,180).red());
    const auto shape=document.addLayer(EffectKind::Heart); canvas.select(shape);
    const auto geometry=document.state().layers.last();
    drag(LightingCanvas::Tool::Gradient,{260,200},{380,200});
    const auto& shapeFill=document.state().layers.last(); QVERIFY(shapeFill.gradientFill);
    QCOMPARE(shapeFill.effect,EffectKind::Heart); QCOMPARE(shapeFill.center,geometry.center); QCOMPARE(shapeFill.extent,geometry.extent);
    QTemporaryDir saved; const auto path=saved.filePath("shape-gradient.json"); document.save(path);
    LightingDocument restored(catalog()); restored.load(path); QCOMPARE(restored.render(),document.render());
    QVERIFY(restored.state().layers.last().gradientFill);
    QFile jsonFile(path); QVERIFY(jsonFile.open(QIODevice::ReadOnly)); auto json=QJsonDocument::fromJson(jsonFile.readAll()).object(); jsonFile.close();
    json["schemaVersion"]=11; QVERIFY(jsonFile.open(QIODevice::WriteOnly)); jsonFile.write(QJsonDocument(json).toJson()); jsonFile.close();
    QVERIFY_EXCEPTION_THROWN(restored.load(path),std::invalid_argument);
    auto paint=document.state().layers[0]; paint.cycleRate=1; paint.colorCycle={Qt::red,Qt::blue};
    QCOMPARE(renderEffect(paint,0).pixelColor(200,180),QColor(Qt::red)); QCOMPARE(renderEffect(paint,.5).pixelColor(200,180),QColor(Qt::blue));
    auto gradient=layer(Qt::red); gradient.effect=EffectKind::LinearGradient; gradient.center={250,200}; gradient.extent={200,400}; gradient.secondaryColor=Qt::blue;
    QCOMPARE(renderEffect(gradient).pixelColor(639,200),QColor(Qt::blue));
    gradient.driftX=100; QCOMPARE(renderEffect(gradient,1).pixelColor(300,200),renderEffect(gradient,0).pixelColor(200,200));
    LightingWindow window(catalog(),[] { return QVector<ProfileCandidate>{}; },nullptr,{channelProfile()}); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window));
    addEffect(window,EffectKind::LinearGradient); addEffect(window,EffectKind::Circle);
    window.findChild<QDoubleSpinBox*>("driftX")->setValue(80);
    QCOMPARE(window.document().state().layers[0].driftX,0.0);window.findChild<QPushButton*>("applyMotionToAll")->click();
    QCOMPARE(window.document().state().layers[0].driftX,80.0); QCOMPARE(window.document().state().layers[1].driftX,80.0);
    window.findChild<QCheckBox*>("enableColorCycle")->setChecked(true); QVERIFY(window.findChild<QPushButton*>("playPreview")->isChecked());
    window.findChild<QPushButton*>("applyMotionToAll")->click();
    for(const auto& source:window.document().state().layers) QVERIFY(source.cycleRate>0);
    for(const auto& name:{"driftX","driftY","zoom","warp","breathe","trails"}) {
        auto* value=window.findChild<QDoubleSpinBox*>(name); bool linked=false;
        for(auto* slider:window.findChildren<QSlider*>()) if(slider->property("numericField").value<QObject*>()==value) linked=true;
        QVERIFY2(linked,name);
    }
    auto* tree=window.findChild<QTreeView*>("hardwareComponents"); QCOMPARE(tree->model(),window.findChild<QTreeWidget*>("layoutComponents")->model());
    const auto component=window.document().addComponent("test.strip");
    auto* live=window.findChild<LightingCanvas*>("hardwareLivePreview"); auto* time=window.findChild<QDoubleSpinBox*>("previewTime");
    window.findChild<QPushButton*>("playPreview")->setChecked(false); time->setValue(0); const auto first=live->grab().toImage();
    time->setValue(2); const auto next=live->grab().toImage();
    const auto pixel=(live->viewportPoint({80,80})*first.devicePixelRatio()).toPoint(); QVERIFY(first.pixelColor(pixel)!=next.pixelColor(pixel));
    window.findChild<QAction*>("workspaceHardware")->trigger();
    auto* layoutTree=window.findChild<QTreeWidget*>("layoutComponents");
    for(QTreeWidgetItemIterator it(layoutTree);*it;++it) if((*it)->data(0,Qt::UserRole).toUuid()==component) tree->setCurrentIndex(layoutTree->indexFromItem(*it));
    QVERIFY(window.findChild<QPushButton*>("removeHardwareComponent")->isEnabled()); window.findChild<QPushButton*>("removeHardwareComponent")->click();
    QVERIFY(window.document().state().placements.isEmpty()); window.document().history()->undo(); QCOMPARE(window.document().state().placements.size(),1);
    window.document().history()->setClean();
}

void EditorTest::legacyHardware()
{
    QTemporaryDir directory; const auto path=directory.filePath("settings.json");
    QJsonObject component{{"DisplayName","Saved fan"},{"LedCount",3},{"LedMapping",QJsonArray{2,0,1}},
        {"LedCoordinates",QJsonArray{QJsonArray{0,0},QJsonArray{50,0},QJsonArray{100,50}}}};
    QJsonObject saved{{"channelName","Direct"},{"components",QJsonArray{component}},
        {"placement",QJsonObject{{"x",110},{"y",70},{"width",101},{"height",51},{"rotation",0}}}};
    auto write=[&](QJsonObject channel) { QFile file(path); QVERIFY(file.open(QIODevice::WriteOnly)); file.write(QJsonDocument(QJsonObject{{"activeProfile","Saved"},{"profiles",QJsonArray{QJsonObject{{"name","Saved"},{"channels",QJsonArray{channel}}}}}}).toJson()); };
    write(saved); LightingDocument document({},nullptr,{channelProfile()}); DeviceIdentity identity; identity.path="exact-test-endpoint";
    const QVector<ProfileCandidate> candidates{{identity,"test.device"}};
    const auto notes=importLegacyHardware(document,path,candidates); QVERIFY(notes.first().contains("1 channel"));
    QCOMPARE(document.state().assignments.size(),1); QCOMPARE(document.state().assignments[0].endpoint,identity.path);
    const auto definition=document.catalog().first(); QCOMPARE(definition.mapping,QVector<int>({2,0,1}));
    QVERIFY(QLineF(document.componentPoints(document.state().placements[0])[0],QPointF(64,40)).length()<.01);
    document.history()->undo(); QVERIFY(document.state().placements.isEmpty());
    component["LedMapping"]=QJsonArray{0,0,1}; saved["components"]=QJsonArray{component}; write(saved);
    QVERIFY_EXCEPTION_THROWN(importLegacyHardware(document,path,candidates),std::invalid_argument);
    QVERIFY(document.state().placements.isEmpty());
}

void EditorTest::restoredLayoutAndDrawing()
{
    LightingWindow window(catalog(),[] { return QVector<ProfileCandidate>{}; },nullptr,{channelProfile()});
    window.resize(1440,900); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window));
    window.findChild<QPushButton*>("playPreview")->setChecked(false);
    window.findChild<QAction*>("workspaceEffects")->trigger(); window.findChild<QPushButton*>("newLayer")->click();
    auto* canvas=window.findChild<LightingCanvas*>("effectsCanvas");
    auto* color=window.findChild<ColorEditor*>("effectColor"); QVERIFY(color);
    auto setColor=[&](QString value) { auto* hex=color->findChild<QLineEdit*>("rgbHex"); hex->setText(value); QMetaObject::invokeMethod(hex,"editingFinished",Qt::DirectConnection); };
    auto drag=[&](LightingCanvas* target,QPointF a,QPointF b) { QTest::mousePress(target,Qt::LeftButton,Qt::NoModifier,target->viewportPoint(a).toPoint()); QTest::mouseMove(target,target->viewportPoint(b).toPoint()); QTest::mouseRelease(target,Qt::LeftButton,Qt::NoModifier,target->viewportPoint(b).toPoint()); };
    window.findChild<QPushButton*>("rectangleTool")->click(); setColor("#ff0000"); drag(canvas,{80,80},{200,180});
    window.findChild<QPushButton*>("fillTool")->click(); setColor("#0000ff"); QVERIFY(color->isVisible()); drag(canvas,{130,120},{130,120});
    QCOMPARE(window.document().render().pixelColor(130,120),QColor(Qt::blue));
    window.findChild<QPushButton*>("newLayer")->click();
    window.findChild<QPushButton*>("gradientTool")->click(); drag(canvas,{40,240},{500,240});
    const auto gradient=window.document().state().layers.last();
    drag(canvas,{500,240},{400,330});
    const auto adjusted=window.document().state().layers.last(); QVERIFY(adjusted.rotation>10);
    QVERIFY(QLineF(effectTransform(adjusted).map(QPointF(-.5,0)),QPointF(40,240)).length()<3);
    window.document().history()->undo(); QCOMPARE(window.document().state().layers.last().center,gradient.center);
    const auto one=window.document().addComponent("test.strip"),two=window.document().addComponent("test.strip");
    window.document().edit("Space components",[&](auto& state) { state.placements[0].origin={150,120}; state.placements[1].origin={250,180}; });
    ChannelAssignment chain; chain.endpoint="fake"; chain.profileId="test.device"; chain.channelId="Direct"; chain.software=true; chain.placements={one,two}; window.document().assignChannel(chain);
    window.findChild<QAction*>("workspaceLayout")->trigger(); auto* layout=window.findChild<LightingCanvas*>("layoutCanvas"); layout->select(chain.id);
    layout->select(one);
    const auto original=window.document().state().placements;
    const auto labelPoint=QPointF(140,120-25-layout->fontMetrics().height()/2.-4);
    drag(layout,labelPoint,labelPoint+QPointF(35,20));
    QCOMPARE(layout->selection(),chain.id);
    for(int i=0;i<original.size();++i) QVERIFY(QLineF(window.document().state().placements[i].origin,original[i].origin+QPointF(35,20)).length()<2);
    window.document().history()->undo(); QCOMPARE(window.document().state().placements[0].origin,original[0].origin);
    layout->select(chain.id);
    drag(layout,{135,105},{90,70}); QVERIFY(window.document().state().placements[0].scale.width()>1.1);
    window.document().history()->undo(); QCOMPARE(window.document().state().placements[0].scale,QSizeF(1,1));
    drag(layout,{220,81},{310,155}); QVERIFY(std::abs(window.document().state().placements[0].rotation)>30);
    window.document().history()->undo(); QCOMPARE(window.document().state().placements[0].rotation,0.0);
    QVERIFY(window.grab().save(artifactDirectory()+"/restored-layout.png"));
    window.findChild<QAction*>("workspaceHardware")->trigger(); QVERIFY(window.findChild<QTreeView*>("hardwareComponents")->isVisible()); QVERIFY(!layout->isVisible());
    QCoreApplication::processEvents(); // Let queued dock layout updates settle before capturing the workspace.
    QVERIFY(window.grab().save(artifactDirectory()+"/restored-hardware.png"));
    window.document().history()->setClean();
}

void EditorTest::drawingTools()
{
    LightingDocument document(catalog()); const auto id=document.addPaintLayer();
    QCOMPARE(document.render().pixelColor(100,100).alpha(),0);
    LightingCanvas canvas(document,LightingCanvas::Context::Effects); canvas.resize(800,600); canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas)); canvas.select(id); canvas.setDrawing(Qt::red,12,true);
    auto drag=[&](LightingCanvas::Tool tool,QPointF from,QPointF to) {
        canvas.setTool(tool); const auto a=canvas.viewportPoint(from).toPoint(), b=canvas.viewportPoint(to).toPoint();
        QTest::mousePress(&canvas,Qt::LeftButton,Qt::NoModifier,a); QTest::mouseMove(&canvas,b); QTest::mouseRelease(&canvas,Qt::LeftButton,Qt::NoModifier,b);
    };
    drag(LightingCanvas::Tool::Rectangle,{80,80},{180,180});
    QCOMPARE(document.render().pixelColor(120,120),QColor(Qt::red)); QCOMPARE(document.render().pixelColor(300,300).alpha(),0);
    QCOMPARE(document.history()->count(),2); document.history()->undo(); QCOMPARE(document.render().pixelColor(120,120).alpha(),0); document.history()->redo();
    canvas.setDrawing(Qt::blue,12,true); drag(LightingCanvas::Tool::Fill,{120,120},{120,120});
    QCOMPARE(document.render().pixelColor(120,120),QColor(Qt::blue)); QCOMPARE(document.render().pixelColor(300,300).alpha(),0);
    document.edit("Similar edge colors",[](auto& state) { state.layers[0].image.setPixelColor(121,120,QColor(0,0,235)); });
    canvas.setDrawing(QColor(0,0,245),12,true); drag(LightingCanvas::Tool::Fill,{120,120},{120,120});
    QCOMPARE(document.render().pixelColor(121,120),QColor(0,0,245));
    document.history()->undo(); document.history()->undo(); canvas.setDrawing(Qt::blue,12,true);
    drag(LightingCanvas::Tool::Eraser,{110,120},{150,120}); QCOMPARE(document.render().pixelColor(130,120).alpha(),0);
    document.history()->undo(); QCOMPARE(document.render().pixelColor(130,120),QColor(Qt::blue));
    drag(LightingCanvas::Tool::Line,{250,250},{300,250}); QVERIFY(document.render().pixelColor(275,250).blue()>200);
    drag(LightingCanvas::Tool::Ellipse,{350,250},{400,300}); QVERIFY(document.render().pixelColor(375,275).blue()>200);
    drag(LightingCanvas::Tool::Pencil,{250,300},{300,300}); QVERIFY(document.render().pixelColor(275,300).blue()>200);
    canvas.setMaskTarget(true); canvas.setBrush(20,0,1,0); drag(LightingCanvas::Tool::Pencil,{120,140},{120,140});
    QVERIFY(document.state().layers[0].mask.constScanLine(140)[120]<20); document.history()->undo();
    canvas.setMaskTarget(false);
    const auto blank=document.addPaintLayer(); canvas.select(blank); drag(LightingCanvas::Tool::Gradient,{20,200},{620,200});
    QCOMPARE(document.state().layers.size(),2); QCOMPARE(document.state().layers.last().effect,EffectKind::LinearGradient);
    document.edit("Move gradient stops",[](auto& state) { auto& layer=state.layers.last(); layer.gradientColors={Qt::red,Qt::green,Qt::blue}; layer.gradientPositions={0,.25,1}; });
    const auto expected=document.render(); QTemporaryDir dir; const auto path=dir.filePath("paint.json"); document.save(path);
    LightingDocument loaded(catalog()); loaded.load(path); QCOMPARE(loaded.render(),expected);
    QCOMPARE(loaded.state().layers.last().gradientPositions,QVector<double>({0,.25,1}));
    QVERIFY_EXCEPTION_THROWN(document.edit("Invalid stop order",[](auto& state) { state.layers.last().gradientPositions={0,.7,.4}; }),std::invalid_argument);
    QVERIFY_EXCEPTION_THROWN(document.edit("Invalid paint resolution",[](auto& state) { state.layers[0].image=QImage(8,8,QImage::Format_ARGB32_Premultiplied); }),std::invalid_argument);
    LightingDocument limited(catalog()); for(int i=0;i<64;++i) limited.addLayer();
    LightingCanvas capped(limited,LightingCanvas::Context::Effects); capped.resize(800,600); capped.show(); capped.setTool(LightingCanvas::Tool::Pencil);
    QSignalSpy errors(&capped,&LightingCanvas::editingError); QTest::mouseClick(&capped,Qt::LeftButton,Qt::NoModifier,capped.viewportPoint({320,200}).toPoint());
    QCOMPARE(errors.count(),1); QCOMPARE(limited.state().layers.size(),64);
    const auto heart=document.addLayer(EffectKind::Heart); canvas.select(heart); canvas.setMaskTarget(false); canvas.setDrawing(Qt::green,6,true);
    drag(LightingCanvas::Tool::Fill,{320,200},{320,200});
    QCOMPARE(document.state().layers.last().effect,EffectKind::Heart);
    QCOMPARE(document.state().layers.last().color,QColor(Qt::green));
    document.history()->undo(); document.history()->undo();
    const auto radial=document.addLayer(EffectKind::RadialGradient); canvas.select(radial); canvas.setMaskTarget(false);
    drag(LightingCanvas::Tool::Gradient,{200,180},{400,200});
    QCOMPARE(document.state().layers.last().effect,EffectKind::RadialGradient);
    QVERIFY(QLineF(document.state().layers.last().center,QPointF(200,180)).length()<2);
    drag(LightingCanvas::Tool::Gradient,{400,200},{350,380});
    QVERIFY(QLineF(document.state().layers.last().center,QPointF(200,180)).length()<2);
    QVERIFY(document.state().layers.last().rotation>45);
    document.history()->undo(); document.history()->undo(); document.history()->undo();
    canvas.select(document.state().layers.last().id); canvas.setMaskTarget(false);
    drag(LightingCanvas::Tool::Eraser,{320,200},{330,200});
    QCOMPARE(document.state().layers.last().effect,EffectKind::LinearGradient);
    QVERIFY(document.state().layers.last().mask.constScanLine(200)[325]<255);
}

void EditorTest::globalColorControls()
{
    ColorEditor editor; editor.setColor(Qt::red); QSignalSpy colors(&editor,&ColorEditor::colorChanged);
    auto* hex=editor.findChild<QLineEdit*>("rgbHex"); hex->setText("#12AB34"); QMetaObject::invokeMethod(hex,"editingFinished",Qt::DirectConnection);
    QCOMPARE(editor.color(),QColor("#12ab34")); QCOMPARE(colors.count(),1);
    hex->setText("#bad"); QMetaObject::invokeMethod(hex,"editingFinished",Qt::DirectConnection); QCOMPARE(editor.color(),QColor("#12ab34")); QCOMPARE(colors.count(),1);
    LightingWindow window(catalog(),[] { return QVector<ProfileCandidate>{}; });
    for(const auto& name:{"moveEffectTool","pencilTool","lineTool","rectangleTool","ellipseTool","gradientTool","fillTool","eraserTool"}) QVERIFY(window.findChild<QPushButton*>(name));
    QVERIFY(!window.findChild<QToolButton*>("addEffect_circle"));
    window.findChild<QPushButton*>("newLayer")->click();
    QVERIFY(!QApplication::activeModalWidget()); QCOMPARE(window.document().state().layers.last().effect,EffectKind::Paint);
    QCOMPARE(window.document().render().pixelColor(320,200).alpha(),0);
    window.document().history()->undo();
    window.document().addLayer(EffectKind::LinearGradient); auto* stops=window.findChild<ColorListEditor*>("gradientColors");
    QCOMPARE(stops->colors().size(),2);
    for(auto* button:stops->findChildren<QPushButton*>()) if(button->text()=="Add") button->click();
    QCOMPARE(window.document().state().layers[0].gradientColors.size(),3);
    window.document().history()->undo(); QVERIFY(window.document().state().layers[0].gradientColors.isEmpty());
    window.findChild<QPushButton*>("pencilTool")->click();
    const auto revision=window.document().revision(); const auto oldFrame=window.document().render();
    auto* foreground=window.findChild<ColorEditor*>("effectColor")->findChild<QLineEdit*>("rgbHex"); foreground->setText("#22AA66"); QMetaObject::invokeMethod(foreground,"editingFinished",Qt::DirectConnection);
    QCOMPARE(window.document().revision(),revision); QCOMPARE(window.document().render(),oldFrame);

    for(auto* value:window.findChildren<QDoubleSpinBox*>()) QVERIFY2(!value->toolTip().isEmpty(),qPrintable(value->objectName()));
    for(auto* slider:window.findChildren<QSlider*>()) QVERIFY2(!slider->toolTip().isEmpty(),qPrintable(slider->accessibleName()));
    window.document().history()->setClean();
}

void EditorTest::visualWorkspace()
{
    QDir().mkpath(artifactDirectory());
    LightingWindow window(catalog(),[] { return QVector<ProfileCandidate>{}; });
    window.resize(1200,800); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window));
    window.findChild<QAction*>("workspaceEffects")->trigger();
    auto* canvas=window.findChild<LightingCanvas*>("effectsCanvas"); QVERIFY(canvas->isVisible());
    QVERIFY(canvas->width()>=360); QVERIFY(canvas->height()>=280);
    auto* tabs=window.findChild<QTabWidget*>("layerInspectorTabs"); QVERIFY(!tabs->isEnabled());
    const int before=window.document().history()->count();
    QVERIFY(!window.findChild<QPushButton*>("effectPresets"));
    QCOMPARE(window.document().history()->count(),before); QVERIFY(window.document().state().layers.isEmpty());
    addEffect(window,EffectKind::Star); QVERIFY(tabs->isEnabled());
    auto* tree=window.findChild<QTreeWidget*>("effectLayers");
    QCOMPARE(tree->topLevelItem(0)->data(0,Qt::UserRole+1).toString(),QString("Star"));
    QVERIFY(!window.findChild<QDoubleSpinBox*>("effectGeometry0")->isVisible());
    QVERIFY(!window.findChild<QDoubleSpinBox*>("particleSeed")->isVisible());
    const auto old=window.previewFrame(); window.findChild<QSlider*>("layerTransparency")->setValue(80);
    QVERIFY(window.previewFrame()!=old); QVERIFY(!window.findChild<LightingCanvas*>("hardwareLivePreview")->isVisible());
    window.document().history()->undo(); QCOMPARE(window.previewFrame(),old);
    QSignalSpy output(&window,&LightingWindow::outputRequested);
    QVERIFY(!window.findChild<QAction*>("reviewOutput")->isEnabled());
    window.findChild<QAction*>("reviewOutput")->trigger(); QCOMPARE(output.count(),0);
    window.findChild<QAction*>("workspaceHardware")->trigger();
    QCOMPARE(window.findChild<QStackedWidget*>("applicationPages")->currentIndex(),0);
    QCOMPARE(window.findChild<QTabWidget*>("lightingViews")->currentIndex(),2);
    window.findChild<QAction*>("workspaceEffects")->trigger();
    tabs->setCurrentIndex(2);
    window.findChild<QPushButton*>("pencilTool")->click();
    QVERIFY(!window.findChild<QDoubleSpinBox*>("effectGeometry0")->isVisible());
    tabs->setCurrentIndex(0); window.findChild<QPushButton*>("moveEffectTool")->click();
    window.resize(1060,680); QCoreApplication::processEvents(); QVERIFY(canvas->width()>=360);
    const auto center=canvas->viewportPoint({320,200}); QVERIFY(canvas->rect().contains(center.toPoint()));
    QVERIFY(window.grab().save(artifactDirectory()+"/studio-compact.png"));
    window.document().history()->setClean();
}

void EditorTest::channelScopesAndClocks()
{
    const auto profile=channelProfile(); LightingDocument document(catalog(),nullptr,{profile});
    const auto first=document.addComponent("test.strip"), second=document.addComponent("test.strip");
    ChannelAssignment a; a.endpoint="first"; a.profileId=profile.id; a.channelId="Direct"; a.software=true; a.placements={first};
    auto b=a; b.id=QUuid::createUuid(); b.endpoint="second"; b.placements={second}; b.timeScale=2; b.timeOffset=0.25;
    document.assignChannel(a); document.assignChannel(b);
    document.addLayer(); document.addLayer();
    document.edit("Separate channels",[&](LightingState& state) {
        state.layers[0].color=Qt::red; state.layers[0].allChannels=false; state.layers[0].channelTargets={a.id};
        state.layers[1].color=Qt::blue; state.layers[1].allChannels=false; state.layers[1].channelTargets={b.id};
    });
    QCOMPARE(document.renderForChannel(a.id,0).pixelColor(80,80),QColor(Qt::red));
    QCOMPARE(document.renderForChannel(b.id,0).pixelColor(80,80),QColor(Qt::blue));
    const auto sampled=document.sampleChannels(document.render(),0);
    QCOMPARE(sampled[a.id][0],QColor(Qt::red)); QCOMPARE(sampled[b.id][0],QColor(Qt::blue));
    document.edit("Separate preview components",[&](auto& state) { for(auto& placement:state.placements) if(placement.id==second) placement.origin={240,80}; });
    LightingCanvas live(document,LightingCanvas::Context::Layout); live.resize(800,600); live.setPreviewOnly(); live.show();
    QVERIFY(QTest::qWaitForWindowExposed(&live)); live.setPreviewFrame(document.render(),0); const auto pixels=live.grab().toImage();
    QCOMPARE(pixels.pixelColor((live.viewportPoint({80,80})*pixels.devicePixelRatio()).toPoint()),QColor(Qt::red));
    QCOMPARE(pixels.pixelColor((live.viewportPoint({240,80})*pixels.devicePixelRatio()).toPoint()),QColor(Qt::blue));

    QCOMPARE(document.channelTime(b.id,1),2.25); QCOMPARE(document.channelTime(b.id,86400),86400.0);
    document.edit("Pulse",[](LightingState& state) { state.layers[1].pulse=1; });
    QVERIFY(document.renderForChannel(b.id,0).pixelColor(80,80).alpha()<255);
    b.timeScale=0; document.assignChannel(b);
    QCOMPARE(document.renderForChannel(b.id,0),document.renderForChannel(b.id,20));
    document.edit("No hardware target",[](LightingState& state) { state.layers[0].channelTargets.clear(); });
    QCOMPARE(document.renderForChannel(a.id,0).pixelColor(80,80).alpha(),0);
    document.history()->undo(); QCOMPARE(document.renderForChannel(a.id,0).pixelColor(80,80),QColor(Qt::red));
    QVERIFY_EXCEPTION_THROWN(document.edit("Invalid target",[](LightingState& state) {
        state.layers[0].channelTargets={QUuid::createUuid()};
    }),std::invalid_argument);
    auto invalid=a; invalid.timeScale=-1; QVERIFY_EXCEPTION_THROWN(document.assignChannel(invalid),std::invalid_argument);
    QTemporaryDir directory; const auto file=directory.filePath("scopes.lighting.json"); document.save(file);
    LightingDocument loaded({},nullptr,{profile}); loaded.load(file);
    QCOMPARE(loaded.renderForChannel(a.id,0),document.renderForChannel(a.id,0));
    QCOMPARE(loaded.state().layers[0].channelTargets,QVector<QUuid>{a.id});
    QCOMPARE(loaded.state().assignments[1].timeScale,0.0);
    QFile input(file); QVERIFY(input.open(QIODevice::ReadOnly)); auto root=QJsonDocument::fromJson(input.readAll()).object(); input.close();
    root.insert("schemaVersion",8); auto layers=root.value("layers").toArray(); auto assignments=root.value("assignments").toArray();
    for(int i=0;i<layers.size();++i) { auto json=layers[i].toObject(); json.remove("targets");json.remove("parentId");json.remove("container"); json.remove("gradientColors"); json.remove("gradientPositions"); layers[i]=json; }
    for(int i=0;i<assignments.size();++i) { auto json=assignments[i].toObject(); json.remove("timeScale"); json.remove("timeOffset"); assignments[i]=json; }
    root.insert("layers",layers); root.insert("assignments",assignments);
    QVERIFY(input.open(QIODevice::WriteOnly)); input.write(QJsonDocument(root).toJson()); input.close(); loaded.load(file);
    QVERIFY(loaded.state().layers[0].allChannels); QCOMPARE(loaded.state().assignments[1].timeScale,1.0);
    auto native=a; native.software=false; document.assignChannel(native);
    QVERIFY(document.state().layers[0].channelTargets.isEmpty()); QVERIFY(!document.state().layers[0].allChannels);

    LightingWindow window(catalog(),[] { return QVector<ProfileCandidate>{}; },nullptr,{profile});
    const auto instance=window.document().addComponent("test.strip"); auto configured=a; configured.placements={instance};
    window.document().assignChannel(configured); window.document().addLayer();
    auto* tree=window.findChild<QTreeWidget*>("effectLayers"); tree->setCurrentItem(tree->topLevelItem(0),1);
    auto* all=window.findChild<QCheckBox*>("allChannels"); all->setChecked(false);
    auto* targets=window.findChild<QListWidget*>("layerChannelTargets"); QCOMPARE(targets->count(),1);
    targets->item(0)->setCheckState(Qt::Checked);
    QTRY_COMPARE(window.document().state().layers[0].channelTargets,QVector<QUuid>{a.id});
    auto* preview=window.findChild<QComboBox*>("previewChannel"); QCOMPARE(preview->count(),2); preview->setCurrentIndex(1);
    window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));window.findChild<QAction*>("workspaceEffects")->trigger();
    auto* canvas=window.findChild<LightingCanvas*>("effectsCanvas");QTest::mouseClick(canvas,Qt::LeftButton,Qt::NoModifier,canvas->viewportPoint({320,200}).toPoint());QCOMPARE(preview->currentIndex(),1);
    window.document().history()->setClean();
}

void EditorTest::sessionRecovery()
{
    QTemporaryDir directory;
    QSettings settings(directory.filePath("settings.ini"),QSettings::IniFormat);
    const auto recovery=directory.filePath("recovery.lighting.json");
    const auto project=directory.filePath("saved.lighting.json");
    {
        LightingWindow window(catalog(),[] { return QVector<ProfileCandidate>{}; });
        AppSession session(window,settings,recovery);
        window.document().addLayer();
        const auto index=window.document().history()->index();
        QVERIFY(session.writeRecovery()); QVERIFY(QFileInfo::exists(recovery));
        QCOMPARE(window.document().history()->index(),index); QVERIFY(!window.document().history()->isClean());
    }
    {
        LightingWindow window({},[] { return QVector<ProfileCandidate>{}; });
        AppSession session(window,settings,recovery); bool asked=false;
        session.restore([&] { asked=true; return true; }); QVERIFY(asked);
        QCOMPARE(window.document().state().layers.size(),1); QVERIFY(!window.document().history()->isClean());
        QCoreApplication::processEvents(); QVERIFY(QFileInfo::exists(recovery));
        QVERIFY(session.writeRecovery());
        window.document().save(project); window.loadProject(project);
        QCoreApplication::processEvents(); QVERIFY(!QFileInfo::exists(recovery));
        window.close(); QCOMPARE(settings.value("project/path").toString(),project);
    }
    {
        LightingWindow window({},[] { return QVector<ProfileCandidate>{}; });
        AppSession session(window,settings,recovery); bool asked=false;
        session.restore([&] { asked=true; return true; }); QVERIFY(!asked);
        QCOMPARE(window.projectFile(),project); QCOMPARE(window.document().state().layers.size(),1);
        QVERIFY(window.document().history()->isClean());
        window.document().addLayer();
        AppSession cannotWrite(window,settings,directory.path());
        QVERIFY(!cannotWrite.writeRecovery()); QVERIFY(!window.document().history()->isClean());
        window.document().history()->setClean(); window.close();
    }
    QVERIFY(QFile::copy(project,recovery));
    {
        LightingWindow window({},[] { return QVector<ProfileCandidate>{}; });
        AppSession session(window,settings,recovery); session.restore([] { return false; });
        QVERIFY(!QFileInfo::exists(recovery)); QCOMPARE(window.document().state().layers.size(),1);
    }
}

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
    QVERIFY(editor.grab().save(artifactDirectory()+"/editor-channel.png"));
    buttons->button(QDialogButtonBox::Ok)->click(); QCOMPARE(editor.result(),int(QDialog::Accepted));
    window.document().assignChannel(editor.assignment());
    auto* tree=window.findChild<QTreeWidget*>("layoutComponents");
    QCoreApplication::processEvents(); QCOMPARE(tree->topLevelItemCount(),3);
    QCOMPARE(tree->topLevelItem(0)->childCount(),2);
    window.document().history()->undo(); QCOMPARE(tree->topLevelItem(0)->childCount(),0);
    window.document().history()->redo(); QCOMPARE(tree->topLevelItem(0)->childCount(),2);
    tree->setCurrentItem(tree->topLevelItem(0));
    bool modalSeen=false, keptSoftware=false;
    QTimer::singleShot(0,[&] {
        auto* dialog=dynamic_cast<ChannelEditor*>(QApplication::activeModalWidget());
        modalSeen=dialog!=nullptr;
        if(dialog) {
            keptSoftware=dialog->assignment().software;
            dialog->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Ok)->click();
        }
    });
    window.findChild<QPushButton*>("headerSettings")->click();
    QVERIFY(modalSeen); QVERIFY(keptSoftware);
    QCOMPARE(window.document().state().assignments.size(),1);
    window.document().history()->setClean();
    auto native=initial; native.channelId="Native";
    ChannelEditor nativeEditor(window.document(),native);
    QCOMPARE(nativeEditor.findChild<QComboBox*>("channelMode")->count(),1);
    nativeEditor.findChild<QComboBox*>("nativeMode")->setCurrentText("breathing");
    nativeEditor.findChild<ColorListEditor*>("nativeColors")->setColors({QColor("#112233"),QColor("#445566")});
    QCOMPARE(nativeEditor.assignment().native.colors.size(),2);
    nativeEditor.findChild<ColorListEditor*>("nativeColors")->setColors({QColor("#80112233")});
    QVERIFY_EXCEPTION_THROWN(nativeEditor.assignment(),std::invalid_argument);
}

void EditorTest::deviceLedArrangement()
{
    LightingDocument document(catalog());
    const auto definition=catalog().first();
    const auto first=document.addComponent(definition.id), second=document.addComponent(definition.id);
    auto positions=definition.positions; positions[0]+=QPointF(2,3);
    document.setComponentGeometry(first,positions);
    const auto local=document.state().placements[0].componentId;
    QVERIFY(local!=definition.id);
    QCOMPARE(document.catalog()[local].positions,positions);
    QCOMPARE(document.catalog()[local].mapping,definition.mapping);
    QCOMPARE(document.state().placements[1].componentId,definition.id);
    QCOMPARE(document.catalog()[definition.id].positions,definition.positions);
    QImage frame(LightingDocument::canvasSize(),QImage::Format_ARGB32_Premultiplied); frame.fill(Qt::black);
    const auto points=document.componentPoints(document.state().placements[0]);
    for(int row=0;row<points.size();++row) frame.setPixelColor(points[row].toPoint(),QColor(40+row*20,70,110));
    const auto sampled=document.sampleComponents(frame).value(first);
    for(int row=0;row<points.size();++row)
        QCOMPARE(sampled[definition.mapping[row]],QColor(40+row*20,70,110));
    document.history()->undo();
    QCOMPARE(document.state().placements[0].componentId,definition.id);
    QVERIFY(!document.catalog().contains(local));
    document.history()->redo();
    const auto index=document.history()->index();
    QVERIFY_EXCEPTION_THROWN(document.setComponentGeometry(first,{}),std::invalid_argument);
    QVERIFY_EXCEPTION_THROWN(document.setComponentGeometry(QUuid::createUuid(),positions),std::invalid_argument);
    QCOMPARE(document.history()->index(),index);
    QTemporaryDir directory; const auto file=directory.filePath("geometry.json"); document.save(file);
    LightingDocument loaded({}); loaded.load(file);
    QCOMPARE(loaded.catalog()[local].positions,positions);
    QCOMPARE(loaded.state().placements[1].id,second);
    ComponentEditor editor(definition,true,nullptr,true); editor.show();
    QVERIFY(QTest::qWaitForWindowExposed(&editor));
    QVERIFY(!editor.findChild<QSpinBox*>("componentCount")->isEnabled());
    for (const auto& shape:QStringList{"Line","Circle","Ellipse","Rectangle"}) {
        auto* action=editor.findChild<QAction*>("arrange"+shape); QVERIFY(action); action->trigger();
        const auto arranged=editor.component();
        QCOMPARE(arranged.mapping,definition.mapping);
        QCOMPARE(arranged.positions.size(),definition.positions.size());
        for(const auto& point:arranged.positions) QVERIFY(std::isfinite(point.x()) && std::isfinite(point.y()));
    }
    editor.findChild<QPushButton*>("resetLedArrangement")->click();
    QCOMPARE(editor.component().positions,definition.positions);
    LightingWindow window(catalog(),[] { return QVector<ProfileCandidate>{}; });
    const auto id=window.document().addComponent(definition.id);
    window.findChild<LightingCanvas*>("layoutCanvas")->select(id);
    QVERIFY(window.findChild<QPushButton*>("editLayoutLeds")->isEnabled());
    QVERIFY(window.findChild<QPushButton*>("editHardwareLeds")->isEnabled());
    QTimer::singleShot(0,&window,[&window] {
        auto* dialog=window.findChild<QDialog*>();
        QVERIFY(dialog);
        dialog->findChild<QAction*>("arrangeCircle")->trigger();
        dialog->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Ok)->click();
    });
    window.findChild<QPushButton*>("editLayoutLeds")->click();
    QVERIFY(window.document().state().placements[0].componentId!=definition.id);
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

void EditorTest::keyboardLedPresentation()
{
    const auto definition=loadComponentDefinition(QString(TEST_COMPONENT_DIR)+"/peripheral-vulcan.json");
    QCOMPARE(definition.nonPhysicalSlots.size(),39); QVERIFY(!definition.nonPhysicalSlots.contains(0));
    auto legacy=definition.toJson(); legacy.remove("nonPhysicalSlots");
    QCOMPARE(ComponentDefinition::fromJson(legacy).nonPhysicalSlots,definition.nonPhysicalSlots);
    ComponentEditor editor(definition,true,nullptr,true); editor.show();
    QVERIFY(QTest::qWaitForWindowExposed(&editor)); QCoreApplication::processEvents();
    auto* view=editor.findChild<QGraphicsView*>("componentGeometry");
    QVector<QGraphicsEllipseItem*> dots;
    for (auto* item:view->scene()->items()) if (item->type()==QGraphicsEllipseItem::Type) {
        auto* dot=static_cast<QGraphicsEllipseItem*>(item); dots.append(dot);
        QCOMPARE(dot->rect().width(),32.0);
        QCOMPARE(dot->brush().color(),QColor("#243b53"));
        QCOMPARE(dot->childItems().size(),1);
        auto* label=static_cast<QGraphicsSimpleTextItem*>(dot->childItems().first());
        QCOMPARE(label->font().pixelSize(),12);QCOMPARE(label->brush().color(),QColor(Qt::white));
        auto* text=dot->childItems().first();
        QVERIFY(QLineF(text->mapRectToParent(text->boundingRect()).center(),QPointF()).length()<.001);
        QVERIFY(dot->rect().contains(text->mapRectToParent(text->boundingRect())));
    }
    QCOMPARE(dots.size(),105);
    for(int i=0;i<dots.size();++i) for(int j=i+1;j<dots.size();++j)
        QVERIFY(!dots[i]->deviceTransform(view->viewportTransform()).mapRect(dots[i]->boundingRect()).intersects(
            dots[j]->deviceTransform(view->viewportTransform()).mapRect(dots[j]->boundingRect())));
    QCOMPARE(editor.component().positions,definition.positions);
    QCOMPARE(editor.component().mapping,definition.mapping);
    editor.findChild<QAction*>("arrangeCircle")->trigger();
    const auto arranged=editor.component();
    QCOMPARE(arranged.nonPhysicalSlots,definition.nonPhysicalSlots);
    for(int i=0;i<definition.positions.size();++i) if(definition.nonPhysicalSlots.contains(definition.mapping[i]))
        QCOMPARE(arranged.positions[i],definition.positions[i]);
    editor.findChild<QPushButton*>("resetLedArrangement")->click();
    QVERIFY(editor.grab().save(artifactDirectory()+"/vulcan-led-arrangement.png"));
    auto fan=catalog().first(); fan.positions.clear();fan.mapping.clear();
    for(int i=0;i<6;++i) {
        const double angle=i*2*3.141592653589793/6;
        fan.positions.append({std::sin(angle),std::cos(angle)});fan.mapping.append(i);
    }
    // Almost equal axis values in circular layouts must not shrink all markers to subpixels.
    fan.positions[2].rx()+=.000001;
    ComponentEditor fanEditor(fan,true,nullptr,true);fanEditor.show();
    QVERIFY(QTest::qWaitForWindowExposed(&fanEditor));QCoreApplication::processEvents();
    auto* fanView=fanEditor.findChild<QGraphicsView*>("componentGeometry");int visible=0;
    for(auto* item:fanView->scene()->items())if(item->type()==QGraphicsEllipseItem::Type) {
        const auto rect=item->deviceTransform(fanView->viewportTransform()).mapRect(item->boundingRect());
        QVERIFY(rect.width()>=32);QVERIFY(rect.width()<=36);++visible;
    }
    QCOMPARE(visible,6);QCOMPARE(fanEditor.component().positions,fan.positions);
    QVERIFY(fanEditor.grab().save(artifactDirectory()+"/fan-led-arrangement.png"));
}

void EditorTest::componentEditor()
{
    ComponentEditor editor(catalog().first(), true);
    editor.show(); QVERIFY(QTest::qWaitForWindowExposed(&editor));
    auto* table = editor.findChild<QTableWidget*>("componentLeds");
    auto* geometry=editor.findChild<QGraphicsView*>("componentGeometry"); QVERIFY(geometry->isVisible());
    const auto oldPoint=editor.component().positions[0];
    const auto from=geometry->mapFromScene(oldPoint*24),to=from+QPoint(24,12);
    QTest::mousePress(geometry->viewport(),Qt::LeftButton,Qt::NoModifier,from);
    QTest::mouseMove(geometry->viewport(),to);
    QTest::mouseRelease(geometry->viewport(),Qt::LeftButton,Qt::NoModifier,to);
    QVERIFY(editor.component().positions[0]!=oldPoint);

    QVERIFY(editor.findChild<QLineEdit*>("componentId")->isReadOnly());
    table->item(1, 0)->setText("4.25");
    QCOMPARE(editor.component().positions[1], QPointF(4.25, 0));
    table->item(1, 2)->setText("2");
    QVERIFY_EXCEPTION_THROWN(editor.component(), std::invalid_argument);
    auto* buttons = editor.findChild<QDialogButtonBox*>();
    buttons->button(QDialogButtonBox::Ok)->click();
    QVERIFY(editor.grab().save(artifactDirectory() + "/editor-component.png"));
    QVERIFY(!editor.findChild<QLabel*>("componentError")->text().isEmpty());
    QCOMPARE(editor.result(), int(QDialog::Rejected));
    table->item(1, 2)->setText("1");
    editor.findChild<QSpinBox*>("componentCount")->setValue(4);
    QCOMPARE(editor.component().mapping, QVector<int>({2, 1, 0, 3}));
    editor.findChild<QTabWidget*>("componentViews")->setCurrentIndex(1);
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
        state.placements[0].origin = {40.5, 30};
        state.placements[1].origin = {20, 30};
    });
    QImage frame(LightingDocument::canvasSize(), QImage::Format_ARGB32);
    frame.fill(Qt::transparent);
    frame.setPixelColor(40, 30, QColor(200, 100, 50, 128));
    frame.setPixelColor(60, 30, Qt::green);
    frame.setPixelColor(80, 30, Qt::blue);
    const auto sampled = document.sampleComponents(frame);
    QCOMPARE(sampled.size(), 2);
    QCOMPARE(sampled[first][0], QColor(Qt::blue));
    QCOMPARE(sampled[first][1], QColor(Qt::green));
    QCOMPARE(sampled[first][2], QColor(100, 50, 25));
    QCOMPARE(sampled[second][2], QColor(Qt::black));
    QCOMPARE(sampled[second][1], QColor(100, 50, 25));
    QCOMPARE(document.componentPoints(document.state().placements[0])[1], QPointF(60.5, 30));
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
    const QVector<QColor> asymmetric{QColor(192,96,32),QColor(48,60,28),QColor(208,196,228),QColor(96,137,201)};
    for(int i=0;i<modes.size();++i) {
        auto top=layer(QColor(192,96,32)); top.blend=modes[i];
        LightingState pair; pair.layers={layer(QColor(64,160,224)),top};
        const auto pixel=LightingDocument::render(pair).pixelColor(10,10);
        for(int channel=0;channel<3;++channel) {
            const auto channelValue=[channel](QColor color) { return channel==0 ? color.red() : channel==1 ? color.green() : color.blue(); };
            QVERIFY(std::abs(channelValue(pixel)-channelValue(asymmetric[i]))<=1);
        }
        pair.layers[1].transparency=50; pair.layers[1].mask.fill(128);
        const auto partial=LightingDocument::render(pair).pixelColor(10,10);
        QVERIFY(std::abs(partial.red()-qRound(64*.75+asymmetric[i].red()*.25))<=2);
    }
    LightingWindow window(catalog(),[] { return QVector<ProfileCandidate>{}; });
    addEffect(window,EffectKind::StaticColor); addEffect(window,EffectKind::StaticColor);
    window.document().edit("Distinct blend colors",[](auto& state) { state.layers[0].color=QColor(64,160,224); state.layers[1].color=QColor(192,96,32); });
    auto* selector=window.findChild<QComboBox*>("layerBlendMode"); QVERIFY(selector);
    for(int i=0;i<modes.size();++i) {
        selector->setCurrentIndex(i); QCOMPARE(window.document().state().layers[1].blend,modes[i]);
        const auto pixel=window.document().render().pixelColor(10,10);
        QVERIFY(std::abs(pixel.red()-asymmetric[i].red())<=1);
    }
    window.document().history()->setClean();
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
    QCOMPARE(tabs->count(), 3);
    tabs->setCurrentIndex(0);
    QVERIFY(layout->isVisible()); QVERIFY(!effects->isVisible());
    layout->select(placement);
    QTest::keyClick(layout, Qt::Key_Right);
    QCOMPARE(window.document().state().placements[0].origin, QPointF(81, 80));
    QCOMPARE(window.document().state().layers[0].mask.constScanLine(200)[320], uchar(255));
    QDir().mkpath(artifactDirectory());
    QVERIFY(window.grab().save(artifactDirectory() + "/editor-layout.png"));
    tabs->setCurrentIndex(1);
    QVERIFY(!layout->isVisible()); QVERIFY(effects->isVisible());
    auto* tree = window.findChild<QTreeWidget*>("effectLayers");
    auto* item = tree->topLevelItem(0);
    const auto row = tree->visualItemRect(item);
    QTest::mouseClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier,
        QPoint(tree->header()->sectionPosition(2) + 15, row.center().y()));
    QCOMPARE(effects->selection(), effect);
    // Target the asserted raster pixel, rather than assuming a large brush covers the widget midpoint.
    const auto center = effects->viewportPoint({320.5,200.5}).toPoint();
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
    QCOMPARE(slider->value(), 100);
    QVERIFY(window.grab().save(artifactDirectory() + "/editor-effects.png"));
    window.document().history()->setClean();
    window.close();
}

void EditorTest::explicitDiscovery()
{
    int scans=0; bool connected=true; auto profile=channelProfile(); profile.channels[0].maxLeds=64;
    LightingWindow window(catalog(),[&] {
        ++scans; if(!connected) return QVector<ProfileCandidate>{};
        ProfileCandidate candidate; candidate.profileId=profile.id; candidate.device.path="exact-test-path";
        return QVector<ProfileCandidate>{candidate,candidate};
    },nullptr,{profile});
    window.show(); QCoreApplication::processEvents(); QCOMPARE(scans,1);
    auto* tree=window.findChild<QTreeWidget*>("layoutComponents"); QCOMPARE(tree->topLevelItemCount(),3);
    tree->setCurrentItem(tree->topLevelItem(0));
    auto attach=[&](const QString& query,int quantity) {
        bool attached=false;
        QTimer::singleShot(0,[&] {
            auto* dialog=dynamic_cast<HardwarePicker*>(QApplication::activeModalWidget()); QVERIFY(dialog);
            dialog->findChild<QLineEdit*>("hardwareSearch")->setText(query);
            dialog->findChild<QSpinBox*>("hardwareQuantity")->setValue(64);
            auto* ok=dialog->findChild<QPushButton*>("confirmAddHardware"); QVERIFY(!ok->isEnabled());
            dialog->findChild<QSpinBox*>("hardwareQuantity")->setValue(quantity); QVERIFY(ok->isEnabled());
            if(query.contains("fan")) {
                auto* search=dialog->findChild<QLineEdit*>("hardwareSearch"); search->clear(); dialog->findChild<QListWidget*>("hardwareProducts")->setCurrentRow(0); QCoreApplication::processEvents();
                QVERIFY(dialog->grab().save(artifactDirectory()+"/hardware-picker.png")); search->setText(query);
            }
            ok->click(); attached=dialog->result()==QDialog::Accepted;
        });
        window.findChild<QPushButton*>("addHeaderComponents")->click(); QVERIFY(attached);
    };
    attach("Generic fan / 8",2); attach("Generic strip / 8",1);
    QCOMPARE(window.document().state().placements.size(),3); QCOMPARE(window.document().state().assignments.size(),1);
    QCOMPARE(window.document().state().assignments[0].endpoint,QString("exact-test-path"));
    for(const auto& placement:window.document().state().placements)
        for(const auto& point:window.document().componentPoints(placement)) QVERIFY(QRectF(0,0,640,400).contains(point));
    QCOMPARE(tree->topLevelItem(0)->childCount(),3);
    tree->setCurrentItem(tree->topLevelItem(0)->child(2));
    window.findChild<QPushButton*>("removeComponent")->click();
    QCOMPARE(window.document().state().placements.size(),2);
    QCOMPARE(window.document().state().assignments[0].placements.size(),2);
    window.document().history()->undo(); QCOMPARE(tree->topLevelItem(0)->childCount(),3);
    tree->setCurrentItem(tree->topLevelItem(0)->child(0));
    tree->setCurrentItem(tree->topLevelItem(0));
    auto* canvas=window.findChild<LightingCanvas*>("layoutCanvas");
    const auto old=window.document().state().placements; QTest::keyClick(canvas,Qt::Key_Right);
    for(int i=0;i<3;++i) QCOMPARE(window.document().state().placements[i].origin,old[i].origin+QPointF(1,0));
    window.document().history()->undo();
    const auto start=canvas->viewportPoint(window.document().componentPoints(old[0])[0]).toPoint();
    const auto end=start+QPoint(20,15);
    QTest::mousePress(canvas,Qt::LeftButton,Qt::NoModifier,start);
    QTest::mouseMove(canvas,end); QTest::mouseRelease(canvas,Qt::LeftButton,Qt::NoModifier,end);
    const auto moved=window.document().state().placements;
    const auto delta=moved[0].origin-old[0].origin; QVERIFY(!delta.isNull());
    QCOMPARE(canvas->selection(),old[0].id);
    for(int i=1;i<3;++i) QCOMPARE(moved[i].origin,old[i].origin);
    window.document().history()->undo(); window.document().history()->undo(); window.document().history()->undo();
    QVERIFY(window.document().state().placements.isEmpty()); QVERIFY(window.document().state().assignments.isEmpty());
    window.document().history()->redo(); window.document().history()->redo(); QCOMPARE(tree->topLevelItem(0)->childCount(),3);
    QCoreApplication::processEvents();
    QVERIFY(window.grab().save(artifactDirectory()+"/header-layout.png"));
    window.resize(1440,900);
    addEffect(window,EffectKind::LinearGradient);
    window.document().edit("Studio preview",[](auto& state) {
        auto& gradient=state.layers.last(); gradient.name="Warm to cool"; gradient.gradientColors={QColor("#ff665c"),QColor("#ffd16b"),QColor("#6397ff")}; gradient.gradientPositions={0,.35,1}; gradient.driftX=25;
    });
    window.findChild<QPushButton*>("newLayer")->click();
    auto* effects=window.findChild<LightingCanvas*>("effectsCanvas"); window.findChild<QPushButton*>("ellipseTool")->click();
    const auto a=effects->viewportPoint({280,130}).toPoint(), b=effects->viewportPoint({410,260}).toPoint();
    QTest::mousePress(effects,Qt::LeftButton,Qt::NoModifier,a); QTest::mouseMove(effects,b); QTest::mouseRelease(effects,Qt::LeftButton,Qt::NoModifier,b);
    auto* layerTree=window.findChild<QTreeWidget*>("effectLayers"); layerTree->setCurrentItem(layerTree->topLevelItem(1));
    window.findChild<QPushButton*>("gradientTool")->click();
    auto* stops=window.findChild<ColorListEditor*>("gradientColors");
    auto* bar=stops->findChild<QWidget*>("colorStopBar");
    const auto position=stops->positions()[1]; const auto from=QPoint(qRound(12+position*(bar->width()-24)),47), to=QPoint(qRound(12+.45*(bar->width()-24)),47);
    const int history=window.document().history()->count();
    QTest::mousePress(bar,Qt::LeftButton,Qt::NoModifier,from); QTest::mouseMove(bar,to); QTest::mouseRelease(bar,Qt::LeftButton,Qt::NoModifier,to);
    QCOMPARE(window.document().history()->count(),history+1); QVERIFY(std::abs(window.document().state().layers[0].gradientPositions[1]-.45)<.02);
    window.document().history()->undo(); QVERIFY(std::abs(window.document().state().layers[0].gradientPositions[1]-.35)<.001);
    auto* play=window.findChild<QPushButton*>("playPreview"); play->setChecked(true);
    QTRY_VERIFY(window.previewSeconds()>0); QCoreApplication::processEvents();
    QVERIFY(window.findChild<LightingCanvas*>("hardwareLivePreview")->isVisible());
    QVERIFY(window.grab().save(artifactDirectory()+"/studio-workspace.png"));
    play->setChecked(false);
    connected=false;
    window.findChild<QPushButton*>("refreshHeaders")->click(); QCOMPARE(scans,2);
    QCOMPARE(tree->topLevelItemCount(),1); QCOMPARE(tree->topLevelItem(0)->childCount(),3);
    QVERIFY(tree->topLevelItem(0)->text(0).contains("offline"));
    window.document().history()->setClean();
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
    addEffect(window);
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
        if (definition.kind < EffectKind::Circle || (definition.kind == EffectKind::Image || definition.kind == EffectKind::Paint) || isParticle(definition.kind)) continue;
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
    for (const auto& definition : effectDefinitions()) if(definition.kind!=EffectKind::Paint && definition.kind!=EffectKind::Rectangle) document.addLayer(definition.kind);
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
    QCOMPARE(root.value("schemaVersion").toInt(), 13);
    // Published schema 2 has explicit effects but no image fields or Image type.
    auto version2 = root; version2.remove("components"); version2.remove("assignments"); QJsonArray version2Layers;
    for (const auto& item : root.value("layers").toArray()) {
        auto value = item.toObject();
        if (value.value("effect").toObject().value("type").toString() == "image" || isParticle(effectDefinition(value.value("effect").toObject().value("type").toString()).kind)) continue;
        if(value.value("effect").toObject().value("type").toString()=="rectangle")continue;
        value.remove("targets");value.remove("parentId");value.remove("container"); value.remove("gradientColors"); value.remove("gradientPositions"); value.remove("advanced"); value.remove("animation"); value.remove("imagePng"); value.remove("imageFill"); version2Layers.append(value);
    }
    version2.insert("schemaVersion", 2); version2.insert("layers", version2Layers);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write(QJsonDocument(version2).toJson()); file.close();
    loaded.load(path); QCOMPARE(loaded.render(), document.render());
    auto version3 = root; version3.remove("components"); version3.remove("assignments"); QJsonArray version3Layers;
    for (const auto& item : root.value("layers").toArray()) {
        auto value = item.toObject(); if (isParticle(effectDefinition(value.value("effect").toObject().value("type").toString()).kind)) continue;
        if(value.value("effect").toObject().value("type").toString()=="rectangle")continue;
        value.remove("targets");value.remove("parentId");value.remove("container"); value.remove("gradientColors"); value.remove("gradientPositions"); value.remove("advanced"); value.remove("animation"); version3Layers.append(value);
    }
    version3.insert("schemaVersion", 3); version3.insert("layers", version3Layers);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write(QJsonDocument(version3).toJson()); file.close(); loaded.load(path);
    QCOMPARE(loaded.render(), document.render());
    auto version4=root; version4.remove("components"); version4.remove("assignments"); QJsonArray version4Layers;
    for (const auto& item : root.value("layers").toArray()) {
        auto value=item.toObject(); if (isParticle(effectDefinition(value.value("effect").toObject().value("type").toString()).kind)) continue;
        if(value.value("effect").toObject().value("type").toString()=="rectangle")continue;
        value.remove("targets");value.remove("parentId");value.remove("container"); value.remove("gradientColors"); value.remove("gradientPositions"); value.remove("advanced"); version4Layers.append(value);
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
    auto legacy = layers[0].toObject(); legacy.remove("targets");legacy.remove("parentId");legacy.remove("container"); legacy.remove("gradientColors"); legacy.remove("gradientPositions"); legacy.remove("advanced"); legacy.remove("animation"); legacy.remove("effect"); legacy.remove("imagePng"); legacy.remove("imageFill");
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
    addEffect(window,EffectKind::Star);
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
    addEffect(window,EffectKind::Star);
    window.document().edit("Preview shape", [](LightingState& state) {
        state.layers[1].color = QColor("#ff409e"); state.layers[1].rotation = 20;
        state.layers[1].transparency = 30;
    });
    window.findChild<QTabWidget*>("lightingViews")->setCurrentIndex(1);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QVERIFY(window.grab().save(artifactDirectory() + "/editor-gradient-shape.png"));
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
    QVERIFY(window.grab().save(artifactDirectory() + "/editor-image.png"));
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
    window.findChild<QPushButton*>("playPreview")->setChecked(false);
    auto* time = window.findChild<QDoubleSpinBox*>("previewTime"); time->setValue(0.5);
    auto* play = window.findChild<QPushButton*>("playPreview"); play->setChecked(true);
    QTRY_VERIFY(time->value() > 0.5); play->setChecked(false);
    QCOMPARE(window.document().history()->count(), count); QVERIFY(window.document().history()->isClean());
    play->setChecked(true);
    window.findChild<QTabWidget*>("layerInspectorTabs")->setCurrentIndex(2);
    QVERIFY(play->isChecked()); // Mask inspection preserves playback under the revised interaction contract.
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

void EditorTest::expandingLayoutAndFullscreen()
{
    LightingWindow window(catalog(),[] { return QVector<ProfileCandidate>{}; },nullptr,{channelProfile()});
    const auto id=window.document().addComponent("test.strip");
    window.document().edit("Expanded layout",[](auto& state){state.placements[0].origin={-200,500};});
    // Independently calculated: LED centers -200,-180,-160 at y=500, with 12px margin.
    QCOMPARE(window.document().layoutBounds(),QRectF(-212,0,852,512));
    const QRectF bounds(-212,0,852,512);
    QImage frame(LightingDocument::canvasSize(),QImage::Format_ARGB32);frame.fill(Qt::black);
    frame.setPixelColor(9,390,Qt::red);frame.setPixelColor(24,390,Qt::green);frame.setPixelColor(39,390,Qt::blue);
    const QVector<QColor> expected{Qt::blue,Qt::green,Qt::red}; // Wire order 2,1,0.
    QCOMPARE(window.document().sampleComponents(frame)[id],expected);
    ChannelAssignment assignment;assignment.endpoint="test";assignment.profileId="test.device";assignment.channelId="Direct";assignment.software=true;assignment.placements={id};
    window.document().assignChannel(assignment);
    QCOMPARE(window.document().sampleChannels(frame)[assignment.id],expected);
    auto preview=window.document().state().placements;preview[0].origin={900,-100};
    QCOMPARE(window.document().layoutBounds(&preview),QRectF(0,-112,952,512));
    QImage draftFrame(frame.size(),frame.format());draftFrame.fill(Qt::black);
    draftFrame.setPixelColor(605,9,Qt::red);draftFrame.setPixelColor(618,9,Qt::green);draftFrame.setPixelColor(631,9,Qt::blue);
    QCOMPARE(window.document().sampleChannels(draftFrame,0,&preview)[assignment.id],expected);
    QImage white(frame.size(),frame.format());white.fill(Qt::white);
    const auto previewColors=window.document().sampleComponents(white,&preview).value(id);
    for(const auto& color:previewColors)QCOMPARE(color,QColor(Qt::white));
    QCOMPARE(window.document().state().placements[0].origin,QPointF(-200,500));
    QTemporaryDir directory;const auto file=directory.filePath("layout.json");window.document().save(file);
    LightingDocument restored(catalog(),nullptr,{channelProfile()});restored.load(file);QCOMPARE(restored.layoutBounds(),bounds);
    window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto* play=window.findChild<QPushButton*>("playPreview");play->setChecked(false);QCOMPARE(play->text(),QString("Play preview"));
    QTest::mouseClick(play,Qt::LeftButton);QCOMPARE(play->text(),QString("Pause preview"));play->setChecked(false);
    QVERIFY(window.findChild<QLabel*>("toolbarOutputStatus")->isVisible());
    auto* canvas=window.findChild<LightingCanvas*>("layoutCanvas");canvas->fitToView();
    auto* originalParent=canvas->parentWidget();auto* fullscreen=canvas->findChild<QPushButton*>("canvasFullscreenButton");
    QTest::mouseClick(fullscreen,Qt::LeftButton);
    auto* host=window.findChild<QDialog*>("canvasFullscreen");QVERIFY(host);QTRY_VERIFY(host->isFullScreen());
    QCOMPARE(canvas->window(),host);QVERIFY(canvas->isVisible());
    QVERIFY(host->grab().save(artifactDirectory()+"/layout-fullscreen.png"));
    auto* fullPlay=host->findChild<QPushButton*>("fullscreenCanvasPlayback");QVERIFY(fullPlay);QTest::mouseClick(fullPlay,Qt::LeftButton);QVERIFY(play->isChecked());play->setChecked(false);
    auto* fullOutput=host->findChild<QLabel*>("fullscreenOutputStatus");QVERIFY(fullOutput);QVERIFY(fullOutput->isVisible());
    window.setOutputState("Running");QCOMPARE(fullOutput->text(),QString("Devices: Running"));
    canvas->zoomBy(4);canvas->repaint();QVERIFY(host->grab().save(artifactDirectory()+"/layout-fullscreen-overflow.png"));
    QTest::keyClick(canvas,Qt::Key_Escape);QTRY_COMPARE(canvas->parentWidget(),originalParent);QVERIFY(canvas->isVisible());
    canvas->zoomBy(4);canvas->repaint(); // Navigation is available only when the fitted content overflows.
    const auto before=canvas->canvasPoint(QPointF(canvas->width()/2.,canvas->height()/2.));
    QTest::mouseClick(canvas,Qt::LeftButton,Qt::NoModifier,QPoint(canvas->width()-176,canvas->height()-110));
    QVERIFY(QLineF(before,canvas->canvasPoint(QPointF(canvas->width()/2.,canvas->height()/2.))).length()>10);
    QVERIFY(window.grab().save(artifactDirectory()+"/layout-expanding.png"));
    window.document().history()->setClean();
}

void EditorTest::canvasScrollbars()
{
    LightingWindow window(catalog(),[] { return QVector<ProfileCandidate>{}; });
    window.document().addComponent("test.strip");window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto* canvas=window.findChild<LightingCanvas*>("layoutCanvas");canvas->fitToView();
    auto* horizontal=canvas->findChild<QScrollBar*>("canvasHorizontalScroll");
    auto* vertical=canvas->findChild<QScrollBar*>("canvasVerticalScroll");
    canvas->repaint();QVERIFY(!horizontal->isVisible());QVERIFY(!vertical->isVisible());
    window.document().edit("Move outside canvas",[](auto& state){state.placements[0].origin={1800,-1200};});
    canvas->repaint();QVERIFY(horizontal->isVisible());QVERIFY(vertical->isVisible());
    horizontal->setValue(horizontal->maximum());vertical->setValue(vertical->maximum());canvas->repaint();
    // Navigation includes the model label (abbreviated when necessary) as well as the LED world edge.
    // The body starts at 1790; text needs its measured width and 12px label padding.
    auto captionFont=canvas->font();captionFont.setPointSizeF(std::max(7.,captionFont.pointSizeF()*.75));const QFontMetrics captionMetrics(captionFont);
    const auto abbreviated=captionMetrics.elidedText("Generic Test strip",Qt::ElideRight,100);
    const double rightEdge=std::max(1852.,1790.+captionMetrics.horizontalAdvance(abbreviated)+8.);
    QVERIFY(qAbs(canvas->viewportPoint({rightEdge,400}).x()-(canvas->width()-16-vertical->width()))<=1.0);
    canvas->select(window.document().state().placements[0].id);canvas->repaint();horizontal->setValue(horizontal->maximum());canvas->repaint();
    const double fullRightEdge=rightEdge; // Selection never changes the caption size or anchor.
    QVERIFY(qAbs(canvas->viewportPoint({fullRightEdge,400}).x()-(canvas->width()-16-vertical->width()))<=1.0);
    canvas->select({});canvas->repaint();horizontal->setValue(horizontal->maximum());canvas->repaint();
    QVERIFY2(qAbs(canvas->viewportPoint({1852,400}).y()-(canvas->height()-16-horizontal->height()))<=1.0,
        qPrintable(QString("bottom=%1 viewport=%2 hbar=%3 v=%4/%5 units/world=%6")
        .arg(canvas->viewportPoint({1852,400}).y()).arg(canvas->height()).arg(horizontal->height()).arg(vertical->value()).arg(vertical->maximum()).arg(window.document().layoutBounds().height())));
    horizontal->setValue(horizontal->minimum());vertical->setValue(vertical->minimum());canvas->repaint();
    QVERIFY(qAbs(canvas->viewportPoint({0,-1212}).x()-16)<=1.0);QVERIFY(qAbs(canvas->viewportPoint({0,-1212}).y()-48)<=1.0);
    vertical->setValue(vertical->maximum()/2);canvas->repaint();
    const auto old=canvas->canvasPoint(QPointF(100,100));
    horizontal->setValue(horizontal->maximum());vertical->setValue(vertical->minimum());canvas->repaint();
    const auto moved=canvas->canvasPoint(QPointF(100,100));QVERIFY(moved.x()>old.x());QVERIFY(moved.y()<old.y());
    const auto bounds=window.document().layoutBounds();
    QVERIFY(qAbs(canvas->canvasPoint(QPointF(16,48)).y()-bounds.top())<3);
    // Minimap lives above/left of the scrollbar intersection, in the lower-right corner.
    const QPoint mapPoint(canvas->width()-176-vertical->width(),canvas->height()-110-horizontal->height());
    const auto before=canvas->canvasPoint(QPointF(100,100));QTest::mouseClick(canvas,Qt::LeftButton,Qt::NoModifier,mapPoint);canvas->repaint();
    QVERIFY(QLineF(before,canvas->canvasPoint(QPointF(100,100))).length()>10);
    const int value=horizontal->value();QVERIFY(value<horizontal->maximum());
    QVERIFY(window.grab().save(artifactDirectory()+"/layout-navigation-overflow.png"));
    window.document().edit("Move back",[](auto& state){state.placements[0].origin={80,80};});
    canvas->fitToView();canvas->repaint();QVERIFY(!horizontal->isVisible());QVERIFY(!vertical->isVisible());
    QVERIFY(window.grab().save(artifactDirectory()+"/layout-scrollbars.png"));
    window.document().history()->setClean();
}

void EditorTest::invalidCanvasZoom_data()
{
    QTest::addColumn<double>("factor");
    QTest::newRow("zero")<<0.;QTest::newRow("negative")<<-1.;
    QTest::newRow("nan")<<std::numeric_limits<double>::quiet_NaN();
    QTest::newRow("positive-infinity")<<std::numeric_limits<double>::infinity();
    QTest::newRow("negative-infinity")<<-std::numeric_limits<double>::infinity();
}
void EditorTest::invalidCanvasZoom()
{
    QFETCH(double,factor);LightingDocument document(catalog());LightingCanvas canvas(document,LightingCanvas::Context::Layout);
    const auto before=canvas.viewportPoint({100,100});canvas.zoomBy(factor);
    QCOMPARE(canvas.viewportPoint({100,100}),before); // Invalid input must leave a usable view unchanged.
}
void EditorTest::canvasZoomLimits()
{
    for(const auto pair:QVector<QPair<double,double>>{{.0001,.001},{.001,.001},{1,1},{4,4},{5,4}}) {
        LightingDocument document(catalog());LightingCanvas canvas(document,LightingCanvas::Context::Layout);canvas.zoomBy(pair.first);
        QVERIFY(qAbs(canvas.viewportPoint({100,0}).x()-canvas.viewportPoint({0,0}).x()-pair.second*100)<1e-8);
    }
}
void EditorTest::invalidLayoutGeometry()
{
    auto components=catalog();components["test.strip"].positions[0].setX(std::numeric_limits<double>::max());
    LightingDocument overflow(components);const int history=overflow.history()->index();
    QVERIFY_EXCEPTION_THROWN(overflow.addComponent("test.strip"),std::invalid_argument);
    QCOMPARE(overflow.history()->index(),history);QVERIFY(overflow.state().placements.isEmpty());
}
void EditorTest::invalidDraftSampling_data()
{
    QTest::addColumn<double>("value");
    QTest::newRow("nan")<<std::numeric_limits<double>::quiet_NaN();
    QTest::newRow("infinity")<<std::numeric_limits<double>::infinity();
    QTest::newRow("negative-infinity")<<-std::numeric_limits<double>::infinity();
    QTest::newRow("overflow")<<std::numeric_limits<double>::max();
}
void EditorTest::invalidDraftSampling()
{
    QFETCH(double,value);LightingDocument document(catalog());document.addComponent("test.strip");
    const auto before=document.state().placements;const int history=document.history()->index();
    auto draft=before;draft[0].origin.setX(value);
    QImage frame(640,400,QImage::Format_ARGB32);frame.fill(Qt::white);
    QVERIFY_EXCEPTION_THROWN(document.sampleComponents(frame,&draft),std::invalid_argument);
    QCOMPARE(document.state().placements[0].origin,before[0].origin);QCOMPARE(document.history()->index(),history);
}
void EditorTest::scrollbarMouseInteraction()
{
    LightingWindow window(catalog(),[] { return QVector<ProfileCandidate>{}; });window.document().addComponent("test.strip");
    window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto* canvas=window.findChild<LightingCanvas*>("layoutCanvas");canvas->fitToView();
    window.document().edit("Overflow on both axes",[](auto& state){state.placements[0].origin={1800,-1200};});canvas->repaint();
    for(auto* bar:canvas->findChildren<QScrollBar*>()) {
        QVERIFY(bar->isVisible());QVERIFY(bar->isEnabled());
        QCOMPARE(canvas->childAt(bar->geometry().center()),static_cast<QWidget*>(bar));
        bar->setValue(bar->maximum()/2);canvas->repaint();
        QStyleOptionSlider option;option.initFrom(bar);option.orientation=bar->orientation();
        option.minimum=bar->minimum();option.maximum=bar->maximum();option.sliderPosition=bar->sliderPosition();option.sliderValue=bar->value();option.pageStep=bar->pageStep();
        const auto thumb=bar->style()->subControlRect(QStyle::CC_ScrollBar,&option,QStyle::SC_ScrollBarSlider,bar);
        QVERIFY(thumb.isValid());const int before=bar->value();const auto view=canvas->canvasPoint(QPointF(100,100));
        const QPoint from=thumb.center(),to=from+(bar->orientation()==Qt::Horizontal ? QPoint(40,0) : QPoint(0,40));
        QTest::mousePress(bar,Qt::LeftButton,Qt::NoModifier,from);QTest::mouseMove(bar,to);QTest::mouseRelease(bar,Qt::LeftButton,Qt::NoModifier,to);canvas->repaint();
        QVERIFY2(bar->value()>before,"Dragging the native scrollbar thumb must move it, not only setValue()");
        const auto after=canvas->canvasPoint(QPointF(100,100));
        QVERIFY(bar->orientation()==Qt::Horizontal ? after.x()>view.x() : after.y()>view.y());
    }
    QVERIFY(window.grab().save(artifactDirectory()+"/scrollbar-mouse-audit.png"));
    window.document().history()->setClean();
}

void EditorTest::scrollbarExactFit_data()
{
    QTest::addColumn<QSize>("size");QTest::addColumn<bool>("horizontal");QTest::addColumn<bool>("vertical");
    QTest::newRow("exact")<<QSize(672,464)<<false<<false;
    QTest::newRow("one-pixel-spare")<<QSize(673,465)<<false<<false;
    QTest::newRow("horizontal-overflow-causes-vertical")<<QSize(671,464)<<true<<true;
    QTest::newRow("vertical-overflow-causes-horizontal")<<QSize(672,463)<<true<<true;
    QTest::newRow("horizontal-only")<<QSize(639,1000)<<true<<false;
    QTest::newRow("vertical-only")<<QSize(1000,399)<<false<<true;
}
void EditorTest::scrollbarExactFit()
{
    QFETCH(QSize,size);QFETCH(bool,horizontal);QFETCH(bool,vertical);
    LightingDocument document(catalog());document.addComponent("test.strip");
    LightingCanvas canvas(document,LightingCanvas::Context::Layout);canvas.resize(size);canvas.zoomBy(1);
    canvas.show();QVERIFY(QTest::qWaitForWindowExposed(&canvas));canvas.repaint();
    QCOMPARE(canvas.findChild<QScrollBar*>("canvasHorizontalScroll")->isVisible(),horizontal);
    QCOMPARE(canvas.findChild<QScrollBar*>("canvasVerticalScroll")->isVisible(),vertical);
    document.history()->setClean();
}

void EditorTest::triangleAfterCrop()
{
    LightingWindow window(catalog(),[] { return QVector<ProfileCandidate>{}; });
    window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window));
    window.findChild<QAction*>("workspaceEffects")->trigger();
    QVERIFY(!window.findChild<QDoubleSpinBox*>("previewTime")->isVisible());
    window.findChild<QCheckBox*>("cropTool")->setChecked(true);
    addEffect(window,EffectKind::Triangle);
    QVERIFY(!window.findChild<QCheckBox*>("cropTool")->isChecked());
    auto* canvas=window.findChild<LightingCanvas*>("effectsCanvas");
    const auto drag=[&](QPointF from,QPointF to) {
        QTest::mousePress(canvas,Qt::LeftButton,Qt::NoModifier,canvas->viewportPoint(from).toPoint());
        QTest::mouseMove(canvas,canvas->viewportPoint(to).toPoint());
        QTest::mouseRelease(canvas,Qt::LeftButton,Qt::NoModifier,canvas->viewportPoint(to).toPoint());
    };
    drag({320,200},{350,220});
    QVERIFY(QLineF(window.document().state().layers.last().center,QPointF(350,220)).length()<2);
    const auto before=window.document().state().layers.last();
    const auto corner=effectTransform(before).map(QPointF(.5,.5));drag(corner,corner+QPointF(30,30));
    QVERIFY(window.document().state().layers.last().extent.width()>before.extent.width());
    window.document().history()->undo();QCOMPARE(window.document().state().layers.last().extent,before.extent);
    window.document().edit("Animated triangle",[](auto& state){auto& shape=state.layers.last();shape.driftX=180;shape.spin=20;shape.zoom=10;});
    window.findChild<QDoubleSpinBox*>("previewTime")->setValue(1.0);
    const auto animated=window.document().state().layers.last();
    const auto shown=effectTransform(animated,1).map(QPointF());
    drag(shown,shown+QPointF(-20,-20));
    QVERIFY(QLineF(window.document().state().layers.last().center,animated.center-QPointF(20,20)).length()<2);
    QCOMPARE(window.findChild<QDoubleSpinBox*>("previewTime")->value(),1.0);
    window.document().history()->setClean();
}

void EditorTest::canvasTransformsAndCrop()
{
    LightingWindow window(catalog(),[] { return QVector<ProfileCandidate>{}; });
    window.document().addLayer(EffectKind::Circle);
    auto* tree=window.findChild<QTreeWidget*>("effectLayers"); tree->setCurrentItem(tree->topLevelItem(0));
    window.findChild<QTabWidget*>("lightingViews")->setCurrentIndex(1);
    window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto* canvas=window.findChild<LightingCanvas*>("effectsCanvas");
    const auto center=canvas->viewportPoint({320,200}).toPoint();
    const auto count=window.document().history()->count();
    QTest::mousePress(canvas,Qt::LeftButton,Qt::NoModifier,center);
    QTest::mouseMove(canvas,canvas->viewportPoint({350,210}).toPoint()); QTest::mouseRelease(canvas,Qt::LeftButton,Qt::NoModifier,canvas->viewportPoint({350,210}).toPoint());
    QVERIFY(QLineF(window.document().state().layers[0].center,QPointF(350,210)).length()<1.5);
    QCOMPARE(window.document().history()->count(),count+1); window.document().history()->undo();
    QTest::mousePress(canvas,Qt::LeftButton,Qt::NoModifier,center);
    QTest::mouseMove(canvas,center+QPoint(50,0)); QTest::keyClick(canvas,Qt::Key_Escape);
    QTest::mouseRelease(canvas,Qt::LeftButton,Qt::NoModifier,center+QPoint(50,0));
    QCOMPARE(window.document().state().layers[0].center,QPointF(320,200));
    const auto rotate=canvas->viewportPoint({320,56}).toPoint();
    QTest::mousePress(canvas,Qt::LeftButton,Qt::NoModifier,rotate);
    QTest::mouseMove(canvas,canvas->viewportPoint({464,200}).toPoint()); QTest::mouseRelease(canvas,Qt::LeftButton,Qt::NoModifier,canvas->viewportPoint({464,200}).toPoint());
    QVERIFY(std::abs(window.document().state().layers[0].rotation-90)<1); window.document().history()->undo();
    const auto corner=canvas->viewportPoint({440,320}).toPoint();
    QTest::mousePress(canvas,Qt::LeftButton,Qt::NoModifier,corner);
    QTest::mouseMove(canvas,corner+QPoint(30,30)); QTest::mouseRelease(canvas,Qt::LeftButton,Qt::NoModifier,corner+QPoint(30,30));
    QVERIFY(window.document().state().layers[0].extent.width()>290); window.document().history()->undo();
    QTemporaryDir dir; const auto imagePath=dir.filePath("crop.png");
    QImage pixels(200,100,QImage::Format_ARGB32); pixels.fill(Qt::red);
    for (int y=0;y<100;++y) for (int x=100;x<200;++x) pixels.setPixelColor(x,y,Qt::blue);
    QVERIFY(pixels.save(imagePath)); window.document().importImage(imagePath);
    tree->setCurrentItem(tree->topLevelItem(0));
    window.findChild<QCheckBox*>("cropTool")->setChecked(true);
    QTest::mousePress(canvas,Qt::LeftButton,Qt::NoModifier,canvas->viewportPoint({325,120}).toPoint());
    QTest::mouseMove(canvas,canvas->viewportPoint({520,280}).toPoint());
    QTest::mouseRelease(canvas,Qt::LeftButton,Qt::NoModifier,canvas->viewportPoint({520,280}).toPoint());
    QVERIFY(window.document().state().layers.last().crop.width()<0.5);
    QCOMPARE(window.document().render().pixelColor(320,200),QColor(Qt::blue));
    window.document().history()->undo(); QCOMPARE(window.document().state().layers.last().crop,QRectF(0,0,1,1));
    auto* cropWidth=window.findChild<QDoubleSpinBox*>("imageCrop2");
    cropWidth->setValue(0.5); auto* cropLeft=window.findChild<QDoubleSpinBox*>("imageCrop0"); cropLeft->setValue(0.8);
    QCOMPARE(window.document().state().layers.last().crop.x(),0.0); // Rejected without escaping a Qt slot.
    QCOMPARE(window.findChild<ColorListEditor*>("colorCycle")->colors().size(),3);
    window.document().history()->setClean();
    QVERIFY(window.grab().save(artifactDirectory()+"/editor-transform.png"));
}

void EditorTest::advancedControls()
{
    LightingWindow window(catalog(),[] { return QVector<ProfileCandidate>{}; });
    addEffect(window,EffectKind::Orbit);
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
    QVERIFY(window.grab().save(artifactDirectory()+"/editor-particles.png"));
    window.findChild<QTabWidget*>("layerInspectorTabs")->setCurrentIndex(1); QCoreApplication::processEvents();
    for(const auto& id:{"particleCount","particleSpeed","particleSize"}) {
        auto* control=window.findChild<QDoubleSpinBox*>(id);QVERIFY(!control->isVisible());window.findChild<QTabWidget*>("layerInspectorTabs")->setCurrentIndex(0);QCoreApplication::processEvents();QVERIFY(control->isVisible());
        if(QGuiApplication::platformName()!="offscreen") QVERIFY2(!control->visibleRegion().isEmpty(),id);
        window.findChild<QTabWidget*>("layerInspectorTabs")->setCurrentIndex(1);QCoreApplication::processEvents();
    }
    QVERIFY(!window.findChild<QDoubleSpinBox*>("particleSeed")->isVisible());
    QVERIFY(window.grab().save(artifactDirectory()+"/editor-motion.png"));
    window.document().history()->setClean();
}

void EditorTest::snapGridBoundaries()
{
    const SnapGrid grid{true,10};
    QCOMPARE(grid.snap(QPointF(14.99,-14.99)),QPointF(10,-10));
    QCOMPARE(grid.snap(QPointF(15,-15)),QPointF(20,-20));
    QCOMPARE(grid.snap(QPointF(0,-0.01)),QPointF(0,0));
    QCOMPARE((SnapGrid{false,10}.snap(13.25)),13.25);
    QCOMPARE((SnapGrid{true,.1}.snap(std::numeric_limits<double>::max())),std::numeric_limits<double>::max());
    for(double spacing:{0.,-1.,std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()})
        QCOMPARE((SnapGrid{true,spacing}.snap(13.25)),13.25);
    QVERIFY(std::isnan(grid.snap(std::numeric_limits<double>::quiet_NaN())));
    QSettings settings;
    for(const QVariant value:{QVariant("garbage"),QVariant(-1),QVariant(0),QVariant(101),QVariant(std::numeric_limits<double>::infinity())}) {
        settings.setValue("editor/gridSpacing",value);QCOMPARE(SnapGrid::load().spacing,10.);
    }
    for(int value:{1,100}) { settings.setValue("editor/gridSpacing",value);QCOMPARE(SnapGrid::load().spacing,double(value)); }
    QImage image(100,100,QImage::Format_ARGB32);image.fill(Qt::black);QPainter painter(&image);
    grid.paint(painter,QRectF(1e100,1e100,1e90,1e90),.001); // Huge coordinates must not hang the grid painter.
}

void EditorTest::snapLayoutGroup()
{
    SnapGrid{true,10}.save();
    LightingDocument document(catalog(),nullptr,{channelProfile()});
    const auto first=document.addComponent("test.strip"),second=document.addComponent("test.strip");
    document.edit("Fixture",[&](auto& state){state.placements[0].origin={103,103};state.placements[1].origin={137,156};});
    ChannelAssignment group;group.id=QUuid::createUuid();group.channelId="Direct";group.profileId="test.device";group.endpoint="test";
    group.placements={first,second};group.software=true;document.assignChannel(group);
    LightingCanvas canvas(document,LightingCanvas::Context::Layout);canvas.resize(900,600);canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));canvas.fitToView();canvas.select(group.id);
    const auto before=document.history()->count();
    // A selected group's empty interior starts a group move without selecting a member.
    const auto from=canvas.viewportPoint({123,140}).toPoint();
    const auto to=canvas.viewportPoint({150,162}).toPoint();
    QTest::mousePress(&canvas,Qt::LeftButton,Qt::NoModifier,from);
    QTest::mouseMove(&canvas,to);QTest::mouseRelease(&canvas,Qt::LeftButton,Qt::NoModifier,to);
    QCOMPARE(document.state().placements[0].origin,QPointF(130,130));
    QCOMPARE(document.state().placements[1].origin,QPointF(164,183));
    QCOMPARE(document.history()->count(),before+1);
    document.history()->undo();QCOMPARE(document.state().placements[0].origin,QPointF(103,103));
    canvas.select(first);QTest::keyClick(&canvas,Qt::Key_Left);
    QCOMPARE(document.state().placements[0].origin,QPointF(90,100));
    QCOMPARE(document.state().placements[1].origin,QPointF(137,156));
    QTest::keyClick(&canvas,Qt::Key_Left,Qt::AltModifier);
    QCOMPARE(document.state().placements[0].origin,QPointF(89,100));
    QTest::keyClick(&canvas,Qt::Key_Right,Qt::ShiftModifier);
    QCOMPARE(document.state().placements[0].origin,QPointF(190,100));
}

void EditorTest::snapEffectAndGradient()
{
    SnapGrid{true,10}.save();
    LightingDocument document(catalog());const auto id=document.addLayer(EffectKind::Triangle);
    document.edit("Fixture",[&](auto& state){state.layers[0].center={303,203};state.layers[0].extent={100,100};});
    LightingCanvas canvas(document,LightingCanvas::Context::Effects);canvas.resize(900,600);canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));canvas.fitToView();canvas.select(id);
    auto from=canvas.viewportPoint({303,203}).toPoint(),to=canvas.viewportPoint({330,227}).toPoint();
    QTest::mousePress(&canvas,Qt::LeftButton,Qt::NoModifier,from);
    QTest::mouseRelease(&canvas,Qt::LeftButton,Qt::NoModifier,to); // Release-only displacement is not lost.
    QCOMPARE(document.state().layers[0].center,QPointF(330,230));
    document.history()->undo();QCOMPARE(document.state().layers[0].center,QPointF(303,203));
    canvas.zoomBy(1.5);from=canvas.viewportPoint({303,203}).toPoint();to=canvas.viewportPoint({332,226}).toPoint();
    QTest::mousePress(&canvas,Qt::LeftButton,Qt::AltModifier,from);
    QTest::mouseRelease(&canvas,Qt::LeftButton,Qt::AltModifier,to);
    QVERIFY(std::abs(document.state().layers[0].center.x()-332)<1);
    QVERIFY(std::abs(document.state().layers[0].center.y()-226)<1);
    document.history()->undo();
    from=canvas.viewportPoint({253,153}).toPoint();to=canvas.viewportPoint({247,147}).toPoint();
    QTest::mousePress(&canvas,Qt::LeftButton,Qt::NoModifier,from);
    QTest::mouseRelease(&canvas,Qt::LeftButton,Qt::NoModifier,to);
    QCOMPARE(document.state().layers[0].center,QPointF(303,203));
    QCOMPARE(document.state().layers[0].extent,QSizeF(106,106));
    document.history()->undo();
    canvas.setTool(LightingCanvas::Tool::Gradient);
    from=canvas.viewportPoint({253,154}).toPoint();to=canvas.viewportPoint({347,247}).toPoint();
    QTest::mousePress(&canvas,Qt::LeftButton,Qt::NoModifier,from);
    QTest::mouseMove(&canvas,to);QTest::mouseRelease(&canvas,Qt::LeftButton,Qt::NoModifier,to);
    const auto& effect=document.state().layers[0];QVERIFY(effect.gradientFill);
    QVERIFY(QLineF(effectTransform(effect).map(effect.gradientStart-QPointF(.5,.5)),QPointF(250,150)).length()<.001);
    QVERIFY(QLineF(effectTransform(effect).map(effect.gradientEnd-QPointF(.5,.5)),QPointF(350,250)).length()<.001);
}

void EditorTest::snapLayoutPositions_data()
{
    QTest::addColumn<double>("zoom");QTest::addColumn<QPointF>("target");QTest::addColumn<QPointF>("expected");
    QTest::addColumn<bool>("bypass");
    QTest::newRow("negative-half-zoom")<<.5<<QPointF(-32,-22)<<QPointF(-30,-20)<<false;
    QTest::newRow("negative-double-zoom")<<2.<<QPointF(-32,-22)<<QPointF(-30,-20)<<false;
    QTest::newRow("positive-half-zoom")<<.5<<QPointF(132,122)<<QPointF(130,120)<<false;
    QTest::newRow("positive-double-zoom")<<2.<<QPointF(132,122)<<QPointF(130,120)<<false;
    QTest::newRow("alt-free")<<1.<<QPointF(132,122)<<QPointF(132,122)<<true;
}

void EditorTest::snapLayoutPositions()
{
    QFETCH(double,zoom);QFETCH(QPointF,target);QFETCH(QPointF,expected);QFETCH(bool,bypass);
    SnapGrid{true,10}.save();LightingDocument document(catalog());const auto id=document.addComponent("test.strip");
    document.edit("Fixture",[](auto& state){state.placements[0].origin={100,100};});
    LightingCanvas canvas(document,LightingCanvas::Context::Layout);canvas.resize(900,600);canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));canvas.fitToView();canvas.zoomBy(zoom);canvas.select(id);canvas.repaint();
    canvas.findChild<QScrollBar*>("canvasHorizontalScroll")->setValue(0);
    canvas.findChild<QScrollBar*>("canvasVerticalScroll")->setValue(0);canvas.repaint();
    const auto from=canvas.viewportPoint({100,100}),to=canvas.viewportPoint(target);
    const auto modifiers=bypass ? Qt::AltModifier : Qt::NoModifier;
    // Deliver exact world-derived coordinates, including off-viewport positions, without OS cursor rounding.
    QMouseEvent press(QEvent::MouseButtonPress,from,canvas.mapToGlobal(from.toPoint()),Qt::LeftButton,Qt::LeftButton,modifiers);
    QApplication::sendEvent(&canvas,&press);
    QMouseEvent release(QEvent::MouseButtonRelease,to,canvas.mapToGlobal(to.toPoint()),Qt::LeftButton,Qt::NoButton,modifiers);
    QApplication::sendEvent(&canvas,&release);
    QVERIFY(QLineF(document.state().placements[0].origin,expected).length()<.0001);
}

void EditorTest::snapLedArrangement()
{
    SnapGrid{true,10}.save();
    auto component=catalog().first();component.positions={{0,0},{3.25,0},{7,0}};
    ComponentEditor editor(component,true);editor.show();QVERIFY(QTest::qWaitForWindowExposed(&editor));
    auto* view=editor.findChild<QGraphicsView*>("componentGeometry");
    QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene({78,0}));
    QCOMPARE(editor.component().positions,component.positions); // Selecting an off-grid LED must not move it.
    auto from=view->mapFromScene({0,0}),to=view->mapFromScene({19,17});
    QTest::mousePress(view->viewport(),Qt::LeftButton,Qt::NoModifier,from);
    QTest::mouseMove(view->viewport(),to);QTest::mouseRelease(view->viewport(),Qt::LeftButton,Qt::NoModifier,to);
    // 24 scene units = one component unit = 20 Layout pixels; 10 px snaps to half units.
    QCOMPARE(editor.component().positions[0],QPointF(1,.5));
    QCOMPARE(editor.component().positions[1],QPointF(3.25,0));
    QCOMPARE(editor.component().mapping,component.mapping);
}

void EditorTest::snapPaletteStops()
{
    QSettings().setValue("editor/paletteSnapPercent",10);
    SnapGrid{true,10}.save();
    ColorListEditor editor(2,10);editor.setGradientMode();editor.setColors({Qt::red,Qt::green,Qt::blue});editor.resize(600,350);editor.show();
    QVERIFY(QTest::qWaitForWindowExposed(&editor));auto* strip=editor.findChild<QWidget*>("colorStopBar");
    const QPoint from(12+(strip->width()-24)/2,47),to(12+qRound((strip->width()-24)*.36),47);
    QTest::mousePress(strip,Qt::LeftButton,Qt::NoModifier,from);QTest::mouseRelease(strip,Qt::LeftButton,Qt::NoModifier,to);
    QVERIFY(std::abs(editor.positions()[1]-.4)<.0001);
    QTest::mousePress(strip,Qt::LeftButton,Qt::AltModifier,QPoint(12+qRound((strip->width()-24)*.4),47));
    QTest::mouseRelease(strip,Qt::LeftButton,Qt::AltModifier,to);
    QVERIFY(std::abs(editor.positions()[1]-.36)<.005);
}

void EditorTest::snapControls()
{
    LightingWindow window(catalog(),[] {return QVector<ProfileCandidate>{};});
    auto* action=window.findChild<QAction*>("snapToGrid");auto* size=window.findChild<QSpinBox*>("gridSpacing");
    QVERIFY(action);QVERIFY(size);QCOMPARE(size->minimum(),1);QCOMPARE(size->maximum(),100);
    action->setChecked(true);size->setValue(20);QVERIFY(SnapGrid::load().enabled);QCOMPARE(SnapGrid::load().spacing,20.);
    window.document().addComponent("test.strip");window.document().addLayer(EffectKind::LinearGradient);
    window.findChild<QPushButton*>("playPreview")->setChecked(false);
    window.resize(1600,960);window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));
    QVERIFY(window.grab().save(artifactDirectory()+"/snap-grid-workspace.png"));
    ComponentEditor editor(catalog().first(),true,&window);
    auto* ledSize=editor.findChild<QSpinBox*>("ledGridSpacing");QCOMPARE(ledSize->value(),20);
    ledSize->setValue(5);QCOMPARE(size->value(),5);QCOMPARE(SnapGrid::load().spacing,5.);
    size->setValue(25);QCOMPARE(ledSize->value(),25);
    editor.show();QVERIFY(QTest::qWaitForWindowExposed(&editor));
    QVERIFY(editor.grab().save(artifactDirectory()+"/snap-grid-leds.png"));
    action->setChecked(false);QVERIFY(!SnapGrid::load().enabled);
    window.document().history()->setClean();
}

QTEST_MAIN(EditorTest)
#include "test_editor.moc"

void EditorTest::hardwareIdentityAndSave()
{
    auto c=catalog().first(); c.name="M25 Gen2 120 #1";
    QCOMPARE(hardwareName(c,4),QString("Phanteks M25 Gen2 120 #4"));
    c.name="Glacier One M25 Gen2 / pump"; QCOMPARE(hardwareManufacturer(c),QString("Phanteks"));
    c.name="Handmade strip";c.id="custom.strip";QCOMPARE(hardwareManufacturer(c),QString("Generic"));
    c.manufacturer="Fixture brand";c.category="Strips";
    const auto restored=ComponentDefinition::fromJson(c.toJson()); QCOMPARE(restored.manufacturer,c.manufacturer);QCOMPARE(restored.category,c.category);QCOMPARE(restored.mapping,c.mapping);
    for(const auto& value:{QJsonValue(17),QJsonValue(""),QJsonValue(QString(161,'x'))}) {
        auto invalid=c.toJson();invalid["manufacturer"]=value;QVERIFY_EXCEPTION_THROWN(ComponentDefinition::fromJson(invalid),std::invalid_argument);
    }
    ComponentDefinition d30;d30.id="hardware.old-d30";d30.name="Phanteks D30-120";
    const QVector<int> oldX{2,1,1,0,0,0,0,0,0,0,0,0,1,1,2};
    for(int i=0;i<15;++i) {d30.positions.append({oldX[i]*3.0/14,i*3.0/14});d30.mapping.append(14-i);}
    const auto ring=ComponentDefinition::fromJson(d30.toJson());QCOMPARE(ring.mapping,d30.mapping);
    for(const auto& point:ring.positions) QVERIFY(qAbs(QLineF(point,QPointF(1.3,1.3)).length()-1.3)<.0001);
    d30.positions[0]+={.01,0};QCOMPARE(ComponentDefinition::fromJson(d30.toJson()).positions,d30.positions);
    HardwarePicker picker(catalog(),{},{});
    auto* category=picker.findChild<QComboBox*>("hardwareCategory");
    for(const auto* text:{"Fans","Strips","Mice","Keyboards","AiO","Pumps","RAM","Graphics cards"}) QVERIFY(category->findText(text)>=0);
    QVERIFY(picker.findChild<QComboBox*>("hardwareManufacturer")->findText("Phanteks")>=0);
    auto* products=picker.findChild<QListWidget*>("hardwareProducts");
    for(int i=0;i<products->count();++i) QVERIFY(!products->item(i)->text().contains("Reverse",Qt::CaseInsensitive));
    QTemporaryDir directory;LightingWindow window(catalog(),[] {return QVector<ProfileCandidate>{};});
    window.document().addComponent("test.strip");
    const auto first=directory.filePath("first.lighting.json"), second=directory.filePath("second.lighting.json");
    window.saveProject(first);QCOMPARE(window.projectFile(),first);
    QFile file(first);QVERIFY(file.open(QIODevice::ReadOnly));const auto original=file.readAll();file.close();
    window.document().addPaintLayer();window.saveProject(second);QCOMPARE(window.projectFile(),second);
    QCOMPARE(window.findChild<QLabel*>("currentProjectName")->toolTip(),second);
    window.document().addPaintLayer();window.findChild<QAction*>("saveProject")->trigger();
    LightingDocument loaded(catalog());loaded.load(second);QCOMPARE(loaded.state().layers.size(),2);
    QVERIFY(file.open(QIODevice::ReadOnly));QCOMPARE(file.readAll(),original);
    QVERIFY_EXCEPTION_THROWN(window.saveProject(directory.filePath("missing/fail.json")),std::runtime_error);
    QCOMPARE(window.projectFile(),second);QVERIFY_EXCEPTION_THROWN(window.saveProject(""),std::invalid_argument);
    window.document().history()->setClean();
}
void EditorTest::workspaceNavigation()
{
    LightingDocument document(catalog());const auto id=document.addComponent("test.strip");
    document.edit("Fixture",[](auto& state){state.placements[0].origin={200,180};});
    LightingCanvas canvas(document,LightingCanvas::Context::Layout);canvas.resize(900,640);canvas.show();QVERIFY(QTest::qWaitForWindowExposed(&canvas));canvas.select(id);canvas.fitToView();canvas.repaint();
    auto* horizontal=canvas.findChild<QScrollBar*>("canvasHorizontalScroll");auto* vertical=canvas.findChild<QScrollBar*>("canvasVerticalScroll");
    QVERIFY(!horizontal->isVisible());QVERIFY(!vertical->isVisible());
    const auto fitted=canvas.canvasPoint({450,300});
    QTest::mouseClick(&canvas,Qt::LeftButton,Qt::NoModifier,{canvas.width()-176,canvas.height()-110});
    QCOMPARE(canvas.canvasPoint({450,300}),fitted);canvas.select(id);
    // Bottom-right resize keeps the opposite corner fixed rather than expanding symmetrically.
    const auto anchor=canvas.viewportPoint({185,165});
    const auto from=canvas.viewportPoint({255.01,195.01}).toPoint();
    QTest::mousePress(&canvas,Qt::LeftButton,Qt::NoModifier,from);QTest::mouseMove(&canvas,from+QPoint(70,30));QTest::mouseRelease(&canvas,Qt::LeftButton,Qt::NoModifier,from+QPoint(70,30));
    const auto placed=document.state().placements[0];QVERIFY(placed.scale.width()>1);
    const auto fixed=placed.origin+(QPointF(185,165)-QPointF(200,180))*placed.scale.width();
    QVERIFY(QLineF(canvas.viewportPoint(fixed),anchor).length()<2);
    document.history()->undo();QCOMPARE(document.state().placements[0].origin,QPointF(200,180));
    canvas.zoomBy(3);canvas.repaint();QVERIFY(horizontal->isVisible());QVERIFY(vertical->isVisible());
    horizontal->setValue(0);vertical->setValue(0);canvas.repaint();
    QCOMPARE(qRound(canvas.viewportPoint({0,0}).x()),16);QCOMPARE(qRound(canvas.viewportPoint({0,0}).y()),48);
    horizontal->setValue(horizontal->maximum());vertical->setValue(vertical->maximum());canvas.repaint();
    QVERIFY(qAbs(canvas.viewportPoint({640,400}).x()-(canvas.width()-16-vertical->width()))<=1);
    QVERIFY(qAbs(canvas.viewportPoint({640,400}).y()-(canvas.height()-16-horizontal->height()))<=1);
    canvas.fitToView();canvas.repaint();canvas.setAutoScrollSpeed(0);
    auto start=canvas.viewportPoint({200,180}).toPoint(), edge=QPoint(canvas.width()-18,canvas.height()/2);
    QTest::mousePress(&canvas,Qt::LeftButton,Qt::NoModifier,start);QTest::mouseMove(&canvas,edge);
    const auto stopped=canvas.canvasPoint({450,300});QTest::qWait(70);QCOMPARE(canvas.canvasPoint({450,300}),stopped);
    canvas.setAutoScrollSpeed(720);QTest::qWait(100);QVERIFY(canvas.canvasPoint({450,300}).x()>stopped.x()+5);
    QTest::keyClick(&canvas,Qt::Key_Escape);const auto cancelled=canvas.canvasPoint({450,300});QTest::qWait(60);QCOMPARE(canvas.canvasPoint({450,300}),cancelled);
    for(double invalid:{-1.,4001.,std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()}) canvas.setAutoScrollSpeed(invalid);
}

void EditorTest::edgeScrolling_data()
{
    QTest::addColumn<QPoint>("direction");
    QTest::newRow("left")<<QPoint(-1,0);QTest::newRow("right")<<QPoint(1,0);
    QTest::newRow("top")<<QPoint(0,-1);QTest::newRow("bottom")<<QPoint(0,1);
}
void EditorTest::edgeScrolling()
{
    QFETCH(QPoint,direction);
    LightingDocument document(catalog());const auto id=document.addComponent("test.strip");
    document.edit("Center fixture",[](auto& state){state.placements[0].origin={300,200};});
    LightingCanvas canvas(document,LightingCanvas::Context::Layout);canvas.resize(900,640);canvas.show();QVERIFY(QTest::qWaitForWindowExposed(&canvas));canvas.fitToView();canvas.select(id);canvas.repaint();
    const QPoint edge(direction.x()<0 ? 17 : direction.x()>0 ? 883 : 450,direction.y()<0 ? 49 : direction.y()>0 ? 623 : 320);
    const auto from=canvas.viewportPoint({300,200}).toPoint();
    QTest::mousePress(&canvas,Qt::LeftButton,Qt::NoModifier,from);QTest::mouseMove(&canvas,edge);
    const auto before=canvas.canvasPoint({450,320});QTest::qWait(120);
    const auto delta=canvas.canvasPoint({450,320})-before;
    QVERIFY(QPointF::dotProduct(delta,QPointF(direction))>10);
    // Invalid speeds retain the previous working speed, rather than reversing or poisoning the camera.
    for(double invalid:{-1.,4001.,std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()}) canvas.setAutoScrollSpeed(invalid);
    const auto valid=canvas.canvasPoint({450,320});QTest::qWait(80);
    QVERIFY(QPointF::dotProduct(canvas.canvasPoint({450,320})-valid,QPointF(direction))>5);
    QTest::mouseRelease(&canvas,Qt::LeftButton,Qt::NoModifier,edge);canvas.repaint();
    auto* bar=canvas.findChild<QScrollBar*>(direction.x() ? "canvasHorizontalScroll" : "canvasVerticalScroll");QVERIFY(bar->isVisible());
    QVERIFY(document.history()->canUndo());document.history()->undo();QCOMPARE(document.state().placements[0].origin,QPointF(300,200));
}

void EditorTest::anchoredResize_data()
{
    QTest::addColumn<int>("corner");QTest::addColumn<double>("factor");
    for(int i=0;i<4;++i) { QTest::newRow(qPrintable(QString("grow-%1").arg(i)))<<i<<1.5; QTest::newRow(qPrintable(QString("shrink-%1").arg(i)))<<i<<.6; }
}
void EditorTest::anchoredResize()
{
    QFETCH(int,corner);QFETCH(double,factor);
    LightingDocument document(catalog());const auto id=document.addComponent("test.strip");
    document.edit("Resize fixture",[](auto& state){state.placements[0].origin={200,180};});
    LightingCanvas canvas(document,LightingCanvas::Context::Layout);canvas.resize(900,640);canvas.show();QVERIFY(QTest::qWaitForWindowExposed(&canvas));canvas.fitToView();canvas.select(id);canvas.repaint();
    const QVector<QPointF> corners{{185,165},{255.01,165},{255.01,195.01},{185,195.01}};
    const auto fixed=corners[(corner+2)%4],from=corners[corner],to=fixed+(from-fixed)*factor;
    const auto a=canvas.viewportPoint(from),b=canvas.viewportPoint(to);
    QMouseEvent press(QEvent::MouseButtonPress,a,canvas.mapToGlobal(a.toPoint()),Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(&canvas,&press);
    QMouseEvent release(QEvent::MouseButtonRelease,b,canvas.mapToGlobal(b.toPoint()),Qt::LeftButton,Qt::NoButton,Qt::NoModifier);QApplication::sendEvent(&canvas,&release);
    const auto placed=document.state().placements[0];QVERIFY(qAbs(placed.scale.width()-factor)<.0001);
    QVERIFY(QLineF(placed.origin+(fixed-QPointF(200,180))*factor,fixed).length()<.0001);
    document.history()->undo();QCOMPARE(document.state().placements[0].origin,QPointF(200,180));
}

void EditorTest::effectsOverflowNavigation()
{
    LightingDocument document(catalog());document.addLayer(EffectKind::Circle);
    document.edit("Oversized animated source",[](auto& state){auto& effect=state.layers[0];effect.extent={1800,1400};effect.spin=20;});
    LightingCanvas canvas(document,LightingCanvas::Context::Effects);canvas.resize(900,640);canvas.show();QVERIFY(QTest::qWaitForWindowExposed(&canvas));canvas.fitToView();canvas.repaint();
    auto* horizontal=canvas.findChild<QScrollBar*>("canvasHorizontalScroll");auto* vertical=canvas.findChild<QScrollBar*>("canvasVerticalScroll");
    QVERIFY(!horizontal->isVisible());QVERIFY(!vertical->isVisible());
    const auto camera=canvas.canvasPoint({450,320});QTest::mouseClick(&canvas,Qt::LeftButton,Qt::NoModifier,{canvas.width()-176,canvas.height()-110});QCOMPARE(canvas.canvasPoint({450,320}),camera);
    canvas.zoomBy(3);canvas.repaint();QVERIFY(horizontal->isVisible());QVERIFY(vertical->isVisible());
    canvas.fitToView();canvas.repaint();QVERIFY(!horizontal->isVisible());QVERIFY(!vertical->isVisible());
}

void EditorTest::protectedGroupLabels()
{
    LightingDocument document(catalog(),nullptr,{channelProfile()});const auto id=document.addComponent("test.strip");
    document.edit("At origin",[](auto& state){state.placements[0].origin={0,0};});
    ChannelAssignment assignment;assignment.profileId="test.device";assignment.endpoint="test";assignment.channelId="Direct";assignment.software=true;assignment.placements={id};document.assignChannel(assignment);
    LightingCanvas canvas(document,LightingCanvas::Context::Layout);canvas.resize(900,640);canvas.show();QVERIFY(QTest::qWaitForWindowExposed(&canvas));canvas.fitToView();canvas.repaint();
    const QPointF labelInterior(-16,-25-canvas.fontMetrics().height()-4);
    const auto screen=canvas.viewportPoint(labelInterior).toPoint();
    const auto image=canvas.grab().toImage();
    // QPixmap::grab uses physical pixels on high-DPI Windows monitors; widget coordinates stay logical.
    const auto pixel=[&](QPoint point){return image.pixelColor((QPointF(point)*image.devicePixelRatio()).toPoint());};
    QVERIFY(screen.y()>=48);QCOMPARE(pixel(screen),QColor("#202735"));
    QCOMPARE(pixel({150,45}),QColor("#17191f"));
}

void EditorTest::eraserPlayback_data()
{
    QTest::addColumn<int>("kind");QTest::addColumn<bool>("running");QTest::addColumn<bool>("mask");
    QTest::newRow("paint-stopped")<<int(EffectKind::Paint)<<false<<false;
    QTest::newRow("shape-stopped")<<int(EffectKind::Circle)<<false<<false;
    QTest::newRow("gradient-running")<<int(EffectKind::LinearGradient)<<true<<false;
    QTest::newRow("shape-running")<<int(EffectKind::Circle)<<true<<false;
    QTest::newRow("explicit-mask-running")<<int(EffectKind::Circle)<<true<<true;
}
void EditorTest::eraserPlayback()
{
    QFETCH(int,kind);QFETCH(bool,running);QFETCH(bool,mask);
    LightingWindow window(catalog(),[] {return QVector<ProfileCandidate>{};});window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));
    addEffect(window,EffectKind::LinearGradient);addEffect(window,EffectKind(kind));
    window.document().edit("Opaque source",[](auto& state){auto& top=state.layers.last();if(top.effect==EffectKind::Paint){top.image=QImage(640,400,QImage::Format_ARGB32_Premultiplied);top.image.fill(Qt::white);}});
    const auto before=window.document().state().layers.last();
    window.findChild<QPushButton*>("eraserTool")->click();window.findChild<QDoubleSpinBox*>("brushDiameter")->setValue(40);
    if(mask) window.findChild<QTabWidget*>("layerInspectorTabs")->setCurrentIndex(2);
    auto* play=window.findChild<QPushButton*>("playPreview");play->setChecked(running);
    auto* canvas=window.findChild<LightingCanvas*>("effectsCanvas");const auto point=canvas->viewportPoint({320,200}).toPoint();
    QTest::mousePress(canvas,Qt::LeftButton,Qt::NoModifier,point);QVERIFY(!play->isChecked());QTest::mouseMove(canvas,point+QPoint(20,0));QTest::mouseRelease(canvas,Qt::LeftButton,Qt::NoModifier,point+QPoint(20,0));
    QCOMPARE(window.document().state().layers.size(),2);QVERIFY(play->isEnabled());QCOMPARE(play->isChecked(),running);
    const auto after=window.document().state().layers.last();
    if(kind==int(EffectKind::Paint) && !mask) QCOMPARE(after.image.pixelColor(320,200).alpha(),0);
    else QVERIFY(after.mask.constScanLine(200)[320]<32);
    window.document().history()->undo();const auto restored=window.document().state().layers.last();QCOMPARE(restored.mask,before.mask);QCOMPARE(restored.image,before.image);
    window.document().history()->redo();QVERIFY(play->isEnabled());
    play->setChecked(false);QTest::mouseClick(play,Qt::LeftButton);QVERIFY(play->isChecked());
    const auto seconds=window.previewSeconds();QTRY_VERIFY(window.previewSeconds()>seconds);
    window.document().history()->setClean();
}
void EditorTest::layerEyesAndStroke()
{
    LightingWindow window(catalog(),[] {return QVector<ProfileCandidate>{};});window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));addEffect(window,EffectKind::Circle);
    auto* tree=window.findChild<QTreeWidget*>("effectLayers");const auto row=tree->visualItemRect(tree->topLevelItem(0));
    QTest::mouseClick(tree->viewport(),Qt::LeftButton,Qt::NoModifier,{row.left()+10,row.center().y()});
    QTRY_VERIFY(!window.document().state().layers[0].visible);
    tree->setFocus();QTest::keyClick(tree,Qt::Key_Space);QTRY_VERIFY(window.document().state().layers[0].visible);
    window.document().history()->undo();QVERIFY(!window.document().state().layers[0].visible);window.document().history()->redo();
    QVERIFY(studioToolIcon("eye").pixmap(24,24).toImage()!=studioToolIcon("eye-off").pixmap(24,24).toImage());
    window.findChild<QPushButton*>("pencilTool")->click();auto* size=window.findChild<QDoubleSpinBox*>("brushDiameter");auto* slider=window.findChild<QSlider*>("brushSizeSlider");
    QTRY_VERIFY2(slider->isVisible(),qPrintable(QString("tool=%1 toolbar=%2 rowHidden=%3 width=%4 action=%5").arg(int(window.findChild<LightingCanvas*>("effectsCanvas")->tool())).arg(window.findChild<QToolBar*>("drawingOptions")->isVisible()).arg(slider->parentWidget()->isHidden()).arg(window.width()).arg(window.findChild<QAction*>("strokeSizeAction")->isVisible())));QCOMPARE(slider->property("numericField").value<QObject*>(),size);
    slider->setValue(slider->minimum());QCOMPARE(size->value(),1.);slider->setValue(slider->maximum());QCOMPARE(size->value(),320.);
    size->setValue(40);QVERIFY(qAbs(slider->value()-122)<=1);
    window.findChild<QPushButton*>("eraserTool")->click();QTRY_VERIFY(slider->isVisible());QCOMPARE(size->value(),40.);
    size->setValue(-1);QCOMPARE(size->value(),1.);size->setValue(1000);QCOMPARE(size->value(),320.);
    QVERIFY(window.grab().save(artifactDirectory()+"/layer-eyes-stroke.png"));
    // Empty eraser gestures do not create a layer or leave a running canvas paused.
    window.findChild<QPushButton*>("deleteLayer")->click();auto* play=window.findChild<QPushButton*>("playPreview");play->setChecked(true);
    auto* canvas=window.findChild<LightingCanvas*>("effectsCanvas");QTest::mouseClick(canvas,Qt::LeftButton,Qt::NoModifier,canvas->viewportPoint({320,200}).toPoint());QVERIFY(play->isChecked());QVERIFY(window.document().state().layers.isEmpty());
    window.document().history()->setClean();
}
void EditorTest::deleteSelections()
{
    LightingWindow window(catalog(),[] {return QVector<ProfileCandidate>{};},nullptr,{channelProfile()});window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));
    addEffect(window,EffectKind::Circle);addEffect(window,EffectKind::Star);
    auto* effects=window.findChild<LightingCanvas*>("effectsCanvas");auto* layers=window.findChild<QTreeWidget*>("effectLayers");
    effects->setFocus();QTest::keyClick(effects,Qt::Key_Delete);QCOMPARE(window.document().state().layers.size(),1);QCOMPARE(window.document().state().layers[0].effect,EffectKind::Circle);
    window.document().history()->undo();QCOMPARE(window.document().state().layers.size(),2);
    layers->setCurrentItem(layers->topLevelItem(0)->child(0));layers->setFocus();QTest::keyClick(layers,Qt::Key_Delete);QCOMPARE(window.document().state().layers.size(),1);window.document().history()->undo();
    auto* size=window.findChild<QDoubleSpinBox*>("brushDiameter");window.findChild<QPushButton*>("pencilTool")->click();size->setFocus();QTest::keyClick(size,Qt::Key_Delete);QCOMPARE(window.document().state().layers.size(),2);
    auto* item=layers->topLevelItem(0);layers->editItem(item,0);auto* edit=layers->findChild<QLineEdit*>();QVERIFY(edit);edit->setText("ABC");edit->setCursorPosition(1);edit->setFocus();QTest::keyClick(edit,Qt::Key_Delete);QCOMPARE(edit->text(),QString("AC"));QCOMPARE(window.document().state().layers.size(),2);QTest::keyClick(edit,Qt::Key_Escape);
    const auto first=window.document().addComponent("test.strip"),second=window.document().addComponent("test.strip");
    ChannelAssignment assignment;assignment.endpoint="test";assignment.profileId="test.device";assignment.channelId="Direct";assignment.software=true;assignment.placements={first,second};window.document().assignChannel(assignment);
    window.document().edit("Target group",[&](auto& state){state.layers[0].allChannels=false;state.layers[0].channelTargets={assignment.id};});
    window.findChild<QAction*>("workspaceLayout")->trigger();auto* layout=window.findChild<LightingCanvas*>("layoutCanvas");layout->select(first);layout->setFocus();
    QTest::keyClick(layout,Qt::Key_Delete);QCOMPARE(window.document().state().placements.size(),1);QCOMPARE(window.document().state().assignments[0].placements,QVector<QUuid>({second}));window.document().history()->undo();
    layout->select(assignment.id);const int index=window.document().history()->index();QTest::keyClick(layout,Qt::Key_Delete);QCOMPARE(window.document().history()->index(),index+1);QVERIFY(window.document().state().placements.isEmpty());QVERIFY(window.document().state().assignments.isEmpty());QVERIFY(window.document().state().layers[0].channelTargets.isEmpty());
    window.document().history()->undo();QCOMPARE(window.document().state().placements.size(),2);QCOMPARE(window.document().state().layers[0].channelTargets,QVector<QUuid>({assignment.id}));
    window.findChild<QAction*>("workspaceHardware")->trigger();auto* hardware=window.findChild<QTreeView*>("hardwareComponents");
    const auto componentIndex=hardware->model()->index(0,0,hardware->model()->index(0,0));QVERIFY(componentIndex.isValid());hardware->setCurrentIndex(componentIndex);hardware->setFocus();QTest::keyClick(hardware,Qt::Key_Delete);QCOMPARE(window.document().state().placements.size(),1);window.document().history()->undo();
    window.findChild<QAction*>("workspaceLayout")->trigger();layout->setFocus();
    layout->select({});const auto noSelection=window.document().history()->index();QTest::keyClick(layout,Qt::Key_Delete);QCOMPARE(window.document().history()->index(),noSelection);
    layout->select(second);QTest::mouseClick(layout->findChild<QPushButton*>("canvasFullscreenButton"),Qt::LeftButton);QTest::keyClick(layout,Qt::Key_Delete);QCOMPARE(window.document().state().placements.size(),1);QTest::keyClick(layout,Qt::Key_Escape);
    window.document().history()->undo();window.document().history()->setClean();
}

void EditorTest::initialMaskBrushSize()
{
    LightingWindow window(catalog(),[] {return QVector<ProfileCandidate>{};});window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));addEffect(window,EffectKind::Circle);
    window.findChild<QTabWidget*>("layerInspectorTabs")->setCurrentIndex(2);
    auto* slider=window.findChild<QSlider*>("brushSizeSlider");QTRY_VERIFY(slider->isVisible());
    QCOMPARE(window.findChild<QDoubleSpinBox*>("brushDiameter")->value(),6.);
    auto* canvas=window.findChild<LightingCanvas*>("effectsCanvas");QTest::mouseClick(canvas,Qt::LeftButton,Qt::NoModifier,canvas->viewportPoint({320,200}).toPoint());
    const auto mask=window.document().state().layers.last().mask;QVERIFY(mask.constScanLine(200)[320]<32);QCOMPARE(mask.constScanLine(200)[330],uchar(255));
    window.document().history()->setClean();
}

void EditorTest::scrollbarAppearance()
{
    QWidget window;applyStudioStyle(&window);window.resize(240,260);
    QScrollBar horizontal(Qt::Horizontal,&window),vertical(Qt::Vertical,&window);
    horizontal.setGeometry(20,20,180,14);vertical.setGeometry(20,60,14,180);
    for(auto* bar:{&horizontal,&vertical}) {bar->setRange(0,100);bar->setPageStep(20);bar->setValue(50);}
    window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));QTest::mouseMove(&window,{230,10});QCoreApplication::processEvents();
    for(auto* bar:{&horizontal,&vertical}) {
        QStyleOptionSlider option;option.initFrom(bar);option.orientation=bar->orientation();option.rect=bar->rect();option.minimum=0;option.maximum=100;option.sliderPosition=50;option.sliderValue=50;option.pageStep=20;
        const auto handle=bar->style()->subControlRect(QStyle::CC_ScrollBar,&option,QStyle::SC_ScrollBarSlider,bar);
        const auto image=bar->grab().toImage();const auto pixel=[&](QPoint point){return image.pixelColor((QPointF(point)*image.devicePixelRatio()).toPoint());};
        QVERIFY(!handle.isEmpty());QCOMPARE(pixel(handle.center()),QColor("#9aa8c2"));
        // Check both track regions: native page rendering must not leave a patterned or differently colored rail.
        QCOMPARE(pixel(bar->orientation()==Qt::Horizontal ? QPoint(5,7) : QPoint(7,5)),QColor("#20242c"));
        QCOMPARE(pixel(bar->orientation()==Qt::Horizontal ? QPoint(174,7) : QPoint(7,174)),QColor("#20242c"));
    }
    QVERIFY(window.grab().save(artifactDirectory()+"/scrollbar-contrast.png"));
}
void EditorTest::effectPreviewDivider()
{
    LightingWindow window(catalog(),[] {return QVector<ProfileCandidate>{};});window.document().addComponent("test.strip");window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));addEffect(window,EffectKind::LinearGradient);
    auto* effects=window.findChild<LightingCanvas*>("effectsCanvas");auto* layout=window.findChild<LightingCanvas*>("layoutCanvas");
    auto* fullscreen=effects->findChild<QPushButton*>("canvasFullscreenButton");QVERIFY(!fullscreen->isVisible());fullscreen->click();QVERIFY(!window.findChild<QDialog*>("canvasFullscreen"));
    window.resize(1500,940);QCoreApplication::processEvents();QVERIFY(!fullscreen->isVisible());QVERIFY(!layout->findChild<QPushButton*>("canvasFullscreenButton")->isHidden());
    auto* splitter=window.findChild<QSplitter*>("effectWorkspace");auto* handle=splitter->handle(1);QVERIFY(handle->isVisible());QVERIFY(handle->height()>=14);QCOMPARE(handle->cursor().shape(),Qt::SplitVCursor);QVERIFY(!handle->toolTip().isEmpty());
    const auto before=splitter->sizes();const QPoint start(handle->width()-22,handle->height()/2);
    QTest::mousePress(handle,Qt::LeftButton,Qt::NoModifier,start);QTest::mouseMove(handle,start+QPoint(0,-60));QTest::mouseRelease(handle,Qt::LeftButton,Qt::NoModifier,start+QPoint(0,-60));
    const auto after=splitter->sizes();QVERIFY(after[0]<before[0]);QVERIFY(after[1]>before[1]);
    QVERIFY(window.findChild<QWidget*>("effectTools")->isVisible());QVERIFY(window.findChild<QWidget*>("drawingOptions")->isVisible());
    // Both extremes must collapse one pane, with the divider still available to restore it.
    for(int y:{-10000,10000}) {
        const auto point=handle->rect().center();
        QTest::mousePress(handle,Qt::LeftButton,Qt::NoModifier,point);QTest::mouseMove(handle,{point.x(),y});QTest::mouseRelease(handle,Qt::LeftButton,Qt::NoModifier,{point.x(),y});
        const auto sizes=splitter->sizes();QCOMPARE(sizes[y<0 ? 0 : 1],0);QVERIFY(sizes[y<0 ? 1 : 0]>0);QVERIFY(handle->isVisible());
        const auto restore=handle->rect().center();const int delta=y<0 ? 350 : -350;
        QTest::mousePress(handle,Qt::LeftButton,Qt::NoModifier,restore);QTest::mouseMove(handle,restore+QPoint(0,delta));QTest::mouseRelease(handle,Qt::LeftButton,Qt::NoModifier,restore+QPoint(0,delta));
        QVERIFY(splitter->sizes()[0]>0);QVERIFY(splitter->sizes()[1]>0);
    }
    auto* saveAs=window.findChild<QAction*>("saveProjectAs");QVERIFY(saveAs);QVERIFY(!saveAs->icon().isNull());
    QVERIFY(saveAs->icon().pixmap(24,24).toImage()!=studioToolIcon("save").pixmap(24,24).toImage());
    splitter->setSizes({500,200});QVERIFY(window.grab().save(artifactDirectory()+"/effects-preview-divider.png"));window.document().history()->setClean();
}

void EditorTest::sharedCanvasExtent()
{
    LightingDocument document(catalog());document.addComponent("test.strip");document.addLayer(EffectKind::LinearGradient);
    LightingCanvas effects(document,LightingCanvas::Context::Effects),hardware(document,LightingCanvas::Context::Layout);
    hardware.setPreviewOnly();effects.resize(1000,700);hardware.resize(1000,700);effects.show();hardware.show();
    QVERIFY(QTest::qWaitForWindowExposed(&effects));QVERIFY(QTest::qWaitForWindowExposed(&hardware));
    QImage frame(640,400,QImage::Format_ARGB32);
    for(int y=0;y<400;++y)for(int x=0;x<640;++x)frame.setPixelColor(x,y,QColor(x*255/639,y*255/399,80));
    // Grow to the right/bottom and left/top, then shrink again. Canonical raster and project data stay intact.
    for(const auto origin:{QPointF(1200,600),QPointF(-500,-300),QPointF(100,100)}) {
        document.edit("Move hardware",[&](auto& state){state.placements[0].origin=origin;});
        effects.fitToView();hardware.fitToView();effects.setPreviewFrame(frame,1);hardware.setPreviewFrame(frame,1);
        QCoreApplication::processEvents();const auto bounds=document.layoutBounds();
        const auto top=effects.viewportPoint({0,0}),bottom=effects.viewportPoint({640,400});
        QVERIFY(std::abs((bottom.x()-top.x())/(bottom.y()-top.y())-bounds.width()/bounds.height())<.00001);
        const auto effectImage=effects.grab().toImage(),hardwareImage=hardware.grab().toImage();
        for(const auto fraction:{QPointF(.25,.25),QPointF(.7,.65)}) {
            const QPointF raster(fraction.x()*640,fraction.y()*400);
            const auto a=effects.viewportPoint(raster);
            const auto b=hardware.viewportPoint(bounds.topLeft()+QPointF(fraction.x()*bounds.width(),fraction.y()*bounds.height()));
            QVERIFY(QLineF(effects.canvasPoint(a),raster).length()<.00001);
            const auto colorA=effectImage.pixelColor((a*effectImage.devicePixelRatio()).toPoint());
            const auto colorB=hardwareImage.pixelColor((b*hardwareImage.devicePixelRatio()).toPoint());
            QVERIFY(std::abs(colorA.red()-colorB.red())<=2);QVERIFY(std::abs(colorA.green()-colorB.green())<=2);
        }
        effects.zoomBy(4);effects.grab();
        QVERIFY(effects.findChild<QScrollBar*>("canvasHorizontalScroll")->isVisible());
        QVERIFY(effects.findChild<QScrollBar*>("canvasVerticalScroll")->isVisible());
    }
}

void EditorTest::maskPlaybackAndInventory()
{
    LightingWindow window(catalog(),[] {return QVector<ProfileCandidate>{};});window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));addEffect(window,EffectKind::Circle);
    auto* playing=window.findChild<QPushButton*>("playPreview");playing->setChecked(true);
    window.findChild<QPushButton*>("fillTool")->click();
    window.findChild<QTabWidget*>("layerInspectorTabs")->setCurrentIndex(2);
    QVERIFY(playing->isChecked());auto* canvas=window.findChild<LightingCanvas*>("effectsCanvas");QCOMPARE(canvas->tool(),LightingCanvas::Tool::Pencil);
    const auto time=window.previewSeconds();QTRY_VERIFY(window.previewSeconds()>time);
    QTest::mouseClick(canvas,Qt::LeftButton,Qt::NoModifier,canvas->viewportPoint({320,200}).toPoint());
    QVERIFY(window.document().state().layers.last().mask.constScanLine(200)[320]<32);QVERIFY(playing->isChecked());
    window.document().history()->undo();QCOMPARE(window.document().state().layers.last().mask.constScanLine(200)[320],uchar(255));
    auto c=catalog().first();c.id="test.x700";c.name="X700 Black";c.manufacturer="Example";
    auto white=c;white.id="test.x700.white";white.name="X700 White";
    QCOMPARE(hardwareName(c),QString("Example X700"));QCOMPARE(hardwareName(c),hardwareName(white));
    ComponentCatalog products{{c.id,c},{white.id,white}};
    HardwarePicker picker(products,{},{});auto* list=picker.findChild<QListWidget*>("hardwareProducts");QVERIFY(list);
    int variants=0,imported=0;for(int i=0;i<list->count();++i) {auto* item=list->item(i);if(item->text()=="Example X700")++variants;if(item->data(Qt::UserRole).toString().startsWith("hardware."))++imported;}
    QCOMPARE(variants,1);qInfo("Installed SignalRGB definitions visible after filtering/deduplication: %d",imported);
    // Same model name with different wiring must remain a separate choice.
    white.mapping={0,1,2};products[white.id]=white;HardwarePicker different(products,{},{});
    variants=0;for(auto* item:different.findChild<QListWidget*>("hardwareProducts")->findItems("Example X700",Qt::MatchExactly))++variants;
    QCOMPARE(variants,2);window.document().history()->setClean();
}

void EditorTest::stableMinimapAnchor()
{
    LightingDocument document(catalog());document.addLayer(EffectKind::StaticColor);
    LightingCanvas canvas(document,LightingCanvas::Context::Effects);canvas.resize(800,700);canvas.show();QVERIFY(QTest::qWaitForWindowExposed(&canvas));
    const int gutter=canvas.style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    const QPoint anchor(canvas.width()-16-gutter-188,canvas.height()-16-gutter-122);
    for(double zoom:{1.2,2.0,.5}) {
        canvas.zoomBy(zoom);const auto image=canvas.grab().toImage();
        QVERIFY(canvas.findChild<QScrollBar*>("canvasHorizontalScroll")->isVisible());
        const auto pixel=image.pixelColor((QPointF(anchor+QPoint(30,0))*image.devicePixelRatio()).toPoint());
        QCOMPARE(pixel,QColor("#596981"));
        QCOMPARE(canvas.findChild<QScrollBar*>("canvasVerticalScroll")->isVisible(),zoom==2.0);
    }
}

void EditorTest::detailedHardwareArtwork()
{
    auto fan=catalog().first();fan.category="Fans";fan.positions={{0,0},{3,0},{1.5,2.7},{999,999}};fan.mapping={0,1,2,3};fan.nonPhysicalSlots={3};
    const auto original=fan.toJson();const auto body=hardwareBodyBounds(fan);
    QCOMPARE(body.width(),body.height());QVERIFY(body.contains({0,0}));QVERIFY(body.contains({3,0}));QVERIFY(body.right()<10);QCOMPARE(fan.toJson(),original);
    QSet<QByteArray> images;
    for(const auto& category:QStringList{"Fans","Strips","RAM","Graphics cards","Keyboards","Mice","Pumps","AiO","Motherboards","Other"}) {
        QImage image(240,180,QImage::Format_ARGB32_Premultiplied);image.fill(Qt::transparent);
        {QPainter painter(&image);drawHardwareArtwork(painter,category,QRectF(10,10,220,160));}
        int pixels=0;for(int y=0;y<image.height();++y)for(int x=0;x<image.width();++x)if(image.pixelColor(x,y).alpha())++pixels;
        QVERIFY2(pixels>500,qPrintable(category));images.insert(QByteArray(reinterpret_cast<const char*>(image.constBits()),image.sizeInBytes()));
        QVERIFY(image.save(artifactDirectory()+"/hardware-"+QString(category).replace(" ","-")+".png"));
    }
    QCOMPARE(images.size(),10);
    // Presentation must never change the LED coordinates, permutation or project serialization.
    LightingDocument document({{fan.id,fan}});document.addComponent(fan.id);const auto before=document.componentPoints(document.state().placements[0]);
    LightingCanvas layout(document,LightingCanvas::Context::Layout);layout.resize(900,650);layout.show();QVERIFY(QTest::qWaitForWindowExposed(&layout));layout.grab();
    QCOMPARE(document.componentPoints(document.state().placements[0]),before);QCOMPARE(document.catalog()[fan.id].toJson(),original);
    ComponentEditor editor(fan,true);editor.show();QVERIFY(QTest::qWaitForWindowExposed(&editor));QCoreApplication::processEvents();
    auto* scene=editor.findChild<QGraphicsView*>("componentGeometry")->scene();int leds=0;
    for(auto* item:scene->items())if(item->type()==QGraphicsEllipseItem::Type) {++leds;QVERIFY(!item->childItems().isEmpty());}
    QCOMPARE(leds,3);QCOMPARE(editor.component().mapping,fan.mapping);QCOMPARE(editor.component().positions,fan.positions);
    QVERIFY(editor.grab().save(artifactDirectory()+"/detailed-fan-led-editor.png"));document.history()->setClean();
}

void EditorTest::layoutPresentationFixes()
{
    auto glacier=catalog().first();glacier.name="Glacier One 360M25 G2 / pump";glacier.category="Pumps";
    QCOMPARE(hardwareCategory(glacier),QString("AiO"));
    QVERIFY(hardwareName(glacier).endsWith("/ AiO"));
    auto strip=catalog().first();strip.name="Very long strip model description outside the body";
    auto dense=strip;dense.id="dense.strip";dense.positions={{0,0},{.5,0},{1,0},{1.5,0},{2,0}};dense.mapping={0,1,2,3,4};
    const auto raster=[](const ComponentDefinition& c) {
        QImage image(500,100,QImage::Format_ARGB32_Premultiplied);image.fill(Qt::transparent);
        QPainter p(&image);p.translate(40,50);p.scale(200,200);drawHardwareComponentArtwork(p,c);return image;
    };
    QVERIFY(raster(strip)!=raster(dense));
    QVERIFY(raster(strip).save(artifactDirectory()+"/strip-three-leds.png"));
    QVERIFY(raster(dense).save(artifactDirectory()+"/strip-five-leds.png"));
    LightingDocument document({{strip.id,strip}});const auto id=document.addComponent(strip.id);
    document.edit("Rotate fixture",[](auto& state){state.placements[0].origin={250,180};state.placements[0].rotation=90;});
    LightingCanvas canvas(document,LightingCanvas::Context::Layout);canvas.resize(900,650);canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));canvas.select(id);canvas.grab();
    const auto image=canvas.grab().toImage();
    // At 90 degrees the grip is right of the body; the same location must also accept rotation input.
    const auto grip=canvas.viewportPoint({289,200});
    const auto pixel=image.pixelColor((canvas.viewportPoint({283,200})*image.devicePixelRatio()).toPoint());
    QVERIFY(pixel.blue()>pixel.red());
    QTest::mousePress(&canvas,Qt::LeftButton,Qt::AltModifier,grip.toPoint());
    QTest::mouseMove(&canvas,canvas.viewportPoint({250,239}).toPoint());
    QTest::mouseRelease(&canvas,Qt::LeftButton,Qt::AltModifier,canvas.viewportPoint({250,239}).toPoint());
    QVERIFY(std::abs(std::abs(document.state().placements[0].rotation)-180)<3);
    document.history()->undo();QCOMPARE(document.state().placements[0].rotation,90.);
    // Exercise cached names after edits/undo, including a duplicate becoming unique again.
    const auto firstName=document.componentName(document.state().placements[0]);
    document.addComponent(strip.id);QVERIFY(document.componentName(document.state().placements[0]).endsWith(" #1"));
    document.history()->undo();QCOMPARE(document.componentName(document.state().placements[0]),firstName);
    canvas.grab().save(artifactDirectory()+"/layout-rotated-grip-label.png");
    QElapsedTimer clock;clock.start();for(int i=0;i<30;++i)canvas.grab();
    qInfo("Layout 30 warm frames: %lld ms",clock.elapsed());
    document.history()->setClean();
}

void EditorTest::denseHardwareArtwork()
{
    ComponentDefinition strimer;strimer.id="fixture.strimer";strimer.name="Lian Li GPU Strimer Plus";strimer.category="Strips";
    for(int i=0;i<27;++i){strimer.positions.append({3.*i/26,0});strimer.mapping.append(i);}
    QCOMPARE(hardwareCategory(strimer),QString("Strips"));QCOMPARE(hardwareArtworkCategory(strimer),QString("Cables"));
    auto duplicatedBrand=strimer;duplicatedBrand.manufacturer="LianLi";duplicatedBrand.name="LianLi Lian Li GPU Strimer Plus";
    QCOMPARE(hardwareName(duplicatedBrand),QString("Lian Li GPU Strimer Plus"));
    auto legacy=strimer;legacy.category="Graphics cards";QCOMPARE(hardwareCategory(legacy),QString("Strips"));
    QVERIFY(hardwarePreviewLedRadius(strimer,{1,1})<1.);
    for(const auto scale:{QSizeF(.1,.1),QSizeF(1,1),QSizeF(5,.2)})
        QVERIFY(hardwarePreviewLedRadius(strimer,scale)*2.3<60.*scale.width()/26.);
    auto reserved=strimer;reserved.positions.append({0.000001,0});reserved.mapping.append(27);reserved.nonPhysicalSlots={27};
    QCOMPARE(hardwarePreviewLedRadius(reserved,{1,1}),hardwarePreviewLedRadius(strimer,{1,1}));
    auto duplicate=strimer;duplicate.positions.append(duplicate.positions.first());duplicate.mapping.append(27);
    QCOMPARE(hardwarePreviewLedRadius(duplicate,{1,1}),hardwarePreviewLedRadius(strimer,{1,1}));
    auto large=strimer;large.positions.clear();large.mapping.clear();
    for(int i=0;i<65535;++i){large.positions.append({i*.001,0});large.mapping.append(i);}
    QVERIFY(hardwarePreviewLedRadius(large,{1,1})<.01);
    QVector<ComponentDefinition> fixtures;
    for(const auto& category:QStringList{"Fans","Strips","Cables","RAM","Graphics cards","Keyboards","Mice","Pumps","AiO","Motherboards","Other"}) {
        auto c=strimer;c.id="fixture."+category;c.category=category;c.name=category=="Cables" ? strimer.name : category;
        if(category=="Fans" || category=="Pumps" || category=="AiO") {
            c.positions.clear();c.mapping.clear();
            for(int i=0;i<16;++i){const double a=i*6.283185307179586/16;c.positions.append({1.5+1.5*std::cos(a),1.5+1.5*std::sin(a)});c.mapping.append(i);}
        } else if(category=="Graphics cards" || category=="Motherboards" || category=="Other") {c.positions={{0,0}};c.mapping={0};}
        else if(category=="Keyboards")c=loadComponentCatalog(TEST_COMPONENT_DIR)["peripheral.vulcan"];
        else if(category=="Mice")c=loadComponentCatalog(TEST_COMPONENT_DIR)["peripheral.basilisk"];
        fixtures.append(c);
    }
    QImage sheet(1200,1200,QImage::Format_ARGB32_Premultiplied);sheet.fill(QColor("#17191f"));QPainter gallery(&sheet);
    for(int index=0;index<fixtures.size();++index) {
        const auto& c=fixtures[index];const auto original=c.toJson();const auto bounds=hardwareBodyBounds(c);
        const double radius=hardwarePreviewLedRadius(c,{1,1})/20;
        for(int i=0;i<c.positions.size();++i)for(int j=i+1;j<c.positions.size();++j) {
            if(c.nonPhysicalSlots.contains(c.mapping[i]) || c.nonPhysicalSlots.contains(c.mapping[j]))continue;
            const double distance=QLineF(c.positions[i],c.positions[j]).length();
            if(distance>0)QVERIFY2(radius*2.3<distance,qPrintable(c.name));
        }
        const double scale=std::min(340./bounds.width(),220./bounds.height());
        QImage tile(400,300,QImage::Format_ARGB32_Premultiplied);tile.fill(QColor("#17191f"));
        { QPainter p(&tile);p.setRenderHint(QPainter::Antialiasing);p.translate(200,135);p.scale(scale,scale);p.translate(-bounds.center());
          drawHardwareComponentArtwork(p,c); }
        // Enough real SVG artwork must survive the LED overlay, including the 27 tightly packed Strimer addresses.
        const auto artwork=tile;
        { QPainter p(&tile);p.setRenderHint(QPainter::Antialiasing);p.translate(200,135);p.scale(scale,scale);p.translate(-bounds.center());
          p.setBrush(Qt::yellow);p.setPen(QPen(Qt::white,radius*.2));
          for(int i=0;i<c.positions.size();++i)if(!c.nonPhysicalSlots.contains(c.mapping[i]))p.drawEllipse(c.positions[i],radius,radius); }
        int remaining=0;
        for(int y=0;y<260;++y)for(int x=0;x<400;++x)if(artwork.pixelColor(x,y)==QColor("#92a2ba") && tile.pixel(x,y)==artwork.pixel(x,y))++remaining;
        QVERIFY2(remaining>20,qPrintable(c.name));QCOMPARE(c.toJson(),original);
        {QPainter p(&tile);p.setPen(Qt::white);p.drawText(QRect(10,265,380,30),Qt::AlignCenter,hardwareName(c));}
        gallery.drawImage((index%3)*400,(index/3)*300,tile);
    }
    gallery.end();QVERIFY(sheet.save(artifactDirectory()+"/all-hardware-led-visibility.png"));
    for(const auto* file:{"fan","strip","strip-led","cable","ram","gpu","keyboard","mouse","pump","aio","motherboard","other"}) {
        QSvgRenderer svg(QString(":/hardware/%1.svg").arg(file));QVERIFY2(svg.isValid(),file);
    }
    LightingDocument doc({{strimer.id,strimer}});const auto id=doc.addComponent(strimer.id);
    doc.edit("Center dense strip",[](auto& state){state.placements[0].origin={290,200};});
    LightingCanvas canvas(doc,LightingCanvas::Context::Layout);canvas.resize(900,600);canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));canvas.select(id);canvas.zoomBy(2);
    const auto before=doc.componentPoints(doc.state().placements[0]);
    QVERIFY(canvas.grab().save(artifactDirectory()+"/strimer-layout-svg.png"));
    QCOMPARE(doc.componentPoints(doc.state().placements[0]),before);doc.history()->setClean();
}

void EditorTest::gamingWorkspace()
{
    LightingWindow window(catalog(),[] {return QVector<ProfileCandidate>{};});window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));
    window.document().addComponent("test.strip");addEffect(window,EffectKind::LinearGradient);
    auto* canvas=window.findChild<LightingCanvas*>("effectsCanvas");auto* inspector=window.findChild<QDockWidget*>("propertiesDock");
    auto* navigator=window.findChild<QDockWidget*>("layersDock");QVERIFY(inspector->isVisible());QVERIFY(navigator->isVisible());
    auto* output=window.findChild<QLabel*>("toolbarOutputStatus");auto* play=window.findChild<QPushButton*>("playPreview");
    window.setOutputState("Running");QVERIFY(output->isVisible());QVERIFY(output->text().contains("Running"));
    auto* panels=window.findChild<QToolButton*>("workspacePanels");const auto control=panels->grab().toImage();
    const auto colorAt=[&](QPoint p){return control.pixelColor((QPointF(p)*control.devicePixelRatio()).toPoint());};
    const auto luminance=[](QColor color) {
        const auto channel=[](double value){return value<=.04045 ? value/12.92 : std::pow((value+.055)/1.055,2.4);};
        return .2126*channel(color.redF())+.7152*channel(color.greenF())+.0722*channel(color.blueF());
    };
    const auto edge=luminance(colorAt({panels->width()/2,0})),fill=luminance(colorAt({6,panels->height()/2}));
    QVERIFY((std::max(edge,fill)+.05)/(std::min(edge,fill)+.05)>=3.);
    play->setChecked(true);play->setChecked(false);QCOMPARE(output->text(),QString("Devices: Running"));
    QCOMPARE(window.findChild<QLabel*>("previewPlaybackStatus")->text(),QString("Preview: paused"));
    window.resize(1000,720);QCoreApplication::processEvents();QVERIFY(!inspector->isVisible());QVERIFY(canvas->width()>window.width()/2);
    auto* preview=window.findChild<QToolBar*>("previewToolbar");auto* navigation=window.findChild<QToolBar*>("workspaceNavigation");
    QVERIFY(preview->geometry().top()>navigation->geometry().top());QVERIFY(output->isVisible());
    QVERIFY(window.grab().save(artifactDirectory()+"/gaming-compact.png"));
    window.findChild<QAction*>("toggleInspector")->trigger();QVERIFY(inspector->isVisible());
    window.findChild<QAction*>("workspaceHardware")->trigger();QVERIFY(!navigator->isVisible());QVERIFY(!inspector->isVisible());
    QVERIFY(window.findChild<QLabel*>("hardwareEmptyState")->isVisible());QVERIFY(output->isVisible());
    window.findChild<QAction*>("workspaceEffects")->trigger();QVERIFY(inspector->isVisible());
    window.resize(1440,900);QCoreApplication::processEvents();QVERIFY(inspector->isVisible());
    window.hide();window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));QVERIFY(inspector->isVisible());
    window.findChild<QAction*>("toggleInspector")->trigger();QVERIFY(!inspector->isVisible());
    window.findChild<QAction*>("workspaceLayout")->trigger();QVERIFY(!inspector->isVisible());
    window.findChild<QAction*>("resetWorkspace")->trigger();QVERIFY(inspector->isVisible());QVERIFY(navigator->isVisible());
    window.findChild<QAction*>("toggleNavigator")->trigger();QVERIFY(!navigator->isVisible());
    window.findChild<QAction*>("toggleNavigator")->trigger();QVERIFY(navigator->isVisible());
    window.findChild<QAction*>("workspaceEffects")->trigger();
    QCoreApplication::processEvents();
    QVERIFY(window.grab().save(artifactDirectory()+"/gaming-effects.png"));
    window.setOutputState("Stopped");window.findChild<QAction*>("workspaceHardware")->trigger();QCoreApplication::processEvents();
    QVERIFY(window.grab().save(artifactDirectory()+"/gaming-hardware-offline.png"));
    window.document().history()->setClean();
}

void EditorTest::layerModeAndOrder()
{
    LightingWindow window(catalog(),[] {return QVector<ProfileCandidate>{};});window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));
    window.findChild<QAction*>("workspaceEffects")->trigger();
    const auto bottom=window.document().addLayer(EffectKind::StaticColor),middle=window.document().addLayer(EffectKind::LinearGradient),top=window.document().addLayer(EffectKind::Circle);
    auto* tree=window.findChild<QTreeWidget*>("effectLayers");tree->setCurrentItem(tree->topLevelItem(1));
    window.findChild<QPushButton*>("moveLayerUpButton")->click();
    QCOMPARE(window.document().state().layers[2].id,middle);QVERIFY(!window.findChild<QAction*>("moveLayerUp")->isEnabled());
    window.document().history()->undo();QCOMPARE(window.document().state().layers[1].id,middle);
    tree->setFocus();QTest::keyClick(tree,Qt::Key_PageDown,Qt::ControlModifier);
    QCOMPARE(window.document().state().layers[0].id,middle);window.document().history()->undo();
    QCOMPARE(window.document().state().layers[0].id,bottom);QCOMPARE(window.document().state().layers[2].id,top);
    auto* tabs=window.findChild<QTabWidget*>("layerInspectorTabs");tabs->setCurrentIndex(2);
    QVERIFY(window.findChild<QLabel*>("layerEditingContext")->text().contains("mask visibility"));
    tabs->setCurrentIndex(0);QCOMPARE(tabs->tabText(0),QString("Source"));
    QVERIFY(window.findChild<QLabel*>("layerEditingContext")->text().contains("effect source"));
    window.findChild<QAction*>("workspaceLayout")->trigger();QVERIFY(!window.findChild<QAction*>("moveLayerDown")->isEnabled());
    window.document().history()->setClean();
}

void EditorTest::fixedComponentLabels()
{
    auto c=catalog().first();c.name="A deliberately long lighting component model description";
    LightingDocument doc({{c.id,c}});const auto first=doc.addComponent(c.id),second=doc.addComponent(c.id);
    doc.edit("Adjacent fixtures",[](auto& state){state.placements[0].origin={80,80};state.placements[1].origin={110,80};});
    LightingCanvas canvas(doc,LightingCanvas::Context::Layout);canvas.resize(900,650);canvas.show();QVERIFY(QTest::qWaitForWindowExposed(&canvas));
    const auto camera=canvas.viewportPoint({80,80});const auto hidden=canvas.grab().toImage();
    QTest::mouseClick(&canvas,Qt::LeftButton,Qt::NoModifier,canvas.viewportPoint({75,102}).toPoint());QVERIFY(canvas.selection().isNull());
    canvas.setShowLabels(true);const auto visible=canvas.grab().toImage();QCOMPARE(canvas.viewportPoint({80,80}),camera);QVERIFY(hidden!=visible);
    QTest::mouseClick(&canvas,Qt::LeftButton,Qt::NoModifier,canvas.viewportPoint({75,102}).toPoint());QCOMPARE(canvas.selection(),first);
    QTest::mouseClick(&canvas,Qt::LeftButton,Qt::NoModifier,canvas.viewportPoint({155,102}).toPoint());QCOMPARE(canvas.selection(),second);
    // A nearby housing must never push this caption down from its fixed anchor.
    doc.edit("Crowd caption",[](auto& state){state.placements[0].origin={110,100};});canvas.select({});canvas.grab();
    QTest::mouseClick(&canvas,Qt::LeftButton,Qt::NoModifier,canvas.viewportPoint({155,102}).toPoint());QCOMPARE(canvas.selection(),second);
    const auto hoverPosition=canvas.viewportPoint(doc.componentPoints(doc.state().placements[1]).first());
    QMouseEvent hover(QEvent::MouseMove,hoverPosition,canvas.mapToGlobal(hoverPosition.toPoint()),Qt::NoButton,Qt::NoButton,Qt::NoModifier);
    QApplication::sendEvent(&canvas,&hover);QVERIFY(canvas.toolTip().contains(c.name));
    QVERIFY(canvas.grab().save(artifactDirectory()+"/fixed-small-component-labels.png"));doc.history()->setClean();
}

void EditorTest::labelRevealControls()
{
    LightingWindow window(catalog(),[] {return QVector<ProfileCandidate>{};});window.document().addComponent("test.strip");window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto* canvas=window.findChild<LightingCanvas*>("layoutCanvas");auto* eye=window.findChild<QAction*>("showComponentLabels");
    QVERIFY(eye);QVERIFY(!eye->icon().isNull());QVERIFY(!eye->isChecked());QVERIFY(!canvas->showLabels());
    const auto revision=window.document().revision();const auto camera=canvas->viewportPoint({80,80});
    canvas->setFocus();QTest::keyPress(canvas,Qt::Key_H);QVERIFY(canvas->showLabels());QVERIFY(!eye->isChecked());
    QVERIFY(!QSettings().value("editor/showComponentLabels").toBool());canvas->grab();QCOMPARE(canvas->viewportPoint({80,80}),camera);
    QKeyEvent repeat(QEvent::KeyRelease,Qt::Key_H,Qt::NoModifier,QString{},true);QApplication::sendEvent(canvas,&repeat);QVERIFY(canvas->showLabels());
    auto* inventory=window.findChild<QTreeWidget*>("layoutComponents");inventory->setFocus();QCoreApplication::processEvents();QVERIFY(canvas->showLabels());
    QTest::keyRelease(inventory,Qt::Key_H);QVERIFY(!canvas->showLabels());
    eye->setChecked(true);QVERIFY(canvas->showLabels());QVERIFY(QSettings().value("editor/showComponentLabels").toBool());
    { LightingWindow restored(catalog(),[] {return QVector<ProfileCandidate>{};});QVERIFY(restored.findChild<LightingCanvas*>("layoutCanvas")->showLabels()); }
    QTest::keyPress(canvas,Qt::Key_H);QTest::keyRelease(canvas,Qt::Key_H);QVERIFY(canvas->showLabels());eye->setChecked(false);
    QLineEdit editor(&window);canvas->setFocus();QTest::keyPress(canvas,Qt::Key_H);QVERIFY(canvas->showLabels());
    editor.show();editor.setFocus();QCoreApplication::processEvents();QVERIFY(!canvas->showLabels());QTest::keyRelease(&editor,Qt::Key_H);QTest::keyClick(&editor,Qt::Key_H);
    QCOMPARE(editor.text().toLower(),QString("h"));QVERIFY(!canvas->showLabels());editor.hide();
    canvas->setFocus();QTest::keyPress(canvas,Qt::Key_H);QVERIFY(canvas->showLabels());
    QEvent deactivate(QEvent::WindowDeactivate);QApplication::sendEvent(&window,&deactivate);QVERIFY(!canvas->showLabels());QTest::keyRelease(canvas,Qt::Key_H);
    canvas->setFocus();QTest::keyPress(canvas,Qt::Key_H);window.hide();QVERIFY(!canvas->showLabels());window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));
    canvas->setFocus();QTest::keyPress(canvas,Qt::Key_H);window.findChild<QAction*>("workspaceEffects")->trigger();QVERIFY(!canvas->showLabels());QVERIFY(!eye->isEnabled());
    window.document().addLayer();auto* effects=window.findChild<LightingCanvas*>("effectsCanvas");QTest::keyRelease(effects,Qt::Key_H);
    window.activateWindow();effects->setFocus();QCoreApplication::processEvents();QTest::keyClick(effects,Qt::Key_L);
    QTRY_COMPARE(effects->tool(),LightingCanvas::Tool::Line);
    window.findChild<QAction*>("workspaceLayout")->trigger();QVERIFY(eye->isEnabled());
    canvas->findChild<QPushButton*>("canvasFullscreenButton")->click();auto* host=window.findChild<QDialog*>("canvasFullscreen");QVERIFY(host);QTRY_VERIFY(host->isVisible());
    host->activateWindow();QCoreApplication::processEvents();canvas->setFocus();QTest::keyPress(canvas,Qt::Key_H);QVERIFY(canvas->showLabels());
    QTest::keyRelease(host,Qt::Key_H);QVERIFY(!canvas->showLabels());host->reject();
    QCOMPARE(window.document().revision(),revision+1); // Only adding the effect changed the document.
    window.document().history()->setClean();
}

void EditorTest::hardwarePreviewProjection()
{
    auto component=catalog().first();component.positions.append({999,999});component.mapping.append(3);component.nonPhysicalSlots={3};
    LightingDocument doc({{component.id,component}});doc.addComponent("test.strip");const auto revision=doc.revision();
    LightingCanvas preview(doc,LightingCanvas::Context::Layout);preview.setPreviewOnly();preview.resize(900,260);preview.show();QVERIFY(QTest::qWaitForWindowExposed(&preview));
    QImage frame(640,400,QImage::Format_ARGB32);for(int y=0;y<400;++y)for(int x=0;x<640;++x)frame.setPixelColor(x,y,QColor(x*255/639,y*255/399,80));
    preview.setPreviewFrame(frame,0);const double full=QLineF(preview.viewportPoint({80,80}),preview.viewportPoint({120,80})).length();
    preview.fitToView();const auto image=preview.grab().toImage();
    QVERIFY(std::abs(QLineF(preview.viewportPoint({80,80}),preview.viewportPoint({120,80})).length()-full)<.001);
    for(const auto point:{QPointF(80,80),QPointF(100,80),QPointF(120,80)}) {
        const auto pixel=image.pixelColor((preview.viewportPoint(point)*image.devicePixelRatio()).toPoint());QCOMPARE(pixel,frame.pixelColor(point.toPoint()));
        QVERIFY(QLineF(preview.canvasPoint(preview.viewportPoint(point)),point).length()<.00001);
    }
    QCOMPARE(doc.revision(),revision);QVERIFY(image.save(artifactDirectory()+"/gaming-focused-hardware.png"));
    preview.fitToView();QVERIFY(std::abs(QLineF(preview.viewportPoint({80,80}),preview.viewportPoint({120,80})).length()-full)<.001);
    doc.history()->setClean();
}

void EditorTest::deviceArtworkAlignment()
{
    auto strip=catalog().first();strip.id="fixture.d30-line";strip.name="Phanteks Phantek D30-120";strip.positions.clear();strip.mapping.clear();
    for(int i=0;i<15;++i){strip.positions.append({i*4./14,0});strip.mapping.append(i);}
    QCOMPARE(hardwareCategory(strip),QString("Fans"));QCOMPARE(hardwareArtworkCategory(strip),QString("Strips"));
    QVERIFY(hardwareBodyBounds(strip).height()<.4);QVERIFY(hardwareBodyBounds(strip).width()>4);
    QCOMPARE(hardwareName(strip),QString("Phanteks D30-120"));
    auto diagonal=strip;for(auto& point:diagonal.positions)point={point.x(),point.x()};QCOMPARE(hardwareArtworkCategory(diagonal),QString("Strips"));
    auto ring=strip;ring.positions[7].ry()+=.01;QCOMPARE(hardwareArtworkCategory(ring),QString("Fans"));
    auto apex=loadComponentDefinition(QString(TEST_COMPONENT_DIR)+"/peripheral-apex3.json");
    auto vulcan=loadComponentDefinition(QString(TEST_COMPONENT_DIR)+"/peripheral-vulcan.json");
    auto mouse=loadComponentDefinition(QString(TEST_COMPONENT_DIR)+"/peripheral-basilisk.json");
    // Legacy projects bake rotation/anisotropic scaling into positions; never rewrite them to fit an icon.
    for(auto& point:mouse.positions)point={-point.y()*1.3,point.x()*1.2};
    for(auto& point:vulcan.positions)point={point.x()*2.4-6.9,point.y()*1.85-1.15};
    ComponentCatalog components{{strip.id,strip},{apex.id,apex},{vulcan.id,vulcan},{mouse.id,mouse}};
    auto profile=channelProfile();profile.channels.clear();
    for(const auto& name:QStringList{"Strip","Vulcan","Basilisk","SteelSeriesApex3"}) {ChannelProfile channel;channel.id=name;channel.maxLeds=200;channel.direct=true;profile.channels.append(channel);}
    LightingDocument document(components,nullptr,{profile});
    const auto stripId=document.addComponent(strip.id),vulcanId=document.addComponent(vulcan.id),mouseId=document.addComponent(mouse.id),apexId=document.addComponent(apex.id);
    document.edit("Reproduce imported artwork",[&](auto& state){
        state.placements[0].origin={60,70};state.placements[0].scale={4,4};
        state.placements[1].origin={570,100};state.placements[1].rotation=.85;
        state.placements[2].origin={850,100};state.placements[2].rotation=90.88;
        state.placements[3].origin={400,290};state.placements[3].scale={3.5,3.5};
    });
    const QVector<QUuid> ids{stripId,vulcanId,mouseId,apexId};
    for(int i=0;i<ids.size();++i){ChannelAssignment assignment;assignment.profileId=profile.id;assignment.endpoint="fixture";assignment.channelId=profile.channels[i].id;assignment.software=true;assignment.placements={ids[i]};document.assignChannel(assignment);}
    document.addLayer(EffectKind::StaticColor);
    const auto before=document.state().components;QVector<QVector<QPointF>> points;for(const auto& placement:document.state().placements)points.append(document.componentPoints(placement));
    LightingCanvas canvas(document,LightingCanvas::Context::Layout);applyStudioStyle(&canvas);canvas.resize(1400,800);canvas.show();QVERIFY(QTest::qWaitForWindowExposed(&canvas));canvas.select(apexId);canvas.fitToView();canvas.repaint();
    QVERIFY(canvas.grab().save(artifactDirectory()+"/device-artwork-alignment.png"));
    for(int i=0;i<points.size();++i)QCOMPARE(document.componentPoints(document.state().placements[i]),points[i]);
    for(auto it=before.begin();it!=before.end();++it)QCOMPARE(document.catalog()[it.key()].toJson(),it->toJson());
    // The visible housing's bottom-right corner must be covered by the resize handles, not outside them.
    const auto& placement=document.state().placements[3];QTransform transform;transform.translate(400,290);transform.scale(70,70);
    const auto housing=transform.mapRect(hardwareBodyBounds(apex));
    QVERIFY(housing.height()>100);const auto corner=housing.bottomRight()+QPointF(5,5);
    const auto start=canvas.viewportPoint(corner).toPoint();
    QTest::mousePress(&canvas,Qt::LeftButton,Qt::NoModifier,start);QTest::mouseMove(&canvas,start+QPoint(45,25));QTest::mouseRelease(&canvas,Qt::LeftButton,Qt::NoModifier,start+QPoint(45,25));
    QVERIFY(document.state().placements[3].scale.width()>3.5);document.history()->undo();
    QCOMPARE(document.state().placements[3].scale,QSizeF(3.5,3.5));document.history()->setClean();
}

void EditorTest::singleZoneGpuArtwork()
{
    const auto gpu=loadComponentDefinition(QString(TEST_COMPONENT_DIR)+"/gpu.aero5080.json");
    QCOMPARE(hardwareCategory(gpu),QString("Graphics cards"));const auto bounds=hardwareBodyBounds(gpu);
    QVERIFY(bounds.width()>=6);QVERIFY(bounds.height()>=2.8);QCOMPARE(bounds.center(),QPointF(0,0));
    LightingDocument document({{gpu.id,gpu}});document.addComponent(gpu.id);
    const auto before=document.componentPoints(document.state().placements[0]);
    LightingCanvas canvas(document,LightingCanvas::Context::Layout);applyStudioStyle(&canvas);canvas.resize(900,650);canvas.show();QVERIFY(QTest::qWaitForWindowExposed(&canvas));canvas.fitToView();
    const auto image=canvas.grab().toImage();int outlines=0;
    const auto center=canvas.viewportPoint({80,80});
    for(int y=qRound(center.y()-30);y<qRound(center.y()+30);++y)for(int x=qRound(center.x()-65);x<qRound(center.x()+65);++x) {
        const auto color=image.pixelColor((QPointF(x,y)*image.devicePixelRatio()).toPoint());
        if(std::abs(color.red()-146)+std::abs(color.green()-162)+std::abs(color.blue()-186)<30)++outlines;
    }
    QVERIFY(outlines>100);QCOMPARE(document.componentPoints(document.state().placements[0]),before);
    QVERIFY(canvas.grab().save(artifactDirectory()+"/single-zone-aero.png"));document.history()->setClean();
}

void EditorTest::modelLabelsAndFullsizeKeyboard()
{
    auto vulcan=loadComponentDefinition(QString(TEST_COMPONENT_DIR)+"/peripheral-vulcan.json");
    auto apex=loadComponentDefinition(QString(TEST_COMPONENT_DIR)+"/peripheral-apex3.json");
    auto other=vulcan;other.id="custom.vulcan";other.name="Roccat Vulcan 100/120 AIMO #9";
    LightingDocument document({{vulcan.id,vulcan},{apex.id,apex},{other.id,other}});
    const auto first=document.addComponent(vulcan.id);QCOMPARE(document.componentName(document.state().placements[0]),QString("Roccat Vulcan 100/120 AIMO"));
    document.addComponent(apex.id);QCOMPARE(document.componentName(document.state().placements[1]),QString("SteelSeries Apex 3"));
    document.addComponent(other.id);QCOMPARE(document.componentName(document.state().placements[0]),QString("Roccat Vulcan 100/120 AIMO #1"));QCOMPARE(document.componentName(document.state().placements[2]),QString("Roccat Vulcan 100/120 AIMO #2"));
    document.history()->undo();QCOMPARE(document.componentName(document.state().placements[0]),QString("Roccat Vulcan 100/120 AIMO"));
    for(const auto& placement:document.state().placements)QCOMPARE(placement.rotation,0.);
    const auto picture=[](const ComponentDefinition& component){
        QImage image(810,270,QImage::Format_ARGB32_Premultiplied);image.fill(Qt::transparent);
        QPainter painter(&image);const auto bounds=hardwareBodyBounds(component);
        painter.scale(810/bounds.width(),270/bounds.height());painter.translate(-bounds.topLeft());
        drawHardwareComponentArtwork(painter,component);return image;
    };
    const auto a=picture(vulcan),b=picture(apex);int difference=0;
    for(int y=0;y<a.height();++y)for(int x=0;x<a.width();++x)if(a.pixel(x,y)!=b.pixel(x,y))++difference;
    QVERIFY2(difference<100,"Fullsize models must render the same keyboard artwork after normalizing size");
    document.edit("Separate keyboards",[](auto& state){state.placements[0].origin={80,100};state.placements[0].scale={3,3};state.placements[1].origin={80,270};state.placements[1].scale={3,3};});
    LightingCanvas canvas(document,LightingCanvas::Context::Layout);applyStudioStyle(&canvas);canvas.resize(1200,800);canvas.show();QVERIFY(QTest::qWaitForWindowExposed(&canvas));canvas.fitToView();canvas.select(first);canvas.setShowLabels(true);
    QVERIFY(canvas.grab().save(artifactDirectory()+"/fullsize-model-labels.png"));document.history()->setClean();
}

void EditorTest::dedicatedDeviceIsNotAChain()
{
    const auto apex=loadComponentDefinition(QString(TEST_COMPONENT_DIR)+"/peripheral-apex3.json");
    const auto profile=DeviceProfile::load(QString(TEST_COMPONENT_DIR)+"/../profiles/apex3-161a.json");
    ProfileCandidate candidate;candidate.profileId=profile.id;candidate.device.path="fixture://apex";
    LightingWindow window({{apex.id,apex}},[candidate]{return QVector<ProfileCandidate>{candidate};},nullptr,{profile});window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto* tree=window.findChild<QTreeWidget*>("layoutComponents");QTRY_COMPARE(tree->topLevelItemCount(),1);
    auto* device=tree->topLevelItem(0);QCOMPARE(device->childCount(),0);QCOMPARE(device->text(0),QString("SteelSeries Apex 3"));
    QCOMPARE(window.document().state().assignments.size(),1);QCOMPARE(window.document().state().placements.size(),1);
    QCOMPARE(device->data(0,Qt::UserRole).toUuid(),window.document().state().placements[0].id);
    QCOMPARE(window.document().state().placements[0].rotation,0.);
    QTest::mouseClick(tree->viewport(),Qt::LeftButton,Qt::NoModifier,tree->visualItemRect(device).center());QTRY_COMPARE(window.findChild<LightingCanvas*>("layoutCanvas")->selection(),window.document().state().placements[0].id);
    bool rotation=false,oldName=false;for(auto* label:window.findChildren<QLabel*>()){rotation|=label->text()=="Rotation";oldName|=label->text()=="Rotation deg";}
    QVERIFY(rotation);QVERIFY(!oldName);
    auto* settings=window.findChild<QPushButton*>("headerSettings");QCOMPARE(settings->text(),QString("Device settings"));
    bool seen=false,appendVisible=true,removeEnabled=true;QTimer::singleShot(0,[&] {
        if(auto* dialog=dynamic_cast<ChannelEditor*>(QApplication::activeModalWidget())) {
            seen=true;appendVisible=dialog->findChild<QPushButton*>("appendInstance")->isVisible();
            removeEnabled=dialog->findChild<QPushButton*>("removeInstance")->isEnabled();dialog->reject();
        }
    });
    settings->click();QVERIFY(seen);QVERIFY(!appendVisible);QVERIFY(!removeEnabled);window.document().history()->setClean();
}

void EditorTest::fixedDeviceSettings_data()
{
    QTest::addColumn<QString>("profileFile");QTest::addColumn<QString>("channelId");
    QTest::newRow("keyboard")<<QString("apex3-161a.json")<<QString("SteelSeriesApex3");
    QTest::newRow("mouse")<<QString("basilisk-0099.json")<<QString("RazerBasiliskV3");
    QTest::newRow("ram")<<QString("xpg-ram-71.json")<<QString("XpgRam1");
    QTest::newRow("gpu")<<QString("gigabyte-aero-5080.json")<<QString("GigabyteAero5080");
    QTest::newRow("integrated-zone")<<QString("msi-mpg-b850-edge-ti-wifi.json")<<QString("IOShield");
}
void EditorTest::fixedDeviceSettings()
{
    QFETCH(QString,profileFile);QFETCH(QString,channelId);
    const auto profile=DeviceProfile::load(QString(TEST_COMPONENT_DIR)+"/../profiles/"+profileFile);
    const auto channel=assignmentChannel(profile,channelId);QVERIFY(channelHasFixedComponent(channel));
    auto components=loadComponentCatalog(TEST_COMPONENT_DIR);const auto componentId=channel.address["componentId"].toString();
    LightingDocument document(components,nullptr,{profile});const auto device=document.addComponent(componentId);
    document.addComponent("generic.strip-8");
    ChannelAssignment initial;initial.profileId=profile.id;initial.endpoint="fixture://device";initial.channelId=channel.id;initial.software=true;initial.placements={device};document.assignChannel(initial);
    ChannelEditor editor(document,initial);editor.show();QVERIFY(QTest::qWaitForWindowExposed(&editor));QVERIFY(editor.windowTitle().startsWith("Device settings"));
    auto* chain=editor.findChild<QListWidget*>("channelChain");QVERIFY(!chain->isVisible());QVERIFY(!chain->isEnabled());QCOMPARE(chain->dragDropMode(),QAbstractItemView::NoDragDrop);
    QVERIFY(!editor.findChild<QComboBox*>("availableInstances")->isVisible());
    for(const auto* name:{"appendInstance","removeInstance","instanceUp","instanceDown"}) {
        auto* button=editor.findChild<QPushButton*>(name);QVERIFY(!button->isVisible());QVERIFY(!button->isEnabled());button->click();
    }
    QCOMPARE(editor.assignment().placements,QVector<QUuid>{device});
    // Hidden-list mutation must not bypass the fixed-device contract when accepting settings.
    chain->clear();auto* item=new QListWidgetItem("Wrong component",chain);item->setData(Qt::UserRole,document.state().placements.last().id);
    editor.findChild<QDoubleSpinBox*>("channelRate")->setValue(2);
    QCOMPARE(editor.assignment().placements,QVector<QUuid>{device});QCOMPARE(editor.assignment().timeScale,2.);
    auto* mode=editor.findChild<QComboBox*>("channelMode");
    if(mode->findData(false)>=0) {mode->setCurrentIndex(mode->findData(false));mode->setCurrentIndex(mode->findData(true));QVERIFY(!chain->isVisible());}
    editor.findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Ok)->click();QCOMPARE(editor.result(),int(QDialog::Accepted));
    document.assignChannel(editor.assignment());QCOMPARE(document.state().assignments.first().placements,QVector<QUuid>{device});document.history()->setClean();
}
void EditorTest::fixedDeviceRejectsChain()
{
    const auto profile=DeviceProfile::load(QString(TEST_COMPONENT_DIR)+"/../profiles/apex3-161a.json");
    ComponentDefinition half;half.id="test.five";half.name="Generic five LED strip";
    for(int i=0;i<5;++i){half.positions.append({double(i),0});half.mapping.append(i);}
    LightingDocument document({{half.id,half}},nullptr,{profile});
    const auto first=document.addComponent(half.id),second=document.addComponent(half.id);
    ChannelAssignment invalid;invalid.profileId=profile.id;invalid.endpoint="fixture://apex";invalid.channelId="SteelSeriesApex3";invalid.software=true;invalid.placements={first,second};
    // Two five-LED strips fit the ten-slot capacity, but must still be rejected as a keyboard chain.
    const int history=document.history()->index();QVERIFY_EXCEPTION_THROWN(document.assignChannel(invalid),std::invalid_argument);
    QCOMPARE(document.history()->index(),history);QVERIFY(document.state().assignments.isEmpty());document.history()->setClean();
}

void EditorTest::integerDisplayRoundTrip()
{
    ParameterSpinBox field;field.setRange(0,10);field.setDisplayUnit(60," /min");field.setValue(.25);
    QCOMPARE(field.text(),QString("15 /min"));QCOMPARE(field.value(),.25);
    field.setValue(.123456789);const auto exact=field.value();QVERIFY(field.text().contains(QChar(0x2248)));
    field.show();field.setFocus();QTest::keyClick(&field,Qt::Key_Tab);QCOMPARE(field.value(),exact);
    field.setFocus();QTest::keyClick(&field,Qt::Key_A,Qt::ControlModifier);QTest::keyClicks(&field,"30");QTest::keyClick(&field,Qt::Key_Return);QCOMPARE(field.value(),.5);
    field.setDisplayUnit(1000," ms");field.setValue(.375);QCOMPARE(field.text(),QString("375 ms"));
    QTest::keyClick(&field,Qt::Key_Up);QCOMPARE(field.value(),.376);
    LightingWindow window(catalog(),[]{return QVector<ProfileCandidate>{};});addEffect(window,EffectKind::Circle);
    const double precise=.12345678901234567;
    window.document().edit("Legacy precision",[=](auto& state){auto& layer=state.layers[0];layer.pulse=precise;layer.breathe=.8765432109876543;layer.center={123.456789012345,210.987654321};layer.trails=.000123456789;});
    const auto original=window.document().state().layers[0];const auto revision=window.document().revision();
    window.findChild<QAction*>("workspaceEffects")->trigger();window.findChild<QTabWidget*>("layerInspectorTabs")->setCurrentIndex(1);
    QCOMPARE(window.document().revision(),revision);QCOMPARE(window.document().state().layers[0].pulse,original.pulse);
    QCOMPARE(window.findChild<QComboBox*>("intensityMode")->currentText(),QString("Combined"));
    window.findChild<QDoubleSpinBox*>("animationRate0")->setValue(42);
    QCOMPARE(window.document().state().layers[0].pulse,original.pulse);QCOMPARE(window.document().state().layers[0].center,original.center);
    QTemporaryDir dir;window.document().save(dir.filePath("exact.json"));LightingDocument restored(catalog());restored.load(dir.filePath("exact.json"));
    QCOMPARE(restored.state().layers[0].pulse,original.pulse);QCOMPARE(restored.state().layers[0].trails,original.trails);
    const auto assignment=channelProfile();ChannelAssignment channel;channel.profileId=assignment.id;channel.channelId=assignment.channels.first().id;channel.endpoint="test";channel.timeScale=precise;channel.timeOffset=precise;
    LightingDocument document(catalog(),nullptr,{assignment});ChannelEditor editor(document,channel);QCOMPARE(editor.assignment().timeScale,precise);QCOMPARE(editor.assignment().timeOffset,precise);
    window.document().history()->setClean();
}

void EditorTest::liveColorPicker()
{
    LightingWindow window(catalog(),[]{return QVector<ProfileCandidate>{};});addEffect(window,EffectKind::Circle);window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));
    window.findChild<QAction*>("workspaceEffects")->trigger();window.findChild<QPushButton*>("playPreview")->setChecked(false);
    auto* color=window.findChild<ColorEditor*>("sourceColor");const auto original=window.document().state().layers[0].color;auto* history=window.document().history();history->setClean();const int before=history->count();
    color->findChild<QPushButton*>()->click();auto* popup=color->findChild<QDialog*>("liveColorPicker");QVERIFY(popup);QVERIFY(!popup->isModal());
    popup->findChild<QSpinBox*>("pickerSaturation")->setValue(100);popup->findChild<QSpinBox*>("pickerHue")->setValue(210);
    QVERIFY(window.document().state().layers[0].color!=original);
    for(auto* button:popup->findChildren<QPushButton*>())if(button->text()=="Screen eyedropper..."){button->click();break;}
    auto* screenDialog=window.findChild<QDialog*>("screenColorDialog");QVERIFY(screenDialog);screenDialog->reject();
    QVERIFY(popup->isVisible());QTest::keyClick(popup,Qt::Key_Escape);QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
    QCOMPARE(window.document().state().layers[0].color,original);QCOMPARE(history->count(),before);QVERIFY(history->isClean());
    color->findChild<QPushButton*>()->click();popup=color->findChild<QDialog*>("liveColorPicker");QVERIFY(popup);
    popup->findChild<QSpinBox*>("pickerSaturation")->setValue(100);popup->findChild<QSpinBox*>("pickerHue")->setValue(120);
    popup->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Apply)->click();QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
    QCOMPARE(history->count(),before+1);QCOMPARE(window.document().state().layers[0].color,QColor(Qt::green));history->undo();QCOMPARE(window.document().state().layers[0].color,original);
    // A canceled preview must preserve an existing redo branch, including autosave.
    QVERIFY(history->canRedo());const int priorIndex=history->index(),priorCount=history->count();
    color->findChild<QPushButton*>()->click();popup=color->findChild<QDialog*>("liveColorPicker");
    popup->findChild<QSpinBox*>("pickerSaturation")->setValue(100);popup->findChild<QSpinBox*>("pickerHue")->setValue(240);
    QTemporaryDir snapshot;window.document().save(snapshot.filePath("autosave.json"),false);LightingDocument saved(catalog());saved.load(snapshot.filePath("autosave.json"));QCOMPARE(saved.state().layers[0].color,original);
    popup->reject();QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
    QCOMPARE(history->index(),priorIndex);QCOMPARE(history->count(),priorCount);QVERIFY(history->canRedo());history->redo();QCOMPARE(window.document().state().layers[0].color,QColor(Qt::green));
    window.document().history()->setClean();
}

void EditorTest::auditWorkspace()
{
    LightingWindow window(catalog(),[]{return QVector<ProfileCandidate>{};});window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));
    QVERIFY(window.findChild<QLabel*>("studioBrand"));QVERIFY(window.findChild<QAction*>("aboutLightingStudio"));QVERIFY(window.findChild<QMenu*>("addLayerMenu"));
    addEffect(window,EffectKind::Rain);window.findChild<QAction*>("workspaceEffects")->trigger();QCoreApplication::processEvents();
    auto* inspector=window.findChild<QTabWidget*>("layerInspectorTabs");inspector->setCurrentIndex(0);QVERIFY(window.findChild<QWidget*>("particleParameters")->isVisible());
    QVERIFY(!window.findChild<QAction*>("showComponentLabels")->isVisible());QVERIFY(!window.findChild<QCheckBox*>("allLayerMotion"));
    QVERIFY(window.findChild<QPushButton*>("randomizeParticleSeed"));QVERIFY(window.findChild<QDoubleSpinBox*>("particleSeed")->isVisible());
    inspector->setCurrentIndex(1);QCoreApplication::processEvents();QSettings().setValue("editor/reduceMotion",false);
    auto* disclosure=window.findChild<QWidget*>("advancedMotion")->findChild<QToolButton*>();auto* turn=disclosure->findChild<QVariantAnimation*>();QVERIFY(turn);QSignalSpy motion(turn,&QVariantAnimation::stateChanged);
    QTest::mouseClick(disclosure,Qt::LeftButton);QVERIFY(!motion.isEmpty());QTRY_COMPARE(disclosure->property("arrowAngle").toDouble(),90.0);
    QSettings().setValue("editor/reduceMotion",true);QTest::mouseClick(disclosure,Qt::LeftButton);QCOMPARE(turn->state(),QAbstractAnimation::Stopped);QCOMPARE(disclosure->property("arrowAngle").toDouble(),0.0);
    auto* spacing=window.findChild<QSpinBox*>("gridSpacing");const int before=spacing->value();
    if(auto* stop=window.findChild<QSpinBox*>("paletteSnapPercent"))stop->setValue(5);QCOMPARE(spacing->value(),before);
    const auto revision=window.document().revision();auto* opacity=window.findChild<QSlider*>("layerTransparency");QCOMPARE(opacity->value(),100);
    opacity->setValue(25);QCOMPARE(window.document().state().layers[0].transparency,75);window.document().history()->undo();QCOMPARE(window.document().revision(),revision+2);
    window.resize(1060,720);QCoreApplication::processEvents();QVERIFY(window.findChild<QToolButton*>("workspacePanels")->isVisible());
    window.document().history()->setClean();
}

void EditorTest::objectLayersAndGestures()
{
    LightingWindow window(catalog(),[]{return QVector<ProfileCandidate>{};});window.resize(1440,900);window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));
    window.findChild<QAction*>("workspaceEffects")->trigger();window.findChild<QPushButton*>("playPreview")->setChecked(false);
    window.findChild<QToolButton*>("addLayer")->click();
    auto& document=window.document();QCOMPARE(document.state().layers.size(),1);QVERIFY(document.state().layers[0].container);
    const auto root=document.state().layers[0].id;
    auto* canvas=window.findChild<LightingCanvas*>("effectsCanvas");
    auto drag=[&](QPointF from,QPointF to){QTest::mousePress(canvas,Qt::LeftButton,Qt::NoModifier,canvas->viewportPoint(from).toPoint());QTest::mouseMove(canvas,canvas->viewportPoint(to).toPoint());QTest::mouseRelease(canvas,Qt::LeftButton,Qt::NoModifier,canvas->viewportPoint(to).toPoint());};
    window.findChild<QPushButton*>("ellipseTool")->click();
    const auto revision=document.revision();QTest::mouseClick(canvas,Qt::LeftButton,Qt::NoModifier,canvas->viewportPoint({320,200}).toPoint());QCOMPARE(document.revision(),revision);QCOMPARE(document.state().layers.size(),1);
    drag({150,100},{300,250});QCOMPARE(document.state().layers.size(),2);QCOMPARE(document.state().layers.last().parentId,root);QCOMPARE(document.state().layers.last().effect,EffectKind::Circle);
    const auto first=document.state().layers.last().id;
    drag({340,120},{460,260});QCOMPARE(document.state().layers.size(),3);QCOMPARE(document.state().layers.last().parentId,root);
    auto* tree=window.findChild<QTreeWidget*>("effectLayers");QCOMPARE(tree->topLevelItemCount(),1);QCOMPARE(tree->topLevelItem(0)->childCount(),2);
    QTest::mouseClick(canvas,Qt::LeftButton,Qt::NoModifier,canvas->viewportPoint({220,180}).toPoint());QCOMPARE(canvas->selection(),first);QCOMPARE(tree->currentItem()->data(0,Qt::UserRole).toUuid(),first);
    window.findChild<QPushButton*>("gradientTool")->click();canvas->setGradientStops({Qt::red,Qt::blue},{0,1});drag({170,180},{280,180});
    QCOMPARE(document.state().layers.size(),3);QVERIFY(document.state().layers[1].gradientFill);QCOMPARE(document.state().layers[1].effect,EffectKind::Circle);
    const auto before=document.render();QCOMPARE(before.pixelColor(20,20).alpha(),0);QVERIFY(before.pixelColor(200,180).red()>before.pixelColor(270,180).red());
    document.history()->undo();QVERIFY(!document.state().layers[1].gradientFill);document.history()->redo();QCOMPARE(document.render(),before);
    window.findChild<QPushButton*>("rectangleTool")->click();const int count=document.history()->count();
    QTest::mousePress(canvas,Qt::LeftButton,Qt::NoModifier,canvas->viewportPoint({50,40}).toPoint());QTest::mouseMove(canvas,canvas->viewportPoint({100,90}).toPoint());QTest::keyClick(canvas,Qt::Key_Escape);QTest::mouseRelease(canvas,Qt::LeftButton,Qt::NoModifier,canvas->viewportPoint({100,90}).toPoint());
    QCOMPARE(document.history()->count(),count);QCOMPARE(document.state().layers.size(),3);QCOMPARE(document.render(),before);
    document.edit("Group opacity",[&](auto& state){state.layers[0].transparency=50;});QCOMPARE(document.render().pixelColor(200,180).alpha(),128);document.history()->undo();QCOMPARE(document.render(),before);
    QTemporaryDir directory;const auto path=directory.filePath("objects.json");document.save(path);LightingDocument loaded(catalog());loaded.load(path);QCOMPARE(loaded.render(),before);QCOMPARE(loaded.state().layers[1].parentId,root);
    QVERIFY_EXCEPTION_THROWN(document.edit("Invalid parent",[](auto& state){state.layers[1].parentId=QUuid::createUuid();}),std::invalid_argument);QCOMPARE(document.render(),before);
    QVERIFY(!window.findChild<QCheckBox*>("focusLiveHardware"));QVERIFY(!window.findChild<QAction*>("fitCanvas")->icon().isNull());QVERIFY(!window.findChild<QAction*>("zoomIn")->icon().isNull());QVERIFY(!window.findChild<QAction*>("zoomOut")->icon().isNull());
    window.findChild<QAction*>("workspaceLayout")->trigger();QVERIFY(!window.findChild<QComboBox*>("previewChannel")->isVisible());
    window.findChild<QAction*>("workspaceEffects")->trigger();QCoreApplication::processEvents();QVERIFY(window.grab().save(artifactDirectory()+"/object-layers.png"));
    document.history()->setClean();
}

void EditorTest::routedCompositionCache()
{
    LightingDocument document(catalog(),nullptr,{channelProfile()});
    const auto first=document.addComponent("test.strip"),second=document.addComponent("test.strip");
    ChannelAssignment a;a.endpoint="first";a.profileId="test.device";a.channelId="Direct";a.software=true;a.placements={first};document.assignChannel(a);
    auto b=a;b.id=QUuid::createUuid();b.endpoint="second";b.placements={second};document.assignChannel(b);
    const auto root=document.addEmptyLayer();const auto red=document.addLayer(EffectKind::StaticColor,root),blue=document.addLayer(EffectKind::StaticColor,root);
    document.edit("Route composition",[&](auto& state){for(auto& item:state.layers){if(item.id==root){item.allChannels=false;item.channelTargets={a.id};}if(item.id==red)item.color=Qt::red;if(item.id==blue)item.color=Qt::blue;}});
    const auto global=document.render(),routed=document.renderForChannel(a.id,0),other=document.renderForChannel(b.id,0);
    QCOMPARE(global,routed);QCOMPARE(global.pixelColor(320,200),QColor(Qt::blue));QCOMPARE(other.pixelColor(320,200).alpha(),0);
    QCOMPARE(routed.cacheKey(),document.renderForChannel(a.id,0).cacheKey());QCOMPARE(global.cacheKey(),routed.cacheKey());
    auto samples=document.sampleChannels(global);QCOMPARE(samples[a.id][0],QColor(Qt::blue));QCOMPARE(samples[b.id][0],QColor(Qt::black));
    document.edit("Swap objects",[](auto& state){state.layers.swapItemsAt(1,2);});QCOMPARE(document.render().pixelColor(320,200),QColor(Qt::red));
    document.history()->undo();QCOMPARE(document.render(),global);
    document.edit("Common clock",[&](auto& state){state.layers[0].allChannels=true;state.layers[0].channelTargets.clear();});
    QCOMPARE(document.renderForChannel(a.id,1).cacheKey(),document.renderForChannel(b.id,1).cacheKey());
    QElapsedTimer timer;timer.start();for(int i=0;i<30;++i){const auto frame=document.render(i/60.0);document.sampleChannels(frame,i/60.0);}qInfo()<<"Scoped/shared 30 frames:"<<timer.elapsed()<<"ms";
}
