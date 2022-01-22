import QtQuick 2.0 
import QtQuick.Window 2.0;
import Sailfish.Silica 1.0 
import harbour.shmoose 1.0 

Page {
    id: page
    allowedOrientations: Orientation.All
    property string jid
    property string imagePath : shmoose.rosterController.getAvatarImagePathForJid(jid)
    property string jidName : shmoose.rosterController.getNameForJid(jid)

    Column {

        PageHeader {
            title: jid+" "+qsTr("settings")
        }

        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingMedium
        }            
        
        Image {
            id: avatar
            anchors {
                horizontalCenter: parent.horizontalCenter
            }            
            width: Theme.iconSizeExtraLarge
            height: width
            smooth: true
            source: imagePath != "" ? imagePath : "image://theme/icon-l-people"
            fillMode: Image.PreserveAspectCrop
            antialiasing: true

            Rectangle {
                z: -1;
                color: "black"
                opacity: 0.35
                anchors.fill: parent
            }
        }

        TextField {
            width: parent.width
            text:  jidName
            label: qsTr("Contact name")
            Component.onDestruction: {
                if(text != jidName && text.length > 0)
                    shmoose.rosterController.updateNameForJid(text); 
            }
        }
    }
}
