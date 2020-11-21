import QtQuick 2.12
import QtQuick.Controls 2.12

import org.twc.unionize 1.0
import QZXing 2.3

Page {
    id: root
    title: qsTr("Get Checked")

    Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12
        Label {
            text: qsTr("Let Signer scan this:")
            font.bold: true

            anchors.left: parent.left
            anchors.right: parent.right

            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Image {
            id: qrUUID


            source: "image://qzxing/encode/%1?border=true".arg(App.uuid)
//?border=true&format=qrcode
            cache: false

            fillMode: Image.PreserveAspectFit

            sourceSize.width: 320
            sourceSize.height: 320

            anchors.left: parent.left
            anchors.right: parent.right
        }
    }
}
