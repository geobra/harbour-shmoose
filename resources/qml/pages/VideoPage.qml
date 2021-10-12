import QtQuick 2.2
import Sailfish.Silica 1.0
import QtMultimedia 5.0

Page {
    id: page
    allowedOrientations: Orientation.All
    property alias path: video.source
    property bool showCommands: true


    Timer {
        interval: 1000; running: true; repeat: true
        onTriggered: {
            slider.value = video.position;
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
        onStatusChanged: {
            if (status == MediaPlayer.Loaded) {
                slider.maximumValue = duration;
            }
        }
    }
    MouseArea {
        anchors.fill:parent
        onClicked: {
            showCommands = !showCommands;
        }
    }
    IconButton {
        id: iconButton
        visible: showCommands || video.playbackState !== MediaPlayer.PlayingState
        anchors.centerIn: parent
        icon.source: video.playbackState !== MediaPlayer.PlayingState ? "image://theme/icon-l-play" : "image://theme/icon-l-pause"
        onClicked: {
            if (video.playbackState === MediaPlayer.PlayingState) {
                video.pause();
            } else {
                video.play();
                showCommands = false;
            }
        }
    }
    Column {
        anchors {
            bottom: parent.bottom
            leftMargin: Theme.paddingLarge
            rightMargin: Theme.paddingLarge
            bottomMargin: parent.height/8
        }
        width: parent.width
        visible: showCommands

        Slider {
            id: slider
            width: parent.width
            enabled: video.seekable
            minimumValue: 0
            handleVisible: video.seekable
            stepSize: 1000
            valueText: formatDuration(sliderValue)
            onDownChanged: {
                if(!down) {
                    video.seek(value);
                }
            }
        }
        Row {
            spacing: Theme.paddingMedium
            anchors.horizontalCenter : parent.horizontalCenter

            IconButton {
                visible: video.seekable
                icon.source: "image://theme/icon-m-10s-back"
                onClicked: {
                    video.seek(Math.max(0, video.position-10000));
                }
            }
            IconButton {
                visible: video.seekable
                icon.source: "image://theme/icon-m-10s-forward"
                onClicked: {
                    video.seek(Math.min(video.duration, video.position+10000));
                }
            }
            IconButton {
                icon.source: video.muted == 0 ? "image://theme/icon-m-speaker-mute" : "image://theme/icon-m-speaker-on"
                onClicked: {
                    video.muted = !video.muted;
                }
            }
        }
    }
    function formatDuration(duration){
        var seconds = Math.abs(Math.ceil(duration / 1000)),
        h = (seconds - seconds % 3600) / 3600,
        m = (seconds - seconds % 60) / 60 % 60,
        s = seconds % 60;
        return (duration < 0 ? '-' : '') + (h > 0 ? h + ':' : '') + zeroPad(m.toString(), 2) + ':' + zeroPad(s.toString(), 2);
    }
    function zeroPad(num, places) {
        var zero = places - num.toString().length + 1;
        return Array(+(zero > 0 && zero)).join("0") + num;
    }
}