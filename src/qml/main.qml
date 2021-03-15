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
    id: window

    width: 1280
    height: 720
    visible: true
    color: "#848895"

    TypedFileDialog {
        id: fileDialog
    }

    MainMenu {
        id: mainMenu

        onModelCloseRequested: {
            stlModels.model = []
            gcodeModels.model = []
        }
    }

    View3D {
        id: view3d
        anchors.fill: parent
        camera: sceneBase.camera

        property var gcodeModel
        property vector3d gcodeModelCenter: Qt.vector3d(0, 0, 0)

        SceneBase {
            id: sceneBase
            sceneCenter: Qt.vector3d(100, 100, 0)
        }

        Ubot3DCameraWasdController {
            id: controller
            mouseEnabled: !modelGroupDrag.isActive
            controlledObject: sceneBase.camera
            camera: sceneBase.camera
            isMouseDragInverted: true //modelControls.mouseInvertCheckBox.checked
        }

        Repeater3D {
            id: stlModels

            delegate: StlModel {
                id: stlModel
                inputFile: stlModels.model[index]
            }
        }

        Repeater3D {
            id: gcodeModels
            property var gcodeGeometry: (objectAt(0) === null ? null : objectAt(0).geometry)
            model: []
            delegate: GCodeGeometryRepeaterModelDelegate {
                inputFile: gcodeModels.model[index]
            }
        }

        PickArea {
            id: pickArea
            camera: sceneBase.camera
        }

        ModelGroupDrag {
            id: modelGroupDrag
            connectedPickArea: pickArea
        }

        Connections {
            id: pickAreaToModelGroupDrag
            target: pickArea
            enabled: target
            function onPressed(mouse) {
                modelGroupDrag.start(mouse)
            }
            function onReleased(mouse) {
                modelGroupDrag.finish()
            }
            function onPositionChanged(mouse) {
                if(modelGroupDrag.isActive) {
                    modelGroupDrag.dragPositionChanged(sceneBase.camera.getOriginAndRay(mouse.x, mouse.y))
                    pickArea.isNextMouseClickDisabled = true
                }
            }
        }

        Component.onCompleted: {
            if (gcodeModel) {
                camera.lookAt(getModelCenter(codeModel))
                modelControls.resetSliders()
            }
            else {
                camera.lookAt(Qt.vector3d(50,50,0))
            }
        }
    }
}
