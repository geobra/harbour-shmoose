import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

import harbour.shmoose 1.0


RowLayout {
    anchors.fill: parent

    ListView {
        id: roster

        Layout.fillHeight: true
        width: 400

        model: shmoose.rosterController.rosterList
        //model: shmoose.persistence.sessionController

        delegate: Rectangle {
            height: 25
            width: parent.width
            color: "green"
            Text { text: jid}

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    roster.currentIndex = index

                    //console.log( "clicked: " + roster.currentIndex)
                    console.log( "jid: " + shmoose.rosterController.rosterList[roster.currentIndex].jid)

                    shmoose.setCurrentChatPartner(shmoose.rosterController.rosterList[roster.currentIndex].jid)
                }
            }
        }
    }

    Rectangle {
        id: chat

        width: 400
        anchors.left: roster.right

        Layout.fillHeight: true

        property string jid: "foo"

        Text {
            anchors.centerIn: parent
            text: jid
        }
        ListView {
            id: messages

            height: 300
            width: 300

            model: shmoose.persistence.messageController

            delegate: Component {
                Item {
                    width: parent.width
                    height: 20

                    Column {
                        Text { text: message }
                    }
                }
            }
        }

        TextField {
            id: texttosend

            width: 200
            height: 30

            anchors.top: messages.bottom
            anchors.left: messages.left

            placeholderText: qsTr("Enter message")
        }
        Button {
            id: sendbutton

            width: 30
            height: 30

            anchors.top: messages.bottom
            anchors.left: texttosend.right

            text: "send"
            onClicked: {
                console.log("send to: " + shmoose.rosterController.rosterList[roster.currentIndex].jid)
                shmoose.sendMessage(shmoose.rosterController.rosterList[roster.currentIndex].jid, texttosend.text, "txt")
            }
        }
    }

}
