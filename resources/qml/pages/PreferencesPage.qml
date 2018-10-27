import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page;

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        VerticalScrollDecorator {}

        Column {
            id: column
            width: parent.width

            PageHeader { title: "Settings" }

            SectionHeader { text: "Privacy" }

            TextSwitch {
                id: readNotificationSwitch
                checked: shmoose.settings.SendReadNotifications
                text: "Send Read Notifications"
                onClicked: {
                    shmoose.settings.SendReadNotifications = readNotificationSwitch.checked;
                }
            }
        }

    }

}
