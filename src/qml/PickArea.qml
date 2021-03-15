import QtQuick 2.15
import QtQml 2.15


MouseArea {
    id: pickArea
    property bool isNextMouseClickDisabled: false

    anchors.fill: view3d
    onDoubleClicked: {
        gcodeModel.position = Qt.vector3d(0,0,0).minus(getModelCenter(gcodeModel))
        camera.lookAtModel(gcodeModel)
    }

    Timer {
        id: doubleClickTimer
        interval: 200
        repeat: false
    }

    onClicked: {
        if (doubleClickTimer.running) {
            doubleClickTimer.stop()
            doubleClicked(mouse)
            return
        }

        doubleClickTimer.start()

        var closestPick = getClosestPick(mouse)
        if (!closestPick.model)
        {
            stlModels.deselectAll()
            return
        }

        if (isNextMouseClickDisabled) {
            isNextMouseClickDisabled = false
            return
        }

        closestPick.model.isPicked = !closestPick.model.isPicked
    }

    function getOriginAndRay(x,y) {
        var origin = camera.position
        var pointAtScreen = Qt.vector3d(x/view3d.width, y/view3d.height, 0)
        var pointSceneTo = camera.mapFromViewport(pointAtScreen)
        var ray = pointSceneTo.minus(origin).normalized()
        return {origin, ray}
    }

    function getClosestPick(mouse) {
        var originAndRay = getOriginAndRay(mouse.x, mouse.y)
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
