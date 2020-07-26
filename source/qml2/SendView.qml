import QtQuick.Layouts 1.2
import QtQuick 2.3
import QtQuick.Controls 2.4


Rectangle {
    color: "green"
    RowLayout {

        //height: parent.height
        width: parent.width
        //anchors.top: parent.bottom

        ScrollView {
            anchors.left: parent.left
            //anchors.right: sendbutton.left
            TextArea {
                id: texttosend

                anchors.fill: parent

                //width: parent.width - sendbutton.width
                Layout.preferredWidth: parent.width

                text: "TextArea\n...\n...\n...\n...\n...\n...\n"
            }
        }

        Button {
            id: sendbutton

            anchors.right: parent.right

            //width: 50
            //height: parent.height

            text: "send"
            onClicked: {
                console.log("send to: " + shmoose.rosterController.rosterList[roster.currentIndex].jid)
                shmoose.sendMessage(shmoose.rosterController.rosterList[roster.currentIndex].jid, texttosend.text, "txt")
                texttosend.text = ""
            }
        }
    }

}
