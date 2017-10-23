import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

import harbour.shmoose 1.0


GridLayout {
    anchors.fill: parent
    columns: 2

    readonly property int margin: 10;

    ListView {
        id: roster

        Layout.fillHeight: true
        Layout.fillWidth: true

        //model: rosterList
        model: shmoose.rosterController.rosterList

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

        anchors.left: roster.right

        Layout.fillHeight: true
        Layout.fillWidth: true

        property string jid: "foo"

        Text {
            anchors.centerIn: parent
            text: jid
        }
        ListView {
            id: messages

            height: parent.height
            width: parent.width

            model: shmoose.persistence.messageController
            //model: messageController

            delegate: Item {
                    id: item

                    width: parent.width
                    height: messageText.height + msgStatus.height
                    //height: children.height

                    readonly property bool alignRight: (direction == 1);

                    Column {

                        width: messages.width
                        spacing: 200

                        Rectangle {

                            width: parent.width - 50
                            height: item.height
                            //height: messageText.height + msgStatus.height
                            radius: margin

                            anchors {
                                left: (item.alignRight ? parent.left : undefined);
                                right: (!item.alignRight ? parent.right : undefined);
                                margins: margin
                            }

                            color: (item.alignRight ? "yellow" : "lightblue")


                            Text {
                                id: messageText
                                horizontalAlignment: (item.alignRight ? Text.AlignLeft : Text.AlignRight)
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere;
                                width: parent.width

                                anchors {
                                    left: (item.alignRight ? parent.left : undefined);
                                    right: (!item.alignRight ? parent.right : undefined);
                                }

                                text: message

                                // make a hand over the link
                                MouseArea {
                                    anchors.fill: parent
                                    acceptedButtons: Qt.NoButton // we don't want to eat clicks on the Text
                                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                                }

                                onLinkActivated: Qt.openUrlExternally(link)
                            }

                            Image {
                                id: msgStatus
                                //source: "img/check.png"

                                anchors.right: parent.right
                                anchors.bottom: parent.bottom

                                source: {
                                    if (msgstate == 3) {
                                        return "img/read_until_green.png"
                                    }
                                    if (msgstate == 2) {
                                        return "img/2check.png"
                                    }
                                    if (msgstate == 1) {
                                        return "img/check.png"
                                    }
                                    return ""
                                }

                            }

                        }

                    }
                }

        }

        Row {
            anchors.bottom: chat.bottom
            height: 30
            width: parent.width

            TextField {
                id: texttosend

                width: parent.width - sendbutton.width
                height: parent.height

                placeholderText: qsTr("Enter message")
            }
            Button {
                id: sendbutton

                width: 50
                height: parent.height

                text: "send"
                onClicked: {
                    console.log("send to: " + shmoose.rosterController.rosterList[roster.currentIndex].jid)
                    shmoose.sendMessage(shmoose.rosterController.rosterList[roster.currentIndex].jid, texttosend.text, "txt")
                    texttosend.text = ""
                }
            }
        }
    }
}
