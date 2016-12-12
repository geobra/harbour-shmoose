import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 1.4

import harbour.shmoose 1.0

Window {
    id: chat

    property string jid: "foo"
    width: 300
    height: 300

    Text {
        anchors.centerIn: parent
        text: jid
    }
    ListView {
        id: messages

        height: 300
        width: 300

        model: shmoose.persistence.messageController

        delegate: Component {
            Item {
                width: parent.width
                height: 20

                Column {
                    Text { text: message }
                }
            }

        }
    }

    TextField {
        id: texttosend
        placeholderText: qsTr("Enter message")
    }
    Button {
        id: sendbutton
        text: "send"
        onClicked: {
            shmoose.sendMessage(jid, texttosend.text)
        }
    }
}
