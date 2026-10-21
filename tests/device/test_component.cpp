// Unit tests for the Component data model (JSON load/save, placeholder).
// Pure data/parsing, no HID/device access.
#include <QtTest/QtTest>

#include "device/Component.h"
#include <stdexcept>

class ComponentTest : public QObject
{
    Q_OBJECT

private slots:
    void testShapesResetAndMappingValidation();
    void testMalformedCoordinatesRejected();
    void testFromJson_roundTrip();
    void testFromJson_tooShortArrayThrows();
    void testFromJson_tooLongArrayTruncates();
    void testFromJson_negativeLedCountThrows();
    void testMakeGenericPlaceholder();
};

void ComponentTest::testFromJson_roundTrip()
{
    QJsonObject obj;
    obj["ProductName"] = "Test Fan";
    obj["DisplayName"] = "Test Fan - 3 LED";
    obj["Brand"] = "Test";
    obj["Type"] = "Fan";
    obj["LedCount"] = 3;
    obj["Width"] = 3; obj["Height"] = 1;
    obj["LedMapping"] = QJsonArray{0,1,2};
    obj["LedCoordinates"] = QJsonArray{QJsonArray{0,0}, QJsonArray{1,0}, QJsonArray{2,0}};
    obj["LedNames"] = QJsonArray{"LED 1","LED 2","LED 3"};
    obj["ImageUrl"] = "";
    Component c = Component::fromJson(obj);
    QCOMPARE(c.ledCount, 3);
    QCOMPARE(static_cast<int>(c.ledCoordinates.size()), 3);
    QCOMPARE(c.ledCoordinates[1].first, 1);
    QCOMPARE(c.toJson()["ProductName"].toString(), QString("Test Fan"));
}

void ComponentTest::testFromJson_tooShortArrayThrows()
{
    QJsonObject obj;
    obj["ProductName"] = "Broken"; obj["LedCount"] = 3;
    obj["LedCoordinates"] = QJsonArray{QJsonArray{0,0}}; // only 1, needs 3
    obj["LedNames"] = QJsonArray{"A","B","C"};
    obj["LedMapping"] = QJsonArray{0,1,2};
    QVERIFY_EXCEPTION_THROWN(Component::fromJson(obj), std::invalid_argument);
}

// Controller-ruling addition (not in the brief's 3 given tests): the header
// doc comment for fromJson specifies that an array LONGER than ledCount is
// truncated rather than thrown. Exercises that branch, which none of the
// brief's given tests cover.
void ComponentTest::testFromJson_tooLongArrayTruncates()
{
    QJsonObject obj;
    obj["ProductName"] = "TooLong"; obj["LedCount"] = 2;
    obj["LedCoordinates"] = QJsonArray{QJsonArray{0,0}, QJsonArray{1,0}, QJsonArray{2,0}, QJsonArray{3,0}};
    obj["LedNames"] = QJsonArray{"A","B","C","D"};
    obj["LedMapping"] = QJsonArray{0,1,2,3};
    Component c = Component::fromJson(obj);
    QCOMPARE(c.ledCount, 2);
    QCOMPARE(static_cast<int>(c.ledCoordinates.size()), 2);
    QCOMPARE(static_cast<int>(c.ledNames.size()), 2);
    QCOMPARE(static_cast<int>(c.ledMapping.size()), 2);
    QCOMPARE(c.ledCoordinates[1].first, 1);
    QCOMPARE(QString::fromStdString(c.ledNames[1]), QString("B"));
}

// Review-mandated addition: fromJson rejects a negative LedCount with
// std::invalid_argument (added beyond the brief's literal text to keep the
// documented exception contract honest — see Component.h/.cpp). Without this
// test, that guard had zero coverage.
void ComponentTest::testFromJson_negativeLedCountThrows()
{
    QJsonObject obj;
    obj["ProductName"] = "Broken";
    obj["LedCount"] = -1;
    obj["LedCoordinates"] = QJsonArray{};
    obj["LedNames"] = QJsonArray{};
    obj["LedMapping"] = QJsonArray{};
    QVERIFY_EXCEPTION_THROWN(Component::fromJson(obj), std::invalid_argument);
}

void ComponentTest::testMakeGenericPlaceholder()
{
    Component c = Component::makeGenericPlaceholder(5);
    QCOMPARE(c.ledCount, 5);
    QCOMPARE(static_cast<int>(c.ledCoordinates.size()), 5);
    QCOMPARE(c.ledCoordinates[4].first, 4);
    QCOMPARE(c.ledCoordinates[4].second, 0);
}

void ComponentTest::testMalformedCoordinatesRejected()
{
    auto json=Component::makeGenericPlaceholder(1).toJson();
    json["LedCoordinates"]=QJsonArray{QJsonArray{2147483647,-2147483647}};
    QVERIFY_EXCEPTION_THROWN(Component::fromJson(json),std::invalid_argument);
    json["LedCoordinates"]=QJsonArray{QJsonArray{"wrong",1}};
    QVERIFY_EXCEPTION_THROWN(Component::fromJson(json),std::invalid_argument);
}

void ComponentTest::testShapesResetAndMappingValidation()
{
    auto component=Component::makeFan(6,true);const auto original=component.ledCoordinates;const auto mapping=component.ledMapping;
    for(const auto* shape:{"Line","Circle","Ellipse","Rectangle"}){QVERIFY(component.arrange(shape));QVERIFY(component.valid());QCOMPARE(component.ledMapping,mapping);}
    component.resetLayout();QCOMPARE(component.ledCoordinates,original);
    const auto restored=Component::fromJson(component.toJson());QCOMPARE(restored.layoutDefaults,original);QCOMPARE(restored.physicalLedsPerAddress,2);
    component.ledCoordinates[1]=component.ledCoordinates[0];QCOMPARE(component.effectAddress(1),component.effectAddress(0));
    component.ledMapping[1]=component.ledMapping[0];QVERIFY(!component.valid());QVERIFY_EXCEPTION_THROWN(Component::fromJson(component.toJson()),std::invalid_argument);
}

QTEST_MAIN(ComponentTest)
#include "test_component.moc"
