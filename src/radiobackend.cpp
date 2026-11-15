// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "radiobackend.h"

#include "bearwavepaths.h"
#include "countrynames.h"
#include "stationartworkservice.h"
#include "stationcatalog.h"
#include "stationidentity.h"
#include "stationimagecache.h"
#include "stationlibrary.h"
#include "stationlogostyle.h"
#include "streamurl.h"
#include "radiostation.h"

#include <QClipboard>
#include <QDebug>
#include <QGuiApplication>

RadioBackend::RadioBackend(QObject *parent)
    : QObject(parent)
{
    m_library = new StationLibrary(this);
    m_catalog = new StationCatalog(m_library, this);
    m_stationImageCache = new StationImageCache(this);
    m_stationArtworkService = new StationArtworkService(m_stationImageCache, this);
    m_player = new BearPlayer(this);
    setupConnections();
    BearwavePaths::maybeMigrateLegacyConfig();
    m_library->load();
    m_player->setVolume(m_library->savedVolume());
}

int RadioBackend::stationImageRevision() const
{
    return (m_stationImageCache ? m_stationImageCache->revision() : 0)
           + (m_stationArtworkService ? m_stationArtworkService->revision() : 0);
}

QString RadioBackend::stationImageSource(const QString &remoteUrl)
{
    return m_stationImageCache
               ? m_stationImageCache->sourceForUrl(remoteUrl)
               : QStringLiteral("qrc:/assets/app/bearwave.svg");
}

QString RadioBackend::stationLogoSource(const QString &stationKey,
                                        const QString &faviconUrl,
                                        const QString &homepageUrl)
{
    return m_stationArtworkService
               ? m_stationArtworkService->sourceForStation(
                     stationKey, faviconUrl, homepageUrl)
               : QString();
}

QString RadioBackend::stationInitials(const QString &stationName) const
{
    return StationLogoStyle::initials(stationName);
}

int RadioBackend::stationLogoPaletteIndex(const QString &stationKey,
                                          int paletteSize) const
{
    return StationLogoStyle::paletteIndex(stationKey, paletteSize);
}

QList<QObject*> RadioBackend::stations() const
{
    QList<QObject*> list;
    for (RadioStation *station : m_catalog->visible()) {
        list.append(station);
    }
    return list;
}

QList<QObject*> RadioBackend::favoriteStations() const
{
    QList<QObject*> list;
    for (RadioStation *station : m_library->favorites()) {
        list.append(station);
    }
    return list;
}

QList<QObject*> RadioBackend::manualStations() const
{
    QList<QObject*> list;
    for (RadioStation *station : m_library->manuals()) {
        list.append(station);
    }
    return list;
}

QVariantList RadioBackend::countryPickerOptions() const
{
    return m_catalog->countryPickerOptions();
}

QVariantList RadioBackend::recentStations() const
{
    return m_library->recentsWithFavoriteFlags();
}

QVariantList RadioBackend::countries() const
{
    return m_catalog->countries();
}

bool RadioBackend::loading() const
{
    return m_catalog->loading();
}

bool RadioBackend::canResumeLastStation() const
{
    return !m_library->lastStationUrl().isEmpty();
}

QString RadioBackend::lastStationName() const
{
    return m_library->lastStationName();
}

QString RadioBackend::filterQuery() const
{
    return m_catalog->filterQuery();
}

QString RadioBackend::sortMode() const
{
    return m_catalog->sortMode();
}

void RadioBackend::setupConnections()
{
    connect(m_catalog, &StationCatalog::stationsChanged,
            this, &RadioBackend::onCatalogStationsChanged);
    connect(m_catalog, &StationCatalog::stationsReloaded,
            this, &RadioBackend::onCatalogReloaded);
    connect(m_catalog, &StationCatalog::listsChanged,
            this, &RadioBackend::listsChanged);
    connect(m_catalog, &StationCatalog::loadingChanged,
            this, &RadioBackend::loadingChanged);
    connect(m_catalog, &StationCatalog::lastErrorChanged, this, [this]() {
        setLastError(m_catalog->lastError());
    });
    connect(m_catalog, &StationCatalog::filterQueryChanged,
            this, &RadioBackend::filterQueryChanged);
    connect(m_catalog, &StationCatalog::sortModeChanged,
            this, &RadioBackend::sortModeChanged);
    connect(m_catalog, &StationCatalog::countriesChanged,
            this, &RadioBackend::countriesChanged);

    connect(m_library, &StationLibrary::favoritesChanged, this, [this]() {
        m_catalog->applyFavoriteFlags();
        emit favoritesChanged();
        emit stationsChanged();
        emit listsChanged();
    });
    connect(m_library, &StationLibrary::manualsChanged,
            this, &RadioBackend::manualStationsChanged);
    connect(m_library, &StationLibrary::manualsChanged,
            this, &RadioBackend::listsChanged);
    connect(m_library, &StationLibrary::recentsChanged,
            this, &RadioBackend::listsChanged);
    connect(m_library, &StationLibrary::resumeStateChanged,
            this, &RadioBackend::resumeStateChanged);

    connect(m_stationImageCache, &StationImageCache::revisionChanged,
            this, &RadioBackend::stationImageRevisionChanged);
    connect(m_stationArtworkService, &StationArtworkService::revisionChanged,
            this, &RadioBackend::stationImageRevisionChanged);

    connect(m_player, &BearPlayer::currentStationChanged, this, [this](const QString &name) {
        if (!name.isEmpty()) {
            m_library->setLastStation(name, m_library->lastStationUrl());
            savePlaybackState();
        } else {
            m_currentStationUuid.clear();
            m_currentStationUrl.clear();
            emit currentStationChanged();
        }
    });

    connect(m_player, &BearPlayer::volumeChanged, this, [this]() {
        savePlaybackState();
    });
}

void RadioBackend::onCatalogStationsChanged()
{
    syncStationListIndex();
    emit stationsChanged();

    if (!m_selectedStation.isEmpty()) {
        const QString selectedUuid = m_selectedStation.value(QStringLiteral("uuid")).toString();
        const QString selectedUrl = m_selectedStation.value(QStringLiteral("urlResolved")).toString();
        bool stillVisible = false;
        for (RadioStation *station : m_catalog->visible()) {
            if (StationIdentity::matches(station, selectedUuid, selectedUrl)) {
                stillVisible = true;
                break;
            }
        }
        if (!stillVisible) {
            setSelectedStation(m_catalog->visible().isEmpty()
                                   ? QVariantMap()
                                   : StationIdentity::toVariantMap(m_catalog->visible().first()));
        }
    } else if (!m_catalog->visible().isEmpty()) {
        setSelectedStation(StationIdentity::toVariantMap(m_catalog->visible().first()));
    }
}

void RadioBackend::onCatalogReloaded()
{
    if (!m_catalog->visible().isEmpty()) {
        setSelectedStation(StationIdentity::toVariantMap(m_catalog->visible().first()));
    } else {
        setSelectedStation(QVariantMap());
    }
    syncStationListIndex();
}

void RadioBackend::loadGermanStations()
{
    m_catalog->loadGermanStations();
}

void RadioBackend::loadDutchStations()
{
    m_catalog->loadDutchStations();
}

void RadioBackend::loadTopStations()
{
    m_catalog->loadTopStations();
}

void RadioBackend::loadWorldStations()
{
    m_catalog->loadWorldStations();
}

void RadioBackend::loadCountries()
{
    m_catalog->loadCountries();
}

bool RadioBackend::countryMatches(const QVariantMap &country, const QString &query) const
{
    return CountryNames::matches(country.value(QStringLiteral("code")).toString(),
                                 country.value(QStringLiteral("englishName")).toString(),
                                 country.value(QStringLiteral("name")).toString(),
                                 query);
}

void RadioBackend::loadByTag(const QString &tag)
{
    m_catalog->loadByTag(tag);
}

void RadioBackend::loadByCountryCode(const QString &countryCode)
{
    m_catalog->loadByCountryCode(countryCode);
}

void RadioBackend::searchStations(const QString &query)
{
    m_catalog->searchStations(query);
}

void RadioBackend::playStation(int index)
{
    if (!m_catalog->visibleAt(index)) {
        return;
    }

    m_currentFromFavorites = false;
    m_currentFromManual = false;
    m_currentFromHistory = false;
    m_standalonePlayback = false;
    m_currentIndex = index;
    playCurrentSelection();
}

void RadioBackend::playFavoriteStation(int index)
{
    if (index < 0 || index >= m_library->favorites().size()) {
        return;
    }
    m_currentFromFavorites = true;
    m_currentFromManual = false;
    m_currentFromHistory = false;
    m_standalonePlayback = false;
    m_currentIndex = index;
    playCurrentSelection();
}

void RadioBackend::playManualStation(int index)
{
    if (index < 0 || index >= m_library->manuals().size()) {
        return;
    }
    m_currentFromFavorites = false;
    m_currentFromManual = true;
    m_currentFromHistory = false;
    m_standalonePlayback = false;
    m_currentIndex = index;
    playCurrentSelection();
}

void RadioBackend::selectStation(int index)
{
    RadioStation *station = m_catalog->visibleAt(index);
    if (!station) {
        return;
    }
    setSelectedStation(StationIdentity::toVariantMap(station));
}

void RadioBackend::selectFavoriteStation(int index)
{
    if (index < 0 || index >= m_library->favorites().size()) {
        return;
    }
    setSelectedStation(StationIdentity::toVariantMap(m_library->favorites().at(index)));
}

void RadioBackend::selectManualStation(int index)
{
    if (index < 0 || index >= m_library->manuals().size()) {
        return;
    }
    setSelectedStation(StationIdentity::toVariantMap(m_library->manuals().at(index)));
}

bool RadioBackend::selectRecentByUuid(const QString &uuid, const QString &urlResolved)
{
    const QVariantList recent = recentStations();
    const int recentIndex = StationIdentity::recentIndex(recent, uuid, urlResolved);
    if (recentIndex < 0) {
        return false;
    }
    setSelectedStation(recent.at(recentIndex).toMap());
    return true;
}

bool RadioBackend::playSelectedStation()
{
    const QString uuid = m_selectedStation.value(QStringLiteral("uuid")).toString();
    const QString urlResolved = m_selectedStation.value(QStringLiteral("urlResolved")).toString();
    if (uuid.isEmpty() && urlResolved.isEmpty()) {
        return false;
    }

    for (int i = 0; i < m_catalog->visible().size(); ++i) {
        if (StationIdentity::matches(m_catalog->visible().at(i), uuid, urlResolved)) {
            playStation(i);
            return true;
        }
    }
    for (int i = 0; i < m_library->favorites().size(); ++i) {
        if (StationIdentity::matches(m_library->favorites().at(i), uuid, urlResolved)) {
            playFavoriteStation(i);
            return true;
        }
    }
    for (int i = 0; i < m_library->manuals().size(); ++i) {
        if (StationIdentity::matches(m_library->manuals().at(i), uuid, urlResolved)) {
            playManualStation(i);
            return true;
        }
    }
    return playRecentByUuid(uuid, urlResolved);
}

void RadioBackend::playNextStation()
{
    if (m_standalonePlayback) {
        return;
    }
    if (m_currentFromHistory) {
        if (m_library->recents().isEmpty()) {
            return;
        }
        const int nextIndex = m_currentIndex < 0 ? 0 : m_currentIndex + 1;
        if (nextIndex >= m_library->recents().size()) {
            return;
        }
        playHistoryAtIndex(nextIndex, false);
        return;
    }

    const QList<RadioStation*> list = currentList();
    if (list.isEmpty() || m_currentIndex < 0) {
        return;
    }
    if (m_currentIndex < list.size() - 1) {
        ++m_currentIndex;
    }
    playCurrentSelection();
}

void RadioBackend::playPreviousStation()
{
    if (m_standalonePlayback) {
        return;
    }
    if (m_currentFromHistory) {
        if (m_currentIndex <= 0) {
            return;
        }
        playHistoryAtIndex(m_currentIndex - 1, false);
        return;
    }

    const QList<RadioStation*> list = currentList();
    if (list.isEmpty() || m_currentIndex <= 0) {
        return;
    }
    --m_currentIndex;
    playCurrentSelection();
}

bool RadioBackend::hasNextStation() const
{
    if (m_standalonePlayback) {
        return false;
    }
    if (m_currentFromHistory) {
        if (m_library->recents().isEmpty()) {
            return false;
        }
        if (m_currentIndex < 0) {
            return true;
        }
        return m_currentIndex < m_library->recents().size() - 1;
    }
    const QList<RadioStation*> list = currentList();
    if (list.isEmpty() || m_currentIndex < 0) {
        return false;
    }
    return m_currentIndex < list.size() - 1;
}

bool RadioBackend::hasPreviousStation() const
{
    if (m_standalonePlayback) {
        return false;
    }
    if (m_currentFromHistory) {
        return m_currentIndex > 0;
    }
    const QList<RadioStation*> list = currentList();
    if (list.isEmpty()) {
        return false;
    }
    return m_currentIndex > 0;
}

void RadioBackend::toggleFavorite(int index)
{
    if (index < 0 || index >= m_catalog->all().size()) {
        return;
    }
    RadioStation *station = m_catalog->all().at(index);
    m_library->toggleFavoriteFromCatalog(station);
    setSelectedStation(StationIdentity::toVariantMap(station));
}

void RadioBackend::toggleFavoriteById(const QString &uuid, const QString &urlResolved)
{
    const int stationIndex = StationIdentity::indexOf(m_catalog->all(), uuid, urlResolved);
    if (stationIndex >= 0) {
        toggleFavorite(stationIndex);
        return;
    }
    if (!m_library->removeFavorite(uuid, urlResolved)) {
        return;
    }
    for (RadioStation *station : m_catalog->all()) {
        if (StationIdentity::matches(station, uuid, urlResolved)) {
            station->setIsFavorite(false);
            setSelectedStation(StationIdentity::toVariantMap(station));
        }
    }
    if (StationIdentity::matchesMap(m_selectedStation, uuid, urlResolved)) {
        QVariantMap station = m_selectedStation;
        station.insert(QStringLiteral("isFavorite"), false);
        setSelectedStation(station);
    }
}

void RadioBackend::addManualStation(const QString &name, const QString &url, const QString &country)
{
    QString error;
    if (!m_library->addOrUpdateManual(name, url, country, tr("My stations"), &error)) {
        if (error == QLatin1String("scheme")) {
            setLastError(tr("Stream URL must use http:// or https://"));
        }
        return;
    }
    setLastError(QString());
    const QString resolvedUrl = url.trimmed();
    for (RadioStation *mine : m_library->manuals()) {
        if (mine->urlResolved() == resolvedUrl || mine->url() == resolvedUrl) {
            setSelectedStation(StationIdentity::toVariantMap(mine));
            break;
        }
    }
}

void RadioBackend::sortStations(const QString &mode)
{
    m_catalog->sortStations(mode);
}

QVariantList RadioBackend::getRecentStations() const
{
    return m_library->recents();
}

QVariantList RadioBackend::getFavoriteStations() const
{
    return m_library->favoriteMaps();
}

bool RadioBackend::playRecentByUuid(const QString &uuid, const QString &urlResolved)
{
    const int recentIndex = m_library->recentIndex(uuid, urlResolved);
    if (recentIndex >= 0) {
        playHistoryAtIndex(recentIndex, true);
        return true;
    }
    for (RadioStation *station : m_catalog->all()) {
        if (StationIdentity::matches(station, uuid, urlResolved)) {
            m_library->recordRecent(StationIdentity::toVariantMap(station));
            playHistoryAtIndex(0, false);
            return true;
        }
    }
    return false;
}

bool RadioBackend::playFavoriteByUuid(const QString &uuid, const QString &urlResolved)
{
    const int index = m_library->favoriteIndex(uuid, urlResolved);
    if (index < 0) {
        return false;
    }
    playFavoriteStation(index);
    return true;
}

void RadioBackend::resumeLastStation()
{
    if (m_library->lastStationUrl().isEmpty()) {
        return;
    }

    m_currentFromFavorites = false;
    m_currentFromManual = false;
    m_currentFromHistory = false;
    m_standalonePlayback = true;
    m_currentIndex = -1;

    const QString lastUrl = m_library->lastStationUrl();
    const QString lastName = m_library->lastStationName().isEmpty()
                                 ? tr("Last played")
                                 : m_library->lastStationName();
    QVariantMap station;
    station.insert(QStringLiteral("name"), lastName);
    station.insert(QStringLiteral("urlResolved"), lastUrl);
    m_library->recordRecent(station);
    savePlaybackState();

    QString resolvedUuid;
    auto matchUrl = [&lastUrl](const RadioStation *station) {
        return station->urlResolved() == lastUrl || station->url() == lastUrl;
    };
    for (const RadioStation *station : m_catalog->all()) {
        if (matchUrl(station)) {
            resolvedUuid = station->uuid();
            break;
        }
    }
    if (resolvedUuid.isEmpty()) {
        for (const RadioStation *station : m_library->favorites()) {
            if (matchUrl(station)) {
                resolvedUuid = station->uuid();
                break;
            }
        }
    }
    if (resolvedUuid.isEmpty()) {
        resolvedUuid = m_library->findUuidByUrl(lastUrl);
    }
    m_currentStationUuid = resolvedUuid;
    m_currentStationUrl = lastUrl;
    emit currentStationChanged();

    if (!startPlayback(lastUrl, lastName)) {
        m_library->clearLastStation();
        m_currentStationUrl.clear();
        m_currentStationUuid.clear();
        emit currentStationChanged();
    }
}

void RadioBackend::copyToClipboard(const QString &text) const
{
    if (text.isEmpty() || !QGuiApplication::clipboard()) {
        return;
    }
    QGuiApplication::clipboard()->setText(text);
}

void RadioBackend::removeManualStation(const QString &uuid, const QString &urlResolved)
{
    RadioStation *station = m_library->takeManual(uuid, urlResolved);
    if (!station) {
        return;
    }
    if (StationIdentity::matches(station, m_currentStationUuid, m_currentStationUrl)) {
        m_currentIndex = -1;
        m_currentFromManual = false;
    }
    if (StationIdentity::matchesMap(m_selectedStation, uuid, urlResolved)
        || StationIdentity::matches(station,
                                    m_selectedStation.value(QStringLiteral("uuid")).toString(),
                                    m_selectedStation.value(QStringLiteral("urlResolved")).toString())) {
        setSelectedStation(m_library->manuals().isEmpty()
                               ? QVariantMap()
                               : StationIdentity::toVariantMap(m_library->manuals().first()));
    }
    station->deleteLater();
}

void RadioBackend::setFilterQuery(const QString &query)
{
    m_catalog->setFilterQuery(query);
}

void RadioBackend::setLastError(const QString &error)
{
    if (m_lastError == error) {
        return;
    }
    m_lastError = error;
    emit lastErrorChanged();
}

QList<RadioStation*> RadioBackend::currentList() const
{
    if (m_currentFromManual) {
        return m_library->manuals();
    }
    if (m_currentFromFavorites) {
        return m_library->favorites();
    }
    return m_catalog->visible();
}

void RadioBackend::playHistoryAtIndex(int index, bool updateRecent)
{
    if (index < 0 || index >= m_library->recents().size()) {
        return;
    }

    const QVariantMap station = m_library->recents().at(index).toMap();
    const QString url = station.value(QStringLiteral("urlResolved")).toString();
    if (url.isEmpty() || !isAllowedStreamUrl(url)) {
        return;
    }

    m_currentFromFavorites = false;
    m_currentFromManual = false;
    m_currentFromHistory = true;
    m_standalonePlayback = false;
    m_currentIndex = index;

    m_library->setLastStation(station.value(QStringLiteral("name")).toString(), url);

    if (updateRecent) {
        m_library->recordRecent(station);
        m_currentIndex = 0;
    }

    setSelectedStation(station);
    savePlaybackState();
    m_currentStationUuid = station.value(QStringLiteral("uuid")).toString();
    m_currentStationUrl = url;
    emit currentStationChanged();
    if (!startPlayback(url, m_library->lastStationName().isEmpty() ? tr("Last played")
                                                                   : m_library->lastStationName())) {
        m_library->setLastStation(m_library->lastStationName(), QString());
        m_currentStationUrl.clear();
        emit currentStationChanged();
    }
}

void RadioBackend::syncStationListIndex()
{
    if (m_currentFromHistory || m_currentFromFavorites || m_currentFromManual) {
        return;
    }

    m_currentIndex = -1;
    for (int i = 0; i < m_catalog->visible().size(); ++i) {
        if (StationIdentity::matches(m_catalog->visible().at(i),
                                     m_currentStationUuid,
                                     m_currentStationUrl)) {
            m_currentIndex = i;
            break;
        }
    }
}

bool RadioBackend::startPlayback(const QString &url, const QString &name)
{
    if (!isAllowedStreamUrl(url)) {
        setLastError(tr("Stream URL must use http:// or https://"));
        qWarning() << "Rejected stream playback for disallowed URL scheme";
        return false;
    }
    setLastError(QString());
    m_player->playUrl(url, name);
    return true;
}

void RadioBackend::playCurrentSelection()
{
    const QList<RadioStation*> list = currentList();
    if (list.isEmpty() || m_currentIndex < 0 || m_currentIndex >= list.size()) {
        return;
    }

    m_currentFromHistory = false;
    m_standalonePlayback = false;

    RadioStation *station = list[m_currentIndex];
    setSelectedStation(StationIdentity::toVariantMap(station));
    QString url = station->urlResolved().isEmpty() ? station->url() : station->urlResolved();
    if (!isAllowedStreamUrl(url)) {
        setLastError(tr("Stream URL must use http:// or https://"));
        qWarning() << "Rejected station list playback for disallowed URL scheme";
        return;
    }
    m_library->setLastStation(station->name(), url);
    m_library->recordRecent(StationIdentity::toVariantMap(station));
    savePlaybackState();
    m_currentStationUuid = station->uuid();
    m_currentStationUrl = url;
    emit currentStationChanged();
    startPlayback(url, station->name());
}

QObject* RadioBackend::currentStation() const
{
    for (RadioStation *station : m_library->manuals()) {
        if (StationIdentity::matches(station, m_currentStationUuid, m_currentStationUrl)) {
            return station;
        }
    }
    for (RadioStation *station : m_library->favorites()) {
        if (StationIdentity::matches(station, m_currentStationUuid, m_currentStationUrl)) {
            return station;
        }
    }
    for (RadioStation *station : m_catalog->all()) {
        if (StationIdentity::matches(station, m_currentStationUuid, m_currentStationUrl)) {
            return station;
        }
    }
    return nullptr;
}

void RadioBackend::setSelectedStation(const QVariantMap &station)
{
    if (m_selectedStation == station) {
        return;
    }
    m_selectedStation = station;
    emit selectedStationChanged();
}

void RadioBackend::savePlaybackState()
{
    m_library->saveState(m_player->volume());
}

void RadioBackend::editManualStation(QObject *stationObj,
                                     const QString &name,
                                     const QString &url,
                                     const QString &country)
{
    RadioStation *station = qobject_cast<RadioStation*>(stationObj);
    if (!station) {
        return;
    }
    const QString oldUrl = station->url();
    const QString oldUrlResolved = station->urlResolved();
    const QString oldUuid = station->uuid();

    QString error;
    if (!m_library->editManual(station, name, url, country, tr("My stations"), &error)) {
        if (error == QLatin1String("scheme")) {
            setLastError(tr("Stream URL must use http:// or https://"));
        }
        return;
    }
    setLastError(QString());
    m_catalog->applyManualEdits(oldUuid, oldUrl, oldUrlResolved, station);

    if (m_currentStationUrl == oldUrl || m_currentStationUrl == oldUrlResolved) {
        m_currentStationUrl = station->urlResolved();
        m_library->setLastStation(station->name(), station->urlResolved());
        savePlaybackState();
        emit currentStationChanged();
    }

    const QString selectedUrl = m_selectedStation.value(QStringLiteral("urlResolved")).toString();
    if (selectedUrl == oldUrl || selectedUrl == oldUrlResolved) {
        setSelectedStation(StationIdentity::toVariantMap(station));
    }
    emit stationsChanged();
}
