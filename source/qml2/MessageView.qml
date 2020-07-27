import QtQuick.Layouts 1.2
import QtQuick 2.3
import QtQuick.Controls 2.4


Rectangle {
    property string attachmentPath: shmoose.getAttachmentPath();

    ListView {
        height: parent.height
        width: parent.width

        rotation: 180

        model: shmoose.persistence.messageController
        delegate: Item {
                id: item

                rotation: 180

                width: parent.width
                height: messageText.height + imageView.height + timestampText.height + msgStatus.height + 10 // + msgStatus.height

                readonly property bool alignRight: (direction == 1);

                Column {

                    width: parent.width
                    height: parent.height

                    Rectangle {

                        width: item.width
                        height: messageText.height + imageView.height + timestampText.height + msgStatus.height + 5
                        radius: 10

                        color: (alignRight ? "cornsilk" : "lightcyan")

                        Text {
                            id: messageText
                            horizontalAlignment: (item.alignRight ? Text.AlignLeft : Text.AlignRight)
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere;
                            width: parent.width - 20

                            font.pixelSize: (type === "image") ? 8 : 14

                            x: 5

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

                        Image {
                            id: imageView
                            anchors.top: messageText.bottom

                            width: Math.min (item.width, sourceSize.width);
                            fillMode: Image.PreserveAspectFit;
                            visible: (type === "image")

                            source: ( (type === "image") ? attachmentPath + "/" + shmoose.getLocalFileForUrl(message) : "");
                        }

                        Text {
                            id: timestampText

                            width: parent.width - 20
                            horizontalAlignment: (item.alignRight ? Text.AlignLeft : Text.AlignRight)
                            anchors.top: imageView.bottom

                            text: Qt.formatDateTime (new Date (timestamp * 1000), "yyyy-MM-dd hh:mm:ss");

                            color: "dimgrey";
                            font.pixelSize: 10
                        }

                        Image {
                            id: msgStatus

                            anchors.right: parent.right
                            anchors.top: timestampText.bottom

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
}
