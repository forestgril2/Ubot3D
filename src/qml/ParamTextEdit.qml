import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2

Frame {
	id: textEditFrame
    property alias text: textField.text
	property var defaultWidth: 50
    height: 25
    width: defaultWidth > implicitWidth ? defaultWidth : implicitWidth

	TextInput {
		id: textField
		anchors.fill: parent
		Layout.alignment: Qt.AlignRight
		horizontalAlignment: TextInput.AlignRight
        rightPadding: 4
		onEditingFinished: {
			setParameterValue(paramData, text)
		}
		
        function parseNumber(text) {
			if (parseInt(text) !== NaN)
			{
                console.log(" parseNumberValue paramValue: ", paramValue)
                setControlValue(parseFloat(text))
			}
		}
		
        function parseText(text) {
            console.log(" parseTextValue paramValue: ", paramValue)
            setControlValue(text)
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

    function setParameterValue(paramData, text) {
        console.log(" setParameterValue: paramValue, value - ", paramValue, ", ", text)
        switch (getValueType(paramData)) {
        case ParamControl.Number:
            textField.parseNumber(text)
            break;
        case ParamControl.Text:
            textField.parseText(text)
            break;
        }
    }
}
