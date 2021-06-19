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
    id: raftOptions

    property alias raftOffset: raftOffsetSlider.value

    property bool isGeneratingRafts: raftSwitch.position === 1
    property bool areRafstExported: raftExportSwitch.position === 1
    minimumWidth: 400
    minimumHeight: 200

    title: "Raft options"

    property var floorVertices

    Column
    {
        leftPadding: 10

        Switch {
            id: raftSwitch
            text: "Generate rafts"
        }
        Switch {
            id: raftExportSwitch
            text: "STL Export rafts with model"
        }

        Row {
            spacing: 10
            Label {
                text: "Raft offset [mm]"
            }

            TextField {
                id: raftOffsetTextField
                text: raftOffsetSlider.value
                width: raftOffsetSlider.width

                onEditingFinished: {
                    if (parseInt(text) !== NaN)
                    {
                        raftOffsetSlider.value = parseFloat(text)
                    }
                }
            }
        }
        Slider {
            id: raftOffsetSlider
            from: -10
            to: 10
            value: 0
            orientation: Qt.Horizontal
            width: 200
        }

        Row {
            spacing: 10
            Label {
                text: "Raft height [mm]:"
            }
            TextField {
                id: raftHeightTextField
                text: raftHeightSlider.value
                width: raftHeightSlider.width

                onEditingFinished: {
                    if (parseInt(text) !== NaN)
                    {
                        raftHeightSlider.value = parseFloat(text)
                    }
                }
            }
        }

        Slider {
            id: raftHeightSlider
            from: 0.1
            to: 5
            orientation: Qt.Horizontal
            width: 200
        }
    }
}
