// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QFile>
#include <QJSEngine>
#include <QJSValue>
#include <QtTest>

class BarModelTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void buildsStationTitleLine();
    void readsMprisMetadataMap();
    void stripsMarkupShapedInput();
    void clipsLongFields();

private:
    QString callNowPlaying(const QString &station, const QString &title);

    QJSEngine m_engine;
    QJSValue m_nowPlayingLine;
    QJSValue m_nowPlayingFromPlayer;
};

void BarModelTest::initTestCase()
{
    QFile file(QStringLiteral(BEARWAVE_SOURCE_DIR "/omarchy/BarModel.js"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QByteArray source = file.readAll();
    source.replace(".pragma library", "");
    const QJSValue result = m_engine.evaluate(QString::fromUtf8(source), file.fileName());
    QVERIFY2(!result.isError(), qPrintable(result.toString()));
    m_nowPlayingLine = m_engine.globalObject().property(QStringLiteral("nowPlayingLine"));
    QVERIFY(m_nowPlayingLine.isCallable());
    m_nowPlayingFromPlayer = m_engine.globalObject().property(QStringLiteral("nowPlayingFromPlayer"));
    QVERIFY(m_nowPlayingFromPlayer.isCallable());
}

QString BarModelTest::callNowPlaying(const QString &station, const QString &title)
{
    return m_nowPlayingLine.call(QJSValueList{station, title}).toString();
}

void BarModelTest::buildsStationTitleLine()
{
    QCOMPARE(callNowPlaying(QString(), QString()), QString());
    QCOMPARE(callNowPlaying(QStringLiteral("FluxFM"), QString()), QStringLiteral("(FluxFM)"));
    QCOMPARE(callNowPlaying(QStringLiteral("FluxFM"), QStringLiteral("FluxFM")),
             QStringLiteral("(FluxFM)"));
    QCOMPARE(callNowPlaying(QStringLiteral("FluxFM"), QStringLiteral("Night Call")),
             QStringLiteral("(FluxFM) Night Call"));
    QCOMPARE(m_nowPlayingLine.call(QJSValueList{
                                       QStringLiteral(".977 80s"),
                                       QStringLiteral("These Dreams"),
                                       QStringLiteral("Heart")
                                   }).toString(),
             QStringLiteral("(.977 80s) Heart - These Dreams"));
    QCOMPARE(m_nowPlayingLine.call(QJSValueList{
                                       QStringLiteral(".977 80s"),
                                       QStringLiteral("Heart - These Dreams"),
                                       QStringLiteral("Heart")
                                   }).toString(),
             QStringLiteral("(.977 80s) Heart - These Dreams"));
}

void BarModelTest::readsMprisMetadataMap()
{
    QJSValue player = m_engine.newObject();
    QJSValue metadata = m_engine.newObject();
    metadata.setProperty(QStringLiteral("xesam:album"), QStringLiteral(".977 80s"));
    metadata.setProperty(QStringLiteral("xesam:title"), QStringLiteral("These Dreams"));
    player.setProperty(QStringLiteral("metadata"), metadata);

    metadata.setProperty(QStringLiteral("xesam:artist"), QStringLiteral("Heart"));
    QCOMPARE(m_nowPlayingFromPlayer.call(QJSValueList{player}).toString(),
             QStringLiteral("(.977 80s) Heart - These Dreams"));
}

void BarModelTest::stripsMarkupShapedInput()
{
    QCOMPARE(callNowPlaying(QStringLiteral("<img src=\"https://example.invalid/tracker\">FluxFM"),
                            QStringLiteral("<b>Night</b> Call")),
             QStringLiteral("(FluxFM) Night Call"));
    QCOMPARE(callNowPlaying(QStringLiteral("<script>alert(1)</script>"), QString()), QString());
}

void BarModelTest::clipsLongFields()
{
    const QString longStation = QStringLiteral("X").repeated(80);
    const QString line = callNowPlaying(longStation, QString());
    QVERIFY(line.startsWith(QLatin1Char('(')));
    QVERIFY(line.endsWith(QLatin1Char(')')));
    QVERIFY(line.contains(QStringLiteral("…")));
}

QTEST_GUILESS_MAIN(BarModelTest)
#include "bar_model_test.moc"
