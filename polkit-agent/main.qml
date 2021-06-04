import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import FishUI 1.0 as FishUI

Rectangle {
    id: root

    width: 550
    height: column.implicitHeight + radius * 4

    radius: FishUI.Theme.bigRadius
    color: FishUI.Theme.backgroundColor

    FishUI.WindowShadow {
        view: rootWindow
        geometry: Qt.rect(root.x, root.y, root.width, root.height)
        radius: root.radius
    }

    FontMetrics {
        id: fontMetrics
    }

    RowLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: FishUI.Theme.bigRadius * 2

        Image {
            id: icon
            source: "qrc:/svg/emblem-warning.svg"
            sourceSize.width: 96
            sourceSize.height: 96
            smooth: true
            Layout.alignment: Qt.AlignTop
        }

        Item {
            width: FishUI.Theme.bigRadius
        }

        ColumnLayout {
            id: column
            spacing: FishUI.Units.smallSpacing

            Text {
                text: confirmation.message
                font.bold: false
                Layout.fillWidth: true
                Layout.fillHeight: true
                maximumLineCount: 2
                wrapMode: Text.Wrap
                elide: Text.ElideRight
            }

            Item {
                height: FishUI.Units.smallSpacing
            }

            TextField {
                id: userTextField
                text: confirmation.identity
                enabled: false
                Layout.fillWidth: true
            }

            TextField {
                id: passwordInput
                placeholderText: qsTr("Password")
                echoMode: TextField.Password
                Layout.fillWidth: true
                selectByMouse: true
                focus: true
                onAccepted: {
                    if (passwordInput.text)
                        confirmation.setConfirmationResult(passwordInput.text)
                }
            }

            Item {
                height: FishUI.Units.smallSpacing
            }

            RowLayout {
                spacing: FishUI.Units.largeSpacing

                Button {
                    text: qsTr("Cancel")
                    Layout.fillWidth: true
                    height: 50
                    onClicked: confirmation.setConfirmationResult("")
                }

                Button {
                    id: doneButton
                    text: qsTr("Done")
                    Layout.fillWidth: true
                    flat: true
                    height: 50
                    onClicked: confirmation.setConfirmationResult(passwordInput.text)
                }
            }
        }
    }
}
