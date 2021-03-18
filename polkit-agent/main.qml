import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import MeuiKit 1.0 as Meui

Rectangle {
    id: root

    width: 550
    height: column.implicitHeight + radius * 4

    radius: Meui.Theme.bigRadius
    color: Meui.Theme.backgroundColor
    border.color: Qt.rgba(0, 0, 0, 0.4)
    border.width: 1

    Meui.WindowShadow {
        view: rootWindow
        geometry: Qt.rect(root.x, root.y, root.width, root.height)
        radius: root.radius
    }

    RowLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: parent.radius * 2

        Image {
            id: icon
            source: "qrc:/svg/emblem-warning.svg"
            sourceSize.width: 96
            sourceSize.height: 96
            smooth: true
            Layout.alignment: Qt.AlignTop
        }

        Item {
            width: Meui.Units.smallSpacing
        }

        ColumnLayout {
            id: column

            Label {
                text: confirmation.message
                wrapMode: Text.WordWrap
                font.bold: true
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            Item {
                height: Meui.Units.smallSpacing
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
                height: Meui.Units.smallSpacing
            }

            RowLayout {
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
