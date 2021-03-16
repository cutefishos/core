import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

Item {
    id: control
    width: 150
    height: width

    property string text
    property string icon
    property color hoveredColor: Qt.rgba(255, 255, 255, 0.1)
    property color pressedColor: Qt.rgba(255, 255, 255, 0.2)
    signal clicked

    Rectangle {
        anchors.fill: parent
        color: mouseArea.pressed ? pressedColor : mouseArea.containsMouse ? hoveredColor : "transparent"
        radius: 10
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        onClicked: control.clicked()
    }

    ColumnLayout {
        id: layout
        anchors.fill: parent
        spacing: 15

        Item {
            Layout.fillHeight: true
        }

        Image {
            id: image
            source: control.icon
            width: control.width * 0.5
            height: width
            sourceSize.width: width
            sourceSize.height: width
            Layout.alignment: Qt.AlignCenter
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: control.text
            color: "white"
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
