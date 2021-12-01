import Qt.labs.platform 1.1
import QtQuick.Controls 2.15
import QtQuick 2.15
import "HelperFunctions.js" as QmlHelpers

FileDialog {
    id: root

    // TODO: This doesn't belong here.
    property string slicerExec: utils.getDefaultSlicerExecutablePath()

    enum FileType {
        GcodeImport,
        StlImport,
        StlExport,
        JsonImport,
        JsonExport,
        SlicerExec
    }

    property int fileType: TypedFileDialog.StlImport

    onAccepted:
    {
        function generateSystemFilePath(qtFilePath) {
            var fullSystemFilePath = qtFilePath.toString()
            return utils.generateSystemFilePath(fullSystemFilePath)
        }

        function generateSystemFilepathList(qtFilePathList) {
            var retList = []
            qtFilePathList.forEach(function (qtFilePath, index) {
                retList.push(generateSystemFilePath(qtFilePath))
            })
            return retList
        }

        switch(fileType) {
            case TypedFileDialog.StlImport:
                console.log(" ### TypedFileDialog.StlImport: " + fileDialog.currentFiles)
                stlObjectsRepeater.model = generateSystemFilepathList(fileDialog.currentFiles)
                break
            case TypedFileDialog.StlExport:
                var exportedModels = [stlObjectsRepeater.objectAt(0)]
                if (stlObjectsRepeater.count > 1) {
                    exportedModels = QmlHelpers.getSelectedModels(stlObjectsRepeater)
                }
                fileImportExport.exportModelsToSTL(prepareStlExportData(exportedModels),
                                           generateSystemFilePath(fileDialog.currentFile))
                console.log(" ### TypedFileDialog.StlExport: " + fileDialog.currentFiles)
                break
            case TypedFileDialog.GcodeImport:
                gCodeObjectsRepeater.model = [generateSystemFilePath(fileDialog.currentFile)]
                console.log(" ### TypedFileDialog.GcodeImport: " + gCodeObjectsRepeater.model[0])
                break
            case TypedFileDialog.JsonImport:
                slicerParameters.paramGroups = JSON.parse(fileImportExport.readJsonFile(generateSystemFilePath(fileDialog.currentFile)))
                console.log(" ### TypedFileDialog.JsonsImport: " + fileDialog.currentFile)
                break
            case TypedFileDialog.JsonExport:
                fileImportExport.saveJsonFile(JSON.stringify(slicerParameters.paramGroups), generateSystemFilePath(fileDialog.currentFile))
                console.log(" ### TypedFileDialog.JsonsExport: " + fileDialog.currentFile)
                break
            case TypedFileDialog.SlicerExec:
                slicerExec = generateSystemFilePath(fileDialog.currentFile)
                console.log(" ### TypedFileDialog.SlicerExec: " + slicerExec)
                break

        }
    }

    function selectSlicerExec() {
        nameFilters = [".exe files (*.exe)"]
        fileMode = FileDialog.OpenFiles
        fileType = TypedFileDialog.SlicerExec
        open()
    }

    function openImportJsonFileDialog() {
        nameFilters = [".json files (*.json)"]
        fileMode = FileDialog.OpenFiles
        fileType = TypedFileDialog.JsonImport
        open()
    }

    function openExportJsonFileDialog() {
        nameFilters = [".json files (*.json)"]
        fileMode = FileDialog.SaveFile
        fileType = TypedFileDialog.JsonExport
        open()
    }

    function openImportStlFileDialog() {
        nameFilters = ["STL files (*.stl *.STL)", "Object files (*.obj)"]
        fileMode = FileDialog.OpenFiles
        fileType = TypedFileDialog.StlImport
        open()
    }

    function openExportStlFileDialog() {
        console.log(" ### stlObjectsRepeater.count:" + stlObjectsRepeater.count )
        if (0 === stlObjectsRepeater.count)
        {
            popup.backgroundColor = "yellow"
            popup.messageColor = "black"
            popup.messageText = "No models on scene. Please load model(s) to export to STL."
            popup.open()
            return
        }

        if (stlObjectsRepeater.count > 1 && QmlHelpers.getSelectedModels(stlObjectsRepeater).length === 0) {
            popup.backgroundColor = "yellow"
            popup.messageColor = "black"
            popup.messageText = "There are multiple models on scene, but no models selected. Please select model(s) to export to STL."
            popup.open()
            return
        }

        nameFilters = ["STL files (*.stl *.STL)"]
        fileMode = FileDialog.SaveFile
        fileType = TypedFileDialog.StlExport
        open()
    }

    function openImportGcodeFileDialog() {
        nameFilters = ["Gcode files (*.gcode)"]
        fileMode = FileDialog.OpenFile
        fileType = TypedFileDialog.GcodeImport
        open()
    }

    function prepareStlExportData(stlModelList) {
        var exportDataList = []
        for (var i=0; i<stlModelList.length; i++) {
            exportDataList.push({
                                 geometry:          stlModelList[i].geometry,
                                 transform:         stlModelList[i].sceneTransform,
                                 isSupportExported: supportOptions.isSupportExported
                               })
        }
        return exportDataList;
    }

    function getDefaultSlicerExecutable() {

    }
}
