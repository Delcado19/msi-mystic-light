#include "components/ComponentCatalog.h"
#include <QtTest>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <stdexcept>
#include <limits>

class ComponentsTest : public QObject {
    Q_OBJECT
    QJsonObject definition() const {
        return {{"schemaVersion", 1}, {"id", "test"}, {"name", "Test strip"},
            {"ledCount", 2}, {"mapping", QJsonArray{1, 0}},
            {"positions", QJsonArray{QJsonArray{0, 0}, QJsonArray{1, 0}}}};
    }
private slots:
    void componentLayersJson() {
        // Display-only depth layers (owner request 2026-11-25): optional, never written when all zero.
        auto plain = ComponentDefinition::fromJson(definition());
        QVERIFY(plain.layers.isEmpty()); QCOMPARE(plain.layerOf(1), 0);
        QVERIFY(!plain.toJson().contains("layers"));
        auto layered = definition(); layered["layers"] = QJsonArray{0, 1};
        const auto parsed = ComponentDefinition::fromJson(layered);
        QCOMPARE(parsed.layers, (QVector<int>{0, 1})); QCOMPARE(parsed.layerOf(1), 1);
        QCOMPARE(ComponentDefinition::fromJson(parsed.toJson()).layers, (QVector<int>{0, 1}));
        auto zeros = definition(); zeros["layers"] = QJsonArray{0, 0};
        QVERIFY(ComponentDefinition::fromJson(zeros).layers.isEmpty());
        QVERIFY(!ComponentDefinition::fromJson(zeros).toJson().contains("layers"));
        for (const auto& bad : {QJsonValue(QJsonArray{0}), QJsonValue(QJsonArray{0, 16}), QJsonValue(QJsonArray{-1, 0}),
                                QJsonValue(QJsonArray{0, 0.5}), QJsonValue("1")}) {
            auto invalid = definition(); invalid["layers"] = bad;
            QVERIFY_EXCEPTION_THROWN(ComponentDefinition::fromJson(invalid), std::invalid_argument);
        }
        QCOMPARE(overlapLayers({{0, 0}, {1, 0}, {0, 0}, {0, 0}}), (QVector<int>{0, 0, 1, 2}));
        QVERIFY(overlapLayers({{0, 0}, {1, 0}}).isEmpty());
    }
    void parseAndMap() {
        const auto component = ComponentDefinition::fromJson(definition());
        QCOMPARE(component.positions[1], QPointF(1, 0));
        ComponentCatalog catalog{{component.id, component}};
        ChannelProfile channel;
        channel.maxLeds = 4;
        QCOMPARE(channelMapping(channel, {"test", "test"}, catalog), (QVector<int>{1, 0, 3, 2}));
        QVERIFY(channelMapping(channel, {}, catalog).isEmpty());
        // Explicit one-based positions may share addresses; 0 continues after the previous component.
        QCOMPARE(channelMapping(channel, {"test", "test"}, catalog, {1, 1}), (QVector<int>{1, 0, 1, 0}));
        QCOMPARE(channelMapping(channel, {"test", "test"}, catalog, {3, 1}), (QVector<int>{3, 2, 1, 0}));
        QCOMPARE(chainOffsets({2, 2, 2}, {0, 1, 0}), (QVector<int>{0, 0, 2}));
        // JARGB2 shape: two fans and a longer strip all starting at LED 1.
        const auto sharing = chainSharing({6, 6, 15}, {1, 1, 1});
        QCOMPARE(sharing[0].size(), 1); QCOMPARE(sharing[0][0].source, 0);
        QCOMPARE(sharing[1].size(), 1); QCOMPARE(sharing[1][0].source, 0); QCOMPARE(sharing[1][0].lastLed, 6);
        QCOMPARE(sharing[2].size(), 2); QCOMPARE(sharing[2][0].lastLed, 6); QCOMPARE(sharing[2][0].source, 0);
        QCOMPARE(sharing[2][1].firstLed, 7); QCOMPARE(sharing[2][1].lastLed, 15); QCOMPARE(sharing[2][1].source, 2);
        QCOMPARE(chainSharing({2, 2}, {})[1][0].source, 1);
        QVERIFY_EXCEPTION_THROWN(channelMapping(channel, {"test", "test"}, catalog, {4, 0}), std::invalid_argument);
        QVERIFY_EXCEPTION_THROWN(channelMapping(channel, {"test", "test"}, catalog, {1}), std::invalid_argument);
        QVERIFY_EXCEPTION_THROWN(chainOffsets({2}, {-1}), std::invalid_argument);
        channel.fixedLeds = 4;
        // A fixed count is the occupied address range, so two mirrored components do not fill it.
        QVERIFY_EXCEPTION_THROWN(channelMapping(channel, {"test", "test"}, catalog, {1, 1}), std::invalid_argument);
        QCOMPARE(channelMapping(channel, {"test", "test"}, catalog).size(), 4);
        QVERIFY_EXCEPTION_THROWN(channelMapping(channel, {"test"}, catalog), std::invalid_argument);
        channel.fixedLeds = 0;
        QVERIFY_EXCEPTION_THROWN(channelMapping(channel, {"test", "test", "test"}, catalog), std::invalid_argument);
        QVERIFY_EXCEPTION_THROWN(channelMapping(channel, {"missing"}, catalog), std::invalid_argument);
        catalog["test"].mapping = {2, 0};
        QVERIFY_EXCEPTION_THROWN(channelMapping(channel, {"test"}, catalog), std::invalid_argument);
    }
    void reservedSlots() {
        auto json=definition(); json["nonPhysicalSlots"]=QJsonArray{1};
        const auto component=ComponentDefinition::fromJson(json);
        QCOMPARE(ComponentDefinition::fromJson(component.toJson()).nonPhysicalSlots,QVector<int>{1});
        for(const auto invalid : {QJsonArray{1,1},QJsonArray{2},QJsonArray{0,1},QJsonArray{.5}}) {
            json["nonPhysicalSlots"]=invalid;
            QVERIFY_EXCEPTION_THROWN(ComponentDefinition::fromJson(json),std::invalid_argument);
        }
        json["nonPhysicalSlots"]="bad";
        QVERIFY_EXCEPTION_THROWN(ComponentDefinition::fromJson(json),std::invalid_argument);
    }
    void rejectInvalidDefinitions() {
        for (const auto& key : {"schemaVersion", "id", "name", "ledCount", "mapping", "positions"}) {
            auto json = definition(); json.remove(key);
            QVERIFY_EXCEPTION_THROWN(ComponentDefinition::fromJson(json), std::invalid_argument);
        }
        for (const auto bad : {QJsonArray{0, 0}, QJsonArray{0, 2}, QJsonArray{0}, QJsonArray{0, 0.5}}) {
            auto json = definition(); json["mapping"] = bad;
            QVERIFY_EXCEPTION_THROWN(ComponentDefinition::fromJson(json), std::invalid_argument);
        }
        auto json = definition(); json["positions"] = QJsonArray{QJsonArray{0, "bad"}, QJsonArray{1, 0}};
        QVERIFY_EXCEPTION_THROWN(ComponentDefinition::fromJson(json), std::invalid_argument);
        json = definition(); json["ledCount"] = 2.5;
        QVERIFY_EXCEPTION_THROWN(ComponentDefinition::fromJson(json), std::invalid_argument);
        json = definition(); json["schemaVersion"] = 2;
        QVERIFY_EXCEPTION_THROWN(ComponentDefinition::fromJson(json), std::invalid_argument);
    }
    void numericAndFileBoundaries() {
        auto json=definition();
        for(const auto bad : {QJsonValue(-1),QJsonValue(0),QJsonValue(65536),QJsonValue(1.5),QJsonValue("2"),QJsonValue(true),QJsonValue(QJsonValue::Null)}) {
            json=definition();json["ledCount"]=bad;QVERIFY_EXCEPTION_THROWN(ComponentDefinition::fromJson(json),std::invalid_argument);
        }
        json=definition();json["ledCount"]=1;json["mapping"]=QJsonArray{0};json["positions"]=QJsonArray{QJsonValue(QJsonArray{-1,0})};
        QVERIFY(json["positions"].toArray()[0].isArray());
        QCOMPARE(ComponentDefinition::fromJson(json).positions,QVector<QPointF>({{-1,0}}));
        QJsonArray positions,mapping;for(int i=0;i<65535;++i){positions.append(QJsonArray{0,0});mapping.append(i);}
        json["ledCount"]=65535;json["mapping"]=mapping;json["positions"]=positions;json["nonPhysicalSlots"]=QJsonArray{65534};
        const auto maximum=ComponentDefinition::fromJson(json);QCOMPARE(maximum.mapping.size(),65535);QCOMPARE(maximum.mapping.last(),65534);QCOMPARE(maximum.nonPhysicalSlots,QVector<int>{65534});
        for(double bad : {std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity()}) {
            json=definition();json["positions"]=QJsonArray{QJsonArray{bad,0},QJsonArray{1,0}};
            QVERIFY_EXCEPTION_THROWN(ComponentDefinition::fromJson(json),std::invalid_argument);
        }
        for(const auto bad : {QJsonValue(-1),QJsonValue(2),QJsonValue(.5),QJsonValue("0"),QJsonValue(QJsonValue::Null)}) {
            json=definition();json["nonPhysicalSlots"]=QJsonArray{bad};QVERIFY_EXCEPTION_THROWN(ComponentDefinition::fromJson(json),std::invalid_argument);
        }
        QTemporaryDir directory;const auto path=directory.filePath("limit.json");
        auto bytes=QJsonDocument(definition()).toJson(QJsonDocument::Compact);bytes.append(QByteArray(1024*1024-bytes.size(),' '));
        QFile file(path);QVERIFY(file.open(QIODevice::WriteOnly));QCOMPARE(file.write(bytes),qint64(1024*1024));file.close();
        QCOMPARE(loadComponentDefinition(path).mapping,QVector<int>({1,0}));
        QVERIFY(file.open(QIODevice::Append));QCOMPARE(file.write(" "),qint64(1));file.close();
        QVERIFY_EXCEPTION_THROWN(loadComponentDefinition(path),std::invalid_argument);
    }
    void catalogBoundary() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(loadComponentCatalog(dir.path()).isEmpty());
        QVERIFY_EXCEPTION_THROWN(loadComponentCatalog(dir.filePath("missing")), std::invalid_argument);
        auto write = [&](const QString& name, const QByteArray& bytes) {
            QFile file(dir.filePath(name));
            QVERIFY(file.open(QIODevice::WriteOnly)); QCOMPARE(file.write(bytes), bytes.size());
        };
        const auto bytes = QJsonDocument(definition()).toJson();
        write("a.json", bytes);
        QCOMPARE(loadComponentCatalog(dir.path()).size(), 1);
        write("b.json", bytes);
        QVERIFY_EXCEPTION_THROWN(loadComponentCatalog(dir.path()), std::invalid_argument);
        write("b.json", "{");
        QVERIFY_EXCEPTION_THROWN(loadComponentCatalog(dir.path()), std::invalid_argument);
        write("b.json", QByteArray(1024 * 1024 + 1, ' '));
        QVERIFY_EXCEPTION_THROWN(loadComponentCatalog(dir.path()), std::invalid_argument);
        const auto bundled = loadComponentCatalog(TEST_COMPONENT_DIR);
        QCOMPARE(bundled.value("generic.strip-8").mapping.size(), 8);
    }
};
QTEST_APPLESS_MAIN(ComponentsTest)
#include "test_components.moc"
