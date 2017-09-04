import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.shmoose 1.0

Page {
    id: page;

    SilicaListView {
        id: jidlist
        header: Column {
            spacing: Theme.paddingMedium;
            anchors {
                left: parent.left;
                right: parent.right;
            }

            PageHeader {
                title: qsTr("Contacts");
            }
            //            SearchField {
            //                placeholderText: qsTr ("Filter");
            //                anchors {
            //                    left: parent.left;
            //                    right: parent.right;
            //                }
            //            }
        }
        model: shmoose.rosterController.rosterList
        delegate: ListItem {
            id: item;
            menu: contextMenu
            contentHeight: Theme.itemSizeMedium;
            onClicked: {
                shmoose.setCurrentChatPartner(jid)
                pageStack.push (pageMessaging, { "conversationId" : jid });
            }

            Image {
                id: img;
                width: height;
                source: imagePath != "" ? imagePath : getImage(jid)
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
                }

                Label {
                    id: nameId;
                    text: name
                    color: (item.highlighted ? Theme.highlightColor : Theme.primaryColor);
                    font.pixelSize: Theme.fontSizeMedium;
                }
                Row {
                    Image {
                        id: subscriptionImage;
                        visible: ! shmoose.rosterController.isGroup(jid)
                        source: getSubscriptionImage(subscription);
                    }
                    Image {
                        id: availabilityImage;
                        source: getAvailabilityImage(availability)
                    }
                    Label {
                        id: jidId;
                        text: jid;
                        color: Theme.secondaryColor;
                        font.pixelSize: Theme.fontSizeTiny;
                    }
                }
                Label {
                    id: statusId;
                    text: status;
                    color: Theme.secondaryColor;
                    font.pixelSize: Theme.fontSizeTiny;
                }
                Component {
                    id: contextMenu
                    ContextMenu {
                        MenuItem {
                            text: "Remove"
                            onClicked: {
                                if (shmoose.rosterController.isGroup(jid)) {
                                    shmoose.removeRoom(jid)
                                }
                                else {
                                    shmoose.rosterController.removeContact(jid)
                                }
                            }
                        }
                    }
                }
            }
        }

        anchors.fill: parent;

        PullDownMenu {
            enabled: true
            visible: true

            //            MenuItem {
            //                text: qsTr ("Create Room TBD");
            //                onClicked: {
            //                    console.log("create room")
            //                }
            //            }

            MenuItem {
                text: qsTr("Join room by address");
                onClicked: {
                    pageStack.push(dialogJoinRoom)
                }
            }

            MenuItem {
                text: qsTr("Add contact");
                onClicked: {
                    pageStack.push(dialogCreateContact)
                }
            }

        }
    }

    function getImage(jid) {
        if (shmoose.rosterController.isGroup(jid)) {
            return "image://theme/icon-l-image";
        } else {
            return "image://theme/icon-l-people"
        }
    }

    function getSubscriptionImage(subs) {
        if (subs === RosterItem.SUBSCRIPTION_NONE) {
            return "image://theme/icon-cover-cancel"
        } else if (subs === RosterItem.SUBSCRIPTION_TO) {
            return "image://theme/icon-cover-next"
        } else if (subs === RosterItem.SUBSCRIPTION_FROM) {
            return "image://theme/icon-cover-previous"
        } else {
            return "image://theme/icon-cover-transfers"
        }
    }

    function getAvailabilityImage(avail) {
        if (avail === RosterItem.AVAILABILITY_ONLINE) {
            return "image://theme/icon-s-chat"
        } else if (avail === RosterItem.AVAILABILITY_OFFLINE) {
            return "image://theme/icon-s-high-importance"
        } else {
            return "image://theme/icon-s-timer"
        }
    }

}


