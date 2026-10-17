// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import theme 1.0

// Circular/rounded button with MediaIcon (no emoji).
Button {
    id: control

    property string iconName: "play"
    property int iconSize: 20
    property bool round: false
    property bool accentFill: false
    property color iconColor: {
        if (!control.enabled)
            return BearTheme.textMuted
        if (control.accentFill)
            return BearTheme.onAccent
        if (control.highlighted || control.down)
            return BearTheme.accent
        return BearTheme.textMain
    }

    focusPolicy: Qt.TabFocus
    implicitWidth: 44
    implicitHeight: 44
    padding: 0

    contentItem: Item {
        MediaIcon {
            anchors.centerIn: parent
            width: control.iconSize
            height: control.iconSize
            name: control.iconName
            color: control.iconColor
        }
    }

    background: Rectangle {
        radius: control.round && !BearTheme.isOmarchy ? width / 2 : BearTheme.cornerRadius
        color: {
            if (control.accentFill)
                return control.down ? Qt.darker(BearTheme.accent, 1.22)
                       : (control.hovered ? Qt.lighter(BearTheme.accent, 1.12) : BearTheme.accent)
            if (control.flat && !control.hovered && !control.highlighted && !control.down)
                return "transparent"
            if (control.highlighted || control.down)
                return BearTheme.controlActive
            if (control.hovered)
                return BearTheme.cardHover
            return BearTheme.control
        }
        border.width: control.visualFocus ? 1.5 : (control.highlighted && !control.accentFill ? 1 : 0)
        border.color: control.visualFocus || control.highlighted ? BearTheme.accent : "transparent"
        opacity: control.enabled ? 1.0 : 0.5
        Behavior on color { ColorAnimation { duration: 100 } }
    }
}
