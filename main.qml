//import QtQuick.Dialogs 1.3

import Qt.labs.platform 1.1

import Qt3D.Core 2.15
import QtQuick3D.Helpers 1.15

//import QtQuick.Scene3D 2.15

import Qt3D.Render 2.15
//import Qt3D.Input 2.15
import Qt3D.Extras 2.15
import QtQuick 2.15
import QtQuick.Window 2.15
//import QtQuick.Dialogs 1.2
import QtQuick3D 1.15
import QtQuick.Controls 2.15
import customgeometry 1.0

Window {
    id: window
    width: 1280
    height: 720
    visible: true
    color: "#848895"

    FileDialog {
        id: fileDialog
        onAccepted:
        {
            console.log(fileDialog.currentFile)
            var fullSystemFilePath = fileDialog.currentFile.toString()
            console.log(fullSystemFilePath)
            var pos = fullSystemFilePath.search("file:///");
            console.log(pos)
            fullSystemFilePath = fullSystemFilePath.substring(pos+8)
            console.log(fullSystemFilePath)
            triangleModel.geometry.inputFile = fullSystemFilePath
        }
    }

    MainMenu {

    }

    View3D {
        id: v3d
        anchors.fill: parent
        camera: camera

         PerspectiveCamera {
            id: camera
            fieldOfView: 45
            clipNear: 0.1
            clipFar: 10000.0
            x: 0
            y: -75
            z: 50

            onRotationChanged: {
                console.log(" camera eulerRotation: " + eulerRotation)
            }
        }

        DirectionalLight {
            position: Qt.vector3d(-500, 500, -100)
            color: Qt.rgba(0.4, 0.2, 0.6, 1.0)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        PointLight {
            position: Qt.vector3d(0, 0, 100)
            color: Qt.rgba(0.1, 1.0, 0.1, 1.0)
            ambientColor: Qt.rgba(0.2, 0.2, 0.2, 1.0)
        }

        Model {
            visible: radioGridGeom.checked
            scale: Qt.vector3d(100, 100, 100)
            geometry: GridGeometry {
                id: grid
                horizontalLines: 20
                verticalLines: 20
            }
            materials: [
                DefaultMaterial {
                    lineWidth: sliderLineWidth.value
                }
            ]
        }

        Model {
            id: triangleModel
            property alias geometry: triangleModel.geometry
            visible: radioCustGeom.checked
            scale: Qt.vector3d(1, 1, 1)
            geometry: ExampleTriangleGeometry {
                normals: cbNorm.checked
                normalXY: sliderNorm.value
                uv: cbUV.checked
                uvAdjust: sliderUV.value
                warp: modelWarpSlider.value

                onBoundsChanged: {
                    console.log(" model bounds min: " + triangleModel.geometry.minBounds)
                    console.log(" model bounds max: " + triangleModel.geometry.maxBounds)
                    console.log(" modelCenter : " + modelCenter)
                }
            }

            materials: [
                DefaultMaterial {
                    Texture {
                        id: baseColorMap
                        source: "qt_logo_rect.png"
                    }
                    cullMode: DefaultMaterial.NoCulling
                    diffuseMap: cbTexture.checked ? baseColorMap : null
                    specularAmount: 0.5
                }
            ]
        }

        Model {
            visible: radioPointGeom.checked
            scale: Qt.vector3d(100, 100, 100)
            geometry: ExamplePointGeometry {
            }
            materials: [
                DefaultMaterial {
                    lighting: DefaultMaterial.NoLighting
                    cullMode: DefaultMaterial.NoCulling
                    diffuseColor: "yellow"
//                    pointSize: sliderPointSize.value
                }
            ]
        }
    }

    WasdController {
        controlledObject: camera
    }


    MouseArea {
        anchors.fill: parent
        onDoubleClicked: {
            var modelCenter = triangleModel.geometry.minBounds.plus(triangleModel.geometry.maxBounds).times(0.5)
            camera.lookAt(modelCenter)

            //               var eulerRotation = camera.eulerRotation
            //                camera.setEulerRotation(Qt.vector3d(eulerRotation.x,0,eulerRotation.z))

            var dir = (modelCenter.minus(camera.position)).normalized()
            var up = Qt.vector3d(0,0,1)
            var hor = (up.crossProduct(dir)).normalized()
            up = (dir.crossProduct(hor)).normalized()

            // hor.x hor.y hor.z
            // up.x  up.y  up.z
            // dir.x dir.y dir.z

            var qw = Math.sqrt(1+hor.x + up.y + dir.z)/2
            var qx = (up.z - dir.y)/(4 * qw)
            var qy = (dir.x - hor.z)/(4 * qw)
            var qz = (hor.y - up.x)/(4 * qw)

//            console.log("camera.rotation: " + camera.rotation);

            camera.rotation = Qt.quaternion(qw,qx,qy,qz)
        }
    }

    Slider {
        id: modelWarpSlider
        anchors.left: parent
        orientation: Qt.Vertical
        from: -0.1
        to: 0.1
        width: 50
    }
}
