import QtQuick 2.15
import QtQuick3D 1.15

Node {
    id: node091_W_Aya_100K_obj

    Model {
        id: node091_W_Aya_100K_01
        objectName: "JapanGirl"
        source: "meshes/node091_W_Aya_100K_01.mesh"
        pickable: true
        property bool isPicked: false

        DefaultMaterial {
            id: node091_W_Aya_2K_01_material
            diffuseColor: "#ff999999"
        }
        materials: [
            node091_W_Aya_2K_01_material
        ]
    }
}
