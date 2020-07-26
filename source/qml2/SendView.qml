import QtQuick.Layouts 1.2
import QtQuick 2.3
import QtQuick.Controls 2.4

RowLayout {

    height: parent.height
    width: parent.width

    ScrollView {
        TextArea {
            text: "TextArea\n...\n...\n...\n...\n...\n...\n"
        }
    }

    Button {
        //anchors.fill: parent

        text: "send"
    }
}
