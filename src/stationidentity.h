// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STATIONIDENTITY_H
#define STATIONIDENTITY_H

#include <QList>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class QObject;
class RadioStation;

namespace StationIdentity {

QVariantMap toVariantMap(const RadioStation *station);
bool matches(const RadioStation *station, const QString &uuid, const QString &urlResolved);
bool matchesMap(const QVariantMap &station, const QString &uuid, const QString &urlResolved);
int indexOf(const QList<RadioStation *> &stations, const QString &uuid, const QString &urlResolved);
int recentIndex(const QVariantList &recent, const QString &uuid, const QString &urlResolved);
RadioStation *clone(const RadioStation *source, QObject *parent);
void sortByName(QList<RadioStation *> &stations);

} // namespace StationIdentity

#endif
