import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick 2.7

Pane {

    Material.theme: Material.Dark
    Material.accent: Material.Blue

    Column {
        Button {
            text: qsTr("Button")
        }

        anchors.fill: parent

        RadioButton { text: qsTr("Small") }
        RadioButton { text: qsTr("Medium");  checked: true }
        RadioButton { text: qsTr("Large") }
    }
}
