import QtQuick
import Quickshell
import Quickshell.Io
import qs.Commons
import qs.Commons as Commons
import qs.Ui

Panel {
  id: root

  moduleName: "de.nerdbear.bearwave"
  ipcTarget: "de.nerdbear.bearwave"
  manageIpc: false

  property var anchorItem: null
  property var hostWidget: null
  property string binaryPath: ""
  property string installerPath: ""
  property bool installed: false
  property bool openedFromHotkey: false
  property bool startingService: false
  property string page: "home"
  property var state: ({
    running: false,
    playbackStatus: "Stopped",
    connectionState: "idle",
    station: "",
    title: "",
    artist: "",
    artwork: "",
    favorites: [],
    recent: []
  })

  readonly property var barIdentity: hostWidget || root
  readonly property color foreground: bar ? bar.foreground : Commons.Color.foreground
  readonly property color accent: Commons.Color.accent
  readonly property string fontFamily: bar ? bar.fontFamily : Style.font.family
  readonly property bool playing: state.playbackStatus === "Playing"
  readonly property bool hasStation: String(state.station || "") !== ""

  function safeText(value, limit) {
    return String(value === undefined || value === null ? "" : value).slice(0, limit)
  }

  function safeStation(value) {
    var item = value && typeof value === "object" ? value : ({})
    return {
      name: safeText(item.name, 160),
      uuid: safeText(item.uuid, 96),
      urlResolved: safeText(item.urlResolved, 2048),
      favicon: safeText(item.favicon, 2048),
      country: safeText(item.country, 120),
      codec: safeText(item.codec, 24),
      isFavorite: item.isFavorite === true
    }
  }

  function safeList(value) {
    if (!Array.isArray(value)) return []
    var result = []
    for (var index = 0; index < Math.min(value.length, 12); ++index)
      result.push(safeStation(value[index]))
    return result
  }

  function applyState(value) {
    if (!value || typeof value !== "object") return
    state = {
      running: value.running === true,
      playbackStatus: safeText(value.playbackStatus, 32) || "Stopped",
      connectionState: safeText(value.connectionState, 32) || "idle",
      station: safeText(value.station, 160),
      title: safeText(value.title, 240),
      artist: safeText(value.artist, 240),
      artwork: safeText(value.artwork, 2048),
      favorites: safeList(value.favorites),
      recent: safeList(value.recent)
    }
  }

  function validArtwork(url) {
    var value = String(url || "")
    return value.indexOf("https://") === 0 || value.indexOf("file://") === 0 ? value : ""
  }

  function open() {
    openedFromHotkey = false
    controller.show()
    refresh()
  }

  function openFromHotkey() {
    openedFromHotkey = true
    controller.show()
    refresh()
  }

  function close() {
    controller.hide()
  }

  function toggle() {
    opened ? close() : open()
  }

  function refresh() {
    if (!installed || binaryPath === "" || stateQuery.running) return
    stateQuery.command = [binaryPath, "--widget-state"]
    stateQuery.running = true
  }

  function ensureService() {
    if (!installed || startingService || state.running) return
    startingService = true
    Quickshell.execDetached([binaryPath, "--background", "--no-tray"])
    serviceRetry.restart()
  }

  function callControl(method, station) {
    var command = [
      "gdbus", "call", "--session", "--dest", "org.mpris.MediaPlayer2.bearwave",
      "--object-path", "/org/mpris/MediaPlayer2", "--method", "de.nerdbear.BearWave.Control." + method
    ]
    if (station) command.push(station.uuid || "", station.urlResolved || "")
    Quickshell.execDetached(command)
    actionRefresh.restart()
  }

  function playStation(station, favorite) {
    if (!state.running) {
      ensureService()
      pendingStation.station = station
      pendingStation.favorite = favorite
      return
    }
    callControl(favorite ? "PlayFavoriteByUuid" : "PlayRecentByUuid", station)
  }

  function playPause() {
    if (!state.running) {
      ensureService()
      return
    }
    callControl("PlayPause", null)
  }

  QtObject {
    id: pendingStation
    property var station: null
    property bool favorite: false
  }

  Timer {
    id: pollTimer
    interval: 1500
    running: root.opened
    repeat: true
    onTriggered: root.refresh()
  }

  Timer {
    id: actionRefresh
    interval: 350
    onTriggered: root.refresh()
  }

  Timer {
    id: serviceRetry
    interval: 650
    repeat: true
    onTriggered: root.refresh()
  }

  Process {
    id: stateQuery
    stdout: StdioCollector {
      waitForEnd: true
      onStreamFinished: {
        try {
          root.applyState(JSON.parse(String(text || "{}")))
        } catch (error) {
          root.applyState({ running: false })
        }
      }
    }
    onExited: function(exitCode) {
      if (root.state.running) {
        root.startingService = false
        serviceRetry.stop()
        if (pendingStation.station) {
          var station = pendingStation.station
          var favorite = pendingStation.favorite
          pendingStation.station = null
          root.playStation(station, favorite)
        }
      } else if (root.opened && !root.startingService) {
        root.ensureService()
      }
    }
  }

  ShellIpc {
    target: root.ipcTarget
    function open(): void { root.openFromHotkey() }
    function close(): void { root.close() }
    function show(): void { root.openFromHotkey() }
    function hide(): void { root.close() }
    function toggle(): void { root.toggle() }
  }

  KeyboardPanel {
    id: popup
    anchorItem: root.anchorItem
    owner: root.barIdentity
    bar: root.bar
    open: root.opened
    centerOnBar: false
    focusTarget: popup.loadedContent
    contentWidth: popup.fittedContentWidth(Style.space(430))
    contentHeight: popup.cappedContentHeight(Style.space(610))

    content: PanelKeyCatcher {
      anchors.fill: parent
      onCloseRequested: root.close()
      onTabRequested: function(direction) { root.switchPanel(direction) }

      Column {
        anchors.fill: parent
        spacing: Style.space(12)

        Row {
          width: parent.width
          height: Style.space(38)
          spacing: Style.space(8)

          Button {
            visible: root.page !== "home"
            iconText: "󰁍"
            foreground: root.foreground
            fontFamily: root.fontFamily
            onClicked: root.page = "home"
          }

          Text {
            width: parent.width - (root.page !== "home" ? Style.space(82) : Style.space(38))
            anchors.verticalCenter: parent.verticalCenter
            textFormat: Text.PlainText
            text: root.page === "now" ? "NOW PLAYING" : root.page === "favorites" ? "FAVORITEN" : root.page === "recent" ? "ZULETZT GEHÖRT" : "BEARWAVE"
            color: root.foreground
            font.family: root.fontFamily
            font.pixelSize: Style.font.subtitle
            font.bold: true
            font.letterSpacing: 1.2
            elide: Text.ElideRight
          }

          Text {
            anchors.verticalCenter: parent.verticalCenter
            textFormat: Text.PlainText
            text: root.playing ? "● LIVE" : "●"
            color: root.playing ? root.accent : Qt.darker(root.foreground, 1.8)
            font.family: root.fontFamily
            font.pixelSize: Style.font.bodySmall
          }
        }

        Item {
          width: parent.width
          height: parent.height - Style.space(128)

          Flickable {
            anchors.fill: parent
            visible: root.page !== "now"
            contentWidth: width
            contentHeight: library.implicitHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            Column {
              id: library
              width: parent.width
              spacing: Style.space(14)

              Text {
                width: parent.width
                visible: root.page === "home"
                textFormat: Text.PlainText
                text: "Dein Radio, direkt in Omarchy."
                color: Qt.darker(root.foreground, 1.35)
                font.family: root.fontFamily
                font.pixelSize: Style.font.body
              }

              SectionHeader {
                visible: root.page === "home" || root.page === "recent"
                width: parent.width
                title: "ZULETZT GEHÖRT"
                showAll: root.page === "home" && root.state.recent.length > 4
                onActivated: root.page = "recent"
              }

              Column {
                width: parent.width
                visible: root.page === "home" || root.page === "recent"
                spacing: Style.space(6)
                Repeater {
                  model: root.page === "home" ? root.state.recent.slice(0, 4) : root.state.recent
                  StationRow {
                    required property var modelData
                    width: parent.width
                    station: modelData
                    foreground: root.foreground
                    accent: root.accent
                    fontFamily: root.fontFamily
                    active: root.state.station === modelData.name
                    playing: root.playing
                    onActivated: root.playStation(modelData, false)
                  }
                }
                EmptyState {
                  visible: root.state.recent.length === 0
                  width: parent.width
                  text: "Noch keine Sender gehört"
                  foreground: root.foreground
                  fontFamily: root.fontFamily
                }
              }

              SectionHeader {
                visible: root.page === "home" || root.page === "favorites"
                width: parent.width
                title: "FAVORITEN"
                showAll: root.page === "home" && root.state.favorites.length > 4
                onActivated: root.page = "favorites"
              }

              Column {
                width: parent.width
                visible: root.page === "home" || root.page === "favorites"
                spacing: Style.space(6)
                Repeater {
                  model: root.page === "home" ? root.state.favorites.slice(0, 4) : root.state.favorites
                  StationRow {
                    required property var modelData
                    width: parent.width
                    station: modelData
                    foreground: root.foreground
                    accent: root.accent
                    fontFamily: root.fontFamily
                    active: root.state.station === modelData.name
                    playing: root.playing
                    onActivated: root.playStation(modelData, true)
                  }
                }
                EmptyState {
                  visible: root.state.favorites.length === 0
                  width: parent.width
                  text: "Noch keine Favoriten"
                  foreground: root.foreground
                  fontFamily: root.fontFamily
                }
              }
            }
          }

          Column {
            anchors.fill: parent
            visible: root.page === "now"
            spacing: Style.space(18)

            Artwork {
              width: Math.min(parent.width, Style.space(250))
              height: width
              anchors.horizontalCenter: parent.horizontalCenter
              sourceUrl: root.validArtwork(root.state.artwork)
              fallbackText: "BW"
              foreground: root.foreground
              accent: root.accent
            }

            Text {
              width: parent.width
              textFormat: Text.PlainText
              text: root.state.title || root.state.station || "Bereit zum Hören"
              color: root.foreground
              font.family: root.fontFamily
              font.pixelSize: Style.font.title
              font.bold: true
              horizontalAlignment: Text.AlignHCenter
              elide: Text.ElideRight
            }

            Text {
              width: parent.width
              textFormat: Text.PlainText
              text: root.state.artist || root.state.station
              color: Qt.darker(root.foreground, 1.35)
              font.family: root.fontFamily
              font.pixelSize: Style.font.body
              horizontalAlignment: Text.AlignHCenter
              elide: Text.ElideRight
            }

            Row {
              anchors.horizontalCenter: parent.horizontalCenter
              spacing: Style.space(18)
              Button { iconText: "󰒮"; foreground: root.foreground; fontFamily: root.fontFamily; onClicked: root.callControl("PlayPrevious", null) }
              Button { iconText: root.playing ? "󰏤" : "󰐊"; active: true; foreground: root.foreground; fontFamily: root.fontFamily; onClicked: root.playPause() }
              Button { iconText: "󰒭"; foreground: root.foreground; fontFamily: root.fontFamily; onClicked: root.callControl("PlayNext", null) }
            }
          }
        }

        BorderSurface {
          width: parent.width
          height: Style.space(66)
          radius: Style.cornerRadius
          color: Style.normalFillFor(root.foreground, root.accent)
          borderSpec: Border.controlSpec("normal", root.foreground, root.accent)

          Row {
            id: miniPlayerRow
            anchors.fill: parent
            anchors.margins: Style.space(8)
            spacing: Style.space(10)

            Artwork {
              id: miniPlayerArtwork
              width: parent.height
              height: parent.height
              sourceUrl: root.validArtwork(root.state.artwork)
              fallbackText: "BW"
              foreground: root.foreground
              accent: root.accent
              TapHandler { onTapped: root.page = "now" }
            }

            Column {
              width: Math.max(0, miniPlayerRow.width - miniPlayerArtwork.width - miniPlayerButton.implicitWidth - miniPlayerRow.spacing * 2)
              y: Math.max(0, (parent.height - implicitHeight) / 2)
              spacing: Style.space(3)
              Text {
                width: parent.width
                textFormat: Text.PlainText
                text: root.state.title || root.state.station || "Sender auswählen"
                color: root.foreground
                font.family: root.fontFamily
                font.pixelSize: Style.font.body
                font.bold: true
                elide: Text.ElideRight
              }
              Text {
                width: parent.width
                textFormat: Text.PlainText
                text: root.state.artist || (root.state.running ? root.state.connectionState : "BearWave wird gestartet …")
                color: Qt.darker(root.foreground, 1.35)
                font.family: root.fontFamily
                font.pixelSize: Style.font.bodySmall
                elide: Text.ElideRight
              }
              TapHandler { onTapped: root.page = "now" }
            }

            Button {
              id: miniPlayerButton
              y: Math.max(0, (parent.height - height) / 2)
              iconText: root.playing ? "󰏤" : "󰐊"
              active: root.playing
              foreground: root.foreground
              fontFamily: root.fontFamily
              onClicked: root.playPause()
            }
          }
        }
      }
    }
  }
}
