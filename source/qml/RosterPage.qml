import QtQuick 2.1
import harbour.shmoose 1.0

Flickable {
    ListView {
        anchors.fill: parent
        model: shmoose.rosterController.rosterList
        delegate: Rectangle {
            height: 25
            width: parent.width
            Text { text: model.modelData.jid}
        }
    }
}
