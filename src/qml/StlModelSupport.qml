import QtQuick3D
import customgeometry 1.0

Model {
    id: stlModel
    property var mainModel

    property bool isPicked: false
    property vector3d modelCenter
    pickable: true

    position: mainModel.position
    rotation: mainModel.rotation

    materials: [
        DefaultMaterial {
            cullMode: DefaultMaterial.NoCulling
            diffuseColor: stlModel.isPicked ? "cyan" : "lightgrey"
            specularAmount: 0.5
        }
    ]
}
