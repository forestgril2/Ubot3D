import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick3D.Helpers 1.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick3D 1.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Row {
    id: root
    property var specifier
    spacing: 5

    ParamControl {
        id: paramControl
        paramData: (specifier && specifier.paramData !== undefined) ? specifier.paramData : false
        visible: paramControl.paramData
    }

    Label {
        id: paramLabel
        visible: (specifier && specifier.paramName) ? true : false
        text: (specifier && specifier.paramName) ? specifier.paramName : ""
        height: 25
    }

    Rectangle {
        id: blankSpace
        visible: !specifier
        color: "white"
        height: 25
        width: 1
    }

//	onParamValueChanged: {
//		root.paramValueChanged(param, paramValue)
//	}
}
