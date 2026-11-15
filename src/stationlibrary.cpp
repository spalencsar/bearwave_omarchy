// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stationlibrary.h"

#include "bearwavepaths.h"
#include "radiostation.h"
#include "stationidentity.h"
#include "streamurl.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QUuid>

namespace {
constexpr int kRecentLimit = 20;

bool writePrivateJson(const QString &path, const QJsonDocument &document)
{
    QSaveFile file(path);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    if (!file.setPermissions(BearwavePaths::privateFilePermissions)) {
        return false;
    }
    const QByteArray data = document.toJson(QJsonDocument::Indented);
    if (file.write(data) != data.size() || !file.commit()) {
        return false;
    }
    return BearwavePaths::secureConfigFile(path);
}

QString configDir()
{
    return BearwavePaths::configDir();
}

bool stationUrlAllowed(const QJsonObject &object)
{
    const QString url = object.value(QStringLiteral("url")).toString();
    const QString urlResolved = object.value(QStringLiteral("urlResolved")).toString();
    const QString streamUrl = !urlResolved.isEmpty() ? urlResolved : url;
    return isAllowedStreamUrl(streamUrl)
           && (url.isEmpty() || isAllowedStreamUrl(url))
           && (urlResolved.isEmpty() || isAllowedStreamUrl(urlResolved));
}

} // namespace

StationLibrary::StationLibrary(QObject *parent)
    : QObject(parent)
{
}

void StationLibrary::load()
{
    loadFavorites();
    loadManuals();
    loadState();
}

void StationLibrary::saveState(double volume) const
{
    QJsonObject obj;
    obj.insert(QStringLiteral("lastStationName"), m_lastStationName);
    obj.insert(QStringLiteral("lastStationUrl"), m_lastStationUrl);
    obj.insert(QStringLiteral("volume"), volume);
    obj.insert(QStringLiteral("recentStations"), QJsonArray::fromVariantList(m_recents));
    writePrivateJson(configDir() + QStringLiteral("/state.json"), QJsonDocument(obj));
}

QVariantList StationLibrary::recentsWithFavoriteFlags() const
{
    QVariantList updatedList;
    for (const QVariant &entry : m_recents) {
        QVariantMap station = entry.toMap();
        const QString uuid = station.value(QStringLiteral("uuid")).toString();
        const QString urlResolved = station.value(QStringLiteral("urlResolved")).toString();
        station.insert(QStringLiteral("isFavorite"), containsFavorite(uuid, urlResolved));
        updatedList.append(station);
    }
    return updatedList;
}

QVariantList StationLibrary::favoriteMaps() const
{
    QVariantList result;
    for (RadioStation *station : m_favorites) {
        result.push_back(StationIdentity::toVariantMap(station));
    }
    return result;
}

void StationLibrary::setLastStation(const QString &name, const QString &url)
{
    m_lastStationName = name;
    m_lastStationUrl = url;
    emit resumeStateChanged();
}

void StationLibrary::clearLastStation()
{
    m_lastStationName.clear();
    m_lastStationUrl.clear();
    emit resumeStateChanged();
}

bool StationLibrary::containsFavorite(const QString &uuid, const QString &urlResolved) const
{
    return favoriteIndex(uuid, urlResolved) >= 0;
}

int StationLibrary::favoriteIndex(const QString &uuid, const QString &urlResolved) const
{
    return StationIdentity::indexOf(m_favorites, uuid, urlResolved);
}

int StationLibrary::manualIndex(const QString &uuid, const QString &urlResolved) const
{
    return StationIdentity::indexOf(m_manuals, uuid, urlResolved);
}

int StationLibrary::recentIndex(const QString &uuid, const QString &urlResolved) const
{
    return StationIdentity::recentIndex(m_recents, uuid, urlResolved);
}

bool StationLibrary::toggleFavoriteFromCatalog(RadioStation *station)
{
    if (!station) {
        return false;
    }
    const int existing = favoriteIndex(station->uuid(), station->urlResolved());
    if (existing >= 0) {
        m_favorites.takeAt(existing)->deleteLater();
        station->setIsFavorite(false);
        saveFavorites();
        emit favoritesChanged();
        return true;
    }

    RadioStation *copy = StationIdentity::clone(station, this);
    copy->setIsFavorite(true);
    m_favorites.append(copy);
    StationIdentity::sortByName(m_favorites);
    station->setIsFavorite(true);
    saveFavorites();
    emit favoritesChanged();
    return true;
}

bool StationLibrary::removeFavorite(const QString &uuid, const QString &urlResolved)
{
    const int index = favoriteIndex(uuid, urlResolved);
    if (index < 0) {
        return false;
    }
    m_favorites.takeAt(index)->deleteLater();
    saveFavorites();
    emit favoritesChanged();
    return true;
}

void StationLibrary::applyFavoriteEdits(const QString &oldUuid,
                                        const QString &oldUrl,
                                        const QString &oldUrlResolved,
                                        RadioStation *updated)
{
    if (!updated) {
        return;
    }
    for (RadioStation *fav : m_favorites) {
        if (fav == updated
            || StationIdentity::matches(fav, oldUuid, oldUrlResolved)
            || (fav->uuid().isEmpty()
                && (fav->url() == oldUrl || fav->urlResolved() == oldUrlResolved))) {
            fav->setName(updated->name());
            fav->setUrl(updated->url());
            fav->setUrlResolved(updated->urlResolved());
            fav->setCountry(updated->country());
        }
    }
    saveFavorites();
    emit favoritesChanged();
}

bool StationLibrary::addOrUpdateManual(const QString &name,
                                       const QString &url,
                                       const QString &country,
                                       const QString &defaultCountry,
                                       QString *error)
{
    if (name.trimmed().isEmpty() || url.trimmed().isEmpty()) {
        return false;
    }
    if (!isAllowedStreamUrl(url)) {
        if (error) {
            *error = QStringLiteral("scheme");
        }
        return false;
    }

    const QString resolvedUrl = url.trimmed();
    const QString countryLabel =
        country.trimmed().isEmpty() ? defaultCountry : country.trimmed();

    for (RadioStation *mine : m_manuals) {
        if (mine->urlResolved() == resolvedUrl || mine->url() == resolvedUrl) {
            mine->setName(name.trimmed());
            mine->setCountry(countryLabel);
            saveManuals();
            emit manualsChanged();
            return true;
        }
    }

    auto *station = new RadioStation(this);
    station->setUuid(QUuid::createUuid().toString(QUuid::WithoutBraces));
    station->setName(name.trimmed());
    station->setUrl(resolvedUrl);
    station->setUrlResolved(resolvedUrl);
    station->setCountry(countryLabel);
    station->setCodec(QStringLiteral("unknown"));
    station->setBitrate(0);
    station->setVotes(0);
    station->setIsOnline(true);
    station->setIsFavorite(false);
    m_manuals.append(station);
    StationIdentity::sortByName(m_manuals);
    saveManuals();
    emit manualsChanged();
    return true;
}

bool StationLibrary::editManual(RadioStation *station,
                                const QString &name,
                                const QString &url,
                                const QString &country,
                                const QString &defaultCountry,
                                QString *error)
{
    if (!station || name.trimmed().isEmpty() || url.trimmed().isEmpty()) {
        return false;
    }
    if (!isAllowedStreamUrl(url)) {
        if (error) {
            *error = QStringLiteral("scheme");
        }
        return false;
    }

    const QString oldUrl = station->url();
    const QString oldUrlResolved = station->urlResolved();
    const QString oldUuid = station->uuid();
    const QString countryLabel =
        country.trimmed().isEmpty() ? defaultCountry : country.trimmed();

    station->setName(name.trimmed());
    station->setUrl(url.trimmed());
    station->setUrlResolved(url.trimmed());
    station->setCountry(countryLabel);

    bool foundManual = false;
    for (RadioStation *mine : m_manuals) {
        if (mine == station
            || StationIdentity::matches(mine, oldUuid, oldUrlResolved)
            || StationIdentity::matches(mine, oldUuid, oldUrl)) {
            mine->setName(station->name());
            mine->setUrl(station->url());
            mine->setUrlResolved(station->urlResolved());
            mine->setCountry(station->country());
            if (mine->uuid().isEmpty()) {
                mine->setUuid(station->uuid().isEmpty()
                                  ? QUuid::createUuid().toString(QUuid::WithoutBraces)
                                  : station->uuid());
            }
            foundManual = true;
        }
    }
    if (foundManual) {
        saveManuals();
        emit manualsChanged();
    }

    applyFavoriteEdits(oldUuid, oldUrl, oldUrlResolved, station);
    return true;
}

RadioStation *StationLibrary::takeManual(const QString &uuid, const QString &urlResolved)
{
    const int index = manualIndex(uuid, urlResolved);
    if (index < 0) {
        return nullptr;
    }
    RadioStation *station = m_manuals.takeAt(index);
    saveManuals();
    emit manualsChanged();
    return station;
}

void StationLibrary::recordRecent(const QVariantMap &stationData)
{
    const QString url = stationData.value(QStringLiteral("urlResolved")).toString();
    if (url.isEmpty() || !isAllowedStreamUrl(url)) {
        return;
    }

    QVariantMap normalizedStation = stationData;
    if (!normalizedStation.contains(QStringLiteral("name"))) {
        normalizedStation.insert(QStringLiteral("name"), QString());
    }

    for (int i = 0; i < m_recents.size(); ++i) {
        const QVariantMap existing = m_recents[i].toMap();
        const QString existingUuid = existing.value(QStringLiteral("uuid")).toString();
        const QString newUuid = normalizedStation.value(QStringLiteral("uuid")).toString();
        if ((!newUuid.isEmpty() && existingUuid == newUuid)
            || (newUuid.isEmpty()
                && existing.value(QStringLiteral("urlResolved")).toString() == url)) {
            m_recents.removeAt(i);
            break;
        }
    }

    m_recents.prepend(normalizedStation);
    while (m_recents.size() > kRecentLimit) {
        m_recents.removeLast();
    }
    emit recentsChanged();
}

QString StationLibrary::findUuidByUrl(const QString &url) const
{
    auto matchUrl = [&url](const RadioStation *station) {
        return station->urlResolved() == url || station->url() == url;
    };
    for (const RadioStation *station : m_manuals) {
        if (matchUrl(station)) {
            return station->uuid();
        }
    }
    for (const RadioStation *station : m_favorites) {
        if (matchUrl(station)) {
            return station->uuid();
        }
    }
    return {};
}

void StationLibrary::loadFavorites()
{
    QFile file(configDir() + QStringLiteral("/favorites.json"));
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        return;
    }

    for (const QJsonValue &value : doc.array()) {
        if (!value.isObject()) {
            continue;
        }
        const QJsonObject object = value.toObject();
        if (!stationUrlAllowed(object)) {
            qWarning() << "Skipping favorite with disallowed stream URL scheme";
            continue;
        }
        auto *station = new RadioStation(this);
        station->setUuid(object.value(QStringLiteral("uuid")).toString());
        station->setName(object.value(QStringLiteral("name")).toString());
        station->setUrl(object.value(QStringLiteral("url")).toString());
        station->setUrlResolved(object.value(QStringLiteral("urlResolved")).toString());
        station->setCountry(object.value(QStringLiteral("country")).toString());
        station->setCountryCode(object.value(QStringLiteral("countrycode")).toString());
        station->setIsFavorite(true);
        m_favorites.append(station);
    }
    StationIdentity::sortByName(m_favorites);
}

void StationLibrary::saveFavorites() const
{
    QJsonArray array;
    for (RadioStation *station : m_favorites) {
        QJsonObject object;
        object.insert(QStringLiteral("uuid"), station->uuid());
        object.insert(QStringLiteral("name"), station->name());
        object.insert(QStringLiteral("url"), station->url());
        object.insert(QStringLiteral("urlResolved"), station->urlResolved());
        object.insert(QStringLiteral("country"), station->country());
        object.insert(QStringLiteral("countrycode"), station->countryCode());
        array.append(object);
    }
    writePrivateJson(configDir() + QStringLiteral("/favorites.json"), QJsonDocument(array));
}

void StationLibrary::loadManuals()
{
    QFile file(configDir() + QStringLiteral("/my_stations.json"));
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        return;
    }

    for (const QJsonValue &value : doc.array()) {
        if (!value.isObject()) {
            continue;
        }
        const QJsonObject object = value.toObject();
        if (!stationUrlAllowed(object)) {
            qWarning() << "Skipping my-station with disallowed stream URL scheme";
            continue;
        }
        auto *station = new RadioStation(this);
        QString uuid = object.value(QStringLiteral("uuid")).toString();
        if (uuid.isEmpty()) {
            uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        }
        const QString url = object.value(QStringLiteral("url")).toString();
        const QString urlResolved = object.value(QStringLiteral("urlResolved")).toString();
        station->setUuid(uuid);
        station->setName(object.value(QStringLiteral("name")).toString());
        station->setUrl(url);
        station->setUrlResolved(urlResolved.isEmpty() ? url : urlResolved);
        station->setCountry(object.value(QStringLiteral("country")).toString());
        station->setIsOnline(true);
        station->setIsFavorite(false);
        m_manuals.append(station);
    }
    StationIdentity::sortByName(m_manuals);
}

void StationLibrary::saveManuals() const
{
    QJsonArray array;
    for (RadioStation *station : m_manuals) {
        QJsonObject object;
        object.insert(QStringLiteral("uuid"), station->uuid());
        object.insert(QStringLiteral("name"), station->name());
        object.insert(QStringLiteral("url"), station->url());
        object.insert(QStringLiteral("urlResolved"), station->urlResolved());
        object.insert(QStringLiteral("country"), station->country());
        array.append(object);
    }
    writePrivateJson(configDir() + QStringLiteral("/my_stations.json"), QJsonDocument(array));
}

void StationLibrary::loadState()
{
    QFile file(configDir() + QStringLiteral("/state.json"));
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return;
    }

    const QJsonObject object = doc.object();
    m_lastStationName = object.value(QStringLiteral("lastStationName")).toString();
    m_lastStationUrl = object.value(QStringLiteral("lastStationUrl")).toString();
    if (!m_lastStationUrl.isEmpty() && !isAllowedStreamUrl(m_lastStationUrl)) {
        qWarning() << "Discarding last station URL with disallowed scheme";
        m_lastStationUrl.clear();
        m_lastStationName.clear();
    }
    m_savedVolume = object.value(QStringLiteral("volume")).toDouble(0.5);
    m_recents.clear();
    for (const QJsonValue &value : object.value(QStringLiteral("recentStations")).toArray()) {
        if (!value.isObject()) {
            continue;
        }
        const QVariantMap recentStation = value.toObject().toVariantMap();
        const QString recentUrl = recentStation.value(QStringLiteral("urlResolved")).toString();
        if (recentUrl.isEmpty() || !isAllowedStreamUrl(recentUrl)) {
            continue;
        }
        m_recents.append(recentStation);
        if (m_recents.size() >= kRecentLimit) {
            break;
        }
    }
    emit resumeStateChanged();
}
