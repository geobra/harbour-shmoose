import QtQuick 2.0 
import QtQuick.Window 2.0;
import Sailfish.Silica 1.0 
import harbour.shmoose 1.0 

Page {
    id: page
    allowedOrientations: Orientation.All
    property string conversationId
    readonly property int limitCompression : shmoose.settings.LimitCompression
    property int maxUploadSize : shmoose.getMaxUploadSize();

    Timer {
        interval: 2000; running: true; repeat: true
        onTriggered: {
            maxUploadSize = shmoose.getMaxUploadSize();
        }
    }
    Image {
        //source: "image://glass/qrc:///qml/img/photo.png"
        opacity: 0.85
        sourceSize: Qt.size (Screen.width, Screen.height)
        asynchronous: false
        anchors.centerIn: parent
    }
    Column {
        PageHeader {
            title: conversationId+qsTr(" settings")
        }

        ComboBox {
            label: qsTr("Chat notifications")
            width: page.width
            currentIndex: (
                shmoose.settings.ForceOnNotifications.indexOf(conversationId) >= 0 ? 1 :
                shmoose.settings.ForceOffNotifications.indexOf(conversationId) >= 0 ? 2 :
                0
            )
            menu: ContextMenu {
                MenuItem {
                    text: qsTr("Default setting")
                    onClicked: {
                        shmoose.settings.removeForceOnNotifications(conversationId);
                        shmoose.settings.removeForceOffNotifications(conversationId);
                    }
                }
                MenuItem {
                    text: qsTr("On")
                    onClicked: {
                        shmoose.settings.addForceOnNotifications(conversationId);
                        shmoose.settings.removeForceOffNotifications(conversationId);
                    }
                }
                MenuItem {
                    text: qsTr("Off")
                    onClicked: {
                        shmoose.settings.removeForceOnNotifications(conversationId);
                        shmoose.settings.addForceOffNotifications(conversationId);
                    }
                }
            }
        }

        TextSwitch {
            id: sendOmemoMsg
            enabled: shmoose.isOmemoUser(conversationId)
            checked: {
                if ( shmoose.isOmemoUser(conversationId) === false) {
                    return false;
                }
                else if (shmoose.settings.SendPlainText.indexOf(conversationId) >= 0) {
                    return false;
                }
                else {
                    return true;
                }
            }
            text: qsTr("Send omemo encrypted messages")
            onClicked: {
                if (sendOmemoMsg.checked) {
                    shmoose.settings.removeForcePlainTextSending(conversationId)
                }
                else {
                    shmoose.settings.addForcePlainTextSending(conversationId)
                }
            }
        }

        TextSwitch {
            id: compressImagesSwitch
            checked: shmoose.settings.CompressImages
            text: qsTr("Limit compression to")
            onClicked: {
                shmoose.settings.CompressImages = compressImagesSwitch.checked;
                limitCompressionSizeSlider.enabled = compressImagesSwitch.checked;
            }
        }

        Slider {
            id: limitCompressionSlider
            enabled: shmoose.settings.CompressImages
            width: parent.width
            minimumValue: 100000
            maximumValue: Math.max(maxUploadSize, limitCompression)
            stepSize: 100000
            value: limitCompression
            valueText: value/1000 + qsTr(" KB")

            onValueChanged: {
                shmoose.settings.LimitCompression = sliderValue;
            }
        }

        TextSwitch {
            id: sendOnlyImagesSwitch
            checked: shmoose.settings.SendOnlyImages
            text: qsTr("Send images only")
            onClicked: {
                shmoose.settings.SendOnlyImages = sendOnlyImagesSwitch.checked;
            }
        }
    }
}
