import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick 2.15
import QtQuick.Controls 2.15

Row {
	id: parameterRow
    property var param
	
	Switch {
		id: twoExtrudersSwitch
		property bool isUsingTwoExtruders: (position == 1.0)
        text: param.name
	}
	
}
