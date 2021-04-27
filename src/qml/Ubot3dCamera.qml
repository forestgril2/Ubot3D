import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick3D.Helpers 1.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick3D 1.15
import QtQuick.Controls 2.15
import customgeometry 1.0

PerspectiveCamera {
	id: camera
	
    property vector3d initDistToModel: Qt.vector3d(-50, -50, 100)

	function lookAt(point)
	{
		var direction = point.minus(camera.position)
		var upDirection = Qt.vector3d(0,0,1)
		var lookAtRotation = helpers3D.getRotationFromDirection(direction, upDirection)
		camera.rotation = lookAtRotation
        setUpPlane(Qt.vector3d(0,0,1))
	}

    function lookAtSceneCenter()
    {
        lookAt(sceneCenter)
    }
	
	function setUpPlane(upVector) {
		var newPointSceneTo = mapFromViewport(Qt.vector3d(0.5, 0.5, 0))
		var newDirection = newPointSceneTo.minus(position)
		setRotation(helpers3D.getRotationFromDirection(newDirection, upVector))
	}
}
