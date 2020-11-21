import QtQuick 2.12
import QtQuick.Controls 2.12

import org.twc.unionize 1.0

ApplicationWindow {
    id: window
    width: 640
    height: 480
    visible: true
    title: qsTr("QUnionize")

    header: ToolBar {
        contentHeight: toolButton.implicitHeight

        ToolButton {
            id: toolButton
            text: stackView.depth > 1 ? "\u25C0" : "\u2630"
            font.pixelSize: Qt.application.font.pixelSize * 1.6
            onClicked: {
                if (stackView.depth > 1) {
                    stackView.pop()
                } else {
                    drawer.open()
                }
            }
        }

        Label {
            text: stackView.currentItem.title
            anchors.centerIn: parent
        }
    }

    Drawer {
        id: drawer
        width: window.width * 0.66
        height: window.height

        Column {
            anchors.fill: parent

            ItemDelegate {
                text: qsTr("Account")
                width: parent.width
                onClicked: {
                    stackView.push("AccountPage.qml")
                    drawer.close()
                }
            }
            ItemDelegate {
                text: qsTr("Statistics")
                width: parent.width
                onClicked: {
                    stackView.push("StatsPage.qml")
                    drawer.close()
                }
            }
            ItemDelegate {
                text: qsTr("Get Checked")
                width: parent.width
                onClicked: {
                    stackView.push("GetCheckedPage.qml")
                    drawer.close()
                }
            }
        }
    }

    StackView {
        id: stackView
        initialItem: "HomePage.qml"
        anchors.fill: parent
    }

    Connections {
        target: App
        onSetupPageRequested: {
            stackView.push("AccountPage.qml")
        }
    }

    Component.onCompleted: {
        App.start();
    }
}
