// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STATIONCATALOG_H
#define STATIONCATALOG_H

#include <QList>
#include <QObject>
#include <QString>
#include <QVariantList>

class RadioBrowser;
class RadioStation;
class StationLibrary;

// Radio-browser station lists, local filter, country picker.
class StationCatalog : public QObject
{
    Q_OBJECT

public:
    explicit StationCatalog(StationLibrary *library, QObject *parent = nullptr);

    const QList<RadioStation *> &all() const { return m_stations; }
    const QList<RadioStation *> &visible() const { return m_filteredStations; }
    QVariantList countries() const { return m_countries; }
    QVariantList countryPickerOptions() const { return m_countryPickerOptions; }
    QString filterQuery() const { return m_filterQuery; }
    QString sortMode() const { return m_sortMode; }
    bool loading() const { return m_loading; }
    QString lastError() const { return m_lastError; }
    bool currentLoadSatisfied() const { return m_currentLoadSatisfied; }

    RadioStation *visibleAt(int index) const;
    void applyFavoriteFlags();
    void applyManualEdits(const QString &oldUuid,
                          const QString &oldUrl,
                          const QString &oldUrlResolved,
                          RadioStation *updated);

    void setFilterQuery(const QString &query);
    void sortStations(const QString &mode);

    void loadGermanStations();
    void loadDutchStations();
    void loadTopStations();
    void loadWorldStations();
    void loadCountries();
    void loadByTag(const QString &tag);
    void loadByCountryCode(const QString &countryCode);
    void searchStations(const QString &query);

signals:
    void stationsChanged();
    void listsChanged();
    void loadingChanged();
    void lastErrorChanged();
    void filterQueryChanged();
    void sortModeChanged();
    void countriesChanged();
    void stationsReloaded();

private slots:
    void onStationsLoaded(const QList<RadioStation *> &stations);
    void onCountriesLoaded(const QVariantList &countries);

private:
    StationLibrary *m_library = nullptr;
    RadioBrowser *m_radioBrowser = nullptr;
    QList<RadioStation *> m_stations;
    QList<RadioStation *> m_filteredStations;
    QVariantList m_countries;
    QVariantList m_countryPickerOptions;
    QString m_filterQuery;
    QString m_sortMode = QStringLiteral("name");
    QString m_lastError;
    bool m_loading = false;
    bool m_currentLoadSatisfied = false;

    void beginLoad();
    void setLoading(bool loading);
    void setLastError(const QString &error);
    void rebuildFilteredStations(bool emitFilterSignal = true);
    void applySort();
    void buildCountryPickerOptions();
};

#endif
