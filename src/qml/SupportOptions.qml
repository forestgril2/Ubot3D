import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick3D.Helpers 1.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick3D 1.15
import QtQuick.Controls 2.15
import customgeometry 1.0
import "HelperFunctions.js" as QmlHelpers

Window {
	id: supportOptions

    property alias alphaValue: alphaSlider.value

    property bool isGeneratingSupport: supportSwitch.position === 1
    property bool isSupportExported: supportExportSwitch.position === 1
	minimumWidth: 300
	title: "Support options"

    property var supportVertices
	
    Column
    {
        Switch {
            id: supportSwitch
            text: "Generate support"
        }
        Switch {
            id: supportExportSwitch
            text: "STL Export support with model"
        }

        TextField {
            id: textField
            text: alphaSlider.value
            width: alphaSlider.width

            onEditingFinished: {
                if (parseInt(text) !== NaN)
                {
                    alphaSlider.value = parseFloat(text)
                }
            }
        }

        Slider {
            id: alphaSlider
            from: 0.01
            to: 10
            orientation: Qt.Horizontal
            width: 200
        }

        Rectangle {
            color: "white"
            border.color: "black"
            border.width: 4
            width: 1000
            height: 1000
        }
    }

    Component.onCompleted: {
        console.log(" ### isGeneratingSupport:" + isGeneratingSupport)
        console.log(" ### isSupportExported:" + isSupportExported)
    }
}
