// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "omarchythemecontroller.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>

namespace {
QString defaultColorsPath()
{
    return QDir::homePath()
        + QStringLiteral("/.local/state/omarchy/current/theme/colors.toml");
}

QColor paletteColor(const QHash<QString, QString> &values, const QString &key,
                    const QColor &fallback = {})
{
    const QColor color(values.value(key));
    return color.isValid() ? color : fallback;
}
} // namespace

OmarchyThemeController::OmarchyThemeController(bool enabled, QObject *parent)
    : OmarchyThemeController(enabled ? defaultColorsPath() : QString(), parent)
{
}

OmarchyThemeController::OmarchyThemeController(const QString &colorsPath, QObject *parent)
    : QObject(parent)
    , m_colorsPath(colorsPath)
    , m_enabled(!colorsPath.isEmpty())
{
    connect(&m_watcher, &QFileSystemWatcher::fileChanged,
            this, &OmarchyThemeController::refresh);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged,
            this, &OmarchyThemeController::refresh);
    refresh();
}

void OmarchyThemeController::syncWatchPaths()
{
    if (!m_enabled) {
        return;
    }

    const QFileInfo colorsInfo(m_colorsPath);
    const QString themeDirectory = colorsInfo.absolutePath();
    const QString currentDirectory = QFileInfo(themeDirectory).absolutePath();
    const QStringList candidates = {currentDirectory, themeDirectory, m_colorsPath};
    for (const QString &path : candidates) {
        if (QFileInfo::exists(path)
            && !m_watcher.files().contains(path)
            && !m_watcher.directories().contains(path)) {
            m_watcher.addPath(path);
        }
    }
}

void OmarchyThemeController::refresh()
{
    syncWatchPaths();

    QFile file(m_colorsPath);
    if (!m_enabled || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (m_available) {
            m_available = false;
            emit themeChanged();
        }
        return;
    }

    QHash<QString, QString> values;
    static const QRegularExpression assignment(QStringLiteral(
        "^\\s*([a-z_]+)\\s*=\\s*\"([^\"]*)\"\\s*(?:#.*)?$"));
    while (!file.atEnd()) {
        const QString line = QString::fromUtf8(file.readLine());
        const QRegularExpressionMatch match = assignment.match(line);
        if (match.hasMatch()) {
            values.insert(match.captured(1), match.captured(2));
        }
    }

    const QColor background = paletteColor(values, QStringLiteral("background"));
    const QColor foreground = paletteColor(values, QStringLiteral("foreground"));
    const QColor accent = paletteColor(values, QStringLiteral("accent"));
    if (!background.isValid() || !foreground.isValid() || !accent.isValid()) {
        if (m_available) {
            m_available = false;
            emit themeChanged();
        }
        return;
    }

    m_darkMode = values.value(QStringLiteral("mode")).compare(
                     QStringLiteral("light"), Qt::CaseInsensitive) != 0;
    m_background = background;
    m_foreground = foreground;
    m_accent = accent;
    m_selection = paletteColor(values, QStringLiteral("selection"), accent);
    m_muted = paletteColor(values, QStringLiteral("muted"), foreground);
    m_darkBackground = paletteColor(values, QStringLiteral("dark_background"), background);
    m_darkerBackground = paletteColor(values, QStringLiteral("darker_background"), m_darkBackground);
    m_lighterBackground = paletteColor(values, QStringLiteral("lighter_background"), background);
    m_darkForeground = paletteColor(values, QStringLiteral("dark_foreground"), m_muted);
    m_lightForeground = paletteColor(values, QStringLiteral("light_foreground"), foreground);
    m_brightForeground = paletteColor(values, QStringLiteral("bright_foreground"), foreground);
    m_warning = paletteColor(values, QStringLiteral("red"), QColor(QStringLiteral("#ff6b6b")));
    m_available = true;
    syncWatchPaths();
    emit themeChanged();
}
