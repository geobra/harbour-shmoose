import QtQuick 2.0
import QtQuick.Window 2.0;
import Sailfish.Silica 1.0
import harbour.shmoose 1.0

Page {
    id: page;

    onStatusChanged: {
        if (status === PageStatus.Active) {
            pageStack.pushAttached(Qt.resolvedUrl("MessagingSettingsPage.qml"),{ 'conversationId': conversationId })
        }
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

    PageHeader {
        id: banner;
        title: shmoose.rosterController.getNameForJid(conversationId);
        Image {
            id: avatar;
            parent: banner.extraContent;
            width: Theme.iconSizeMedium;
            height: width;
            smooth: true;
            source: imagePath != "" ? imagePath : getImage(conversationId);
            fillMode: Image.PreserveAspectCrop;
            antialiasing: true;
            anchors {
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
    }
    SilicaListView {
        id: view;

        verticalLayoutDirection: ListView.BottomToTop;
        clip: true;
        spacing: Theme.paddingMedium;

        model: shmoose.persistence.messageController

        delegate: ListItem {
            id: item;

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
                    id: msgImg
                    property string picPath: attachmentPath + "/" + shmoose.getLocalFileForUrl(message)
                    source: ( (type === "image") ? picPath : "");
                    width: Math.min (item.maxContentWidth, sourceSize.width);
                    fillMode: Image.PreserveAspectFit;
                    visible: (type === "image")
                    anchors {
                        left: (item.alignRight ? parent.left : undefined);
                        right: (!item.alignRight ? parent.right : undefined);
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            pageStack.push(Qt.resolvedUrl("ImagePage.qml"),{ 'imgUrl': msgImg.picPath })
                        }
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
                            return "../img/read_green.png"
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
                MenuItem {
                    visible: isGroup;
                    text: qsTr("Status")
                    onClicked:  {
                        shmoose.persistence.gcmController.setFilterOnMsg(id);
                        pageStack.push(pageMsgStatus);
                    }
                }
            }

        }
        anchors {
            top: banner.bottom;
            left: parent.left;
            right: parent.right;
            bottom: displaymsgview.top;
        }

    }
    Row {
        id: displaymsgview

        spacing: 5

        anchors {
            left: parent.left;
            right: parent.right;
            bottom:sendmsgview.top;
            margins: view.spacing;
        }
        Label {
            id: displaymsgviewlabel
            text:  shmoose.persistence.getResourcesOfNewestDisplayedMsgforJid(conversationId);
            enabled: isGroup;
            color: Theme.highlightColor;
            font {
                family: Theme.fontFamily;
                pixelSize: Theme.fontSizeTiny;
            }
        }
        Image {
            source: "../img/read_until_green.png";
            visible: displaymsgviewlabel.text.length > 0 ? true: false;
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
                if (editbox.text.length === 0 && sendmsgview.attachmentPath.length === 0 && shmoose.canSendFile()) {
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

                    displaymsgviewlabel.text = "";
                }
            }
            function processAttachment(path) {
                //console.log(path)
                sendmsgview.attachmentPath = path
                sendButton.icon.source = getSendButtonImage()
            }
        }
        Connections {
            target: shmoose
            onSignalCanSendFile: {
                console.log("HTTP uploads enabled");
                sendButton.icon.source = getSendButtonImage();
            }
        }
    }

    function getSendButtonImage() {
        if (editbox.text.length === 0 && sendmsgview.attachmentPath.length === 0) {
            if (shmoose.canSendFile()) {
                return "image://theme/icon-m-attach"
            } else {
                return "image://theme/icon-m-enter-accept"
            }
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
