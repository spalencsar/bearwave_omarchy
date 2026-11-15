// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stationcatalog.h"

#include "countrynames.h"
#include "radiobrowser.h"
#include "radiostation.h"
#include "stationidentity.h"
#include "stationlibrary.h"

#include <QCoreApplication>
#include <QDebug>
#include <QLocale>
#include <algorithm>

StationCatalog::StationCatalog(StationLibrary *library, QObject *parent)
    : QObject(parent)
    , m_library(library)
{
    m_radioBrowser = new RadioBrowser(this);
    connect(m_radioBrowser, &RadioBrowser::stationsLoaded,
            this, &StationCatalog::onStationsLoaded);
    connect(m_radioBrowser, &RadioBrowser::countriesLoaded,
            this, &StationCatalog::onCountriesLoaded);
    connect(m_radioBrowser, &RadioBrowser::error,
            this, [this](const QString &error) {
        qWarning() << "RadioBrowser error:" << error;
        setLoading(false);
        if (!m_currentLoadSatisfied) {
            setLastError(error);
        } else {
            setLastError(QString());
        }
    });
    buildCountryPickerOptions();
}

RadioStation *StationCatalog::visibleAt(int index) const
{
    if (index < 0 || index >= m_filteredStations.size()) {
        return nullptr;
    }
    return m_filteredStations[index];
}

void StationCatalog::applyFavoriteFlags()
{
    if (!m_library) {
        return;
    }
    for (RadioStation *station : m_stations) {
        station->setIsFavorite(
            m_library->containsFavorite(station->uuid(), station->urlResolved()));
    }
}

void StationCatalog::applyManualEdits(const QString &oldUuid,
                                      const QString &oldUrl,
                                      const QString &oldUrlResolved,
                                      RadioStation *updated)
{
    if (!updated) {
        return;
    }
    for (RadioStation *station : m_stations) {
        if (station == updated
            || StationIdentity::matches(station, oldUuid, oldUrlResolved)
            || (station->uuid().isEmpty()
                && (station->url() == oldUrl || station->urlResolved() == oldUrlResolved))) {
            station->setName(updated->name());
            station->setUrl(updated->url());
            station->setUrlResolved(updated->urlResolved());
            station->setCountry(updated->country());
        }
    }
    emit stationsChanged();
}

void StationCatalog::setFilterQuery(const QString &query)
{
    if (m_filterQuery == query && !m_filteredStations.isEmpty() && !m_stations.isEmpty()) {
        return;
    }
    m_filterQuery = query;
    rebuildFilteredStations();
}

void StationCatalog::sortStations(const QString &mode)
{
    const QString next = (mode == QStringLiteral("bitrate") || mode == QStringLiteral("votes"))
                             ? mode
                             : QStringLiteral("name");
    const bool modeChanged = m_sortMode != next;
    m_sortMode = next;
    applySort();
    rebuildFilteredStations(false);
    if (modeChanged) {
        emit sortModeChanged();
    }
}

void StationCatalog::applySort()
{
    if (m_sortMode == QStringLiteral("bitrate")) {
        std::sort(m_stations.begin(), m_stations.end(), [](RadioStation *a, RadioStation *b) {
            return a->bitrate() > b->bitrate();
        });
        return;
    }
    if (m_sortMode == QStringLiteral("votes")) {
        std::sort(m_stations.begin(), m_stations.end(), [](RadioStation *a, RadioStation *b) {
            return a->votes() > b->votes();
        });
        return;
    }
    StationIdentity::sortByName(m_stations);
}

void StationCatalog::loadGermanStations()
{
    beginLoad();
    m_radioBrowser->getGermanStations();
}

void StationCatalog::loadDutchStations()
{
    beginLoad();
    m_radioBrowser->getDutchStations();
}

void StationCatalog::loadTopStations()
{
    beginLoad();
    m_radioBrowser->getTopStations(100);
}

void StationCatalog::loadWorldStations()
{
    beginLoad();
    m_radioBrowser->getWorldStations(200);
}

void StationCatalog::loadCountries()
{
    beginLoad();
    m_radioBrowser->getCountries();
}

void StationCatalog::loadByTag(const QString &tag)
{
    if (tag.trimmed().isEmpty()) {
        return;
    }
    beginLoad();
    m_radioBrowser->getByTag(tag.trimmed());
}

void StationCatalog::loadByCountryCode(const QString &countryCode)
{
    if (countryCode.trimmed().isEmpty()) {
        return;
    }
    beginLoad();
    m_radioBrowser->getByCountry(countryCode.trimmed().toUpper());
}

void StationCatalog::searchStations(const QString &query)
{
    if (query.isEmpty()) {
        return;
    }
    qDebug() << "RadioBackend: searchStations called with query:" << query;
    beginLoad();
    m_radioBrowser->search(query);
}

void StationCatalog::onStationsLoaded(const QList<RadioStation *> &stations)
{
    qDeleteAll(m_stations);
    m_stations.clear();
    m_filteredStations.clear();

    m_stations = stations;
    for (RadioStation *station : m_stations) {
        station->setParent(this);
    }
    applyFavoriteFlags();
    applySort();

    rebuildFilteredStations(false);
    m_currentLoadSatisfied = true;
    setLastError(QString());
    setLoading(false);
    emit stationsReloaded();
    qDebug() << "Loaded" << stations.size() << "stations";
}

void StationCatalog::onCountriesLoaded(const QVariantList &countries)
{
    m_countries.clear();
    for (const QVariant &item : countries) {
        QVariantMap map = item.toMap();
        const QString code = map.value(QStringLiteral("code")).toString();
        const QString englishName = map.value(QStringLiteral("name")).toString();
        map.insert(QStringLiteral("englishName"), englishName);
        map.insert(QStringLiteral("name"), CountryNames::displayName(code, englishName));
        m_countries.append(map);
    }

    std::sort(m_countries.begin(), m_countries.end(), [](const QVariant &a, const QVariant &b) {
        return QString::localeAwareCompare(a.toMap().value(QStringLiteral("name")).toString(),
                                           b.toMap().value(QStringLiteral("name")).toString())
               < 0;
    });
    emit countriesChanged();
    m_currentLoadSatisfied = true;
    setLastError(QString());
    setLoading(false);
}

void StationCatalog::beginLoad()
{
    m_currentLoadSatisfied = false;
    setLoading(true);
}

void StationCatalog::setLoading(bool loading)
{
    if (m_loading == loading) {
        return;
    }
    m_loading = loading;
    emit loadingChanged();
}

void StationCatalog::setLastError(const QString &error)
{
    if (m_lastError == error) {
        return;
    }
    m_lastError = error;
    emit lastErrorChanged();
}

void StationCatalog::rebuildFilteredStations(bool emitFilterSignal)
{
    m_filteredStations.clear();
    if (m_filterQuery.trimmed().isEmpty()) {
        m_filteredStations = m_stations;
    } else {
        const QString lowerQuery = m_filterQuery.toLower();
        for (RadioStation *station : m_stations) {
            if (station->name().toLower().contains(lowerQuery)
                || station->tags().toLower().contains(lowerQuery)
                || station->country().toLower().contains(lowerQuery)) {
                m_filteredStations.append(station);
            }
        }
    }

    if (emitFilterSignal) {
        emit filterQueryChanged();
    }
    emit stationsChanged();
    emit listsChanged();
}

void StationCatalog::buildCountryPickerOptions()
{
    static const char *kCodes[] = {
        "DE", "AT", "CH", "NL", "BE", "LU", "FR", "GB", "IE", "ES", "PT", "IT",
        "PL", "CZ", "SK", "HU", "RO", "BG", "GR", "SE", "NO", "DK", "FI", "IS",
        "RU", "UA", "BY", "EE", "LV", "LT", "US", "CA", "MX", "BR", "AR", "CL",
        "CO", "PE", "AU", "NZ", "JP", "KR", "CN", "TW", "IN", "ID", "TH", "VN",
        "TR", "IL", "EG", "ZA", "NG", "KE", "MA", "AE", "SA", "PH", "MY", "SG"
    };

    QVariantList options;
    QVariantMap none;
    none.insert(QStringLiteral("code"), QString());
    none.insert(QStringLiteral("name"),
                QCoreApplication::translate("RadioBackend", "Not specified"));
    options.append(none);

    QList<QPair<QString, QString>> named;
    named.reserve(int(sizeof(kCodes) / sizeof(kCodes[0])));
    const QLocale locale;
    for (const char *codeC : kCodes) {
        const QString code = QString::fromLatin1(codeC);
        const QString english = QLocale::territoryToString(QLocale::codeToTerritory(code));
        const QString label = CountryNames::displayName(code, english, locale);
        named.append({label, code});
    }
    std::sort(named.begin(), named.end(), [](const QPair<QString, QString> &a,
                                             const QPair<QString, QString> &b) {
        return QString::localeAwareCompare(a.first, b.first) < 0;
    });
    for (const auto &pair : named) {
        QVariantMap row;
        row.insert(QStringLiteral("code"), pair.second);
        row.insert(QStringLiteral("name"), pair.first);
        options.append(row);
    }
    m_countryPickerOptions = options;
}
