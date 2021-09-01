import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.shmoose 1.0

Page {
    id: page;
    allowedOrientations: Orientation.All;

    SilicaListView {
        id: view;
        header: Column {
            spacing: Theme.paddingMedium;
            anchors {
                left: parent.left;
                right: parent.right;
            }

            PageHeader {
                title: qsTr("Conversations");
            }
        }
        ViewPlaceholder {
            enabled: view.count == 0
            text: qsTr("Empty")
            hintText: qsTr("Select a contact to start a conversation")
        }
        model: shmoose.persistence.sessionController
        delegate: ListItem {
            id: item;
            contentHeight: Theme.itemSizeMedium;

            onClicked: {
                console.log("set current char partner: " + jid);
                pageStack.push (pageMessaging, { "conversationId" : jid });
                shmoose.setCurrentChatPartner(jid);
            }

            Image {
                id: img;
                width: height;
                source: getImage(jid)
                anchors {
                    top: parent.top;
                    left: parent.left;
                    bottom: parent.bottom;
                }

                Rectangle {
                    z: -1;
                    color: (model.index % 2 ? "black" : "white");
                    opacity: 0.15;
                    anchors.fill: parent;
                }
            }
            Column {
                anchors {
                    left: img.right;
                    margins: Theme.paddingMedium;
                    verticalCenter: parent.verticalCenter;
                    right: parent.right
                }

                Row {
                    Rectangle {
                        width: 45
                        height: 45
                        color: "#999900"
                        radius: width*0.5
                        visible: (unreadmessages > 0) ? true : false
                        Label {
                            text: unreadmessages
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Label {
                        id: nameId;
                        wrapMode: Text.NoWrap
                        maximumLineCount: 1
                        text: shmoose.rosterController.getNameForJid(jid)
                        color: (item.highlighted ? Theme.highlightColor : Theme.primaryColor);
                        font.pixelSize: Theme.fontSizeMedium;
                    }
                }
                Label {
                    id: jidId;
                    text: jid;
                    color: Theme.secondaryColor;
                    font.pixelSize: Theme.fontSizeTiny;
                }
                Label {
                    id: statusId;
                    text: lastmessage;
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    wrapMode: Text.WrapAnywhere
                    maximumLineCount: 1
                    color: Theme.secondaryColor;
                    font.pixelSize: Theme.fontSizeTiny;
                }
            }

            menu: ContextMenu {
                MenuItem {
                    text: qsTr("Delete")
                    onClicked: {
                        remorseAction(qsTr("Delete conversation"),
                                      function() {
                                            shmoose.persistence.removeConversation(jid);
                                        })
                    }
                }
            }
        }

        anchors.fill: parent;

    }

    function getImage(jid) {
        var imagePath = shmoose.rosterController.getAvatarImagePathForJid(jid);

        if (imagePath.length > 0) {
            return imagePath;
        } else if (shmoose.rosterController.isGroup(jid)) {
            return "image://theme/icon-l-image";
        } else {
            return "image://theme/icon-l-people"
        }
    }
}


