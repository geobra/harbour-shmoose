import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    objectName: "aboutPage"
    allowedOrientations: Orientation.All

    Column {
        id: headerPart
        anchors.top: parent.top
        width: parent.width
        spacing: Theme.paddingLarge

        PageHeader {
            title: qsTr("Shmoose - Xmpp Client")
        }

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            fillMode: Image.PreserveAspectFit
            source: "/usr/share/harbour-shmoose/icons/86x86/harbour-shmoose.png"
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.secondaryColor
            text: qsTr("Version") + " " + shmoose.getVersion()
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.secondaryColor
            text: qsTr("Contributions:") + "<br\>GNUuser, slohse, mazhe, ron282<br\>rogora, milotype, marmistrz,<br\>eson57, dashinfantry, comradekingu,<br\>Tititesouris, ReleaseRoll"
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

