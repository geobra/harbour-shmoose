import QtQuick 2.2
import Sailfish.Silica 1.0
import QtMultimedia 5.0

Page {
    id: page
    allowedOrientations: Orientation.All
    property alias path: video.source

    IconButton {
        id: iconButton
        visible: video.playbackState !== MediaPlayer.PlayingState
        anchors.centerIn: parent
        icon.source: "image://theme/icon-l-play?" + (pressed
                     ? Theme.highlightColor
                     : Theme.primaryColor)
        onClicked: mouseArea.onClicked("")
    }
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: {
            if (video.playbackState === MediaPlayer.PlayingState) {                
                video.pause();
            } else {
                video.play();
            }
        }
    }
    Video {
        id: video
        anchors.fill: parent
        autoPlay: false 
        fillMode: VideoOutput.PreserveAspectFit
        muted: false
        onErrorChanged: {
            if (error != MediaPlayer.NoError) {
                source = ""
            }
        }
    }
}