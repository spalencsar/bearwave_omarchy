// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bearwavecontroladaptor.h"

#include "bearplayer.h"
#include "radiobackend.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
QString boundedString(const QVariantMap &station, const QString &key, int limit = 256)
{
    return station.value(key).toString().left(limit);
}

QJsonObject stationSummary(const QVariantMap &station)
{
    QJsonObject result;
    result.insert(QStringLiteral("name"), boundedString(station, QStringLiteral("name"), 160));
    result.insert(QStringLiteral("uuid"), boundedString(station, QStringLiteral("uuid"), 96));
    result.insert(QStringLiteral("urlResolved"), boundedString(station, QStringLiteral("urlResolved"), 2048));
    result.insert(QStringLiteral("favicon"), boundedString(station, QStringLiteral("favicon"), 2048));
    result.insert(QStringLiteral("country"), boundedString(station, QStringLiteral("country"), 120));
    result.insert(QStringLiteral("codec"), boundedString(station, QStringLiteral("codec"), 24));
    result.insert(QStringLiteral("isFavorite"), station.value(QStringLiteral("isFavorite")).toBool());
    return result;
}

QJsonArray stationSummaries(const QVariantList &stations, int limit)
{
    QJsonArray result;
    const int count = qMin(limit, stations.size());
    for (int index = 0; index < count; ++index) {
        result.append(stationSummary(stations.at(index).toMap()));
    }
    return result;
}
}

BearWaveControlAdaptor::BearWaveControlAdaptor(RadioBackend *backend)
    : QDBusAbstractAdaptor(backend)
    , m_backend(backend)
{
    connect(m_backend, &RadioBackend::listsChanged, this, &BearWaveControlAdaptor::StationsChanged);
}

QVariantList BearWaveControlAdaptor::GetFavorites() const
{
    return m_backend ? m_backend->getFavoriteStations() : QVariantList();
}

QVariantList BearWaveControlAdaptor::GetRecentStations() const
{
    return m_backend ? m_backend->getRecentStations() : QVariantList();
}

QString BearWaveControlAdaptor::GetWidgetState() const
{
    QJsonObject state;
    state.insert(QStringLiteral("running"), m_backend != nullptr);
    if (!m_backend) {
        return QString::fromUtf8(QJsonDocument(state).toJson(QJsonDocument::Compact));
    }

    BearPlayer *player = m_backend->player();
    const QString station = player ? player->currentStationName().left(160) : QString();
    const QString title = player ? player->currentTrackTitle().left(240) : QString();
    const QString artist = player ? player->currentTrackArtist().left(240) : QString();
    const QString artwork = player ? player->currentCoverArtUrl().left(2048) : QString();

    state.insert(QStringLiteral("playbackStatus"),
                 station.isEmpty() ? QStringLiteral("Stopped")
                                   : (player && player->playing() ? QStringLiteral("Playing")
                                                                  : QStringLiteral("Paused")));
    state.insert(QStringLiteral("connectionState"),
                 player ? player->connectionState().left(32) : QStringLiteral("idle"));
    state.insert(QStringLiteral("station"), station);
    state.insert(QStringLiteral("title"), title);
    state.insert(QStringLiteral("artist"), artist);
    state.insert(QStringLiteral("artwork"), artwork);
    state.insert(QStringLiteral("favorites"), stationSummaries(m_backend->getFavoriteStations(), 12));
    state.insert(QStringLiteral("recent"), stationSummaries(m_backend->recentStations(), 12));
    return QString::fromUtf8(QJsonDocument(state).toJson(QJsonDocument::Compact));
}

bool BearWaveControlAdaptor::PlayFavoriteByUuid(const QString &uuid, const QString &urlResolved)
{
    return m_backend ? m_backend->playFavoriteByUuid(uuid, urlResolved) : false;
}

bool BearWaveControlAdaptor::PlayRecentByUuid(const QString &uuid, const QString &urlResolved)
{
    return m_backend ? m_backend->playRecentByUuid(uuid, urlResolved) : false;
}

bool BearWaveControlAdaptor::HasNext() const
{
    return m_backend ? m_backend->hasNextStation() : false;
}

bool BearWaveControlAdaptor::HasPrevious() const
{
    return m_backend ? m_backend->hasPreviousStation() : false;
}

void BearWaveControlAdaptor::PlayNext()
{
    if (m_backend) {
        m_backend->playNextStation();
    }
}

void BearWaveControlAdaptor::PlayPrevious()
{
    if (m_backend) {
        m_backend->playPreviousStation();
    }
}

void BearWaveControlAdaptor::PlayPause()
{
    if (m_backend && m_backend->player()) {
        m_backend->player()->togglePlayPause();
    }
}

void BearWaveControlAdaptor::Stop()
{
    if (m_backend && m_backend->player()) {
        m_backend->player()->stop();
    }
}
