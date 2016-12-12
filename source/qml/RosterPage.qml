import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Controls 1.2

import harbour.shmoose 1.0


Flickable {
    ListView {
        id: roster
        anchors.fill: parent
        model: shmoose.rosterController.rosterList
        //model: shmoose.persistence.sessionController
        delegate: Rectangle {
            height: 25
            width: parent.width
            Text { text: jid}

            MouseArea {
                anchors.fill: parent
                onClicked: roster.currentIndex = index
            }
        }
        focus: true

        onCurrentItemChanged: {
            console.log("onCurrentItemChanged")

            shmoose.setCurrentChatPartner(model[roster.currentIndex].jid)
            var component = Qt.createComponent("chat.qml")
            var window = component.createObject(parent, {'jid': model[roster.currentIndex].jid})
            window.show()
            //mainLoader.source = "chat.qml"
        }
    }
}
