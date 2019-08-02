import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.shmoose 1.0

Page {
    id: page;

    SilicaListView {
        id: view;
        header: Column {
            spacing: Theme.paddingMedium;
            anchors {
                left: parent.left;
                right: parent.right;
            }

            PageHeader {
                title: qsTr("Message Status");
            }
        }
        model: shmoose.persistence.gcmController
        delegate: ListItem {
            id: item;
            contentHeight: Theme.itemSizeMedium;

            Column {
                anchors {
                    left: img.right;
                    margins: Theme.paddingMedium;
                    verticalCenter: parent.verticalCenter;
                }

                Label {
                    text: id;
                    color: Theme.secondaryColor;
                    font.pixelSize: Theme.fontSizeTiny;
                }
                Label {
                    text: timestamp;
                    color: Theme.secondaryColor;
                    font.pixelSize: Theme.fontSizeTiny;
                }
                Label {
                    text: chatmembername;
                    color: Theme.secondaryColor;
                    font.pixelSize: Theme.fontSizeTiny;
                }
                Label {
                    text: msgstate;
                    color: Theme.secondaryColor;
                    font.pixelSize: Theme.fontSizeTiny;
                }
            }
        }

        anchors.fill: parent;

    }
}


