import QtQuick 2.12
import QtQuick.Controls 2.12

import org.twc.unionize 1.0

Page {
    title: qsTr("Setup Account")

    Column {
        id: row

        anchors.fill: parent
        anchors.margins: 30

        Label {
            width: parent.width
            font.bold: true
            text: qsTr("Please Set Full Company Name and Address for a Perfect Matching")
            wrapMode: Text.WordWrap
        }
        Item {
            height: 32
            width:1
        }

        TextField {
            id: companyName
            width: parent.width
            enabled: !App.registered && !App.registering
            placeholderText: qsTr("Company name")
        }

        TextField {
            id: companyAddress
            width: parent.width
            enabled: !App.registered && !App.registering
            placeholderText: qsTr("Company Address")
        }

        Button {
            id: register
            text: qsTr("Register")
            onClicked: {
                if (companyName.text === '' || companyAddress.text === '') {
                    return;
                }
                App.startRegistration(companyName.text, companyAddress.text)
            }
            enabled: !App.registering && !App.registered && companyName.text !== '' && companyAddress.text !== ''
        }

        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            BusyIndicator {
                anchors.horizontalCenter: parent.horizontalCenter
                running: App.registering
            }
            Text {
                id: message
                visible: App.registering
                text: ""
            }
        }
    }
    state: App.initState || ""
    states: [
        State {
            name: "getting_uuid"
            PropertyChanges {
                target: message
                text: qsTr("Creating Account...")
            }
        },
        State {
            name: "generating"
            PropertyChanges {
                target: message
                text: qsTr("Generating Keys...")
            }
        },
        State {
            name: "registering"
            PropertyChanges {
                target: message
                text: qsTr("Upoading Keys...")
            }
        },
        State {
            name: "success"
            PropertyChanges {
                target: message
                text: qsTr("Registration Completed!")
            }
        },
        State {
            name: "error"
            PropertyChanges {
                target: message
                text: qsTr("Registration Failed!")
            }
        }
    ]
}
