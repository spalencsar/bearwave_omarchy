// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

pragma Singleton
import QtQuick 2.15

// Omarchy palette with a small diagnostic fallback when colors.toml is absent.
QtObject {
    id: theme

    readonly property bool isOmarchy: {
        return typeof bearwaveOmarchyTheme !== "undefined"
                && bearwaveOmarchyTheme && bearwaveOmarchyTheme.available
    }

    readonly property bool isLight: {
        if (isOmarchy)
            return !bearwaveOmarchyTheme.darkMode
        return Qt.styleHints.colorScheme === Qt.ColorScheme.Light
    }

    // --- surfaces ---
    readonly property color bgA: isOmarchy ? alpha(bearwaveOmarchyTheme.background, 0.70) : (isLight ? "#f4f4f6" : "#0a0a0b")
    readonly property color bgB: isOmarchy ? alpha(bearwaveOmarchyTheme.darkBackground, 0.76) : (isLight ? "#ececf0" : "#0e0e10")
    readonly property color panel: isOmarchy ? alpha(bearwaveOmarchyTheme.lighterBackground, 0.22) : (isLight ? "#ffffff" : "#121214")
    readonly property color panelAlt: isOmarchy ? alpha(bearwaveOmarchyTheme.darkBackground, 0.34) : (isLight ? "#f7f7f9" : "#161618")
    readonly property color sidebar: isOmarchy ? alpha(bearwaveOmarchyTheme.lighterBackground, 0.16) : (isLight ? "#f0f0f3" : "#0c0c0e")
    readonly property color card: isOmarchy ? "transparent" : (isLight ? "#ffffff" : "#141416")
    readonly property color cardHover: isOmarchy ? bearwaveOmarchyTheme.lighterBackground : (isLight ? "#e8e8ed" : "#1c1c20")
    readonly property color cardBorder: isOmarchy ? alpha(bearwaveOmarchyTheme.foreground, 0.30) : (isLight ? "#d4d4dc" : "#2a2a30")
    readonly property color selection: isOmarchy ? alpha(bearwaveOmarchyTheme.foreground, 0.14) : (isLight ? "#ffe4ee" : "#24242a")
    readonly property color selectionHover: isOmarchy ? alpha(bearwaveOmarchyTheme.foreground, 0.20) : (isLight ? "#ffd6e6" : "#2e2e36")
    readonly property color selectionBorder: isOmarchy ? alpha(bearwaveOmarchyTheme.foreground, 0.4) : (isLight ? "#ff4f86" : "#3a3a44")
    readonly property color imageWell: isOmarchy ? bearwaveOmarchyTheme.lighterBackground : (isLight ? "#e4e4ea" : "#1a1a1e")

    readonly property color accent: isOmarchy ? bearwaveOmarchyTheme.accent : (isLight ? "#e63d72" : "#ff4f86")
    readonly property color textMain: isOmarchy ? bearwaveOmarchyTheme.foreground : (isLight ? "#141418" : "#f2f2f5")
    readonly property color textMuted: isOmarchy ? bearwaveOmarchyTheme.darkForeground : (isLight ? "#5c5c66" : "#9898a0")
    readonly property color warn: isOmarchy ? bearwaveOmarchyTheme.warning : (isLight ? "#c62828" : "#ff6b6b")

    readonly property string fontFamily: isOmarchy ? "monospace" : ""
    readonly property int cornerRadius: isOmarchy ? 0 : 14
    readonly property int smallRadius: isOmarchy ? 0 : 6
    readonly property color control: isOmarchy ? alpha(textMain, 0.04) : (isLight ? "#ebebf0" : "#1a1a1e")
    readonly property color controlHover: isOmarchy ? alpha(textMain, 0.08) : cardHover
    readonly property color controlActive: isOmarchy ? alpha(textMain, 0.18) : (isLight ? "#ffe4ee" : "#2a2430")
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
