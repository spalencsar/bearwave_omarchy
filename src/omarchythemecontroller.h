// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OMARCHYTHEMECONTROLLER_H
#define OMARCHYTHEMECONTROLLER_H

#include <QColor>
#include <QFileSystemWatcher>
#include <QObject>
#include <QString>

class OmarchyThemeController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY themeChanged)
    Q_PROPERTY(bool darkMode READ darkMode NOTIFY themeChanged)
    Q_PROPERTY(QColor accent READ accent NOTIFY themeChanged)
    Q_PROPERTY(QColor selection READ selection NOTIFY themeChanged)
    Q_PROPERTY(QColor muted READ muted NOTIFY themeChanged)
    Q_PROPERTY(QColor background READ background NOTIFY themeChanged)
    Q_PROPERTY(QColor darkBackground READ darkBackground NOTIFY themeChanged)
    Q_PROPERTY(QColor darkerBackground READ darkerBackground NOTIFY themeChanged)
    Q_PROPERTY(QColor lighterBackground READ lighterBackground NOTIFY themeChanged)
    Q_PROPERTY(QColor foreground READ foreground NOTIFY themeChanged)
    Q_PROPERTY(QColor darkForeground READ darkForeground NOTIFY themeChanged)
    Q_PROPERTY(QColor lightForeground READ lightForeground NOTIFY themeChanged)
    Q_PROPERTY(QColor brightForeground READ brightForeground NOTIFY themeChanged)
    Q_PROPERTY(QColor warning READ warning NOTIFY themeChanged)

public:
    explicit OmarchyThemeController(bool enabled, QObject *parent = nullptr);
    explicit OmarchyThemeController(const QString &colorsPath, QObject *parent = nullptr);

    bool available() const { return m_available; }
    bool darkMode() const { return m_darkMode; }
    QColor accent() const { return m_accent; }
    QColor selection() const { return m_selection; }
    QColor muted() const { return m_muted; }
    QColor background() const { return m_background; }
    QColor darkBackground() const { return m_darkBackground; }
    QColor darkerBackground() const { return m_darkerBackground; }
    QColor lighterBackground() const { return m_lighterBackground; }
    QColor foreground() const { return m_foreground; }
    QColor darkForeground() const { return m_darkForeground; }
    QColor lightForeground() const { return m_lightForeground; }
    QColor brightForeground() const { return m_brightForeground; }
    QColor warning() const { return m_warning; }

public slots:
    void refresh();

signals:
    void themeChanged();

private:
    void syncWatchPaths();

    QString m_colorsPath;
    bool m_enabled = false;
    bool m_available = false;
    bool m_darkMode = true;
    QColor m_accent;
    QColor m_selection;
    QColor m_muted;
    QColor m_background;
    QColor m_darkBackground;
    QColor m_darkerBackground;
    QColor m_lighterBackground;
    QColor m_foreground;
    QColor m_darkForeground;
    QColor m_lightForeground;
    QColor m_brightForeground;
    QColor m_warning;
    QFileSystemWatcher m_watcher;
};

#endif
