// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Rectangle {
    id: root

    required property var app
    readonly property var player: app.backend ? app.backend.player : null

    function connectionStatusText() {
        if (!player) return qsTr("Inactive")
        if (player.playing) {
            return player.currentNowPlaying.length > 0
                    ? (qsTr("Now playing: ") + player.currentNowPlaying)
                    : qsTr("Now playing: No track info")
        }
        switch (player.connectionState) {
        case "connecting": return qsTr("Connecting…")
        case "buffering": return qsTr("Buffering…")
        case "retrying": return qsTr("Reconnecting…")
        case "paused": return qsTr("Paused")
        case "error": return qsTr("Stream unavailable")
        case "idle": return qsTr("Inactive")
        case "playing":
            return player.currentNowPlaying.length > 0
                    ? (qsTr("Now playing: ") + player.currentNowPlaying)
                    : qsTr("Now playing: No track info")
        default:
            return player.currentNowPlaying.length > 0
                    ? (qsTr("Now playing: ") + player.currentNowPlaying)
                    : qsTr("Now playing: No track info")
        }
    }

    implicitHeight: 96
    radius: 0
    color: BearTheme.panel
    border.width: 0

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 1
        color: BearTheme.cardBorder
        opacity: 0.35
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 14

        Item {
            Layout.preferredWidth: 64
            Layout.preferredHeight: 64

            StationLogo {
                anchors.fill: parent
                app: root.app
                stationName: app.backend && app.backend.currentStation
                             ? (app.backend.currentStation.name || "") : ""
                stationKey: app.backend && app.backend.currentStation
                            ? (app.backend.currentStation.uuid
                               || app.backend.currentStation.urlResolved
                               || app.backend.currentStation.url
                               || app.backend.currentStation.name
                               || "") : ""
                faviconUrl: app.backend && app.backend.currentStation
                            ? (app.backend.currentStation.favicon || "") : ""
                homepageUrl: app.backend && app.backend.currentStation
                             ? (app.backend.currentStation.homepage || "") : ""
                logoMargin: 6
            }
        }

        ColumnLayout {
            Layout.preferredWidth: 280
            Layout.maximumWidth: 320
            spacing: 4

            Label {
                Layout.fillWidth: true
                text: root.player && root.player.currentStationName.length > 0
                      ? root.player.currentStationName : qsTr("No station selected")
                textFormat: Text.PlainText
                color: BearTheme.textMain
                font.pixelSize: 14
                font.bold: true
                elide: Text.ElideRight
            }

            Label {
                Layout.fillWidth: true
                text: root.connectionStatusText()
                textFormat: Text.PlainText
                color: root.player && root.player.playing ? BearTheme.textMain : BearTheme.textMuted
                font.pixelSize: 11
                elide: Text.ElideRight
            }
        }

        Item { Layout.fillWidth: true }

        PlayerTransport {
            Layout.preferredWidth: Math.min(620, Math.max(360, root.width * 0.5))
            Layout.preferredHeight: 40
            app: root.app
            controlHeight: 36
            controlWidth: 38
        }
    }
}
