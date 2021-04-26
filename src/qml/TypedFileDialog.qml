import Qt.labs.platform 1.1
import QtQuick.Controls 2.15
import QtQuick 2.15
import "HelperFunctions.js" as QmlHelpers

FileDialog {
    id: root

    enum FileType {
        GcodeImport,
        StlImport,
        StlExport
    }

    property int fileType: TypedFileDialog.StlImport

    onAccepted:
    {
        function generateSystemFilePath(qtFilePath) {
            var fullSystemFilePath = qtFilePath.toString()
            var pos = fullSystemFilePath.search("file:///");
            return fullSystemFilePath.substring(pos+8)
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
                    exportedModels = QmlHelpers.getSelected(stlObjectsRepeater)
                }
                helper3D.exportModelsToSTL(prepareStlExportData(exportedModels),
                                           generateSystemFilePath(fileDialog.currentFile))
                console.log(" ### TypedFileDialog.StlExport: " + fileDialog.currentFiles)
                break
            case TypedFileDialog.GcodeImport:
                gCodeObjectsRepeater.model = [generateSystemFilePath(fileDialog.currentFile)]
                console.log(" ### TypedFileDialog.GcodeImport: " + gCodeObjectsRepeater.model[0])
                break

        }
    }

    function openImportStlFileDialog()
    {
        nameFilters = ["STL files (*.stl *.STL)", "Object files (*.obj)"]
        fileMode = FileDialog.OpenFiles
        fileType = TypedFileDialog.StlImport
        open()
    }

    function openExportStlFileDialog()
    {
        console.log(" ### stlObjectsRepeater.count:" + stlObjectsRepeater.count )
        if (0 === stlObjectsRepeater.count)
        {
            popup.backgroundColor = "yellow"
            popup.messageColor = "black"
            popup.messageText = "No models on scene. Please load model(s) to export to STL."
            popup.open()
            return
        }

        if (stlObjectsRepeater.count > 1 && QmlHelpers.getSelected(stlObjectsRepeater).length === 0) {
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

    function openImportGcodeFileDialog()
    {
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
}
