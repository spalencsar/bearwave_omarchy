// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

// Shared transport with MediaIcon (no emoji).
Item {
    id: root

    required property var app
    property int controlHeight: 34
    property int controlWidth: 40

    readonly property var player: app && app.backend ? app.backend.player : null
    readonly property var backend: app ? app.backend : null
    readonly property bool canToggle: player
                                      && ["connecting", "retrying"]
                                         .indexOf(player.connectionState) === -1
    readonly property bool isMuted: player && player.volume <= 0.001

    property real lastVolume: 0.5

    function toggleMute() {
        if (!player)
            return
        if (player.volume > 0.001) {
            lastVolume = player.volume
            player.setVolume(0)
        } else {
            player.setVolume(lastVolume > 0.05 ? lastVolume : 0.5)
        }
    }

    implicitHeight: controlHeight
    implicitWidth: 280

    RowLayout {
        anchors.fill: parent
        spacing: 6

        IconButton {
            iconName: "prev"
            iconSize: 16
            implicitWidth: root.controlWidth
            implicitHeight: root.controlHeight
            enabled: root.backend ? root.backend.hasPreviousStation() : false
            onClicked: if (root.backend) root.backend.playPreviousStation()
        }
        IconButton {
            iconName: root.player && root.player.playing ? "pause" : "play"
            iconSize: 16
            implicitWidth: root.controlWidth
            implicitHeight: root.controlHeight
            highlighted: true
            enabled: root.canToggle
            onClicked: if (root.player) root.player.togglePlayPause()
        }
        IconButton {
            iconName: "stop"
            iconSize: 14
            implicitWidth: root.controlWidth
            implicitHeight: root.controlHeight
            onClicked: if (root.player) root.player.stop()
        }
        IconButton {
            iconName: "next"
            iconSize: 16
            implicitWidth: root.controlWidth
            implicitHeight: root.controlHeight
            enabled: root.backend ? root.backend.hasNextStation() : false
            onClicked: if (root.backend) root.backend.playNextStation()
        }
        IconButton {
            iconName: root.isMuted ? "mute" : "volume"
            iconSize: 18
            implicitWidth: root.controlWidth
            implicitHeight: root.controlHeight
            ToolTip.visible: hovered
            ToolTip.text: root.isMuted ? qsTr("Unmute") : qsTr("Mute")
            onClicked: root.toggleMute()
        }
        Slider {
            Layout.fillWidth: true
            Layout.preferredHeight: root.controlHeight
            from: 0
            to: 1
            value: root.player ? root.player.volume : 0.5
            onMoved: if (root.player) root.player.setVolume(value)
        }
    }
}
