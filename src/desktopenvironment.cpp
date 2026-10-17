// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktopenvironment.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

namespace {
QStringList sessionNames(const QProcessEnvironment &environment)
{
    QStringList names;
    const QStringList variables = {
        QStringLiteral("XDG_CURRENT_DESKTOP"),
        QStringLiteral("XDG_SESSION_DESKTOP"),
        QStringLiteral("DESKTOP_SESSION"),
    };

    for (const QString &variable : variables) {
        const QString value = environment.value(variable).trimmed().toLower();
        if (value.isEmpty()) {
            continue;
        }

        for (const QString &part : value.split(QLatin1Char(':'), Qt::SkipEmptyParts)) {
            names.append(part.trimmed());
            // DESKTOP_SESSION may contain the path to a session file.
            names.append(QFileInfo(part).completeBaseName());
        }
    }

    return names;
}

bool containsSession(const QStringList &names, const QString &session)
{
    for (const QString &name : names) {
        if (name == session || name.startsWith(session + QLatin1Char('-'))) {
            return true;
        }
    }
    return false;
}
} // namespace

namespace DesktopEnvironment {

Type detectSession(const QProcessEnvironment &environment)
{
    const QStringList names = sessionNames(environment);

    // Omarchy runs Hyprland, so test the distribution/session name before
    // treating the compositor as a generic desktop environment.
    if (containsSession(names, QStringLiteral("omarchy"))) {
        return Type::Omarchy;
    }
    return Type::Other;
}

Type detect()
{
    const Type session = detectSession(QProcessEnvironment::systemEnvironment());
    if (session != Type::Other) {
        return session;
    }

    // Use the installed Omarchy data directory as a fallback when a launcher
    // stripped the desktop-session variables.
    if (QDir(QStringLiteral("/usr/share/omarchy")).exists()) {
        return Type::Omarchy;
    }
    return Type::Other;
}

QString name(Type type)
{
    switch (type) {
    case Type::Omarchy:
        return QStringLiteral("omarchy");
    case Type::Other:
        return QStringLiteral("other");
    }
    return QStringLiteral("other");
}

} // namespace DesktopEnvironment
