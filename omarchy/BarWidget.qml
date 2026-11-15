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
  readonly property string installerPath: decodeURIComponent(String(Qt.resolvedUrl("../install-omarchy.sh")).replace("file://", ""))
  readonly property bool playing: playbackStatus === "Playing"
  readonly property color iconColor: bar ? bar.barForeground : Color.foreground
  readonly property bool opened: panelLoader.item ? panelLoader.item.opened === true : false
  readonly property bool popoutSwitchClosing: panelLoader.item ? panelLoader.item.popoutSwitchClosing === true : false

  function injectPanel() {
    var target = panelLoader.item
    if (!target) return
    if ("bar" in target) target.bar = root.bar
    if ("settings" in target) target.settings = root.settings
    if ("anchorItem" in target) target.anchorItem = button
    if ("hostWidget" in target) target.hostWidget = root
    if ("binaryPath" in target) target.binaryPath = root.binaryPath
    if ("installerPath" in target) target.installerPath = root.installerPath
    if ("installed" in target) target.installed = root.installed
  }

  function refresh() {
    if (!installCheck.running) installCheck.running = true
    if (!statusCheck.running) statusCheck.running = true
    if (panelLoader.item && panelLoader.item.refresh) panelLoader.item.refresh()
  }

  function install() {
    Quickshell.execDetached(["omarchy-launch-floating-terminal-with-presentation", installerPath])
  }

  function togglePanel() {
    if (!installed) {
      install()
      return
    }
    if (panelLoader.item && panelLoader.item.toggle) panelLoader.item.toggle()
  }

  function open() {
    if (!installed) {
      install()
      return
    }
    if (panelLoader.item && panelLoader.item.openFromHotkey) panelLoader.item.openFromHotkey()
  }

  function close() {
    if (panelLoader.item && panelLoader.item.close) panelLoader.item.close()
  }

  function closeForPopoutSwitch() {
    if (panelLoader.item) panelLoader.item.closeForPopoutSwitch()
  }

  function togglePlayback() {
    if (!installed) {
      install()
      return
    }
    if (playbackStatus === "Stopped") {
      togglePanel()
      return
    }
    Quickshell.execDetached([
      "gdbus", "call", "--session", "--dest", "org.mpris.MediaPlayer2.bearwave",
      "--object-path", "/org/mpris/MediaPlayer2", "--method", "org.mpris.MediaPlayer2.Player.PlayPause"
    ])
    delayedRefresh.restart()
  }

  implicitWidth: button.implicitWidth
  implicitHeight: button.implicitHeight
  onBarChanged: injectPanel()
  onSettingsChanged: injectPanel()
  onInstalledChanged: injectPanel()
  Component.onCompleted: refresh()

  Loader {
    id: panelLoader
    active: true
    source: Qt.resolvedUrl("Panel.qml")
    visible: false
    onLoaded: {
      root.injectPanel()
      Qt.callLater(root.injectPanel)
    }
  }

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
    command: [
      "gdbus", "call", "--session", "--dest", "org.mpris.MediaPlayer2.bearwave",
      "--object-path", "/org/mpris/MediaPlayer2", "--method", "org.freedesktop.DBus.Properties.Get",
      "org.mpris.MediaPlayer2.Player", "PlaybackStatus"
    ]
    stdout: StdioCollector {
      waitForEnd: true
      onStreamFinished: {
        var state = String(text || "")
        root.playbackStatus = state.indexOf("'Playing'") !== -1 ? "Playing" : state.indexOf("'Paused'") !== -1 ? "Paused" : "Stopped"
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
    active: root.playing || root.opened
    tooltipText: root.installed ? "BearWave" : "BearWave installieren"
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
      else root.togglePanel()
    }
  }
}
