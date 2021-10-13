import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2

Row {
    property var paramData

    property var paramValue: paramData && paramData.value ? paramData.value : null
    property bool isSwitchEdit: (paramValue && (paramData.editFieldType === "Switch"))

    enum ValueType {
        Boolean,
        Number,
        Text
    }

	Item {
		id: booleanInputSpacer
        visible: paramValue && (getValueType(paramData) === ParamControl.Boolean)
		width:  0//largestEditWidth - (checkBox.visible ? checkBox.width : switchItem.width) - root.padding
		height: 1//textField.width - checkBox.width
	}
	
	CheckBox {
		id: checkBox
		checked: paramValue
        visible: paramValue && (paramData.editFieldType === "CheckBox")
		
		onCheckedChanged: {
			if (!visible)
				return
			paramValue = checked
		}
	}
	
	Switch {
		id: switchItem
		checked: paramValue
        visible: paramValue && (paramData.editFieldType === "Switch")
		
		onCheckedChanged: {
			if (!visible)
				return
			paramValue = checked
		}
	}
	
	ComboBox {
		id: comboBox
        model: paramValue && (paramData.possibleValues) ? paramData.possibleValues : []
        visible: paramValue && (paramData.editFieldType === "ComboBox")
		implicitContentWidthPolicy: ComboBox.WidestText
        width: implicitWidth
    }
	
    ParamTextEdit {
		id: textEditFrame
        visible: paramValue && paramData.editFieldType && (paramData.editFieldType === "TextInput" || (getValueType(paramData) !== ParamControl.Boolean && !comboBox.visible))
	}

    function getValueType(paramData) {
        switch (paramData.valueType) {
        case "Text":
            return ParamControl.Text
        case "Boolean":
            return ParamControl.Boolean
        case "Number":
        default :
            return ParamControl.Number
        }
    }
}
