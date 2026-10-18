// Unit tests for the Component data model (JSON load/save, placeholder).
// Pure data/parsing, no HID/device access.
#include <QtTest/QtTest>

#include "device/Component.h"

class ComponentTest : public QObject
{
    Q_OBJECT

private slots:
    void testFromJson_roundTrip();
    void testFromJson_tooShortArrayThrows();
    void testFromJson_tooLongArrayTruncates();
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

void ComponentTest::testMakeGenericPlaceholder()
{
    Component c = Component::makeGenericPlaceholder(5);
    QCOMPARE(c.ledCount, 5);
    QCOMPARE(static_cast<int>(c.ledCoordinates.size()), 5);
    QCOMPARE(c.ledCoordinates[4].first, 4);
    QCOMPARE(c.ledCoordinates[4].second, 0);
}

QTEST_MAIN(ComponentTest)
#include "test_component.moc"
