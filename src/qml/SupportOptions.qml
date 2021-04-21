import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick3D.Helpers 1.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick3D 1.15
import QtQuick.Controls 2.15
import customgeometry 1.0
import "HelperFunctions.js" as QmlHelpers

Window {
	id: supportOptions
	property alias isGeneratingSupport: supportSwitch.isGeneratingSupport
	minimumWidth: 300
	title: "Support options"
	
	Switch {
		id: supportSwitch
		property bool isGeneratingSupport: (position == 1.0)
		text: "Generate support"
	}
}