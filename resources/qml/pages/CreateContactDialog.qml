import QtQuick 2.2
import Sailfish.Silica 1.0

Dialog {
    property string jid
    property string name

    Column {
        width: parent.width

        DialogHeader {
            id: header
            title: qsTr("Add Contact")
        }

        TextField {
            id: jidField
            width: parent.width
            placeholderText: qsTr("id@domain.org")
            label: qsTr("JID")
        }

        TextField {
            id: nameField
            width: parent.width
            placeholderText: qsTr("Name")
            label: qsTr("NickName")
        }
    }

    onDone: {
        if (result == DialogResult.Accepted) {
            jid = jidField.text
            name = nameField.text

            if (jid.length > 0)
            {
                shmoose.rosterController.addContact(jid, name)
            }
        }
    }
}
