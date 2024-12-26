import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    width:200
    height:200
    Button {
        text: "Click Me"
        anchors.centerIn: parent
        onClicked: {
            console.log("Button clicked!")
        }
    }
}
