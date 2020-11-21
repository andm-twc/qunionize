import QtQuick 2.12
import QtQuick.Controls 2.12

import org.twc.unionize 1.0

Page {
    id: root
    title: qsTr("Home")

    Column {
        anchors.fill: parent
        anchors.margins: 60
        spacing: 12
        Label {
            text: qsTr("Do not use this software in company WLAN!")
            font.bold: true

            anchors.left: parent.left
            anchors.right: parent.right

            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Label {
            text: qsTr("You have no account yet.")
            anchors.left: parent.left
            anchors.right: parent.right

            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Setup...")
            onClicked: {
                App.requestSetupPage();
            }
        }
    }
}
