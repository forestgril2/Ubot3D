import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick3D.Helpers 1.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick3D 1.15
import QtQuick.Controls 2.15
import customgeometry 1.0

Window {
    id: window

    property vector3d sceneCenter: Qt.vector3d(100, 100, 0)

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
        }

        Ubot3DCameraWasdController {
            id: controller
            mouseEnabled: !modelGroupDrag.isActive
            controlledObject: sceneBase.camera
            camera: sceneBase.camera
            isMouseDragInverted: modelControls.mouseInvertCheckBox.checked
        }

        Repeater3D {
            id: stlModels

            delegate: StlModel {
                id: stlModel
                inputFile: stlModels.model[index]
            }

            function deselectAll() {
                for (var i = 0; i < stlModels.count; i++)
                {// Find out, if we are pressing an object.
                    var stlModel = stlModels.objectAt(i)
                    stlModel.isPicked = false
                }
            }
        }

        Repeater3D {
            id: gcodeModels
            property var gcodeGeometry: (objectAt(0) === null ? null : objectAt(0).geometry)
            model: []
            delegate: GCodeGeometryRepeaterModelDelegate { }
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
                    modelGroupDrag.dragPositionChanged(pickArea.getOriginAndRay(mouse.x, mouse.y))
                    pickArea.isNextMouseClickDisabled = true
                }
            }
        }

        ModelControls {
            id: modelControls
        }

        Component.onCompleted: {
            if (gcodeModel) {
                camera.lookAtModel(gcodeModel)
                modelControls.resetSliders()
            }
            else {
                camera.lookAt(Qt.vector3d(50,50,0))
            }
        }
    }
}
