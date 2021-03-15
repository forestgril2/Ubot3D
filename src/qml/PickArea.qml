import QtQuick 2.15
import QtQml 2.15
import "HelperFunctions.js" as QmlHelpers

MouseArea {
    id: pickArea
    property bool isNextMouseClickDisabled: false
    property var postponedOnClickedActionData: undefined
    property var camera

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

    onClicked: {
        if (doubleClickTimer.running) {
            postponedOnClickedActionData = mouse
            return
        }

        onClickedAction(mouse)
    }

    function onClickedAction(mouse) {
        var closestPick = getClosestPick(mouse)
        if (!closestPick.model && !doubleClickTimer.running)
        {
            QmlHelpers.deselectAll(stlModels)
            return
        }

        if (isNextMouseClickDisabled) {
            isNextMouseClickDisabled = false
            return
        }

        closestPick.model.isPicked = !closestPick.model.isPicked
    }

    function onDoubleClickedAction() {
        var model = QmlHelpers.getPickedModel(stlModels)
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
            postponedOnClickedActionData = false
            doubleClicked(mouse)
            return true
        }
        doubleClickTimer.start()
        return false
    }

    function getClosestPick(mouse) {
        var originAndRay = camera.getOriginAndRay(mouse.x, mouse.y)
        var coords = undefined
        var dist = Infinity
        var model = undefined

        for (var i = 0; i < stlModels.count; i++)
        {// Find out, if we are pressing an object.
            var stlModel = stlModels.objectAt(i)

            var modelIntersection = stlModel.geometry.getPick(originAndRay.origin, originAndRay.ray, stlModel.sceneTransform)
            if (!modelIntersection.isHit)
                continue

            var distToHit = modelIntersection.intersection.minus(originAndRay.origin).length()
            if (distToHit >= dist)
                continue

            model = stlModel
            dist = distToHit
            coords = modelIntersection.intersection
        }

        return {model: model, coords: coords}
    }
}
