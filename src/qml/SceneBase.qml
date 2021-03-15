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

    Ubot3dCamera {
        id: camera
        clipFar: 500.0
        clipNear: 0.1
        fieldOfView: 45
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
        color: Qt.rgba(0.1, 1.0, 0.1, 1.0)
        ambientColor: Qt.rgba(0.2, 0.2, 0.2, 1.0)
    }

    Model {
        scale: Qt.vector3d(100, 100, 100)
        position: sceneCenter
        geometry: GridGeometry {
            id: grid
            horizontalLines: 20
            verticalLines: 20
        }
        materials: [
            DefaultMaterial {
            }
        ]
    }
}
