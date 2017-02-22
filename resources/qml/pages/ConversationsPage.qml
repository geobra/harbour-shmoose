import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page;

    SilicaListView {
        id: view;
        spacing: Theme.paddingMedium;
        header: PageHeader {
            title: qsTr ("Conversations");
        }
        model: shmoose.persistence.sessionController
        delegate: BackgroundItem {
            id: item;
            contentHeight: Theme.itemSizeLarge;
            onClicked: {
                pageStack.push (pageMessaging, { "conversationId" : jid });
                shmoose.setCurrentChatPartner(jid);
            }

            Column {
                anchors {
                    left: parent.left;
                    right: parent.right;
                    margins: Theme.paddingMedium;
                    verticalCenter: parent.verticalCenter;
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
                        text: jid;
                        color: Theme.primaryColor;
                    }
                }
                Label {
                    text: lastmessage;
                    color: Theme.secondaryColor;
                }
            }
        }
        anchors.fill: parent;

        VerticalScrollDecorator { }
    }
}
