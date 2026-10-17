// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest>

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "omarchythemecontroller.h"

class OmarchyThemeControllerTest : public QObject
{
    Q_OBJECT

private slots:
    void loadsOmarchyPalette();
    void rejectsIncompletePalette();
};

void OmarchyThemeControllerTest::loadsOmarchyPalette()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("colors.toml"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("mode = \"dark\"\n"
               "accent = \"#5c71c9\"\n"
               "selection = \"#1d1a24\"\n"
               "background = \"#04000c\"\n"
               "dark_background = \"#030009\"\n"
               "lighter_background = \"#1d1a24\"\n"
               "foreground = \"#fdf9f9\"\n"
               "dark_foreground = \"#bebbbb\"\n"
               "red = \"#b35ca0\"\n");
    file.close();

    OmarchyThemeController theme(path);
    QVERIFY(theme.available());
    QVERIFY(theme.darkMode());
    QCOMPARE(theme.accent(), QColor(QStringLiteral("#5c71c9")));
    QCOMPARE(theme.background(), QColor(QStringLiteral("#04000c")));
    QCOMPARE(theme.darkForeground(), QColor(QStringLiteral("#bebbbb")));
    QCOMPARE(theme.warning(), QColor(QStringLiteral("#b35ca0")));
}

void OmarchyThemeControllerTest::rejectsIncompletePalette()
{
    QTemporaryDir directory;
    QFile file(directory.filePath(QStringLiteral("colors.toml")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("accent = \"#5c71c9\"\n");
    file.close();

    OmarchyThemeController theme(file.fileName());
    QVERIFY(!theme.available());
}

QTEST_APPLESS_MAIN(OmarchyThemeControllerTest)
#include "omarchy_theme_controller_test.moc"
