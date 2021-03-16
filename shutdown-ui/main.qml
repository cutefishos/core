import QtQuick 2.12
import QtQuick.Window 2.3
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

ApplicationWindow {
    width: Screen.width
    height: Screen.height
    visible: true
    visibility: Window.FullScreen
    flags: Qt.FramelessWindowHint | Qt.Popup
    id: root

    color: "transparent"

    function exit() {
        Qt.quit()
    }

    background: Rectangle {
        color: "black"
        opacity: 0.8
    }

    onActiveChanged: {
        if (!active)
            exit()
    }

    MouseArea {
        anchors.fill: parent
        onClicked: exit()
    }

    RowLayout {
        id: layout
        anchors.fill: parent
        spacing: root.width * 0.1

        Item {
            Layout.fillWidth: true
        }

        IconButton {
            id: shutdownButton
            Layout.alignment: Qt.AlignVCenter
            text: qsTr("Shutdown")
            icon: "qrc:///icons/system-shutdown.svg"
            onClicked: actions.shutdown()
        }

        IconButton {
            id: rebootButton
            Layout.alignment: Qt.AlignVCenter
            text: qsTr("Reboot")
            icon: "qrc:///icons/system-reboot.svg"
            onClicked: actions.reboot()
        }

        IconButton {
            id: logoutButton
            Layout.alignment: Qt.AlignVCenter
            text: qsTr("Logout")
            icon: "qrc:///icons/system-log-out.svg"
            onClicked: actions.logout()
        }

        IconButton {
            id: suspendButton
            Layout.alignment: Qt.AlignVCenter
            text: qsTr("Suspend")
            icon: "qrc:///icons/system-suspend.svg"
            onClicked: actions.suspend()
        }

        Item {
            Layout.fillWidth: true
        }
    }
}
