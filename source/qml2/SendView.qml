import QtQuick.Layouts 1.2
import QtQuick 2.3
import QtQuick.Controls 2.4


Rectangle {
    color: "green"
    RowLayout {

        //height: parent.height
        width: parent.width
        //anchors.top: parent.bottom

        // https://doc.qt.io/qt-5/qml-qtquick-textedit.html

/*
        Flickable {
             id: flick

             width: 300
             height: 200
             //width: parent.width - sendbutton.width
             contentWidth: edit.paintedWidth
             contentHeight: edit.paintedHeight
             clip: true
*/
             TextEdit {
                 id: edit
                 Layout.fillWidth: true
                 //width: flick.width
                 focus: true
                 wrapMode: TextEdit.Wrap
                 //onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)
                 text: "yfasdf asdf asdf asf sdgv sdfsafd sdfgsdfg df sdfgsdgv asdfas<df sdgv dsfg dsfg dgfsdgf df g"
             }
//         }

        Button {
            id: sendbutton

            //anchors.right: parent.right
            //Layout.alignment: Qt.AlignRight
            Layout.fillWidth: true

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
