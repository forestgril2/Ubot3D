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

    signal paramValueChanged(var param, var value)

    ParamControl {
        id: paramControl
        paramData: (specifier && specifier.paramData !== undefined) ? specifier.paramData : false
        isExtruderParam: !paramData  ? undefined : paramData.isExtruderParam
        extruderIndex: !paramData ? undefined : specifier.isFirstParam ? 0 : 1
        visible: paramControl.paramData

        onParamValueChanged: {
            if (paramValue !== null) root.paramValueChanged(paramData, paramValue)
        }
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
}
