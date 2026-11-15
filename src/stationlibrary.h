// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STATIONLIBRARY_H
#define STATIONLIBRARY_H

#include <QList>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class RadioStation;

// Persisted user stations: favorites, my-stations, recents, last-played.
class StationLibrary : public QObject
{
    Q_OBJECT

public:
    explicit StationLibrary(QObject *parent = nullptr);

    void load();
    void saveState(double volume) const;

    const QList<RadioStation *> &favorites() const { return m_favorites; }
    const QList<RadioStation *> &manuals() const { return m_manuals; }
    const QVariantList &recents() const { return m_recents; }

    QVariantList recentsWithFavoriteFlags() const;
    QVariantList favoriteMaps() const;

    QString lastStationName() const { return m_lastStationName; }
    QString lastStationUrl() const { return m_lastStationUrl; }
    double savedVolume() const { return m_savedVolume; }
    void setLastStation(const QString &name, const QString &url);
    void clearLastStation();

    bool containsFavorite(const QString &uuid, const QString &urlResolved) const;
    int favoriteIndex(const QString &uuid, const QString &urlResolved) const;
    int manualIndex(const QString &uuid, const QString &urlResolved) const;
    int recentIndex(const QString &uuid, const QString &urlResolved) const;

    bool toggleFavoriteFromCatalog(RadioStation *station);
    bool removeFavorite(const QString &uuid, const QString &urlResolved);
    void applyFavoriteEdits(const QString &oldUuid,
                            const QString &oldUrl,
                            const QString &oldUrlResolved,
                            RadioStation *updated);

    bool addOrUpdateManual(const QString &name,
                           const QString &url,
                           const QString &country,
                           const QString &defaultCountry,
                           QString *error);
    bool editManual(RadioStation *station,
                    const QString &name,
                    const QString &url,
                    const QString &country,
                    const QString &defaultCountry,
                    QString *error);
    RadioStation *takeManual(const QString &uuid, const QString &urlResolved);

    void recordRecent(const QVariantMap &stationData);
    QString findUuidByUrl(const QString &url) const;

signals:
    void favoritesChanged();
    void manualsChanged();
    void recentsChanged();
    void resumeStateChanged();

private:
    QList<RadioStation *> m_favorites;
    QList<RadioStation *> m_manuals;
    QVariantList m_recents;
    QString m_lastStationName;
    QString m_lastStationUrl;
    double m_savedVolume = 0.5;

    void loadFavorites();
    void saveFavorites() const;
    void loadManuals();
    void saveManuals() const;
    void loadState();
};

#endif
