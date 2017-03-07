import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Controls.Material 2.1

Pane {
    Material.theme: Material.Dark
    Material.accent: Material.Blue

    GridView {
        anchors.fill: parent

        Component {
            id: controlDelegate

            Button {
                highlighted: true
                Material.foreground: Material.Pink
                Material.background: Material.Grey
                Material.accent: Material.Grey
                onClicked: function() {
                    model.edit = true
                }
                text: display
            }
        }

        model: controlModel
        delegate: controlDelegate
        focus: true
    }
}
