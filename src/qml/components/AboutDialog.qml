// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Dialog {
    id: root

    required property bool compactMode
    required property string changelogDocument
    property string appVersion: Qt.application.version
    property string buildId: "?"
    property bool showingChangelog: false

    readonly property string versionLine: {
        var version = root.appVersion;
        if (version === undefined || version === null || version === "")
            version = Qt.application.version;
        var build = root.buildId;
        if (build === undefined || build === null || build === "")
            build = "?";
        return qsTr("Version") + " " + String(version) + " · " + qsTr("build") + " " + String(build);
    }

    modal: true
    anchors.centerIn: parent
    width: compactMode ? 440 : 640
    height: compactMode ? 620 : 680
    padding: 18
    standardButtons: Dialog.NoButton
    onClosed: showingChangelog = false

    Overlay.modal: Rectangle {
        color: BearTheme.scrim
    }

    background: Rectangle {
        color: BearTheme.dialog
        radius: 0
        border.width: 0
    }

    footer: Item {
        implicitHeight: 54

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 18
            anchors.rightMargin: 18
            anchors.bottomMargin: 10
            spacing: 8

            AppButton {
                visible: root.showingChangelog
                text: qsTr("Back")
                onClicked: root.showingChangelog = false
            }

            Item {
                Layout.fillWidth: true
            }

            AppButton {
                text: qsTr("Close")
                onClicked: root.close()
            }
        }
    }

    contentItem: StackLayout {
        currentIndex: root.showingChangelog ? 1 : 0

        ColumnLayout {
            spacing: 16

            Image {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: root.compactMode ? 260 : 340
                Layout.preferredHeight: root.compactMode ? 117 : 153
                source: "qrc:/assets/app/bearwave_line.png"
                fillMode: Image.PreserveAspectFit
                smooth: true
                mipmap: true
                sourceSize.width: 446
                sourceSize.height: 200
                Accessible.name: "BearWave"
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 4

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: root.versionLine
                    color: BearTheme.textMuted
                    font.pixelSize: 12
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Author: Sebastian Palencsár")
                    color: BearTheme.textMuted
                    font.pixelSize: 12
                }
            }

            AppButton {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("What's New")
                onClicked: root.showingChangelog = true
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("GNU GPLv3 License")
                        color: BearTheme.textMain
                        font.bold: true
                    }
                    Label {
                        text: qsTr("Copyright (c) 2026")
                        color: BearTheme.textMuted
                        font.pixelSize: 11
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 150
                    radius: 0
                    color: BearTheme.dialogAlt
                    border.width: 0

                    ScrollView {
                        id: licenseScroll
                        anchors.fill: parent
                        anchors.margins: 10
                        clip: true
                        ScrollBar.vertical.policy: ScrollBar.AsNeeded

                        Label {
                            width: Math.max(licenseScroll.width, 1)
                            color: BearTheme.textMain
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            text: "This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.\n\nThis program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>."
                        }
                    }
                }
            }
        }

        ColumnLayout {
            spacing: 10

            RowLayout {
                Layout.fillWidth: true

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("What's New")
                        color: BearTheme.textMain
                        font.bold: true
                        font.pixelSize: 22
                    }

                    Label {
                        Layout.fillWidth: true
                        text: root.versionLine
                        color: BearTheme.textMuted
                        font.pixelSize: 12
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 0
                color: BearTheme.dialogAlt
                border.width: 0

                ScrollView {
                    id: changelogScroll
                    anchors.fill: parent
                    anchors.margins: 14
                    clip: true
                    ScrollBar.vertical.policy: ScrollBar.AsNeeded

                    TextEdit {
                        width: Math.max(changelogScroll.availableWidth, 1)
                        text: root.changelogDocument.length > 0 ? root.changelogDocument : qsTr("The changelog could not be loaded.")
                        textFormat: TextEdit.MarkdownText
                        color: BearTheme.textMain
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        readOnly: true
                        selectByMouse: true
                        onLinkActivated: function (link) {
                            if (link.indexOf("https://") === 0 || link.indexOf("http://") === 0) {
                                Qt.openUrlExternally(link);
                            }
                        }
                    }
                }
            }
        }
    }

}
