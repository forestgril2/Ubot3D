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
    property bool isSwitchEdit: paramData && paramData.editFieldType === "Switch"

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

    function isValueTypeConsistent(value) {
        // If it is an extruder param, we should have one value for each extruder
//        console.log(" ### isValueTypeConsistent")
//        console.log(" ### isValueTypeConsistent value:" + value)
//        console.log(" ### isValueTypeConsistent Array:" + Array.isArray(value))
//        console.log(" ### isValueTypeConsistent isExtruderParam :" + isExtruderParam)
        return isExtruderParam === Array.isArray(value)
    }

    function getFixedParamValue(value) {
        if (isExtruderParam && !Array.isArray(value)) {
            // Handle case of only single value defined.
            console.warn(" ### WARNING: One value defined for double extruder parameter. Converting to an array.")
            return [value, value]
        }

        if (!isExtruderParam && Array.isArray(value)) {
            // Handle case of only single value defined.
            console.error(" ### ERROR: Two values defined for non-extruder parameter. Returning first value only.")
            return value[0]
        }

        return value
    }

    function getParamValue() {
        var value = (paramData && paramData.value) ? paramData.value : null
//        console.log(" ### getParamValue() :" + value)

        if (value && !isValueTypeConsistent(value)) {
//            console.log(" ### getParamValue() :" + value)
            value = getFixedParamValue(value)
//            console.log(" ### fixedParamValue() :" + value)
        }

        return value
    }

    function getControlValue() {
        if (!paramValue)
            return null

        if (isExtruderParam)
            return paramValue[extruderIndex]

        return paramValue
    }

    function setControlValue(value) {
        if (isExtruderParam) {
//            console.log(" ### isExtruderParam:" + isExtruderParam)
//            console.log(" ### extruderIndex:" + extruderIndex)
//            console.log(" ### value:" + value)
//            console.log(" ### paramValue:" + paramValue)
            paramValue[extruderIndex] = value
            paramValue = paramValue
//            console.log(" ### paramValue:" + paramValue)
        }
        else {
            paramValue = value
        }
    }

//    Rectangle {
//        property var paramData: (specifier && specifier.paramData !== undefined) ? specifier.paramData : false
//        visible: paramData
//        color: "green"
//        height: 25
//        width: 5
//    }
}
