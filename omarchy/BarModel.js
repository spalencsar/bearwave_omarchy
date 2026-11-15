.pragma library

function sanitize(value) {
    var text = String(value === undefined || value === null ? "" : value)
    text = text.replace(/<(script|style)\b[^>]*>[\s\S]*?<\/\1>/gi, "")
    text = text.replace(/<[^>]*>/g, "")
    text = text.replace(/[\u0000-\u001F\u007F]/g, "")
    text = text.replace(/\s+/g, " ").trim()
    return text
}

function clip(text, maxLength) {
    var value = String(text === undefined || text === null ? "" : text)
    if (maxLength <= 0 || value.length <= maxLength)
        return value
    if (maxLength === 1)
        return "…"
    return value.slice(0, maxLength - 1).trim() + "…"
}

function stripArtistPrefix(title, artist) {
    var titleText = sanitize(title)
    var artistText = sanitize(artist)
    if (!titleText || !artistText)
        return titleText
    if (titleText.toLowerCase().indexOf(artistText.toLowerCase()) !== 0)
        return titleText
    return titleText.slice(artistText.length).replace(/^[\s\-–—:]+/, "").trim()
}

function nowPlayingLine(station, title, artist) {
    var stationText = clip(sanitize(station), 48)
    var artistText = clip(sanitize(artist), 32)
    var titleText = clip(stripArtistPrefix(title, artist), 48)
    if (titleText && titleText === stationText)
        titleText = ""
    if (artistText && artistText === stationText)
        artistText = ""

    var track = ""
    if (artistText && titleText)
        track = artistText + " - " + titleText
    else
        track = titleText || artistText

    if (stationText && track)
        return clip("(" + stationText + ") " + track, 96)
    if (stationText)
        return "(" + stationText + ")"
    return track
}

function metadataField(metadata, key) {
    if (!metadata)
        return ""
    var value = metadata[key]
    if (value === undefined || value === null)
        return ""
    if (typeof value === "object" && typeof value.length === "number" && typeof value !== "string")
        return value.length > 0 ? String(value[0]) : ""
    return String(value)
}

function nowPlayingFromPlayer(player) {
    if (!player)
        return ""
    var meta = player.metadata || {}
    var station = player.trackAlbum || metadataField(meta, "xesam:album") || ""
    var title = player.trackTitle || metadataField(meta, "xesam:title") || ""
    var artist = player.trackArtist || metadataField(meta, "xesam:artist") || ""
    return nowPlayingLine(station, title, artist)
}
