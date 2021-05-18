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

    MainMenu {
        id: mainMenu

        onClearSceneRequested: {
            stlObjectsRepeater.model = []
            gCodeObjectsRepeater.model = []
        }
    }

    TypedFileDialog {
        id: fileDialog
    }

    SlicerParametersInput {
        id: slicerParameters
    }

    SupportOptions {
        id: supportOptions
        supportVertices: view3d.selectedObjects[0]
    }

    RaftOptions {
        id: raftOptions
//        floorVertices: view3d.selectedObjects[0]
    }

    Popup {
        id: popup
        property alias messageText: message.text
        property alias messageColor: message.color
        property alias backgroundColor: backgroundRect.color

        modal: true
        background: Rectangle {
            id: backgroundRect
            anchors.fill: parent
        }

        anchors.centerIn: parent

        contentItem: Text {
            id: message
            text: "Error"
            color: "red"
            font.bold: true
            font.pointSize: 16
        }
    }

    View3D {
        id: view3d
        anchors.fill: parent
        camera: sceneBase.camera

        property var selectedObjects: []
        property var gCodeModel
        property vector3d gcodeModelCenter: Qt.vector3d(0, 0, 0)

        signal pickedModelsChanged()
        onPickedModelsChanged: {
            selectedObjects = QmlHelpers.getSelectedModels(stlObjectsRepeater)
            console.log(" ### selectedObjects.length:" + selectedObjects.length)
        }

        SceneBase {
            id: sceneBase
            sceneCenter: Qt.vector3d(100, 100, 0)
        }

//        Keys.onPressed: {
//            if (event.key === Qt.Key_Delete) {
//                var allModels = stlObjectsRepeater.model
//                var selectedStlIndices = QmlHelpers.getSelectedModelIndices(stlObjectsRepeater)
//                for (var i=0; i<selectedStlIndices.length; i++) {
//                    const index = selectedStlIndices[i]
//                    allModels.splice(index, 1);
//                }

//                for (var i=0; i<allModels.length; i++) {
//                    console.log(allModels[i])
//                }

//                stlObjectsRepeater.model = allModels

//                event.accepted = true;
//            }
//            if (event.modifiers & Qt.ControlModifier) {
//                var selectedModels = QmlHelpers.getSelectedModels(stlObjectsRepeater)
//                if (selectedModels.length === 0) {
//                    event.accepted = true;
//                    return
//                }

//                switch (event.key)
//                {
//                    case Qt.Key_A:
//                        rotateModelLeft()
//                        break;
//                    case Qt.Key_W:
//                        rotateModelForward()
//                        break;
//                    case Qt.Key_S:
//                        rotateModelBackwards()
//                        break;
//                    case Qt.Key_D:
//                        rotateModelRight()
//                        break;
//                    default:
//                        break;
//                }
//                event.accepted = true;
//            }
//        }


        Ubot3DCameraWasdController {
            id: controller
            mouseEnabled: !modelGroupDrag.isActive
            controlledObject: view3d.selectedObjects.length === 1 ? view3d.selectedObjects[0] : sceneBase.camera
            camera: sceneBase.camera
            isMouseDragInverted: true //modelControls.mouseInvertCheckBox.checked
        }

        Repeater3D {
            id: stlObjectsRepeater

            signal delegateLoaded(var modelCenter)

            model: []

            delegate: StlModel {
                id: stlModel
                isSupportGenerated: supportOptions.isGeneratingSupport
                supportAlphaValue: supportOptions.alphaValue
                inputFile: stlObjectsRepeater.model[index]

                Component.onCompleted: {
                   stlObjectsRepeater.delegateLoaded(stlModel.modelCenter)
                }

                onIsPickedChanged: {
                    view3d.pickedModelsChanged()
                }
            }

            onDelegateLoaded: {
                sceneBase.camera.position = modelCenter.plus(sceneBase.camera.initDistToModel)
                sceneBase.camera.lookAt(modelCenter)
            }
        }

        Repeater3D {
            id: gCodeObjectsRepeater
            property var gcodeGeometry: (objectAt(0) === null ? null : objectAt(0).geometry)

            signal delegateLoaded(var modelCenter)

            model: []
            delegate: GCodeGeometryRepeaterModelDelegate {
                id: gCodeModel
                inputFile: gCodeObjectsRepeater.model[index]

                Component.onCompleted: {
                    gCodeObjectsRepeater.delegateLoaded(gCodeModel.modelCenter)
                }
            }

            onDelegateLoaded: {
                sceneBase.camera.position = modelCenter.plus(sceneBase.camera.initDistToModel)
                sceneBase.camera.lookAt(modelCenter)
            }
        }

        Repeater3D {
            id: overhangingTriangles
            model: stlObjectsRepeater.model.length
            delegate: Model {
                visible: true
                scale: Qt.vector3d(1, 1, 1)
                geometry: SimplexGeometry {
                    simplexType: SimplexGeometry.Lines
                    points: stlObjectsRepeater.objectAt(index).geometry.overhangingTriangleVertices
                }
                materials: [
                    DefaultMaterial {
                        pointSize: 5
                        lineWidth: 5
                        lighting: DefaultMaterial.NoLighting
                        cullMode: DefaultMaterial.NoCulling
                        diffuseColor: "red"
                    }
                ]
            }
        }

        Repeater3D {
            id: stlSupportGeometries
            model: (supportOptions.isGeneratingSupport && stlObjectsRepeater.count > 0) ? stlObjectsRepeater.objectAt(0).geometry.supportGeometries : []
            delegate: StlModel {
                geometry: stlObjectsRepeater.objectAt(0).geometry.supportGeometries[index]
                position: stlObjectsRepeater.objectAt(0).position
                rotation: stlObjectsRepeater.objectAt(0).rotation
            }

            onModelChanged: {
                console.log(" ### new model with size:" + model.length)
                console.log(" ### supportOptions.isGeneratingSupport:" + supportOptions.isGeneratingSupport)
            }
        }

//        Repeater3D {
//            id: triangulationResult
//            model: stlObjectsRepeater.model.length
//            delegate: Model {
//                visible: true
//                scale: Qt.vector3d(1, 1, 1)
//                geometry: SimplexGeometry {
//                    simplexType: SimplexGeometry.Lines
//                    points: stlObjectsRepeater.objectAt(index).geometry.triangulationResult
//                }
//                materials: [
//                    DefaultMaterial {
//                        pointSize: 5
//                        lineWidth: 5
//                        lighting: DefaultMaterial.NoLighting
//                        cullMode: DefaultMaterial.NoCulling
//                        diffuseColor: "yellow"
//                    }
//                ]
//            }
//        }

        Repeater3D {
            id: allModelsAlphaShapes
            model: stlObjectsRepeater.model.length

            delegate: Repeater3D {
                id: triangleIslandBoundaries
                model: stlObjectsRepeater.objectAt(index).geometry.alphaShapes
                delegate: Model {
                    visible: true
                    scale: Qt.vector3d(1, 1, 1)
                    geometry: SimplexGeometry {
                        simplexType: SimplexGeometry.Lines
                        points: triangleIslandBoundaries.model[index]
                    }
                    materials: [
                        DefaultMaterial {
                            pointSize: 5
                            lineWidth: 5
                            lighting: DefaultMaterial.NoLighting
                            cullMode: DefaultMaterial.NoCulling
                            diffuseColor: "yellow"
                        }
                    ]
                }
                onModelChanged: console.log(" ### triangleIslandBoundaries:" + model.length)
            }

            onModelChanged: console.log(" ### allModels triangleIslandBoundaries:" + model.length)
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
                    var ray = QmlHelpers.getRay(sceneBase.camera, mouse.x, mouse.y, view3d.width, view3d.height);
                    modelGroupDrag.dragPositionChanged(sceneBase.camera.position, ray)
                }
            }
        }

        ProcessLauncher {
            id: slicerProcessLauncher

            onSlicerError: {
                popup.backgroundColor = "white"
                popup.messageColor = "red"
                popup.messageText = "ERROR: Slicer process failed. Slicer output:\n" + slicerStdOutput
                popup.open()
            }

            onGcodeGenerated: {
                popup.backgroundColor = "lime"
                popup.messageColor = "black"
                popup.messageText = "Slicer process successful."
                popup.open()

                mainMenu.clearSceneRequested();
                gCodeObjectsRepeater.model = [outputFilePath]
            }

            function sliceSelectedModel() {
                var selectedModels = QmlHelpers.getSelectedModels(stlObjectsRepeater)

                if (selectedModels.length === 0) {
                    popup.backgroundColor = "yellow"
                    popup.messageColor = "black"
                    popup.messageText = "No models selected. Please select a model to slice."
                    popup.open()
                    return
                }

                if (selectedModels.length > 1) {
                    popup.backgroundColor = "yellow"
                    popup.messageColor = "text"
                    popup.messageText = "Multiple models selected. Please select only one model to slice."
                    popup.open()
                    return
                }

                var isTwoHeaderExtrusion = slicerParameters.isUsingTwoExtruders
                generateGCode(selectedModels[0].geometry.inputFile, isTwoHeaderExtrusion)
            }
        }
    }
}
