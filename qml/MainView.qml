import QtQuick 2.3
import QtMultimedia 5.8
import mystuffs 1.0

Item {
    Camera {
        id: camera

        captureMode: Camera.CaptureViewfinder

        deviceId: QtMultimedia.availableCameras[0].deviceId

        videoRecorder {
             resolution: "640x480"
             frameRate: 30
        }
    }

    /*MediaPlayer {
        id: player
        source: "D:\\1.mp4"
        autoPlay: true
    }*/

    FrameHandler {
        id: frameHandler
    }

    VideoOutput {
        fillMode: VideoOutput.PreserveAspectCrop
        source: camera
        anchors.fill: parent
        filters: [ frameHandler ]
    }
}
