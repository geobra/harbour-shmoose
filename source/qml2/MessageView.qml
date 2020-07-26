import QtQuick.Layouts 1.2
import QtQuick 2.3
import QtQuick.Controls 2.4


Rectangle {
    color: 'grey'

    ListView {
        height: parent.height
        width: parent.width

        model: shmoose.persistence.messageController
        delegate: Item {
                id: item

                width: parent.width
                //height: parent.height
                height: messageText.height// + msgStatus.height
                //width: messageText.width

                readonly property bool alignRight: (direction == 1);

                Column {

                    //width: parent.width - 50
                    //spacing: 20

                    Rectangle {

                        width: item.width
                        height: item.height
                        //height: messageText.height + msgStatus.height
                        //radius: margin

                        color: (alignRight ? "green" : "lightblue")

/*
                        anchors {
                            left: (direction == 0 ? item.right : undefined);
                            right: (direction == 1 ? item.left : undefined);
                            //margins: 10
                        }
*/



                        Text {
                            id: messageText
                            horizontalAlignment: (item.alignRight ? Text.AlignLeft : Text.AlignRight)
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere;
                            width: parent.width


                            text: message

                            /*
                            // make a hand over the link
                            MouseArea {
                                anchors.fill: parent
                                //acceptedButtons: Qt.NoButton // we don't want to eat clicks on the Text
                                //cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                                onClicked:  {
                                    shmoose.persistence.gcmController.setFilterOnMsg(id);
                                    console.log("seen by: " + shmoose.persistence.getResourcesOfNewestDisplayedMsgforJid(shmoose.rosterController.rosterList[roster.currentIndex].jid));
                                }
                            }
                            */

                            //onLinkActivated: Qt.openUrlExternally(link)
                        }

                        /*
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
                        */
                    }
                }
        }

    }
}
