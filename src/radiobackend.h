// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RADIOBACKEND_H
#define RADIOBACKEND_H

#include <QObject>
#include <QList>
#include <QVariantMap>
#include <QVariantList>

#include "bearplayer.h"

class RadioStation;
class StationArtworkService;
class StationCatalog;
class StationImageCache;
class StationLibrary;

class RadioBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<QObject*> stations READ stations NOTIFY stationsChanged)
    Q_PROPERTY(QList<QObject*> favoriteStations READ favoriteStations NOTIFY favoritesChanged)
    Q_PROPERTY(QList<QObject*> manualStations READ manualStations NOTIFY manualStationsChanged)
    Q_PROPERTY(QVariantList countryPickerOptions READ countryPickerOptions CONSTANT)
    Q_PROPERTY(BearPlayer* player READ player CONSTANT)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(bool canResumeLastStation READ canResumeLastStation NOTIFY resumeStateChanged)
    Q_PROPERTY(QString lastStationName READ lastStationName NOTIFY resumeStateChanged)
    Q_PROPERTY(QString filterQuery READ filterQuery WRITE setFilterQuery NOTIFY filterQueryChanged)
    Q_PROPERTY(QString sortMode READ sortMode NOTIFY sortModeChanged)
    Q_PROPERTY(QVariantList recentStations READ recentStations NOTIFY listsChanged)
    Q_PROPERTY(QVariantMap selectedStation READ selectedStation NOTIFY selectedStationChanged)
    Q_PROPERTY(QString currentStationUuid READ currentStationUuid NOTIFY currentStationChanged)
    Q_PROPERTY(QString currentStationUrl READ currentStationUrl NOTIFY currentStationChanged)
    Q_PROPERTY(QObject* currentStation READ currentStation NOTIFY currentStationChanged)
    Q_PROPERTY(QVariantList countries READ countries NOTIFY countriesChanged)
    Q_PROPERTY(int stationImageRevision READ stationImageRevision NOTIFY stationImageRevisionChanged)

public:
    explicit RadioBackend(QObject *parent = nullptr);

    QList<QObject*> stations() const;
    QList<QObject*> favoriteStations() const;
    QList<QObject*> manualStations() const;
    QVariantList countryPickerOptions() const;
    QVariantList recentStations() const;
    QVariantMap selectedStation() const { return m_selectedStation; }
    QObject* currentStation() const;
    QString currentStationUuid() const { return m_currentStationUuid; }
    QString currentStationUrl() const { return m_currentStationUrl; }
    BearPlayer* player() const { return m_player; }
    bool loading() const;
    QString lastError() const { return m_lastError; }
    bool canResumeLastStation() const;
    QString lastStationName() const;
    QString filterQuery() const;
    QString sortMode() const;
    int stationImageRevision() const;

    Q_INVOKABLE void setFilterQuery(const QString &query);

    Q_INVOKABLE void loadGermanStations();
    Q_INVOKABLE void loadDutchStations();
    Q_INVOKABLE void loadTopStations();
    Q_INVOKABLE void loadWorldStations();
    Q_INVOKABLE void loadCountries();
    Q_INVOKABLE void loadByTag(const QString &tag);
    Q_INVOKABLE void loadByCountryCode(const QString &countryCode);
    Q_INVOKABLE void searchStations(const QString &query);
    Q_INVOKABLE void playStation(int index);
    Q_INVOKABLE void playFavoriteStation(int index);
    Q_INVOKABLE void playManualStation(int index);
    Q_INVOKABLE void selectStation(int index);
    Q_INVOKABLE void selectFavoriteStation(int index);
    Q_INVOKABLE void selectManualStation(int index);
    Q_INVOKABLE bool selectRecentByUuid(const QString &uuid, const QString &urlResolved = QString());
    Q_INVOKABLE bool playSelectedStation();
    Q_INVOKABLE void playNextStation();
    Q_INVOKABLE void playPreviousStation();
    Q_INVOKABLE bool hasNextStation() const;
    Q_INVOKABLE bool hasPreviousStation() const;
    Q_INVOKABLE void toggleFavorite(int index);
    Q_INVOKABLE void toggleFavoriteById(const QString &uuid, const QString &urlResolved);
    Q_INVOKABLE void addManualStation(const QString &name, const QString &url, const QString &country);
    Q_INVOKABLE void editManualStation(QObject *stationObj, const QString &name, const QString &url, const QString &country);
    Q_INVOKABLE void removeManualStation(const QString &uuid, const QString &urlResolved = QString());
    Q_INVOKABLE void sortStations(const QString &mode);
    Q_INVOKABLE QVariantList getRecentStations() const;
    Q_INVOKABLE QVariantList getFavoriteStations() const;
    Q_INVOKABLE bool playRecentByUuid(const QString &uuid, const QString &urlResolved = QString());
    Q_INVOKABLE bool playFavoriteByUuid(const QString &uuid, const QString &urlResolved = QString());
    Q_INVOKABLE void resumeLastStation();
    Q_INVOKABLE void copyToClipboard(const QString &text) const;
    Q_INVOKABLE bool countryMatches(const QVariantMap &country, const QString &query) const;
    Q_INVOKABLE QString stationImageSource(const QString &remoteUrl);
    Q_INVOKABLE QString stationLogoSource(const QString &stationKey,
                                          const QString &faviconUrl,
                                          const QString &homepageUrl);
    Q_INVOKABLE QString stationInitials(const QString &stationName) const;
    Q_INVOKABLE int stationLogoPaletteIndex(const QString &stationKey,
                                            int paletteSize) const;

    QVariantList countries() const;

signals:
    void stationsChanged();
    void favoritesChanged();
    void manualStationsChanged();
    void listsChanged();
    void loadingChanged();
    void lastErrorChanged();
    void resumeStateChanged();
    void filterQueryChanged();
    void sortModeChanged();
    void currentStationChanged();
    void selectedStationChanged();
    void raiseRequested();
    void countriesChanged();
    void stationImageRevisionChanged();

private:
    StationLibrary *m_library = nullptr;
    StationCatalog *m_catalog = nullptr;
    StationImageCache *m_stationImageCache = nullptr;
    StationArtworkService *m_stationArtworkService = nullptr;
    BearPlayer *m_player = nullptr;
    int m_currentIndex = -1;
    bool m_currentFromFavorites = false;
    bool m_currentFromManual = false;
    bool m_currentFromHistory = false;
    bool m_standalonePlayback = false;
    QString m_lastError;
    QVariantMap m_selectedStation;
    QString m_currentStationUuid;
    QString m_currentStationUrl;

    void setupConnections();
    void setLastError(const QString &error);
    QList<RadioStation*> currentList() const;
    void playCurrentSelection();
    void playHistoryAtIndex(int index, bool updateRecent);
    bool startPlayback(const QString &url, const QString &name);
    void syncStationListIndex();
    void setSelectedStation(const QVariantMap &station);
    void savePlaybackState();
    void onCatalogStationsChanged();
    void onCatalogReloaded();
};

#endif
