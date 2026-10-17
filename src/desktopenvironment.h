// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOPENVIRONMENT_H
#define DESKTOPENVIRONMENT_H

#include <QProcessEnvironment>
#include <QString>

namespace DesktopEnvironment {

enum class Type {
    Other,
    Omarchy,
};

Type detect();
Type detectSession(const QProcessEnvironment &environment);
QString name(Type type);

} // namespace DesktopEnvironment

#endif
