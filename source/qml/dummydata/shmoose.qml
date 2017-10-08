import QtQuick 2.2

QtObject { 
	signal connectionStateConnected()
	signal connectionStateDisconnected()

	function getJid() {
		return "dummy@server.org"
	}

	function getPassword() {
		return "badPasswort"
	}

	function saveCredentials(foo) {
	}

	function mainConnect() {
		connectionStateConnected();
	}

	function mainDisconnect() {
	}
}

