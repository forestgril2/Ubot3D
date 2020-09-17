import QtQuick 2.12
import QtQuick.Window 2.12
import Qt.labs.platform 1.1
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.1

import Qt3D.Core 2.12

import QtQuick.Scene3D 2.12

import Qt3D.Core 2.12
import Qt3D.Render 2.12
import Qt3D.Input 2.12
import Qt3D.Extras 2.12

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: "Krice3D"
    header: mainMenu

    MenuBar {
        id: mainMenu
        menus: [
            Menu {
                title: qsTr("File")

                MenuItem {
                    text: qsTr("Open STL")

                    onPressed:
                    {
                        fileDialog.open()
                    }
                }
                MenuItem{
                    text: qsTr("Open GCode")
                }
                MenuItem {
                    text: qsTr("Recently Opened...")
                }
                MenuItem{
                    text: qsTr("Close file")
                }
                MenuSeparator {}
                MenuItem{
                    text: qsTr("Exit")
                }
            },
            Menu {
                title: qsTr("Tools")

                MenuItem {
                    text: qsTr("STL tools")
                }
                MenuItem{
                    text: qsTr("ToolsMenuItem 2")
                }
                MenuItem{
                    text: qsTr("ToolsMenuItem 3")
                }
            },
            Menu {
                title: qsTr("Slicer")

                MenuItem {
                    text: qsTr("Edit slicer options")
                }
                MenuItem{
                    text: qsTr("Generate GCode")
                }
                MenuItem{
                    text: qsTr("???")
                }
            },
            Menu {
                title: qsTr("Options")

                MenuItem {
                    text: qsTr("Support options")
                }
                MenuItem{
                    text: qsTr("Options item 2")
                }
                MenuItem{
                    text: qsTr("Options item 3")
                }
            },
            Menu {
                title: qsTr("Help")

                MenuItem {
                    text: qsTr("Documentation")
                }
                MenuItem{
                    text: qsTr("Licensing")
                }
                MenuSeparator {}
                MenuItem{
                    text: qsTr("About")
                }
            }
        ]
    }

    FileDialog {
        id: fileDialog
        onAccepted:
        {
            console.log(fileDialog.fileUrl)
            sceneLoader.source = fileDialog.fileUrl
        }
    }

    Scene3D {
        anchors.fill: parent

        aspects: ["input", "logic"]
        cameraAspectRatioMode: Scene3D.AutomaticAspectRatio

        Entity {
            id: sceneRoot

            Camera {
                id: camera
                projectionType: CameraLens.PerspectiveProjection
                fieldOfView: 30
                aspectRatio: 16/9
                nearPlane : 0.1
                farPlane : 1000.0
                position: Qt.vector3d( 45.0, 45.0, 45.0 )
                upVector: Qt.vector3d( 0.0, 1.0, 0.0 )
                viewCenter: Qt.vector3d( 0.0, 0.0, 0.0 )
            }

            OrbitCameraController {
                camera: camera
                linearSpeed: 333
                lookSpeed: 100
            }

            components: [
                RenderSettings {
                    activeFrameGraph: ForwardRenderer {
                        clearColor: Qt.rgba(0, 0.5, 1, 1)
                        camera: camera
                    }
                },
                InputSettings {
                }
            ]

            Entity
            {
                id: stlModel
                components: [
                    SceneLoader {
                        id: sceneLoader

                        source: "file:///C:/Users/Grzegorz Ilnicki/Desktop/JOB/Ubot3D/40_mm_ratchet_-_all_in_one_-_opened.stl"
                        onStatusChanged: {
                            if (status == SceneLoader.Ready)
                            {
                                console.log(stlModel.data)
                                console.log(stlModel.childNodes)
                                console.log("stlModel.propertyTrackingOverrides: " + stlModel.propertyTrackingOverrides)
                            }
                            else
                            {
                                console.log(status)
                            }
                        }
                    }
                ]
            }
        }
    }
}
