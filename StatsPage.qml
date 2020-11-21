import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import org.twc.unionize 1.0

Page {
    id: root

    title: qsTr("Workplace Stats")

    Column {
        id: layout
        anchors.margins: 30

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        spacing: 12

        Label {
            anchors.left: layout.left
            anchors.right: layout.right
            text: qsTr("%1 colleagues want to unionize").arg(App.total)
            wrapMode: Text.WordWrap
        }
        Label {
            anchors.left: layout.left
            anchors.right: layout.right
            text: qsTr("%1 colleagues confirmed by union").arg(App.numUnionConfirmed)
            wrapMode: Text.WordWrap
        }
        Label {
            anchors.left: layout.left
            anchors.right: layout.right
            text: qsTr("%1 colleagues confirmed by peers").arg(App.numPeerConfirmed)
            wrapMode: Text.WordWrap
        }
    }
}
