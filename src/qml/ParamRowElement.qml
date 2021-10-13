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

    ParamControl {
        id: paramControl
        paramData: (specifier && specifier.paramData !== undefined) ? specifier.paramData : false
        visible: paramControl.paramData
        height: 30
        width: 50
    }

    Label {
        id: paramLabel
        visible: (specifier && specifier.paramName) ? true : false
        text: (specifier && specifier.paramName) ? specifier.paramName : ""
    }

    Rectangle {
        id: blankSpace
        visible: !specifier
        color: "lightgrey"
        height: 30
        width: 50
    }

//	onParamValueChanged: {
//		root.paramValueChanged(param, paramValue)
//	}
}
