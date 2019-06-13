import QtQuick 2.4
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.1
import qmlplayerLib 1.0

import org.freedesktop.gstreamer.GLVideoItem 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 640
    height: 480
    x: 30
    y: 30
    color: "black"

    function formatTime(timeInMs) {
        if (!timeInMs || timeInMs <= 0) return "0:00"
        var seconds = timeInMs / 1000;
        var minutes = Math.floor(seconds / 60)
        seconds = Math.floor(seconds % 60)
        if (seconds < 10) seconds = "0" + seconds;
        return minutes + ":" + seconds
    }

    Item {
        anchors.fill: parent
        id:rootItem
        objectName: "rootItem"

        property int dur: 1
        property int pos: 0

        GstGLVideoItem {
            id: video
            objectName: "videoItem"
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
        }

        Button {
            id : button
            objectName: "button"
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.margins: 20

            signal pauseOrPlay();

            text: "Start/Pause"
            onClicked: {
                button.pauseOrPlay()
                timer.start();
            }
        }


        Slider {
            id: slider
            objectName: "slider"
            minimumValue: 0
            maximumValue: 1
            value: parent.pos/parent.dur
            anchors.left: button.right
            anchors.bottom: parent.bottom
            anchors.right: position.left
            anchors.margins: 20

            signal seekPosition()

            onPressedChanged: {
                if(pressed)
                {console.log(value)}
                else{
                    console.log(value)
                    seekPosition()
                }
            }

        }


        Text {
            id: position
            objectName: "position"
            anchors.bottom: parent.bottom
            anchors.right: slash.left
            anchors.bottomMargin: 20
            text: formatTime(parent.pos)
            color: "white"

            signal getPosition()

            Timer{
                id: timer
                objectName: "timer"
                interval: 1000
                repeat: true
                onTriggered: position.getPosition()
            }
        }

        Text {
            id: slash
            text: "/"
            anchors.bottom: parent.bottom
            anchors.right: duration.left
            anchors.bottomMargin: 20
            color:"white"
        }

        Text {
            id: duration
            objectName: "duration"
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.margins : 20
            text: formatTime(parent.dur)
            font.pointSize: 12
            color: "white"
        }
    }
}
