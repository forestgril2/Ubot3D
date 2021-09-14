import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2

Row {
    id: root
    property var paramData
    property var paramValue: paramData.value
    property var editFieldWidth: getEditFieldWidth()
    property var largestEditWidth: 0
    padding: 4
    spacing: 11

    enum ValueType {
        Boolean,
        Number,
        Text
    }

    Item {
        id: booleanInputSpacer
        visible: getValueType(paramData) === ParameterInputRow.Boolean
        width:  largestEditWidth - (checkBox.visible ? checkBox.width : switchItem.width) - root.padding
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

    Frame {
        id: textEditFrame
        property var defaultWidth: 50
        width: defaultWidth > largestEditWidth ? defaultWidth : largestEditWidth
        visible: paramData.editFieldType === "TextInput" || (getValueType(paramData) !== ParameterInputRow.Boolean && !comboBox.visible)

        TextInput {
            id: textField
            anchors.fill: parent
            text: paramValue !== "" ? paramValue : root.paramData.defaultValue
            Layout.alignment: Qt.AlignRight
            horizontalAlignment: TextInput.AlignRight
            rightPadding: root.padding
            onEditingFinished: {
                setParameterValue(paramData, text)
            }


            function parseNumberValue(text) {
                if (parseInt(text) !== NaN)
                {
                    paramValue = parseFloat(text)
                    console.log(" parseNumberValue paramValue: ", paramValue)
                }
            }

            function parseTextValue(text) {
                paramValue = text
                console.log(" parseTextValue paramValue: ", paramValue)
            }
        }
    }

    Label {
        text: root.paramData.text ? root.paramData.text : root.paramData.name ? root.paramData.name : root.paramData.description
    }

    function setParameterValue(paramData, value) {
        switch (getValueType(paramData)) {
        case ParameterInputRow.Number:
            textField.parseNumberValue(value)
            break;
        case ParameterInputRow.Value:
            textField.parseTextValue(value)
            break;
        }
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

    function getEditFieldWidth() {
        switch(paramData.editFieldType) {
        case "CheckBox":
            return checkBox.width
        case "ComboBox":
            return comboBox.width
        default:
            return textEditFrame.width
        }
    }
}