import QtQuick 2.2
import Sailfish.Silica 1.0

Dialog {
    property string headline
    property string bodyText

    Column {
        width: parent.width

        DialogHeader {
            id: headerId
            title: headline
        }

        Label {
            id: bodyId
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: Theme.fontSizeNormal
            color: Theme.primaryColor
            text: bodyText
        }
    }
}
