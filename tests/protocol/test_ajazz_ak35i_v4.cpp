// AJAZZ AK35I V4 Direct RGB encoder, checked against owner captures
// (docs/hardware-inventory/ajazz-ak35i-v4-rgb-fixtures.json). Expected bytes come from the
// fixture, never from the encoder under test.
#include "protocol/AjazzAk35iV4Protocol.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QtTest>

class AjazzAk35iV4Test : public QObject {
    Q_OBJECT
    QJsonObject fixture;
    QJsonObject transaction(const QString& name) const {
        for (const auto& value : fixture.value("transactions").toArray())
            if (value.toObject().value("name").toString() == name) return value.toObject();
        return {};
    }
    static QVector<QByteArray> payloads(const QJsonObject& transaction, const QString& operation) {
        QVector<QByteArray> result;
        for (const auto& value : transaction.value("operations").toArray())
            if (value.toObject().value("operation").toString() == operation)
                result.append(QByteArray::fromHex(value.toObject().value("payload_hex").toString().toLatin1()));
        return result;
    }
    template<class Report> static QByteArray payloadOf(const Report& report) {
        return QByteArray(reinterpret_cast<const char*>(report.data()) + 1, int(AJAZZ_V4_PAYLOAD));
    }
    static AjazzReport native(const QByteArray& payload) {
        AjazzReport report{}; std::copy(payload.cbegin(), payload.cend(), report.begin() + 1); return report;
    }
    // Colors in XML order; the fixture lists only the non-black LEDs by light ID.
    std::vector<MsiColor> frameColors(const QJsonObject& transaction) const {
        std::vector<MsiColor> colors(AJAZZ_V4_LEDS);
        const auto lit = transaction.value("input_nonzero_leds").toObject();
        for (int order = 0; order < AJAZZ_V4_LEDS; ++order) {
            const auto rgb = lit.value(QString::number(AJAZZ_V4_LIGHT_IDS[order])).toArray();
            if (!rgb.isEmpty()) colors[order] = {quint8(rgb[0].toInt()), quint8(rgb[1].toInt()), quint8(rgb[2].toInt())};
        }
        return colors;
    }
private slots:
    void initTestCase() {
        QFile file(FIXTURE_FILE); QVERIFY(file.open(QIODevice::ReadOnly));
        fixture = QJsonDocument::fromJson(file.readAll()).object(); QVERIFY(!fixture.isEmpty());
    }
    void lightIdsMatchFixture() {
        const auto entries = fixture.value("led_map").toObject().value("entries").toArray();
        QCOMPARE(entries.size(), AJAZZ_V4_LEDS);
        QSet<int> unique;
        for (int k = 0; k < AJAZZ_V4_LEDS; ++k) {
            QCOMPARE(entries[k].toObject().value("order").toInt(), k);
            QCOMPARE(int(AJAZZ_V4_LIGHT_IDS[k]), entries[k].toObject().value("light_index").toInt());
            QVERIFY(AJAZZ_V4_LIGHT_IDS[k] >= 1 && AJAZZ_V4_LIGHT_IDS[k] <= 123);
            unique.insert(AJAZZ_V4_LIGHT_IDS[k]);
        }
        QCOMPARE(unique.size(), AJAZZ_V4_LEDS);
        QCOMPARE(int(AJAZZ_V4_LIGHT_IDS[0]), 1); // Esc
        for (int id : {111, 85, 94, 103}) QVERIFY2(unique.contains(id), qPrintable(QString::number(id))); // ISO key, ISO Enter, space, Backspace
    }
    void headerMatchesCapture() {
        const auto sets = payloads(transaction("direct-frame"), "set");
        QCOMPARE(payloadOf(BuildAjazzCommand(0x20, 8)), sets.first());
        QCOMPARE(payloadOf(BuildAjazzCommand(0x02)), sets.last());
        QCOMPARE(payloadOf(BuildAjazzCommand(0x18)), payloads(transaction("global-blue-brightness2"), "set").first());
        QCOMPARE(int(BuildAjazzCommand(0x20, 8)[0]), 0); // report ID
        QCOMPARE(BuildAjazzCommand(0x02).size(), AJAZZ_V4_REPORT);
    }
    void directChunksMatchCapture() {
        for (const auto* name : {"direct-frame", "direct-frame-next"}) {
            const auto captured = transaction(name);
            const auto sets = payloads(captured, "set");
            QCOMPARE(sets.size(), 10); // header, 8 data reports, 04 02
            const auto chunks = BuildAjazzDirectChunks(frameColors(captured));
            for (int chunk = 0; chunk < 8; ++chunk) {
                QCOMPARE(int(chunks[chunk][0]), 0);
                QVERIFY2(payloadOf(chunks[chunk]) == sets[1 + chunk], qPrintable(QString("%1 chunk %2").arg(name).arg(chunk)));
            }
            QCOMPARE(payloadOf(chunks[7]), QByteArray(64, '\0'));
        }
    }
    void wrongCountThrows() {
        QVERIFY_EXCEPTION_THROWN(BuildAjazzDirectChunks(std::vector<MsiColor>(104)), std::invalid_argument);
        QVERIFY_EXCEPTION_THROWN(BuildAjazzDirectChunks(std::vector<MsiColor>(106)), std::invalid_argument);
    }
    void ackAcceptsTrailingBytes() {
        // 04 02 replies may carry extra non-zero bytes (spec section 3); only prefix and status count.
        for (const auto* name : {"direct-frame", "global-blue-brightness2", "persistent-esc-red"}) {
            for (const auto& payload : payloads(transaction(name), "get")) {
                if (payload[0] != 0x04 || (quint8(payload[1]) != 0x20 && quint8(payload[1]) != 0x02 && quint8(payload[1]) != 0x18)) continue;
                const auto report = native(payload);
                QVERIFY2(AjazzAckOk(report.data(), report.size(), quint8(payload[1])), payload.toHex().constData());
            }
        }
        auto report = native(payloads(transaction("direct-frame"), "get").first()); // 04 20 ACK
        QVERIFY(AjazzAckOk(report.data(), report.size(), 0x20));
        QVERIFY(!AjazzAckOk(report.data(), report.size(), 0x02));   // opcode mismatch
        QVERIFY(!AjazzAckOk(report.data(), 64, 0x20));              // short read
        auto badStatus = report; badStatus[4] = 0x00; QVERIFY(!AjazzAckOk(badStatus.data(), badStatus.size(), 0x20));
        auto badId = report; badId[0] = 0x01; QVERIFY(!AjazzAckOk(badId.data(), badId.size(), 0x20));
        auto badPrefix = report; badPrefix[1] = 0x05; QVERIFY(!AjazzAckOk(badPrefix.data(), badPrefix.size(), 0x20));
    }
};
QTEST_APPLESS_MAIN(AjazzAk35iV4Test)
#include "test_ajazz_ak35i_v4.moc"
