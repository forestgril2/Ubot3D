import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2

Row {
    id: root
    property var paramData
    property var paramValue: paramData.value
    padding: 4
    spacing: 11

    enum ValueType {
        Boolean,
        Number,
        Text
    }

    Item {
        id: checkBoxSpacer
        visible: checkBox.visible
        width: textField.width - checkBox.width - root.padding
        height: 1//textField.width - checkBox.width
    }

    CheckBox {
        id: checkBox
        visible: getValueType(paramData) === ParameterInputRow.Boolean
        onCheckedChanged: {
            paramValue = checked
        }
    }

    Frame {
        width: 100
        visible: !checkBox.visible

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

    Label {
        text: root.paramData.text ? root.paramData.text : root.paramData.name ? root.paramData.name : root.paramData.description
    }

}
