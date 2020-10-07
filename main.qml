//import QtQuick 2.15
//import QtQuick.Window 2.15
//import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.1

import Qt3D.Core 2.15
//import QtQuick3D 1.15
//import QtQuick3D.Helpers 1.15

//import QtQuick.Scene3D 2.15

import Qt3D.Render 2.15
//import Qt3D.Input 2.15
import Qt3D.Extras 2.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick3D 1.15
import QtQuick.Controls 2.15

Window {
    visible: true
    width: 640
    height: 480
    title: "Krice3D"

    MainMenu {
        id: mainMenu
    }

    Node {
        id: scene3DInstance

        DirectionalLight {
            ambientColor: Qt.rgba(1.0, 1.0, 1.0, 1.0)
        }

        Model {
            id: model
            source: "#Sphere"
//            source: "file:///C:/ProjectsData/stl_files/qml/meshes/node091_W_Aya_100K_01.mesh"

            materials: [material]
        }

        Node {
            id: node091_W_Aya_100K_obj


            Model {
                id: node091_W_Aya_100K_01
                source: "#Sphere"
//                source: "file:///C:/ProjectsData/stl_files/qml/meshes/node091_W_Aya_100K_01.mesh"

                DefaultMaterial {
                    id: node091_W_Aya_2K_01_material
                    diffuseColor: "#ff999999"
                }
                materials: [
                    node091_W_Aya_2K_01_material
                ]
            }
        }

        PhongMaterial {
            id: material
        }

        Transform {
            id: modelTransform
        }

        Mesh {
            id: modelMesh
            source: "file:///C:/Users/Grzegorz Ilnicki/Desktop/JOB/Ubot3D/40_mm_ratchet_-_all_in_one_-_opened.stl"
            //source: "file:///C:/ProjectsData/stl_files/091_W_Aya_100K.obj"
        }

        Entity {
            id: stlModel
            components: [modelMesh, material, modelTransform]
        }

        Node {
            OrthographicCamera {
                id: camera
                clipNear:  0.1
                clipFar: 1000.0
//                position: Qt.vector3d( 45.0, 45.0, 45.0 )
            }
        }
    }

    // The views
    Rectangle {
        id: cover
        anchors.top: mainMenu.bottom
        anchors {
            left: parent.left
            bottom: parent.bottom
            right: parent.right
        }

        color: "#848895"
        border.color: "black"

        View3D {
            id: mainView
            anchors.fill: parent
            importScene: scene3DInstance
            camera: camera
        }

        Label {
            text: "Main View"
            anchors.fill: parent
            anchors.margins: 10
            color: "#222840"
            font.pointSize: 14
        }

        MouseArea {
            enabled: true
            anchors.fill: parent

            onClicked: {
                console.log("CLICKED, model: " + node091_W_Aya_100K_obj.model.bounds)
                camera.lookAt(node091_W_Aya_100K_obj.position)
            }
        }
    }

    Mesh {

    }

//    OrbitCameraController {
//        camera: ortographicCamera
//    }

//    PhongMaterial {
//        id: material
//    }

    FileDialog {
        id: fileDialog
        onAccepted:
        {
            console.log(fileDialog.fileUrl)
            //            sceneLoader.source = fileDialog.fileUrl
            scene3DInstance.modelMesh.source = fileDialog.fileUrl
        }
    }

//    Rectangle {
//        id: scene
//        anchors.fill: parent
//        anchors.margins: 50
//        color: "darkRed"



//        transform: Rotation {
//            id: sceneRotation
//            axis.x: 1
//            axis.y: 0
//            axis.z: 0
//            origin.x: scene.width / 2
//            origin.y: scene.height / 2
//        }
//    }
}
