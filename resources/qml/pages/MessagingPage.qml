import QtQuick 2.0
import QtQuick.Window 2.0;
import Sailfish.Silica 1.0
import harbour.shmoose 1.0

Page {
    id: page;

    onStatusChanged: {
        if (status == PageStatus.Deactivating) {
            if (_navigation == PageNavigation.Back) {
                shmoose.setCurrentChatPartner("")
            }
        }
    }

    property string conversationId : "";
    property bool isGroup : shmoose.rosterController.isGroup(conversationId);
    property string attachmentPath: shmoose.getAttachmentPath();
    property string imagePath : shmoose.rosterController.getAvatarImagePathForJid(conversationId);

    Image {
        //source: "image://glass/qrc:///qml/img/photo.png";
        opacity: 0.85;
        sourceSize: Qt.size (Screen.width, Screen.height);
        asynchronous: false
        anchors.centerIn: parent;
    }
    Item {
        id: banner;
        height: Theme.itemSizeLarge;
        anchors {
            top: parent.top;
            left: parent.left;
            right: parent.right;
        }

        Rectangle {
            z: -1;
            color: "black";
            opacity: 0.15;
            anchors.fill: parent;
        }
        Image {
            id: avatar;
            width: Theme.iconSizeMedium;
            height: width;
            smooth: true;
            source: imagePath != "" ? imagePath : getImage(conversationId);
            fillMode: Image.PreserveAspectCrop;
            antialiasing: true;
            anchors {
                right: parent.right;
                margins: Theme.paddingMedium;
                verticalCenter: parent.verticalCenter;
            }

            Rectangle {
                z: -1;
                color: "black";
                opacity: 0.35;
                anchors.fill: parent;
            }
        }
        Column {
            anchors {
                right: avatar.left;
                margins: Theme.paddingMedium;
                verticalCenter: parent.verticalCenter;
            }

            Label {
                text:  shmoose.rosterController.getNameForJid(conversationId);
                color: Theme.highlightColor;
                font {
                    family: Theme.fontFamilyHeading;
                    pixelSize: Theme.fontSizeLarge;
                }
                anchors.right: parent.right;
            }
            Label {
                //text: qsTr ("last seen yesterday, 12:30 PM");
                text: "";
                color: Theme.secondaryColor;
                font {
                    family: Theme.fontFamilyHeading;
                    pixelSize: Theme.fontSizeTiny;
                }
                anchors.right: parent.right;
            }
        }
    }
    SilicaListView {
        id: view;

        clip: true;
        rotation: 180

        model: shmoose.persistence.messageController

        //        header: Item {
        //            height: view.spacing;
        //            anchors {
        //                left: parent.left;
        //                right: parent.right;
        //            }
        //        }
        //        footer: Item {
        //            height: view.spacing;
        //            anchors {
        //                left: parent.left;
        //                right: parent.right;
        //            }
        //        }
        spacing: Theme.paddingMedium;
        delegate: ListItem {
            id: item;

            rotation: 180
            contentHeight: shadow.height;

            anchors {
                left: parent.left;
                right: parent.right;
                margins: view.spacing;
            }

            readonly property bool alignRight      : (direction == 1);
            readonly property int  maxContentWidth : (width * 0.85);

            Rectangle {
                id: shadow;
                color: "white";
                radius: 3;
                opacity: (item.alignRight ? 0.05 : 0.15);
                antialiasing: true;
                anchors {
                    fill: layout;
                    margins: -Theme.paddingSmall;
                }
            }
            Column {
                id: layout;

                anchors {
                    left: (item.alignRight ? parent.left : undefined);
                    right: (!item.alignRight ? parent.right : undefined);
                    margins: -shadow.anchors.margins;
                    verticalCenter: parent.verticalCenter;
                }


                Label {
                    text: message;
                    color: Theme.primaryColor;
                    //width: Math.min (item.maxContentWidth, contentWidth);
                    width: item.maxContentWidth;
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere;
                    visible: true;

                    font {
                        family: Theme.fontFamilyHeading;
                        pixelSize: (type === "image") ? Theme.fontSizeTiny : Theme.fontSizeMedium;
                    }
                    anchors {
                        left: (item.alignRight ? parent.left : undefined);
                        right: (!item.alignRight ? parent.right : undefined);
                    }

                    onLinkActivated: Qt.openUrlExternally(link)
                }
                Image {
                    source: ( (type === "image") ? attachmentPath + "/" + basename(message) : "");
                    width: Math.min (item.maxContentWidth, sourceSize.width);
                    fillMode: Image.PreserveAspectFit;
                    visible: (type === "image")
                    anchors {
                        left: (item.alignRight ? parent.left : undefined);
                        right: (!item.alignRight ? parent.right : undefined);
                    }
                }

                Label {
                    visible: isGroup;
                    color: Theme.secondaryColor;
                    font {
                        family: Theme.fontFamilyHeading;
                        pixelSize: Theme.fontSizeTiny;
                    }
                    text: resource;
                }

                Label {
                    text: Qt.formatDateTime (new Date (timestamp * 1000), "yyyy-MM-dd hh:mm:ss");
                    color: Theme.secondaryColor;
                    font {
                        family: Theme.fontFamilyHeading;
                        pixelSize: Theme.fontSizeTiny;
                    }
                    anchors {
                        left: (item.alignRight ? parent.left : undefined);
                        right: (!item.alignRight ? parent.right : undefined);
                    }
                }
                Image {
                    source: {
                        if (msgstate == 3) {
                            return "../img/read_until_green.png"
                        }
                        if (msgstate == 2) {
                            return "../img/2check.png"
                        }
                        if (msgstate == 1) {
                            return "../img/check.png"
                        }
                        return ""
                    }
                    anchors.right: parent.right
                }

            }

            menu: ContextMenu {
                MenuItem {
                    text: qsTr("Copy")
                    onClicked: Clipboard.text = message
                }
            }

        }
        anchors {
            top: banner.bottom;
            left: parent.left;
            right: parent.right;
            bottom: sendmsgview.top;
        }

    }

    Row {
        id: sendmsgview

        property var attachmentPath: ""

        anchors {
            left: parent.left;
            right: parent.right;
            bottom: parent.bottom;
        }

        TextArea {
            id: editbox;
            placeholderText: qsTr("Enter message...");
            width: parent.width - 100
            font {
                family: Theme.fontFamily
                pixelSize: Theme.fontSizeMedium
            }

            height: editbox.activeFocus ? font.pixelSize * 6 : font.pixelSize * 4

            onTextChanged: {
                sendButton.icon.source = getSendButtonImage()
            }
        }
        IconButton {
            id: sendButton
            enabled: {
                if (shmoose.connectionState && mainWindow.hasInetConnection) {
                    return true
                }
                else {
                    return false
                }
            }

            anchors.bottom: parent.bottom

            icon.source: getSendButtonImage()
            width: 100
            onClicked: {
                if (editbox.text.length === 0 && sendmsgview.attachmentPath.length === 0) {
                    sendmsgview.attachmentPath = ""
                    fileModel.searchPath = shmoose.settings.ImagePaths
                    pageStack.push(pageImagePicker)
                    pageImagePicker.selected.connect(processAttachment)
                } else {
                    //console.log(sendmsgview.attachmentPath)
                    var msgToSend = editbox.text;
                    if (sendmsgview.attachmentPath.length > 0) {
                        shmoose.sendFile(conversationId, sendmsgview.attachmentPath);
                        sendmsgview.attachmentPath = ""
                    }
                    if (msgToSend.length > 0) {
                        shmoose.sendMessage(conversationId, msgToSend, "text");
                        editbox.text = " ";
                        editbox.text = "";
                    }
                }
            }
            function processAttachment(path) {
                //console.log(path)
                sendmsgview.attachmentPath = path
                sendButton.icon.source = getSendButtonImage()
            }
        }
    }

    function getSendButtonImage() {
        if (editbox.text.length === 0 && sendmsgview.attachmentPath.length === 0) {
            return "image://theme/icon-m-attach"
        } else {
            if (sendmsgview.attachmentPath.length > 0) {
                return "image://theme/icon-m-media"
            } else {
                return "image://theme/icon-m-enter-accept"
            }
        }
    }

    function basename(str)
    {
        return (str.slice(str.lastIndexOf("/")+1))
    }

    function getImage(jid) {
        if (shmoose.rosterController.isGroup(jid)) {
            return "image://theme/icon-l-image";
        } else {
            return "image://theme/icon-l-people"
        }
    }

}
