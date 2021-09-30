import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2

Row {
    id: root
    property var paramData
    property var paramValue: paramData.value
    property var editFieldWidth: parameterEditRepeater.width
    property var largestEditWidth: 0
    property bool isSwitchEdit: paramData.editFieldType === "Switch"
    padding: 4
    spacing: 11

    enum ValueType {
        Boolean,
        Number,
        Text
    }

    Repeater {
        id: parameterEditRepeater
        model: paramData.isExtruderParam ? 2 : 1

        Row {
            Item {
                id: booleanInputSpacer
                visible: getValueType(paramData) === ParameterInputRow.Boolean
                width:  0//largestEditWidth - (checkBox.visible ? checkBox.width : switchItem.width) - root.padding
                height: 1//textField.width - checkBox.width
            }

            CheckBox {
                id: checkBox
                checked: paramValue
                visible: paramData.editFieldType === "CheckBox"

                onCheckedChanged: {
                    if (!visible)
                        return
                    paramValue = checked
                }
            }

            Switch {
                id: switchItem
                checked: paramValue
                visible: paramData.editFieldType === "Switch"

                onCheckedChanged: {
                    if (!visible)
                        return
                    paramValue = checked
                }
            }

            ComboBox {
                id: comboBox
                model: root.paramData.possibleValues
                visible: paramData.editFieldType === "ComboBox"
                implicitContentWidthPolicy: ComboBox.WidestText
                width: implicitWidth > largestEditWidth ? implicitWidth : largestEditWidth
            }

            ParameterInputTextEdit {
                id: textEditFrame
                visible: paramData.editFieldType === "TextInput" || (getValueType(paramData) !== ParameterInputRow.Boolean && !comboBox.visible)
            }
        }
    }

    Label {
        text: root.paramData.text ? root.paramData.text : root.paramData.name ? root.paramData.name : root.paramData.description
    }

    function getValueType(paramData) {
        switch (paramData.valueType) {
        case "Text":
            return ParameterInputRow.Text
        case "Boolean":
            return ParameterInputRow.Boolean
        case "Number":
        default :
            return ParameterInputRow.Number
        }
    }
}
