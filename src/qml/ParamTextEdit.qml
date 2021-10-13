import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2

Frame {
	id: textEditFrame
	property var defaultWidth: 50
    height: 25
    width: defaultWidth > implicitWidth ? defaultWidth : implicitWidth

	TextInput {
		id: textField
		anchors.fill: parent
		text: paramValue !== "" ? paramValue : root.paramData.defaultValue
		Layout.alignment: Qt.AlignRight
		horizontalAlignment: TextInput.AlignRight
        rightPadding: 4
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
}
