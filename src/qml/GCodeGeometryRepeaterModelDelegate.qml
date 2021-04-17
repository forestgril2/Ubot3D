import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick3D.Helpers 1.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick3D 1.15
import QtQuick.Controls 2.15
import customgeometry 1.0

Node {
    id: gCodeModel

    property alias inputFile: gcodeGeometry.inputFile

    property bool isPicked: false
    property bool pickable: false
    property var subGeometryColors: ["lightgreen", "yellow"]


    position: Qt.vector3d(0, 0, 0)
    objectName: "GCode model"
    rotation: Qt.quaternion(0,0,0,0)

    GCodeGeometry
    {//GCodeGeometry exposes a list of sub-geometries for each extruder
        id: gcodeGeometry
        inputFile: parent.model[index]

        //		onModelLoaded: {
        //			modelControls.resetSliders(gcodeGeometry)
        //		}
    }


    Repeater3D
    {
        id: subGeometries
        model: gcodeGeometry.subGeometries

        delegate: Model {
            pickable: gCodeModel.pickable
            geometry: subGeometries.model[index]
            materials: [
                DefaultMaterial {
                    cullMode: DefaultMaterial.NoCulling
                    diffuseColor: subGeometryColors[index]
                    specularAmount: 0.5
                }
            ]
        }

        onModelChanged: {
            console.log(" ### gCodeGeometry with subgeometries: " + model.length)
        }
    }
}
