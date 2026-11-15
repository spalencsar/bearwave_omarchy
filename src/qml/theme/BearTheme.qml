// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

pragma Singleton
import QtQuick 2.15

// Omarchy surfaces throughout. Live colors come from colors.toml; a dark
// diagnostic palette is used when that file cannot be read.
QtObject {
    id: theme

    readonly property var omarchy: (typeof bearwaveOmarchyTheme !== "undefined"
                                    && bearwaveOmarchyTheme)
                                   ? bearwaveOmarchyTheme : null
    readonly property bool hasPalette: !!(omarchy && omarchy.available)

    readonly property bool isLight: hasPalette ? !omarchy.darkMode : false

    readonly property color sourceBackground: hasPalette ? omarchy.background : "#101014"
    readonly property color sourceDarkBackground: hasPalette ? omarchy.darkBackground : "#0c0c10"
    readonly property color sourceLighterBackground: hasPalette ? omarchy.lighterBackground : "#1a1a22"
    readonly property color sourceForeground: hasPalette ? omarchy.foreground : "#f2f2f5"
    readonly property color sourceDarkForeground: hasPalette ? omarchy.darkForeground : "#9898a0"
    readonly property color sourceAccent: hasPalette ? omarchy.accent : "#c9b37a"
    readonly property color sourceWarning: hasPalette ? omarchy.warning : "#ff6b6b"

    readonly property color bgA: alpha(sourceBackground, 0.70)
    readonly property color bgB: alpha(sourceDarkBackground, 0.76)
    readonly property color panel: alpha(sourceLighterBackground, 0.22)
    readonly property color panelAlt: alpha(sourceDarkBackground, 0.34)
    readonly property color sidebar: alpha(sourceLighterBackground, 0.16)
    readonly property color dialog: sourceBackground
    readonly property color dialogAlt: sourceLighterBackground
    readonly property color card: "transparent"
    readonly property color cardHover: sourceLighterBackground
    readonly property color cardBorder: alpha(sourceForeground, 0.30)
    readonly property color selection: alpha(sourceForeground, 0.14)
    readonly property color selectionHover: alpha(sourceForeground, 0.20)
    readonly property color selectionBorder: alpha(sourceForeground, 0.4)
    readonly property color imageWell: sourceLighterBackground

    readonly property color accent: sourceAccent
    readonly property color textMain: sourceForeground
    readonly property color textMuted: sourceDarkForeground
    readonly property color warn: sourceWarning

    readonly property string fontFamily: "monospace"
    readonly property int cornerRadius: 0
    readonly property int smallRadius: 0
    readonly property color control: alpha(textMain, 0.04)
    readonly property color controlHover: alpha(textMain, 0.08)
    readonly property color controlActive: alpha(textMain, 0.18)
    readonly property color scrim: alpha("#000000", isLight ? 0.4 : 0.6)
    readonly property color onAccent: contrastText(accent)

    function alpha(colorValue, amount) {
        return Qt.rgba(colorValue.r, colorValue.g, colorValue.b, amount)
    }

    function contrastText(colorValue) {
        var luminance = 0.2126 * colorValue.r + 0.7152 * colorValue.g + 0.0722 * colorValue.b
        return luminance > 0.56 ? "#090909" : "#ffffff"
    }

    readonly property var worldTags: [
        { name: qsTr("Pop"), tag: "pop", icon: "🎵" },
        { name: qsTr("Rock"), tag: "rock", icon: "🎸" },
        { name: qsTr("Electronic"), tag: "electronic", icon: "🎹" },
        { name: qsTr("Classical"), tag: "classical", icon: "🎻" },
        { name: qsTr("Jazz"), tag: "jazz", icon: "🎷" },
        { name: qsTr("Metal"), tag: "metal", icon: "🤘" },
        { name: qsTr("Hip Hop"), tag: "hiphop", icon: "🎤" },
        { name: qsTr("Chillout"), tag: "chillout", icon: "🌴" },
        { name: qsTr("News / Talk"), tag: "news", icon: "📻" },
        { name: qsTr("Soundtracks"), tag: "soundtrack", icon: "🎬" },
        { name: qsTr("Ambient"), tag: "ambient", icon: "🌌" },
        { name: qsTr("Blues / Soul"), tag: "blues", icon: "🎺" }
    ]
}
