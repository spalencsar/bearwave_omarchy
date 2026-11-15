import QtQuick
import qs.Commons
import qs.Commons as Commons
import qs.Ui

Button {
  id: root

  required property var station
  property bool playing: false
  signal activated()

  height: Style.space(62)
  leftAlign: true
  bordered: false
  selected: active
  foreground: Commons.Color.foreground
  horizontalPadding: Style.space(8)
  verticalPadding: Style.space(6)
  onClicked: activated()

  Item {
    anchors.fill: parent
    anchors.leftMargin: Style.space(8)
    anchors.rightMargin: Style.space(8)

    Row {
      anchors.fill: parent
      spacing: Style.space(10)

      Artwork {
        width: parent.height - Style.space(8)
        height: width
        anchors.verticalCenter: parent.verticalCenter
        sourceUrl: {
          var value = String(root.station.favicon || "")
          return value.indexOf("https://") === 0 || value.indexOf("file://") === 0 ? value : ""
        }
        fallbackText: String(root.station.name || "BW").slice(0, 2).toUpperCase()
        foreground: root.foreground
        accent: root.accent
      }

      Column {
        width: parent.width - Style.space(94)
        anchors.verticalCenter: parent.verticalCenter
        spacing: Style.space(3)
        Text {
          width: parent.width
          textFormat: Text.PlainText
          text: String(root.station.name || "Unbekannter Sender")
          color: root.foreground
          font.family: root.fontFamily
          font.pixelSize: Style.font.body
          font.bold: root.active
          elide: Text.ElideRight
        }
        Text {
          width: parent.width
          textFormat: Text.PlainText
          text: [root.station.country, root.station.codec].filter(function(value) { return String(value || "") !== "" }).join("  ·  ")
          color: Qt.darker(root.foreground, 1.35)
          font.family: root.fontFamily
          font.pixelSize: Style.font.bodySmall
          elide: Text.ElideRight
        }
      }

      Text {
        width: Style.space(24)
        anchors.verticalCenter: parent.verticalCenter
        textFormat: Text.PlainText
        text: root.active && root.playing ? "󰏤" : "󰐊"
        color: root.active ? root.accent : root.foreground
        font.family: root.fontFamily
        font.pixelSize: Style.font.icon
        horizontalAlignment: Text.AlignHCenter
      }
    }
  }
}
