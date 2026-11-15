import QtQuick
import QtQuick.Effects
import Quickshell
import Quickshell.Io
import Quickshell.Services.Mpris
import qs.Commons
import qs.Commons as Commons
import qs.Ui
import "BarModel.js" as BarModel

BarWidget {
  id: root

  moduleName: "de.nerdbear.bearwave"

  property bool binaryPresent: false

  readonly property string homePath: Quickshell.env("HOME")
  readonly property string desktopFilePath: homePath + "/.local/share/applications/de.nerdbear.bearwave.desktop"
  readonly property string installerPath: decodeURIComponent(
    String(Qt.resolvedUrl("../install-omarchy.sh")).replace("file://", ""))
  readonly property var players: Mpris.players ? Mpris.players.values : []
  readonly property var player: {
    for (var i = 0; i < players.length; i++) {
      var candidate = players[i]
      if (isBearWave(candidate))
        return candidate
    }
    return null
  }
  readonly property bool installed: binaryPresent || player !== null
  readonly property string playbackStatus: {
    if (!player)
      return "Stopped"
    if (player.isPlaying)
      return "Playing"
    return MprisPlaybackState.toString(player.playbackState) === "Paused"
           ? "Paused" : "Stopped"
  }
  readonly property bool playing: playbackStatus === "Playing"
  readonly property string nowPlaying: player
    ? BarModel.nowPlayingFromPlayer({
        trackAlbum: player.trackAlbum,
        trackTitle: player.trackTitle,
        trackArtist: player.trackArtist,
        metadata: player.metadata
      })
    : ""
  readonly property color iconColor: bar ? bar.barForeground : Color.foreground
  readonly property string tooltip: {
    if (!installed)
      return strings.t("bar.install")
    if (nowPlaying.length > 0)
      return nowPlaying
    return strings.t("bar.open")
  }

  function isBearWave(candidate) {
    if (!candidate)
      return false
    var entry = String(candidate.desktopEntry || "").toLowerCase()
    if (entry === "de.nerdbear.bearwave" || entry.indexOf("de.nerdbear.bearwave.") === 0)
      return true
    var busName = String(candidate.dbusName || "").toLowerCase()
    if (busName.indexOf("bearwave") !== -1)
      return true
    return String(candidate.identity || "").toLowerCase() === "bearwave"
  }

  function install() {
    Quickshell.execDetached([
      "omarchy-launch-floating-terminal-with-presentation",
      installerPath
    ])
  }

  function launch() {
    if (player && player.canRaise) {
      player.raise()
      return
    }
    if (!installed) {
      install()
      return
    }
    Quickshell.execDetached([
      "uwsm-app", "--", "gtk-launch", "de.nerdbear.bearwave.desktop"
    ])
  }

  function togglePlayback() {
    if (player) {
      if (player.canTogglePlaying)
        player.togglePlaying()
      else if (player.isPlaying && player.canPause)
        player.pause()
      else if (player.canPlay)
        player.play()
      return
    }
    launch()
  }

  function stopPlayback() {
    if (player)
      player.stop()
  }

  Commons.Translations {
    id: strings
    directory: Qt.resolvedUrl("i18n")
  }

  onTooltipChanged: {
    if (button.tooltipHovered && root.bar)
      root.bar.showTooltip(button, root.tooltip)
  }

  implicitWidth: button.implicitWidth
  implicitHeight: button.implicitHeight

  FileView {
    id: desktopFile
    path: root.desktopFilePath
    watchChanges: true
    printErrors: false
    onLoaded: root.binaryPresent = true
    onLoadFailed: root.binaryPresent = false
    onFileChanged: reload()
  }

  FileView {
    path: root.homePath + "/.local/share/applications"
    watchChanges: true
    printErrors: false
    onFileChanged: desktopFile.reload()
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
