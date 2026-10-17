// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest>

#include "desktopenvironment.h"

class DesktopEnvironmentTest : public QObject
{
    Q_OBJECT

private slots:
    void detectsOmarchyFromDesktopSession();
    void detectsOmarchyInDesktopList();
    void leavesGenericHyprlandAsOther();
};

void DesktopEnvironmentTest::detectsOmarchyFromDesktopSession()
{
    QProcessEnvironment environment;
    environment.insert(QStringLiteral("XDG_CURRENT_DESKTOP"), QStringLiteral("Hyprland"));
    environment.insert(QStringLiteral("DESKTOP_SESSION"), QStringLiteral("omarchy"));

    QCOMPARE(DesktopEnvironment::detectSession(environment), DesktopEnvironment::Type::Omarchy);
}

void DesktopEnvironmentTest::detectsOmarchyInDesktopList()
{
    QProcessEnvironment environment;
    environment.insert(QStringLiteral("XDG_CURRENT_DESKTOP"), QStringLiteral("Hyprland:Omarchy"));

    QCOMPARE(DesktopEnvironment::detectSession(environment), DesktopEnvironment::Type::Omarchy);
}

void DesktopEnvironmentTest::leavesGenericHyprlandAsOther()
{
    QProcessEnvironment environment;
    environment.insert(QStringLiteral("XDG_CURRENT_DESKTOP"), QStringLiteral("Hyprland"));

    QCOMPARE(DesktopEnvironment::detectSession(environment), DesktopEnvironment::Type::Other);
}

QTEST_APPLESS_MAIN(DesktopEnvironmentTest)
#include "desktop_environment_test.moc"
