import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    objectName: "aboutPage"

    Column {
        id: headerPart
        anchors.top: parent.top
        width: parent.width

        PageHeader {
            title: qsTr("Shmoose - Xmpp Client")
        }
        Item {
            width: parent.width
            height: Theme.paddingMedium
        }
        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            height: 256
            fillMode: Image.PreserveAspectFit
            source: "/usr/share/harbour-shmoose/icons/86x86/harbour-shmoose.png"
        }
        Item {
            width: parent.width
            height: Theme.paddingMedium
        }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: Theme.fontSizeExtraSmall
            color: Theme.secondaryColor
            text: qsTr("Version ") + shmoose.getVersion()
        }
        Item {
            width: parent.width
            height: Theme.paddingMedium
        }
    }

    Label {
        id: urlPart
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Theme.paddingSmall
        color: Theme.secondaryColor
        font.pixelSize: Theme.fontSizeExtraSmall
        text: "https://github.com/geobra/harbour-shmoose"
    }
}

