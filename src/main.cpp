// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDebug>
#include <QFile>
#include <QFont>
#include <QHash>
#include <QIcon>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QStyleHints>
#include <QTextStream>
#include <QTranslator>

#include <memory>

#include "appsettings.h"
#include "radiobackend.h"
#include "mprisadaptor.h"
#include "bearwavecontroladaptor.h"
#include "notificationmanager.h"
#include "systemtraymanager.h"
#include "desktopenvironment.h"
#include "omarchythemecontroller.h"

namespace {
void configureOmarchyChrome()
{
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        qputenv("QT_QUICK_CONTROLS_STYLE", "Fusion");
    }
    const QByteArray platformTheme = qgetenv("QT_QPA_PLATFORMTHEME").toLower();
    if (platformTheme.contains("kde") || platformTheme.contains("plasma")) {
        qunsetenv("QT_QPA_PLATFORMTHEME");
    }
}
} // namespace

int main(int argc, char *argv[])
{
    for (int index = 1; index < argc; ++index) {
        if (QString::fromLocal8Bit(argv[index]) != QStringLiteral("--widget-state")) {
            continue;
        }

        QCoreApplication stateApp(argc, argv);
        QDBusConnection stateBus = QDBusConnection::sessionBus();
        const bool stateServiceRunning = stateBus.isConnected()
            && stateBus.interface()
            && stateBus.interface()->isServiceRegistered(
                QStringLiteral("org.mpris.MediaPlayer2.bearwave"));
        if (!stateServiceRunning) {
            QTextStream(stdout) << "{\"running\":false}\n";
            return 0;
        }

        QDBusMessage stateCall = QDBusMessage::createMethodCall(
            QStringLiteral("org.mpris.MediaPlayer2.bearwave"),
            QStringLiteral("/org/mpris/MediaPlayer2"),
            QStringLiteral("de.nerdbear.BearWave.Control"),
            QStringLiteral("GetWidgetState"));
        const QDBusReply<QString> stateReply = stateBus.call(stateCall);
        if (!stateReply.isValid()) {
            QTextStream(stdout) << "{\"running\":false}\n";
            return 1;
        }
        QTextStream(stdout) << stateReply.value() << '\n';
        return 0;
    }

    const DesktopEnvironment::Type desktop = DesktopEnvironment::detect();
    configureOmarchyChrome();

    QApplication app(argc, argv);

    // Must run before any QML is loaded.
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")
        || qEnvironmentVariable("QT_QUICK_CONTROLS_STYLE") == QLatin1String("Fusion")) {
        QQuickStyle::setStyle(QStringLiteral("Fusion"));
    }

    QApplication::setApplicationName(QStringLiteral("BearWave"));
    QApplication::setDesktopFileName(QStringLiteral("de.nerdbear.bearwave"));
    QApplication::setOrganizationName(QStringLiteral("BearWave"));
    QApplication::setApplicationVersion(QStringLiteral(BEARWAVE_VERSION));
    if (desktop == DesktopEnvironment::Type::Omarchy) {
        QFont omarchyFont(QStringLiteral("monospace"));
        omarchyFont.setStyleHint(QFont::Monospace);
        app.setFont(omarchyFont);
    }
    app.setProperty("desktopEnvironment", DesktopEnvironment::name(desktop));
    qInfo().noquote() << QStringLiteral("Desktop environment: %1")
                             .arg(DesktopEnvironment::name(desktop));

    QCommandLineParser commandLine;
    commandLine.setApplicationDescription(QStringLiteral("Desktop internet radio player"));
    commandLine.addHelpOption();
    commandLine.addVersionOption();
    const QCommandLineOption languageOption(
        {QStringLiteral("l"), QStringLiteral("language")},
        QStringLiteral("Temporarily override UI and country-name language: system, de, en, nl, or ru"),
        QStringLiteral("language"));
    commandLine.addOption(languageOption);
    const QCommandLineOption noTrayOption(
        QStringLiteral("no-tray"),
        QStringLiteral("Disable the system tray icon (used by the Omarchy bar plugin)"));
    commandLine.addOption(noTrayOption);
    const QCommandLineOption backgroundOption(
        QStringLiteral("background"),
        QStringLiteral("Run as the invisible BearWave widget playback service"));
    commandLine.addOption(backgroundOption);
    const QCommandLineOption widgetStateOption(
        QStringLiteral("widget-state"),
        QStringLiteral("Print the running widget service state as compact JSON and exit"));
    commandLine.addOption(widgetStateOption);
    commandLine.process(app);

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    const bool serviceRunning = sessionBus.isConnected()
        && sessionBus.interface()
        && sessionBus.interface()->isServiceRegistered(QStringLiteral("org.mpris.MediaPlayer2.bearwave"));

    AppSettings appSettings;
    QLocale appLocale = QLocale::system();
    const QString requestedLanguage =
        commandLine.isSet(languageOption)
        ? commandLine.value(languageOption).trimmed().toLower()
        : appSettings.language();
    if (requestedLanguage != QStringLiteral("system")) {
        static const QHash<QString, QString> localeNames = {
            {QStringLiteral("de"), QStringLiteral("de_DE")},
            {QStringLiteral("en"), QStringLiteral("en_US")},
            {QStringLiteral("nl"), QStringLiteral("nl_NL")},
            {QStringLiteral("ru"), QStringLiteral("ru_RU")},
        };
        const auto localeName = localeNames.constFind(requestedLanguage);
        if (localeName != localeNames.constEnd()) {
            appLocale = QLocale(*localeName);
        } else {
            qWarning().noquote()
                << QStringLiteral("Unsupported language '%1'; using system locale %2.")
                       .arg(requestedLanguage, appLocale.name());
        }
    }
    QLocale::setDefault(appLocale);

    if (serviceRunning) {
        if (commandLine.isSet(backgroundOption)) {
            return 0;
        }
        QDBusMessage call = QDBusMessage::createMethodCall(
            QStringLiteral("org.mpris.MediaPlayer2.bearwave"),
            QStringLiteral("/org/mpris/MediaPlayer2"),
            QStringLiteral("org.mpris.MediaPlayer2"),
            QStringLiteral("Raise")
        );
        sessionBus.call(call);
        qInfo() << "BearWave is already running; raised the existing window.";
        return 0;
    }
    app.setWindowIcon(QIcon(QStringLiteral(":/assets/app/bearwave.svg")));
    app.setQuitOnLastWindowClosed(false);

    QTranslator appTranslator;
    const QString uiLanguage = appLocale.name().section(QLatin1Char('_'), 0, 0);
    const QString translationPath =
        QStringLiteral(":/i18n/bearwave_%1.qm").arg(uiLanguage);
    if (appTranslator.load(translationPath)) {
        app.installTranslator(&appTranslator);
        qInfo().noquote() << QStringLiteral("Application locale: %1 (translation loaded)")
                                .arg(appLocale.name());
    } else {
        qInfo().noquote() << QStringLiteral("Application locale: %1 (English UI fallback)")
                                .arg(appLocale.name());
    }

    // QML disk cache can retain stale bytecode across app updates (same qrc paths).
    qputenv("QML_DISABLE_DISK_CACHE", "1");

    QQmlApplicationEngine engine;
    engine.addImportPath(QStringLiteral("qrc:/qml"));

    RadioBackend backend;
    OmarchyThemeController omarchyTheme(desktop == DesktopEnvironment::Type::Omarchy);
    QFile changelogFile(QStringLiteral(":/CHANGELOG.md"));
    QString changelogText;
    if (changelogFile.open(QIODevice::ReadOnly)) {
        changelogText = QString::fromUtf8(changelogFile.readAll());
    }
    engine.rootContext()->setContextProperty("radioBackend", &backend);
    engine.rootContext()->setContextProperty("appLanguageSettings", &appSettings);
    engine.rootContext()->setContextProperty("bearwaveVersion", QStringLiteral(BEARWAVE_VERSION));
    engine.rootContext()->setContextProperty("bearwaveBuildId", QStringLiteral(BEARWAVE_GIT_HASH));
    engine.rootContext()->setContextProperty("bearwaveChangelog", changelogText);
    engine.rootContext()->setContextProperty(
        QStringLiteral("bearwaveDesktopEnvironment"), DesktopEnvironment::name(desktop));
    engine.rootContext()->setContextProperty(
        QStringLiteral("bearwaveIsOmarchy"), desktop == DesktopEnvironment::Type::Omarchy);
    engine.rootContext()->setContextProperty(QStringLiteral("bearwaveOmarchyTheme"), &omarchyTheme);
    auto *mprisRoot = new MprisRootAdaptor(&backend, &app);
    auto *mprisPlayer = new MprisPlayerAdaptor(&backend);
    auto *controlAdaptor = new BearWaveControlAdaptor(&backend);
    auto *notificationManager = new NotificationManager(&backend, &backend);
    Q_UNUSED(mprisRoot)
    Q_UNUSED(controlAdaptor)
    Q_UNUSED(notificationManager)

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [&app, url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            qCritical("Failed to load QML file");
            app.exit(EXIT_FAILURE);
        }
    }, Qt::QueuedConnection);

    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        return EXIT_FAILURE;
    }

    QQuickWindow *mainWindow = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    if (!mainWindow) {
        qCritical("Failed to obtain main window");
        return EXIT_FAILURE;
    }

    QObject::connect(&backend, &RadioBackend::raiseRequested, mainWindow, [mainWindow]() {
        mainWindow->show();
        mainWindow->raise();
        mainWindow->requestActivate();
    });

    if (!sessionBus.registerObject(QStringLiteral("/org/mpris/MediaPlayer2"), &backend, QDBusConnection::ExportAdaptors)) {
        qWarning() << "Failed to register MPRIS D-Bus object";
    }
    if (!sessionBus.registerService(QStringLiteral("org.mpris.MediaPlayer2.bearwave"))) {
        qWarning() << "Failed to register MPRIS D-Bus service org.mpris.MediaPlayer2.bearwave";
    } else {
        mprisPlayer->publishState();
    }

    if (commandLine.isSet(backgroundOption)) {
        mainWindow->hide();
    } else {
        mainWindow->show();
        mainWindow->raise();
        mainWindow->requestActivate();
    }

    std::unique_ptr<SystemTrayManager> trayManager;
    if (!commandLine.isSet(noTrayOption) && !commandLine.isSet(backgroundOption)) {
        trayManager = std::make_unique<SystemTrayManager>(&backend, mainWindow, &app);
    }

    return app.exec();
}
