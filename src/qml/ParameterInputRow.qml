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
        Text,
        Number
    }

    Frame {
        width: 100
        TextInput {
            id: textField
            anchors.fill: parent
            text: paramValue !== "" ? paramValue : root.paramData.defaultValue
            Layout.alignment: Qt.AlignRight
            horizontalAlignment: TextInput.AlignRight
            rightPadding: root.padding
            onEditingFinished: {
                parseParameterValue(paramData, text)
            }

            function parseParameterValue(paramData, text) {
                switch (getValueType(paramData)) {
                case ParameterInputRow.Number:
                    parseNumberValue(text)
                    break;
                case ParameterInputRow.Text:
                    parseTextValue(text)
                    break;
                }
            }

            function getValueType(paramData) {
                switch (paramData.valueType) {
                case "Text":
                    return ParameterInputRow.Text
                case "Number":
                default :
                    return ParameterInputRow.Number
                }
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

}
