import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick 2.7

Pane {
//     anchors.fill: parent
    width:500
    height:800

    Material.theme: Material.Dark
    Material.accent: Material.Blue

    Column {
        Material.theme: Material.Dark
        Material.accent: Material.Blue
        Button {
            text: qsTr("Button")
        }

        anchors.fill: parent
//         anchors.left: parent.left
//         anchors.top: parent.top
//         width: parent.width
//         height: parent.height


        RadioButton { text: qsTr("Small") }
        RadioButton { text: qsTr("Medium");  checked: true }
        RadioButton { text: qsTr("Large") }
    }
}
