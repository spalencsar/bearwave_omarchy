import QtQuick
import qs.Commons

Item {
  property alias text: label.text
  property color foreground: Color.foreground
  property string fontFamily: Style.font.family
  height: Style.space(54)

  Text {
    id: label
    anchors.centerIn: parent
    textFormat: Text.PlainText
    color: Qt.darker(parent.foreground, 1.45)
    font.family: parent.fontFamily
    font.pixelSize: Style.font.bodySmall
  }
}
