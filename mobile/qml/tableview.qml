import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Controls.Material 2.0

Pane {
    Material.theme: Material.Dark
    Material.accent: Material.Blue

    TableView {
//         TableViewColumn {
//             role: "title"
//             title: "Title"
//             width: 100
//         }
//         TableViewColumn {
//             role: "author"
//             title: "Author"
//             width: 200
//         }
        model: controlModel
    }
}
