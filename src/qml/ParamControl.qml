import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2

Row {
    id: root

    property var paramData

    property var paramValue: paramData && paramData.value ? paramData.value : null
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
        checked: visible ? paramData.value : false

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
        checked: visible ? paramData.value : false

        height: 25
        width: 50
		
		onCheckedChanged: {
			if (!visible)
				return
			paramValue = checked
		}
	}
	
	ComboBox {
		id: comboBox
        model: paramData && (paramData.possibleValues) ? paramData.possibleValues : []
        visible: paramData && (paramData.editFieldType === "ComboBox")
		implicitContentWidthPolicy: ComboBox.WidestText
        width: implicitWidth
        height: 25

        onCurrentValueChanged: {
            paramValue = currentValue
        }
    }
	
    ParamTextEdit {
		id: textEditFrame
        text: paramValue !== "" ? paramValue : paramData.defaultValue
        visible: paramData && (paramData.editFieldType === "TextInput" || (getValueType(paramData) !== ParamControl.Boolean && !comboBox.visible))
	}

//    Rectangle {
//        property var paramData: (specifier && specifier.paramData !== undefined) ? specifier.paramData : false
//        visible: paramData
//        color: "green"
//        height: 25
//        width: 5
//    }
}
