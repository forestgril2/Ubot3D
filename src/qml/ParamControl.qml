import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2

Row {
    id: root

    property var paramData
    property var isExtruderParam
    property var extruderIndex
    property var paramValue: getParamValue()
    property var controlValue: getControlValue()
    property bool isSwitchEdit: (paramValue && (paramData.editFieldType === "Switch"))

    enum ValueType {
        Boolean,
        Number,
        Text
    }

//    Rectangle {
//        property var paramData: (specifier && specifier.paramData !== undefined) ? specifier.paramData : false
//        visible: paramData
//        color: "green"
//        height: 25
//        width: 1
//    }
	
	CheckBox {
		id: checkBox
        visible: paramData && (paramData.editFieldType === "CheckBox")
        checked: visible ? controlValue : false

        height: 25
        width: 25
		
		onCheckedChanged: {
			if (!visible)
				return
			paramValue = checked
		}
	}
	
	Switch {
		id: switchItem
        visible: paramData && (paramData.editFieldType === "Switch")
        checked: visible ? controlValue : false

        height: 25
        width: 50
		
		onCheckedChanged: {
			if (!visible)
				return
            setControlValue(checked)
		}
	}
	
	ComboBox {
		id: comboBox
        model: paramData && (paramData.possibleValues) ? paramData.possibleValues : []
        visible: paramData && (paramData.editFieldType === "ComboBox")
		implicitContentWidthPolicy: ComboBox.WidestText
        width: implicitWidth
        height: 25
        currentIndex: model.indexOf(getControlValue())

        onCurrentValueChanged: {
            setControlValue(currentValue)
        }
    }
	
    ParamTextEdit {
		id: textEditFrame
        text: controlValue && controlValue !== "" ? controlValue : (paramData.defaultValue ? paramData.defaultValue : "")
        visible: paramData && (paramData.editFieldType === "TextInput" || (getValueType(paramData) !== ParamControl.Boolean && !comboBox.visible))
	}

    function getControlValue() {
        if (paramValue === null)
            return null

        if (isExtruderParam)
            return paramValue[extruderIndex]

        return paramValue
    }

    function setControlValue(value) {
        if (isExtruderParam) {
            paramValue[extruderIndex] = value
        }
        else {
            paramValue = value
        }
    }

    function getParamValue() {
        return paramData && paramData.value ? paramData.value : null
    }

    function setParamValue() {
    }

//    Rectangle {
//        property var paramData: (specifier && specifier.paramData !== undefined) ? specifier.paramData : false
//        visible: paramData
//        color: "green"
//        height: 25
//        width: 5
//    }
}
