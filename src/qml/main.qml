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

        onModelCloseRequested: {
            stlObjects.model = []
            gCodeObjects.model = []
        }
    }

    TypedFileDialog {
        id: fileDialog
    }

    SlicerParametersInput {
        id: slicerParameters
    }

    View3D {
        id: view3d
        anchors.fill: parent
        camera: sceneBase.camera

        property var gCodeModel
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
            id: stlObjects

            signal delegateLoaded(var modelCenter)

            model: []

            delegate: StlModel {
                id: stlModel
                inputFile: stlObjects.model[index]

                Component.onCompleted: {
                   stlObjects.delegateLoaded(stlModel.modelCenter)
                }
            }

            onDelegateLoaded: {
                sceneBase.camera.position = modelCenter.plus(sceneBase.camera.initDistToModel)
                sceneBase.camera.lookAt(modelCenter)
            }
        }

        Repeater3D {
            id: gCodeObjects
            property var gcodeGeometry: (objectAt(0) === null ? null : objectAt(0).geometry)

            signal delegateLoaded(var modelCenter)

            model: []
            delegate: GCodeGeometryRepeaterModelDelegate {
                id: gCodeModel
                inputFile: gCodeObjects.model[index]

                Component.onCompleted: {
                    gCodeObjects.delegateLoaded(gCodeModel.modelCenter)
                }
            }

             onDelegateLoaded: {
                sceneBase.camera.position = modelCenter.plus(sceneBase.camera.initDistToModel)
                sceneBase.camera.lookAt(modelCenter)
            }
        }

        Repeater3D {
            id: overhangingTriangles
            model: stlObjects.model.length
            delegate: Model {
                visible: true
                scale: Qt.vector3d(1, 1, 1)
                geometry: SimplexGeometry {
                    simplexType: SimplexGeometry.Lines
                    points: stlObjects.objectAt(index).geometry.overhangingTriangleVertices
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
        }

        Repeater3D {
            id: stlSupportGeometries
            model: stlObjects.count>0 ? stlObjects.objectAt(0).geometry.supportGeometries : undefined
            delegate: StlModel {
                geometry: stlObjects.objectAt(0).geometry.supportGeometries[index]
            }

            onModelChanged: {
                console.log(" ### new model with size:" + model.length)
            }
        }

//        Repeater3D {
//            id: triangulationResult
//            model: stlObjects.model.length
//            delegate: Model {
//                visible: true
//                scale: Qt.vector3d(1, 1, 1)
//                geometry: SimplexGeometry {
//                    simplexType: SimplexGeometry.Lines
//                    points: stlObjects.objectAt(index).geometry.triangulationResult
//                }
//                materials: [
//                    DefaultMaterial {
//                        pointSize: 5
//                        lineWidth: 5
//                        lighting: DefaultMaterial.NoLighting
//                        cullMode: DefaultMaterial.NoCulling
//                        diffuseColor: "red"
//                    }
//                ]
//            }
//        }

//        Repeater3D {
//            id: allModelsIslands
//            model: stlObjects.model.length

//            delegate: Repeater3D {
//                id: triangleIslands
//                model: stlObjects.objectAt(index).geometry.triangleIslands
//                delegate: Model {
//                    visible: true
//                    scale: Qt.vector3d(1, 1, 1)
//                    geometry: SimplexGeometry {
//                        simplexType: SimplexGeometry.Lines
//                        points: triangleIslands.model[index]
//                    }
//                    materials: [
//                        DefaultMaterial {
//                            pointSize: 5
//                            lineWidth: 5
//                            lighting: DefaultMaterial.NoLighting
//                            cullMode: DefaultMaterial.NoCulling
//                            diffuseColor: "red"
//                        }
//                    ]
//                }
//            }
//        }

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
                console.log(" ### onSlicerError, slicerStdOutput: " + slicerStdOutput)
            }

            onGcodeGenerated: {
                mainMenu.modelCloseRequested();
                gCodeObjects.model = [outputFilePath]
            }

            function sliceSelectedModel() {
                var models = QmlHelpers.getSelected(stlObjects)

                if (models.length === 0) {
                    console.log(" ### no models selected")
                    return
                }

                if (models.length > 1) {
                    console.log(" ### select only one model")
                    return
                }

                console.log(" ### model.path :" + models[0].geometry.inputFile)

                var isTwoHeaderExtrusion = slicerParameters.isUsingTwoExtruders
                generateGCode(models[0].geometry.inputFile, isTwoHeaderExtrusion)
            }
        }
    }
}
