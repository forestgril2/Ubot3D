.pragma library

function getModelCenterOffset(model) {
    return model.geometry.minBounds.plus(model.geometry.maxBounds).times(0.5)
}

function getModelCenter(model) {
    return model.position.plus(getModelCenterOffset(model))
}

function getRay(camera, x, y, width, height) {
    var origin = camera.position
    var pointAtScreen = Qt.vector3d(x/width, y/height, 0)
    var pointSceneTo = camera.mapFromViewport(pointAtScreen)
    return pointSceneTo.minus(origin).normalized()
}

function deselectAll(models) {
    for (var i = 0; i < models.count; i++)
    {
        models.objectAt(i).isPicked = false
    }
}

function getSelectedModelIndices(modelsListContainer) {
    var selectedIndices = []
    for (var i=0; i<modelsListContainer.count; i++)
    {
        var model = modelsListContainer.objectAt(i)
        if (!model.isPicked)
            continue
        selectedIndices.push(i)
    }
    return selectedIndices
}

function getSelectedModels(modelsListContainer) {
    var selectedModels = []
    var selectedModelIndices = getSelectedModelIndices(modelsListContainer)
    for (var i=0; i<selectedModelIndices.length; i++)
    {
        selectedModels.push(modelsListContainer.objectAt(selectedModelIndices[i]))
    }
    return selectedModels
}

function getPositions(objects) {
    var positions = []
    for (var i=0; i<objects.length; i++)
    {
        var object = objects[i]
        var pos = Qt.vector3d(object.position.x,
                              object.position.y,
                              object.position.z)
        positions.push(pos)
    }
    return positions
}

function getRotations(objects) {
    var rotations = []
    for (var i=0; i<objects.length; i++)
    {
        var object = objects[i]
        var rot = Qt.quaternion(object.rotation.scalar,
                                object.rotation.x,
                                object.rotation.y,
                                object.rotation.z)

        if (rot.scalar === 0) {
            // Handle ugly initial qml 3d object state.
            rot.scalar = 1;
        }
        rotations.push(rot)
    }
    return rotations
}

function getPickedModel(modelsListContainer) {
    var selectedModels = getSelectedModels(modelsListContainer)
    if (selectedModels.length !== 1)
        return undefined
    return selectedModels[0]
}
