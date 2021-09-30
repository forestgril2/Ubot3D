import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2

Frame {
	id: textEditFrame
	property var defaultWidth: 50
	width: defaultWidth > largestEditWidth ? defaultWidth : largestEditWidth

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
