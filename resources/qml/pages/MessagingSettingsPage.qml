import QtQuick 2.0 
import QtQuick.Window 2.0;
import Sailfish.Silica 1.0 
import harbour.shmoose 1.0 

Page {
    id: page
    property string conversationId

    Image {
        //source: "image://glass/qrc:///qml/img/photo.png"
        opacity: 0.85
        sourceSize: Qt.size (Screen.width, Screen.height)
        asynchronous: false
        anchors.centerIn: parent
    }
    Column {
        PageHeader {
            title: conversationId+" settings"
        }
        ComboBox {
            label: "Chat notifications"
            width: page.width
            currentIndex: (
                shmoose.settings.ForceOnNotifications.indexOf(conversationId) >= 0 ? 1 :
                shmoose.settings.ForceOffNotifications.indexOf(conversationId) >= 0 ? 2 :
                0
            )
            menu: ContextMenu {
                MenuItem {
                    text: "Default setting"
                    onClicked: {
                        shmoose.settings.removeForceOnNotifications(conversationId);
                        shmoose.settings.removeForceOffNotifications(conversationId);
                    }
                }
                MenuItem {
                    text: "On"
                    onClicked: {
                        shmoose.settings.addForceOnNotifications(conversationId);
                        shmoose.settings.removeForceOffNotifications(conversationId);
                    }
                }
                MenuItem {
                    text: "Off"
                    onClicked: {
                        shmoose.settings.removeForceOnNotifications(conversationId);
                        shmoose.settings.addForceOffNotifications(conversationId);
                    }
                }
            }
        }
    }
}
