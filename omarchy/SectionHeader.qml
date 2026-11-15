import QtQuick
import qs.Commons
import qs.Ui

Item {
  id: root
  property string title: ""
  property bool showAll: false
  signal activated()
  height: Style.space(28)

  Text {
    anchors.left: parent.left
    anchors.verticalCenter: parent.verticalCenter
    textFormat: Text.PlainText
    text: root.title
    color: Color.foreground
    font.family: Style.font.family
    font.pixelSize: Style.font.bodySmall
    font.bold: true
    font.letterSpacing: 1
  }

  Button {
    anchors.right: parent.right
    anchors.verticalCenter: parent.verticalCenter
    visible: root.showAll
    text: "ALLE"
    onClicked: root.activated()
  }
}
