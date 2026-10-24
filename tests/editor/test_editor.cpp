#include "editor/LightingWindow.h"
#include "editor/ComponentEditor.h"
#include "editor/ChannelEditor.h"
#include "editor/AppSession.h"
#include "editor/StudioWidgets.h"
#include "editor/HardwarePicker.h"
#include "editor/LegacyHardware.h"
#include <QStackedWidget>
#include <QToolButton>
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
#include <QElapsedTimer>
#include <QLineEdit>
#include <QLineF>
#include <QGraphicsView>
#include <QScrollBar>
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
private slots:
    void drawingAndAnimationRegressions();
    void restoredLayoutAndDrawing();
    void legacyHardware();
    void periodicEffectsAndGradientStops();
    void globalColorControls();
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
    write("valid",source); auto invalid=source; invalid.insert("ProductName","Malformed mapping"); invalid.insert("LedMapping",QJsonArray{2,2,0}); write("invalid",invalid);
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
    window.findChild<QCheckBox*>("allLayerMotion")->setChecked(true); window.findChild<QDoubleSpinBox*>("driftX")->setValue(80);
    QCOMPARE(window.document().state().layers[0].driftX,80.0); QCOMPARE(window.document().state().layers[1].driftX,80.0);
    window.findChild<QCheckBox*>("enableColorCycle")->setChecked(true); QVERIFY(window.findChild<QPushButton*>("playPreview")->isChecked());
    for(const auto& source:window.document().state().layers) QVERIFY(source.cycleRate>0);
    for(const auto& name:{"driftX","driftY","zoom","warp","breathe","trails","particleSeed"}) {
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
    for(int i=0;i<layers.size();++i) { auto json=layers[i].toObject(); json.remove("targets"); json.remove("gradientColors"); json.remove("gradientPositions"); layers[i]=json; }
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
    for (const auto& definition : effectDefinitions()) if(definition.kind!=EffectKind::Paint) document.addLayer(definition.kind);
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
    QCOMPARE(root.value("schemaVersion").toInt(), 12);
    // Published schema 2 has explicit effects but no image fields or Image type.
    auto version2 = root; version2.remove("components"); version2.remove("assignments"); QJsonArray version2Layers;
    for (const auto& item : root.value("layers").toArray()) {
        auto value = item.toObject();
        if (value.value("effect").toObject().value("type").toString() == "image" || isParticle(effectDefinition(value.value("effect").toObject().value("type").toString()).kind)) continue;
        value.remove("targets"); value.remove("gradientColors"); value.remove("gradientPositions"); value.remove("advanced"); value.remove("animation"); value.remove("imagePng"); value.remove("imageFill"); version2Layers.append(value);
    }
    version2.insert("schemaVersion", 2); version2.insert("layers", version2Layers);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write(QJsonDocument(version2).toJson()); file.close();
    loaded.load(path); QCOMPARE(loaded.render(), document.render());
    auto version3 = root; version3.remove("components"); version3.remove("assignments"); QJsonArray version3Layers;
    for (const auto& item : root.value("layers").toArray()) {
        auto value = item.toObject(); if (isParticle(effectDefinition(value.value("effect").toObject().value("type").toString()).kind)) continue;
        value.remove("targets"); value.remove("gradientColors"); value.remove("gradientPositions"); value.remove("advanced"); value.remove("animation"); version3Layers.append(value);
    }
    version3.insert("schemaVersion", 3); version3.insert("layers", version3Layers);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write(QJsonDocument(version3).toJson()); file.close(); loaded.load(path);
    QCOMPARE(loaded.render(), document.render());
    auto version4=root; version4.remove("components"); version4.remove("assignments"); QJsonArray version4Layers;
    for (const auto& item : root.value("layers").toArray()) {
        auto value=item.toObject(); if (isParticle(effectDefinition(value.value("effect").toObject().value("type").toString()).kind)) continue;
        value.remove("targets"); value.remove("gradientColors"); value.remove("gradientPositions"); value.remove("advanced"); version4Layers.append(value);
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
    auto legacy = layers[0].toObject(); legacy.remove("targets"); legacy.remove("gradientColors"); legacy.remove("gradientPositions"); legacy.remove("advanced"); legacy.remove("animation"); legacy.remove("effect"); legacy.remove("imagePng"); legacy.remove("imageFill");
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
    auto* play=window.findChild<QPushButton*>("playPreview");play->setChecked(false);QCOMPARE(play->text(),QString("Canvas Start"));
    QTest::mouseClick(play,Qt::LeftButton);QCOMPARE(play->text(),QString("Canvas Stop"));play->setChecked(false);
    QVERIFY(!window.findChild<QLabel*>("toolbarOutputStatus")->isVisible());
    auto* canvas=window.findChild<LightingCanvas*>("layoutCanvas");canvas->fitToView();
    auto* originalParent=canvas->parentWidget();auto* fullscreen=canvas->findChild<QPushButton*>("canvasFullscreenButton");
    QTest::mouseClick(fullscreen,Qt::LeftButton);
    auto* host=window.findChild<QDialog*>("canvasFullscreen");QVERIFY(host);QTRY_VERIFY(host->isFullScreen());
    QCOMPARE(canvas->window(),host);QVERIFY(canvas->isVisible());
    QVERIFY(host->grab().save(artifactDirectory()+"/layout-fullscreen.png"));
    auto* fullPlay=host->findChild<QPushButton*>("fullscreenCanvasPlayback");QVERIFY(fullPlay);QTest::mouseClick(fullPlay,Qt::LeftButton);QVERIFY(play->isChecked());play->setChecked(false);
    QTest::keyClick(canvas,Qt::Key_Escape);QTRY_COMPARE(canvas->parentWidget(),originalParent);QVERIFY(canvas->isVisible());
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
    // Independent world edge: centers 1800..1840 at -1200, plus 12px margin, union the base rectangle.
    QVERIFY(qAbs(canvas->viewportPoint({1852,400}).x()-(canvas->width()-vertical->width()))<=1.0);
    QVERIFY2(qAbs(canvas->viewportPoint({1852,400}).y()-(canvas->height()-horizontal->height()))<=1.0,
        qPrintable(QString("bottom=%1 viewport=%2 hbar=%3 v=%4/%5 units/world=%6")
        .arg(canvas->viewportPoint({1852,400}).y()).arg(canvas->height()).arg(horizontal->height()).arg(vertical->value()).arg(vertical->maximum()).arg(window.document().layoutBounds().height())));
    horizontal->setValue(horizontal->minimum());vertical->setValue(vertical->minimum());canvas->repaint();
    QVERIFY(qAbs(canvas->viewportPoint({0,-1212}).x())<=1.0);QVERIFY(qAbs(canvas->viewportPoint({0,-1212}).y())<=1.0);
    vertical->setValue(vertical->maximum()/2);canvas->repaint();
    const auto old=canvas->canvasPoint(QPointF(100,100));
    horizontal->setValue(horizontal->maximum());vertical->setValue(vertical->minimum());canvas->repaint();
    const auto moved=canvas->canvasPoint(QPointF(100,100));QVERIFY(moved.x()>old.x());QVERIFY(moved.y()<old.y());
    const auto bounds=window.document().layoutBounds();
    QVERIFY(qAbs(canvas->canvasPoint(QPointF(0,0)).y()-bounds.top())<3);
    // Minimap lives above/left of the scrollbar intersection, in the lower-right corner.
    const QPoint mapPoint(canvas->width()-176-vertical->width(),canvas->height()-110-horizontal->height());
    const auto before=canvas->canvasPoint(QPointF(100,100));QTest::mouseClick(canvas,Qt::LeftButton,Qt::NoModifier,mapPoint);canvas->repaint();
    QVERIFY(QLineF(before,canvas->canvasPoint(QPointF(100,100))).length()>10);
    const int value=horizontal->value();QVERIFY(value<horizontal->maximum());
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
    QTest::newRow("exact")<<QSize(640,400)<<false<<false;
    QTest::newRow("one-pixel-spare")<<QSize(641,401)<<false<<false;
    QTest::newRow("horizontal-overflow-causes-vertical")<<QSize(639,400)<<true<<true;
    QTest::newRow("vertical-overflow-causes-horizontal")<<QSize(640,399)<<true<<true;
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
        auto* control=window.findChild<QDoubleSpinBox*>(id); QVERIFY(control->isVisible());
        if(QGuiApplication::platformName()!="offscreen") QVERIFY2(!control->visibleRegion().isEmpty(),id);
    }
    QVERIFY(!window.findChild<QDoubleSpinBox*>("particleSeed")->isVisible());
    QVERIFY(window.grab().save(artifactDirectory()+"/editor-motion.png"));
    window.document().history()->setClean();
}

QTEST_MAIN(EditorTest)
#include "test_editor.moc"
