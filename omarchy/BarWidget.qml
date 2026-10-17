import QtQuick
import QtQuick.Effects
import Quickshell
import Quickshell.Io
import qs.Commons
import qs.Ui

BarWidget {
  id: root

  moduleName: "de.nerdbear.bearwave"

  property bool installed: false
  property string playbackStatus: "Stopped"

  readonly property string homePath: Quickshell.env("HOME")
  readonly property string binaryPath: homePath + "/.local/bin/bearwave"
  readonly property string installerPath: decodeURIComponent(
    String(Qt.resolvedUrl("../install-omarchy.sh")).replace("file://", ""))
  readonly property bool playing: playbackStatus === "Playing"
  readonly property color iconColor: bar ? bar.barForeground : Color.foreground
  readonly property string tooltip: !installed
    ? "BearWave installieren"
    : playing
    ? "BearWave spielt · Klick zum Öffnen · Mittelklick für Pause"
    : playbackStatus === "Paused"
    ? "BearWave pausiert · Mittelklick zum Fortsetzen"
    : "BearWave öffnen"

  function refresh() {
    if (!installCheck.running) installCheck.running = true
    if (!statusCheck.running) statusCheck.running = true
  }

  function install() {
    Quickshell.execDetached([
      "omarchy-launch-floating-terminal-with-presentation",
      installerPath
    ])
  }

  function launch() {
    if (!installed) {
      install()
      return
    }
    Quickshell.execDetached([
      "uwsm-app", "--", "gtk-launch", "de.nerdbear.bearwave.desktop"
    ])
  }

  function togglePlayback() {
    if (!installed || playbackStatus === "Stopped") {
      launch()
      return
    }
    Quickshell.execDetached(["playerctl", "--player=bearwave", "play-pause"])
    delayedRefresh.restart()
  }

  function stopPlayback() {
    if (installed && playbackStatus !== "Stopped") {
      Quickshell.execDetached(["playerctl", "--player=bearwave", "stop"])
      delayedRefresh.restart()
    }
  }

  implicitWidth: button.implicitWidth
  implicitHeight: button.implicitHeight

  Component.onCompleted: refresh()

  Timer {
    interval: 2500
    running: true
    repeat: true
    onTriggered: root.refresh()
  }

  Timer {
    id: delayedRefresh
    interval: 350
    onTriggered: root.refresh()
  }

  Process {
    id: installCheck
    command: ["/usr/bin/test", "-x", root.binaryPath]
    onExited: function(exitCode) {
      root.installed = exitCode === 0
      if (!root.installed) root.playbackStatus = "Stopped"
    }
  }

  Process {
    id: statusCheck
    command: ["playerctl", "--player=bearwave", "status"]
    stdout: StdioCollector {
      waitForEnd: true
      onStreamFinished: {
        var state = String(text || "").trim()
        root.playbackStatus = state === "Playing" || state === "Paused"
          ? state : "Stopped"
      }
    }
    onExited: function(exitCode) {
      if (exitCode !== 0) root.playbackStatus = "Stopped"
    }
  }

  BarIconButton {
    id: button
    anchors.fill: parent
    bar: root.bar
    active: root.playing
    tooltipText: root.tooltip
    iconComponent: Component {
      Item {
        Image {
          id: iconMask
          anchors.fill: parent
          source: Qt.resolvedUrl("../assets/app/bearwave-symbolic.svg")
          fillMode: Image.PreserveAspectFit
          sourceSize.width: Math.round(width * Screen.devicePixelRatio)
          sourceSize.height: Math.round(height * Screen.devicePixelRatio)
          visible: false
          layer.enabled: true
        }

        MultiEffect {
          anchors.fill: iconMask
          source: iconMask
          colorization: 1.0
          colorizationColor: root.iconColor
        }
      }
    }
    onPressed: function(buttonCode) {
      if (buttonCode === Qt.MiddleButton) root.togglePlayback()
      else if (buttonCode === Qt.RightButton) root.stopPlayback()
      else root.launch()
    }
  }
}
