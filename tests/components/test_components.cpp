#include "components/ComponentCatalog.h"
#include <QtTest>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <stdexcept>

class ComponentsTest : public QObject {
    Q_OBJECT
    QJsonObject definition() const {
        return {{"schemaVersion", 1}, {"id", "test"}, {"name", "Test strip"},
            {"ledCount", 2}, {"mapping", QJsonArray{1, 0}},
            {"positions", QJsonArray{QJsonArray{0, 0}, QJsonArray{1, 0}}}};
    }
private slots:
    void parseAndMap() {
        const auto component = ComponentDefinition::fromJson(definition());
        QCOMPARE(component.positions[1], QPointF(1, 0));
        ComponentCatalog catalog{{component.id, component}};
        ChannelProfile channel;
        channel.maxLeds = 4;
        QCOMPARE(channelMapping(channel, {"test", "test"}, catalog), (QVector<int>{1, 0, 3, 2}));
        QVERIFY(channelMapping(channel, {}, catalog).isEmpty());
        channel.fixedLeds = 4;
        QCOMPARE(channelMapping(channel, {"test", "test"}, catalog).size(), 4);
        QVERIFY_EXCEPTION_THROWN(channelMapping(channel, {"test"}, catalog), std::invalid_argument);
        channel.fixedLeds = 0;
        QVERIFY_EXCEPTION_THROWN(channelMapping(channel, {"test", "test", "test"}, catalog), std::invalid_argument);
        QVERIFY_EXCEPTION_THROWN(channelMapping(channel, {"missing"}, catalog), std::invalid_argument);
        catalog["test"].mapping = {2, 0};
        QVERIFY_EXCEPTION_THROWN(channelMapping(channel, {"test"}, catalog), std::invalid_argument);
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
