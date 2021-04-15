import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick3D.Helpers 1.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick3D 1.15
import QtQuick.Controls 2.15
import customgeometry 1.0

Node {
    id: sceneBase
    property alias camera: camera
    property vector3d sceneCenter: Qt.vector3d(100, 100, 0)
    property vector3d gridSize: Qt.vector3d(100, 100, 1)
    property int gridLinesNum: 20

    Ubot3dCamera {
        id: camera
        position: Qt.vector3d(-50, -50, 100)
        clipFar: 500.0
        clipNear: 0.1
        fieldOfView: 45
        rotation: helper3D.getRotationFromDirection(Qt.vector3d(75, 75, -75), Qt.vector3d(0,0,1))
    }

    DirectionalLight {
        eulerRotation.x: 30
        eulerRotation.y: 30
        eulerRotation.z: 30
        color: Qt.rgba(0.7, 0.7, 0.7, 1.0)
        ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
    }

    DirectionalLight {
        eulerRotation.x: -45
        eulerRotation.y: -45
        eulerRotation.z: -45
        color: Qt.rgba(0.3, 0.3, 0.3, 1.0)
        ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
    }

    PointLight {
        position: sceneCenter.plus(Qt.vector3d(30, 0, 100))
        color: Qt.rgba(0.5, 0.5, 0.5, 1.0)
        ambientColor: Qt.rgba(0.2, 0.2, 0.2, 1.0)
    }

    Model {
        scale: gridSize
        position: sceneCenter
        geometry: GridGeometry {
            id: grid
            horizontalLines: gridLinesNum
            verticalLines: gridLinesNum
        }
        materials: [
            DefaultMaterial {
            }
        ]
    }
}
