import QtQuick 2.15
import QtQml 2.15
import "HelperFunctions.js" as QmlHelpers

MouseArea {
    id: pickArea
    property bool isNextMouseClickDisabled: false
    property var postponedOnClickedActionData: undefined
    property var camera: undefined
    property point pressStartPosition
    property int pressMoveIgnoreDist: 1

    acceptedButtons: Qt.LeftButton | Qt.RightButton
    anchors.fill: view3d
    onDoubleClicked: {
        onDoubleClickedAction()
    }

    Timer {
        id: doubleClickTimer
        interval: 200
        repeat: false

        onTriggered: {
            if (postponedOnClickedActionData) {
                onClickedAction(postponedOnClickedActionData)
                postponedOnClickedActionData = undefined
            }
        }
    }

    onPressed: {
        if (doubleClickCountdown(mouse))
            return
    }

    onPositionChanged: {
        // If position changes more than a little, disable further click&double click
        if (Math.abs(mouse.x-pressStartPosition.x) > pressMoveIgnoreDist ||
            Math.abs(mouse.y-pressStartPosition.y) > pressMoveIgnoreDist) {
            isNextMouseClickDisabled = true
        }
    }

    onClicked: {
        if (doubleClickTimer.running) {
            postponedOnClickedActionData = mouse
            return
        }

        onClickedAction(mouse)
    }

    function onClickedAction(mouse) {
        if (isNextMouseClickDisabled) {
            isNextMouseClickDisabled = false
            return
        }

        if (mouse.button === Qt.LeftButton) {
            var pick = getPick(mouse)
            if (!pick.object && !doubleClickTimer.running)
            {
                QmlHelpers.deselectAll(stlObjects)
                return
            }

            pick.object.isPicked = !pick.object.isPicked
        }
    }

    function onDoubleClickedAction() {
        var model = QmlHelpers.getPickedModel(stlObjects)
        if (model) {
            camera.lookAt(QmlHelpers.getModelCenter(model))
        }
        else {
            camera.lookAtSceneCenter()
        }
    }

    function doubleClickCountdown(mouse) {
        if (doubleClickTimer.running) {
            doubleClickTimer.stop()
            postponedOnClickedActionData = undefined
            isNextMouseClickDisabled = true
            doubleClicked(mouse)
            return true
        }
        doubleClickTimer.start()
        return false
    }

    function getPick(mouse) {
        var ray = QmlHelpers.getRay(camera, mouse.x, mouse.y, view3d.width, view3d.height)
        var origin = camera.position
        var coords = undefined
        var dist = Infinity
        var object = undefined

        for (var i = 0; i < stlObjects.count; i++)
        {// Find out, if we are pressing an object.
            var stlModel = stlObjects.objectAt(i)

            var modelIntersection = stlModel.geometry.getPick(origin, ray, stlModel.sceneTransform)
            if (!modelIntersection.isHit)
                continue

            var distToHit = modelIntersection.intersection.minus(origin).length()
            if (distToHit >= dist)
                continue

            object = stlModel
            dist = distToHit
            coords = modelIntersection.intersection
        }

        if (!coords) {
            // If there is no hit, pick point on the ground ground
            var planeHit =
                    helper3D.getLinePlaneIntersection(origin, ray, Qt.vector3d(0,0,1), Qt.vector3d(0,0,0))

            if (planeHit.isHit) {
                coords = planeHit.intersection
            }
        }

        return {object: object, coords: coords}
    }
}
