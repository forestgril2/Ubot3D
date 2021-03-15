.pragma library

function getModelCenterOffset(model) {
    return model.geometry.minBounds.plus(model.geometry.maxBounds).times(0.5)
}

function getModelCenter(model) {
    return model.position.plus(getModelCenterOffset(model))
}
