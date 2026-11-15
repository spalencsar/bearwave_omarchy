// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Rectangle {
    id: root

    required property int index
    required property var modelData
    required property var app
    required property bool compactMode
    required property real listWidth

    width: listWidth
    height: 56
    radius: 0
    color: mouse.containsMouse ? BearTheme.cardHover : "transparent"
    border.width: 0

    readonly property bool isCurrent: {
        if (!app.backend || !modelData) return false
        var currentUuid = app.backend.currentStationUuid
        var currentUrl = app.backend.currentStationUrl
        var cardUuid = modelData.uuid || ""
        var cardUrl = modelData.urlResolved || modelData.url || ""
        if (currentUuid !== "" && cardUuid !== "")
            return currentUuid === cardUuid
        return currentUrl !== "" && currentUrl === cardUrl
    }
    readonly property bool isPlaying: isCurrent && app.backend && app.backend.player && app.backend.player.playing
    readonly property bool isManualPage: app.currentPage === "mystations"
    readonly property bool isFavorite: !!(modelData && modelData.isFavorite)
    readonly property string countryCodeText: {
        var code = ""
        if (modelData) {
            code = String(modelData.countryCode || modelData.countrycode || "")
            if (code.length !== 2)
                code = String(modelData.country || "")
        }
        return code.length === 2 ? code.toUpperCase() : ""
    }
    readonly property string primaryTag: {
        if (!modelData || !modelData.tags) return ""
        var parts = String(modelData.tags).split(/[,;]/)
        for (var i = 0; i < parts.length; ++i) {
            var t = parts[i].trim()
            if (t.length > 0)
                return t
        }
        return ""
    }
    readonly property int bitrateValue: modelData ? Number(modelData.bitrate || 0) : 0
    readonly property string metaLine: {
        var parts = []
        if (root.countryCodeText.length > 0) parts.push(root.countryCodeText)
        if (root.primaryTag.length > 0) parts.push(root.primaryTag)
        return parts.join("  ·  ")
    }

    Behavior on color { ColorAnimation { duration: 80 } }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        width: 2
        color: BearTheme.accent
        visible: root.isCurrent
    }

    function playStation() {
        if (!app.backend) return
        if (app.currentPage === "favorites") {
            app.backend.playFavoriteStation(index)
        } else if (app.currentPage === "mystations") {
            app.backend.playManualStation(index)
        } else if (app.currentPage === "history") {
            app.backend.playRecentByUuid(modelData.uuid, modelData.urlResolved)
        } else {
            app.backend.playStation(index)
        }
    }

    // Visual-only control. Hover comes from the card MouseArea so ListView
    // cannot share containsMouse across sibling areas.
    component HitButton: Rectangle {
        id: hit
        property string iconName: "play"
        property bool active: false
        property bool fillAccent: false
        property bool hovered: false
        property string toolTip: ""

        width: 40
        height: 40
        radius: BearTheme.cornerRadius
        color: {
            if (hit.fillAccent)
                return hit.hovered ? Qt.lighter(BearTheme.accent, 1.12) : BearTheme.accent
            if (hit.hovered)
                return BearTheme.selectionHover
            if (hit.active)
                return BearTheme.controlActive
            return "transparent"
        }

        MediaIcon {
            anchors.centerIn: parent
            width: 20
            height: 20
            name: hit.iconName
            color: hit.fillAccent ? BearTheme.onAccent
                   : (hit.active || hit.hovered ? BearTheme.accent : BearTheme.textMain)
        }

        ToolTip.visible: hit.hovered && hit.toolTip.length > 0
        ToolTip.text: hit.toolTip
        ToolTip.delay: 400
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 8
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            StationLogo {
                Layout.preferredWidth: 36
                Layout.preferredHeight: 36
                app: root.app
                stationName: root.modelData.name || ""
                stationKey: root.modelData.uuid
                            || root.modelData.urlResolved
                            || root.modelData.url
                            || root.modelData.name
                            || ""
                faviconUrl: root.modelData.favicon || ""
                homepageUrl: root.modelData.homepage || ""
                logoMargin: 2
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 1

                Label {
                    Layout.fillWidth: true
                    text: root.modelData.name || ""
                    textFormat: Text.PlainText
                    color: root.isCurrent ? BearTheme.accent : BearTheme.textMain
                    font.pixelSize: 13
                    font.bold: root.isCurrent
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    visible: root.metaLine.length > 0
                    text: root.metaLine
                    textFormat: Text.PlainText
                    color: BearTheme.textMuted
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }
            }

            Label {
                visible: root.bitrateValue > 0
                text: root.bitrateValue + "k"
                textFormat: Text.PlainText
                color: root.isCurrent ? BearTheme.textMain : BearTheme.textMuted
                font.pixelSize: 11
                Layout.preferredWidth: 40
                horizontalAlignment: Text.AlignRight
            }
        }

        Row {
            spacing: 4
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: 84
            Layout.minimumWidth: 84

            HitButton {
                id: trashHit
                visible: root.isManualPage
                iconName: "trash"
                hovered: mouse.overTrash
                toolTip: qsTr("Remove from My stations")
            }

            HitButton {
                id: favHit
                visible: !root.isManualPage
                iconName: root.isFavorite ? "heart" : "heartOutline"
                active: root.isFavorite
                hovered: mouse.overFav
                toolTip: root.isFavorite
                         ? qsTr("Remove from favorites")
                         : qsTr("Add to favorites")
            }

            HitButton {
                id: playHit
                iconName: root.isPlaying ? "pause" : "play"
                fillAccent: root.isPlaying
                hovered: mouse.overPlay
                toolTip: root.isPlaying ? qsTr("Pause") : qsTr("Play")
            }
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        z: 10

        property bool overFav: false
        property bool overPlay: false
        property bool overTrash: false

        function hitsItem(item) {
            if (!item || !item.visible)
                return false
            var p = mapToItem(item, mouseX, mouseY)
            return p.x >= 0 && p.y >= 0 && p.x < item.width && p.y < item.height
        }

        function updateHits() {
            if (!containsMouse) {
                overFav = false
                overPlay = false
                overTrash = false
                return
            }
            overTrash = hitsItem(trashHit)
            overFav = hitsItem(favHit)
            overPlay = hitsItem(playHit)
        }

        onPositionChanged: updateHits()
        onContainsMouseChanged: updateHits()
        onExited: {
            overFav = false
            overPlay = false
            overTrash = false
        }

        onClicked: {
            if (!app.backend)
                return
            if (overTrash) {
                app.backend.removeManualStation(root.modelData.uuid || "",
                                               root.modelData.urlResolved || root.modelData.url || "")
                app.toast(qsTr("Removed from My stations"))
                return
            }
            if (overFav) {
                app.backend.toggleFavoriteById(root.modelData.uuid, root.modelData.urlResolved)
                app.toast(root.isFavorite
                          ? qsTr("Removed from favorites")
                          : qsTr("Added to favorites"))
                return
            }
            if (root.isCurrent)
                app.backend.player.togglePlayPause()
            else
                playStation()
        }
    }
}
