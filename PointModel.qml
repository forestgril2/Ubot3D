import QtQuick 2.15
import QtQuick3D 1.15
import customgeometry 1.0

Model {
    id: pointModel
    property alias geometry: pointModel.geometry
    objectName: "STL points"
    pickable: true
    scale: Qt.vector3d(1, 1, 1)
    rotation: triangleModel.geometry.getRotationFromAxisAndAngle(Qt.vector3d(0,0,1), modelControls.pointModelRotationSlider.value)
    geometry: ExamplePointGeometry {}
    position: Qt.vector3d(modelControls.posXSlider.value, 0, 0)
    materials: [
        DefaultMaterial {
            lighting: DefaultMaterial.NoLighting
            cullMode: DefaultMaterial.NoCulling
            diffuseColor: "yellow"
            //                    pointSize: sliderPointSize.value
        }
    ]

    Connections {
        target: triangleModel.geometry
        function onModelLoaded() {
            pointModel.geometry.updateData()
            pointModel.geometry.update()
        }
    }
}
