import QtQuick.Layouts 1.2
import QtQuick 2.3
import QtQuick.Controls 2.4

Rectangle {
    color: 'teal'

    ListView {
        id: rosterListView
        width: 200
        height: parent.height

        model: shmoose.rosterController.rosterList
        //model: rosterList
        delegate: Rectangle {
            height: 25
            width: parent.width
            color: "yellow"
            Text { text: jid}

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    rosterListView.currentIndex = index
                    console.log( "jid: " + rosterListView.currentIndex)
                    console.log( "jid: " + shmoose.rosterController.rosterList[rosterListView.currentIndex].jid)

                    shmoose.setCurrentChatPartner(shmoose.rosterController.rosterList[rosterListView.currentIndex].jid)
                }
            }
        }
    }
}

