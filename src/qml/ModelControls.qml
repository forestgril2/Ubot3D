import QtQuick 2.15
import QtQuick.Controls 2.15
Row {
    property alias numSubPathsSlider: numSubPathsSlider
    property alias numPointsInSubPathSlider: numPointsInSubPathSlider
    property alias numPathPointsUsedSlider: numPathPointsUsedSlider

    property alias mouseInvertCheckBox: mouseInvertCheckBox
    property alias commonRotationCheckBox: commonRotationCheckBox
    property alias triangleModelWarpSlider: triangleModelWarpSlider

    property int numSubPaths
    property int numPointsInSubPath
    property int numPathPointsUsed

    Slider {
        id: triangleModelWarpSlider
        orientation: Qt.Vertical
        from: -0.1
        to: 0.1
        width: 50

        anchors {
            top: parent.top
            bottom: parent.bottom
        }
    }

    ValueEditSlider {
        id: numSubPathsSlider
        from: 0
        to: numSubPaths
    }

    ValueEditSlider {
        id: numPointsInSubPathSlider

        from: 0
        to: numPointsInSubPath
    }

    ValueEditSlider {
        id: numPathPointsUsedSlider

        from: 0
        to: numPathPointsUsed
    }

    Column {
        anchors {
            bottom: parent.bottom
        }

        Button {
            id: snapToFloor

            text: "Snap model to floor"

            topInset: 20
            topPadding: 20
            height: 70

            onPressed: {
                stlModel.snapToFloor()
            }

        }

        CheckBox {
            id: commonRotationCheckBox
            text: qsTr("Rotate together")
            checkState: Qt.Unchecked
        }

        CheckBox {
            id: mouseInvertCheckBox
            text: qsTr("Invert mouse drag")
            checkState: Qt.Checked
        }
    }
}