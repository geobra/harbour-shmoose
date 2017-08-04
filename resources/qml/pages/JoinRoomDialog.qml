import QtQuick 2.2
import Sailfish.Silica 1.0

Dialog {
    property string jid
    property string name

    Column {
        width: parent.width

        DialogHeader {
            id: header
            title: qsTr("Join Room by address")
        }

        TextField {
            id: jidField
            width: parent.width
            placeholderText: qsTr("id@domain.org")
            label: qsTr("Room address")
        }

        TextField {
            id: nameField
            width: parent.width
            placeholderText: qsTr("Name")
            label: qsTr("Room name")
        }
    }

    onDone: {
        if (result == DialogResult.Accepted) {
            jid = jidField.text
            name = nameField.text

            if (jid.length > 0)
            {
                shmoose.joinRoom(jid, name)
            }
        }
    }
}
