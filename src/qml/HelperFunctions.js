.pragma library

function getModelCenterOffset(model) {
    return model.geometry.minBounds.plus(model.geometry.maxBounds).times(0.5)
}

function getModelCenter(model) {
    return model.position.plus(getModelCenterOffset(model))
}

function deselectAll(models) {
    for (var i = 0; i < models.count; i++)
    {
        models.objectAt(i).isPicked = false
    }
}

function getPickedModel(models) {
    var pickedModels = []
    for (var i=0; i<models.count; i++)
    {// Find out, which objects are selected
        var stlModel = models.objectAt(i)
        if (stlModel.isPicked) {
            pickedModels.push(stlModel)
        }
    }
    if (pickedModels.length != 1)
        return undefined

    return pickedModels[0]
}
