import QtQuick 2.2
import Sailfish.Silica 1.0

Dialog {
    allowedOrientations: Orientation.All

    property string headline
    property string bodyText

    width: parent.width

    DialogHeader {
        id: headerId
        title: headline
    }

    TextArea {
        id: bodyId
        width: parent.width
        height: parent.height
        readOnly: true
        anchors.top: headerId.bottom
        font.pixelSize: Theme.fontSizeMedium
        color: Theme.primaryColor
        text: bodyText
    }
}
