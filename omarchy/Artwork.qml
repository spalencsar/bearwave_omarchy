import QtQuick
import qs.Commons
import qs.Commons as Commons
import qs.Ui

BorderSurface {
  id: root

  property string sourceUrl: ""
  property string fallbackText: "BW"
  property color foreground: Commons.Color.foreground
  property color accent: Commons.Color.accent

  radius: Style.cornerRadius
  color: Style.normalFillFor(foreground, accent)
  borderSpec: Border.controlSpec("normal", foreground, accent)
  clip: true

  Image {
    id: artworkImage
    anchors.fill: parent
    anchors.margins: Style.space(2)
    source: root.sourceUrl
    fillMode: Image.PreserveAspectCrop
    asynchronous: true
    visible: root.sourceUrl !== "" && status === Image.Ready
  }

  Text {
    anchors.centerIn: parent
    textFormat: Text.PlainText
    text: root.fallbackText
    color: root.foreground
    font.family: Style.font.family
    font.pixelSize: Math.max(Style.font.body, Math.min(root.width, root.height) * 0.25)
    font.bold: true
    visible: root.sourceUrl === "" || artworkImage.status !== Image.Ready
  }
}
