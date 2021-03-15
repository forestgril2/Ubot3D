import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick3D.Helpers 1.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick3D 1.15
import QtQuick.Controls 2.15
import customgeometry 1.0

Model {
    property alias inputFile: gcodeGeometry.inputFile
    property bool isPicked: false
	position: Qt.vector3d(0, 0, 0)
	objectName: "gCode geometry"
	pickable: true
    rotation: Qt.quaternion(0,0,0,0)//modelControls.commonRotationCheckBox.checked ?
                                    //    helper3D.getRotationFromAxisAndAngle(Qt.vector3d(0,0,1), modelControls.pointModelRotationSlider.value) :
                                    //    Qt.quaternion(0,0,0,0)
	geometry: GCodeGeometry {
		id: gcodeGeometry
        inputFile: parent.model[index]
		
//		onModelLoaded: {
//			modelControls.resetSliders(gcodeGeometry)
//		}
	}
	materials: [
		DefaultMaterial {
			cullMode: DefaultMaterial.NoCulling
			diffuseColor: "lightgreen"
			specularAmount: 0.5
		}
	]
}
