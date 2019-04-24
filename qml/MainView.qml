import QtQuick 2.3
import QtMultimedia 5.8

Item {
    Camera {
        id: camera

        captureMode: Camera.CaptureViewfinder

        videoRecorder {
             resolution: "640x480"
             frameRate: 30
        }
    }
    VideoOutput {
        fillMode: VideoOutput.PreserveAspectCrop
        source: camera
        anchors.fill: parent
    }
}
