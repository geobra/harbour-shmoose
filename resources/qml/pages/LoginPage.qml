import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.shmoose 1.0

Page {
	id: loginPage
	Column {
		anchors.fill: parent

		PageHeader {
			title: "Login"
		}

		Label { 
	                x: Theme.paddingLarge
	                text: "Login to Jabber Server"
	                color: Theme.secondaryHighlightColor
	                font.pixelSize: Theme.fontSizeExtraLarge
		}

		TextField {
			id: jidTextField
			placeholderText: "jid@server.com"
			label: "Jid"
			text: shmoose.getJid()
			width: parent.width
			
			onTextChanged: {
				checkEnableConnectButton();
			}
		}

		TextField {
			id: passTextField
			placeholderText: "password"
			echoMode: TextInput.Password
			label: "Password"
			text: shmoose.getPassword()
			width: parent.width

			onTextChanged: {
				checkEnableConnectButton();
			}
		}

		Button{
			id: connectButton
			text: "Connect"
			enabled: false

			onClicked: {
				connectButton.enabled = false;
				connectButton.text = "Connecting...";
				shmoose.mainConnect(jidTextField.text, passTextField.text);
			}

		}
	}


        Connections {
		target: shmoose
                onConnectionStateConnected: {
			pageStack.replace(pageMenu)
		}
	}

	function checkEnableConnectButton() {
		if (jidTextField.text.length > 0 && passTextField.text.length > 0) {
			connectButton.enabled = true;
		}
		else {
			connectButton.enabled = false;
		}
	}
}
