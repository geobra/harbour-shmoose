import QtQuick 2.0
import Sailfish.Silica 1.0


Page {
    SilicaListView {
        id: view;
        header: Column {
            spacing: Theme.paddingLarge;
            anchors {
                left: parent.left;
                right: parent.right;
            }

            PageHeader {
                title: "Settings";
            }

            Separator {
                primaryColor: Qt.rgba (1,1,1, 0.5);
                secondaryColor: Qt.rgba (1,1,1, 0.0);
                anchors {
                    left: parent.left;
                    right: parent.right;
                }
            }
        }
        model: [
            { "headline" : qsTr("Omemo"),      "description" : qsTr("Beta! Use at own risk!") },
            //{ "headline" : qsTr("bla"),        "description" : qsTr("Beta! Use at own risk!") },
            //{ "headline" : qsTr("blub"),       "description" : qsTr("Beta! Use at own risk!") },
        ];
        delegate: BackgroundItem {
            id: item;
            height: Theme.itemSizeMedium;
            anchors {
                left: parent.left;
                right: parent.right;
            }
            onClicked: {
                //pageStack.push (modelData ["page"]);
                //pageStack.navigateForward ();
                console.log("clicked");
            }

            TextSwitch {
                text: modelData ["headline"];
                description: modelData ["description"];
                onCheckedChanged: {
                    //device.setStatus(checked ? DeviceState.Armed : DeviceState.Disarmed)
                    console.log("toggeld");
                }
            }
        }
        footer: Column {
            spacing: Theme.paddingLarge;
            anchors {
                left: parent.left;
                right: parent.right;
            }

            Separator {
                primaryColor: Qt.rgba (1,1,1, 0.5);
                secondaryColor: Qt.rgba (1,1,1, 0.0);
                anchors {
                    left: parent.left;
                    right: parent.right;
                }
            }
        }
        anchors.fill: parent

        VerticalScrollDecorator {}
    }
}

