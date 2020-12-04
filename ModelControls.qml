import QtQuick 2.15
import QtQuick.Controls 2.15
Row {
    property alias numSubpathsSlider: numSubpathsSlider
    property alias numPointsInSubpathSlider: numPointsInSubpathSlider

    property alias mouseInvertCheckBox: mouseInvertCheckBox
    property alias commonRotationCheckBox: commonRotationCheckBox
    property alias triangleModelWarpSlider: triangleModelWarpSlider

    property int numSubpaths: 1
    property int numPointsInSubpaths: 1

    Slider {
        id: triangleModelWarpSlider
        orientation: Qt.Vertical
        from: -0.1
        to: 0.1
        width: 50
    }

    Column {
        anchors {
            top: parent.top
            bottom: parent.bottom
        }

        TextEdit {
            text: Math.round(numSubpathsSlider.value)
        }

        Slider {
            id: numSubpathsSlider
            orientation: Qt.Vertical
            from: 0
            to: numSubpaths
            width: 50
        }
    }

    Column {
        anchors {
            top: parent.top
            bottom: parent.bottom
        }

        TextEdit {
            text: Math.round(numPointsInSubpathSlider.value)
        }

        Slider {
            id: numPointsInSubpathSlider
            orientation: Qt.Vertical
            from: 0
            to: numPointsInSubpaths
            width: 50
        }
    }

    Column {

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
            text: qsTr("
            Invert mouse drag")
            checkState: Qt.Checked
        }
    }
}
