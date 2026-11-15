// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stationidentity.h"

#include "radiostation.h"

#include <algorithm>

namespace StationIdentity {

QVariantMap toVariantMap(const RadioStation *station)
{
    QVariantMap map;
    if (!station) {
        return map;
    }
    map.insert(QStringLiteral("uuid"), station->uuid());
    map.insert(QStringLiteral("name"), station->name());
    map.insert(QStringLiteral("country"), station->country());
    map.insert(QStringLiteral("countryCode"), station->countryCode());
    map.insert(QStringLiteral("bitrate"), station->bitrate());
    map.insert(QStringLiteral("votes"), station->votes());
    map.insert(QStringLiteral("isFavorite"), station->isFavorite());
    map.insert(QStringLiteral("favicon"), station->favicon());
    map.insert(QStringLiteral("homepage"), station->homepage());
    map.insert(QStringLiteral("url"), station->url());
    map.insert(QStringLiteral("urlResolved"), station->urlResolved());
    map.insert(QStringLiteral("codec"), station->codec());
    map.insert(QStringLiteral("tags"), station->tags());
    map.insert(QStringLiteral("isOnline"), station->isOnline());
    return map;
}

bool matches(const RadioStation *station, const QString &uuid, const QString &urlResolved)
{
    if (!station) {
        return false;
    }
    if (!uuid.isEmpty() && station->uuid() == uuid) {
        return true;
    }
    if (uuid.isEmpty() && !urlResolved.isEmpty() && station->urlResolved() == urlResolved) {
        return true;
    }
    return false;
}

bool matchesMap(const QVariantMap &station, const QString &uuid, const QString &urlResolved)
{
    const QString stationUuid = station.value(QStringLiteral("uuid")).toString();
    const QString stationUrl = station.value(QStringLiteral("urlResolved")).toString();
    if (!uuid.isEmpty() && stationUuid == uuid) {
        return true;
    }
    if (uuid.isEmpty() && !urlResolved.isEmpty() && stationUrl == urlResolved) {
        return true;
    }
    return false;
}

int indexOf(const QList<RadioStation *> &stations, const QString &uuid, const QString &urlResolved)
{
    for (int i = 0; i < stations.size(); ++i) {
        if (matches(stations[i], uuid, urlResolved)) {
            return i;
        }
    }
    return -1;
}

int recentIndex(const QVariantList &recent, const QString &uuid, const QString &urlResolved)
{
    for (int i = 0; i < recent.size(); ++i) {
        if (matchesMap(recent.at(i).toMap(), uuid, urlResolved)) {
            return i;
        }
    }
    return -1;
}

RadioStation *clone(const RadioStation *source, QObject *parent)
{
    if (!source) {
        return nullptr;
    }
    auto *copy = new RadioStation(parent);
    copy->setUuid(source->uuid());
    copy->setName(source->name());
    copy->setUrl(source->url());
    copy->setUrlResolved(source->urlResolved());
    copy->setHomepage(source->homepage());
    copy->setFavicon(source->favicon());
    copy->setCountry(source->country());
    copy->setCountryCode(source->countryCode());
    copy->setTags(source->tags());
    copy->setCodec(source->codec());
    copy->setBitrate(source->bitrate());
    copy->setVotes(source->votes());
    copy->setIsOnline(source->isOnline());
    copy->setIsFavorite(source->isFavorite());
    return copy;
}

void sortByName(QList<RadioStation *> &stations)
{
    std::sort(stations.begin(), stations.end(), [](RadioStation *a, RadioStation *b) {
        return QString::localeAwareCompare(a->name(), b->name()) < 0;
    });
}

} // namespace StationIdentity
