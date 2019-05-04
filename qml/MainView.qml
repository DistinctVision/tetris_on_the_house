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
        objectEdgesTracker {
            debugEnabled: false
        }
    }

    VideoOutput {
        fillMode: VideoOutput.PreserveAspectCrop
        source: camera
        anchors.fill: parent
        filters: [ frameHandler ]
    }

    GL_View {
        id: gl_view
        anchors.fill: parent
        fillFrameMode: FillMode.PreserveAspectCrop
        inputFrameSize: frameHandler.frameSize

        scenes: [ debugImageScene, testCubeScene ]
    }

    GL_DebugImageScene {
        id: debugImageScene
        enabled: false
        fillMode: gl_view.fillFrameMode
        debugImageObject: frameHandler.objectEdgesTracker
    }

    TestCubeScene {
        id: testCubeScene
        enabled: true
        objectEdgesTracker: frameHandler.objectEdgesTracker
    }
}
