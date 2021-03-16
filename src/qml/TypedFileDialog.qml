import Qt.labs.platform 1.1
import QtQuick.Controls 2.15
import QtQuick 2.15

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
                stlObjects.model = generateSystemFilepathList(fileDialog.currentFiles)
                break
            case TypedFileDialog.StlExport:
                stlModel.exportModelToSTL(generateSystemFilePath(fileDialog.currentFile))
                console.log(" ### TypedFileDialog.StlExport: " + fullSystemFilePath)
                break
            case TypedFileDialog.GcodeImport:
                gCodeObjects.model = [generateSystemFilePath(fileDialog.currentFile)]
                console.log(" ### TypedFileDialog.GcodeImport: " + gCodeObjects.model[0])
                break

        }
    }

    function importStl()
    {
        nameFilters = ["STL files (*.stl *.STL)", "Object files (*.obj)"]
        fileMode = FileDialog.OpenFiles
        fileType = TypedFileDialog.StlImport
        open()
    }

    function exportStl()
    {
        nameFilters = ["STL files (*.stl *.STL)"]
        fileMode = FileDialog.SaveFile
        fileType = TypedFileDialog.StlExport
        open()
    }

    function importGcode()
    {
        nameFilters = ["Gcode files (*.gcode)"]
        fileMode = FileDialog.OpenFile
        fileType = TypedFileDialog.GcodeImport
        open()
    }
}
