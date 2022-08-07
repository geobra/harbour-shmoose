import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page;
    allowedOrientations: Orientation.All;

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        VerticalScrollDecorator {}

        Column {
            id: column
            width: parent.width

            PageHeader { title: qsTr("Settings") }

            SectionHeader { text: qsTr("Notifications") }

            TextSwitch {
                id: chatNotificationSwitch
                checked: shmoose.settings.DisplayChatNotifications
                text: qsTr("Display chat notifications")
                onClicked: {
                    shmoose.settings.DisplayChatNotifications = chatNotificationSwitch.checked;
                }
            }
            TextSwitch {
                id: groupchatNotificationSwitch
                checked: shmoose.settings.DisplayGroupchatNotifications
                text: qsTr("Display group chat notifications")
                onClicked: {
                    shmoose.settings.DisplayGroupchatNotifications = groupchatNotificationSwitch.checked;
                }
            }

            SectionHeader { text: qsTr("Privacy") }

            TextSwitch {
                id: readNotificationSwitch
                checked: shmoose.settings.SendReadNotifications
                text: qsTr("Send Read Notifications")
                onClicked: {
                    shmoose.settings.SendReadNotifications = readNotificationSwitch.checked;
                }
            }

            SectionHeader { text: qsTr("Attachments") }

            TextSwitch {
                id: askBeforeDownloadingSwitch
                checked: shmoose.settings.AskBeforeDownloading
                text: qsTr("Ask before downloading attachments")
                onClicked: {
                    shmoose.settings.AskBeforeDownloading = askBeforeDownloadingSwitch.checked;
                }
            }

            SectionHeader { text: qsTr("Features") }

            TextSwitch {
                id: softwareFeatureOmemoSwitch
                checked: shmoose.settings.EnableSoftwareFeatureOmemo
                text: qsTr("Omemo Message Encryption - Experimental! (Need app restart)")
                onClicked: {
                    shmoose.settings.EnableSoftwareFeatureOmemo = softwareFeatureOmemoSwitch.checked;
                }
            }

        }
    }
}
