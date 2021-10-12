import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick3D.Helpers 1.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick3D 1.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ParameterRowElement {
    var specifier
	
	onParamValueChanged: {
		paramInputRows.paramValueChanged(param, paramValue)
	}
}
