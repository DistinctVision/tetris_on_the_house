import QtQuick 2.3
import QtMultimedia 5.8
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import Qt.labs.settings 1.0
import mystuffs 1.0

Item {
    width: 9 * 60
    height: 16 * 60

    property bool isDebug: false
    property string current_scene: "tetris_scene"

    Settings {
        id: settings
        property double binaryThreshold: 100.0
        property double minBlobArea: 30.0
        property double maxBlobCircularity: 0.25
        property bool useLaplacian: false
        property bool useAdaptiveBinarization: false
        property int adaptiveBinarizationWinSize: 31
        property bool useDilate: false
        property bool useErode: false
    }

    states: [
        State {
            name: "tetris_scene"
            PropertyChanges {
                target: debugImageScene
                enabled: false
            }
            PropertyChanges {
                target: settings_panel
                //enabled: false
                visible: false
            }
            PropertyChanges {
                target: videoOutput
                opacity: 1.0
            }
            PropertyChanges {
                target: gl_view
                orderRender: 1
            }
            PropertyChanges {
                target: frameHandler.objectEdgesTracker
                debugEnabled: false
            }
        },
        State {
            name: "settings"
            PropertyChanges {
                target: debugImageScene
                enabled: true
            }
            PropertyChanges {
                target: settings_panel
                //enabled: true
                visible: true
            }
            PropertyChanges {
                target: videoOutput
                opacity: 0.0
            }
            PropertyChanges {
                target: gl_view
                orderRender: 0
            }
            PropertyChanges {
                target: frameHandler.objectEdgesTracker
                debugEnabled: true
            }
        }
    ]

    state: current_scene

    Camera {

        id: camera

        captureMode: Camera.CaptureVideo

        deviceId: QtMultimedia.availableCameras[0].deviceId

        viewfinder {
            resolution: "1280x720"
        }
    }

    MediaPlayer {
        id: player
        source: "file:///D:/1/6.mp4"
        autoPlay: isDebug
        muted: true
        loops: MediaPlayer.Infinite
    }

    TextureReceiver {
        id: frameTextureReceiver
    }

    FrameHandler {
        id: frameHandler
        maxFrameSize: "600x600"
        orientation: isDebug ? 270 : camera.orientation
        flipHorizontally: isDebug ? false : (camera.position != Camera.FrontFace)
        focalLength: Qt.vector2d(1.5, 1.5)
        opticalCenter: Qt.vector2d(0.5, 0.5)
        objectEdgesTracker {
            useLaplacian: settings.useLaplacian
            useAdaptiveBinarization: settings.useAdaptiveBinarization
            adaptiveBinarizationWinSize: settings.adaptiveBinarizationWinSize
            useDilate: settings.useDilate
            useErode: settings.useErode
            binaryThreshold: settings.binaryThreshold
            minBlobArea: settings.minBlobArea
            maxBlobCircularity: settings.maxBlobCircularity
        }
        gl_view: gl_view
        textureReceiver: frameTextureReceiver
    }

    VideoOutput {
        id: videoOutput
        fillMode: VideoOutput.PreserveAspectCrop
        source: isDebug ? player : camera
        anchors.fill: parent
        autoOrientation: !isDebug
        orientation: 270
        filters: [ frameHandler ]
    }

    GL_View {
        id: gl_view
        anchors.fill: parent
        fillFrameMode: FillMode.PreserveAspectCrop
        inputFrameSize: frameHandler.objectEdgesTracker.frameSize
        focalLength: frameHandler.objectEdgesTracker.focalLength
        opticalCenter: frameHandler.objectEdgesTracker.opticalCenter

        scenes: [ debugImageScene, tetrisScene ]
    }

    GL_DebugImageScene {
        id: debugImageScene
        fillMode: gl_view.fillFrameMode
        debugImageObject: frameHandler.objectEdgesTracker
    }

    /*TestCubeScene {
        id: testCubeScene
        objectEdgesTracker: frameHandler.objectEdgesTracker
    }*/

    TetrisScene {
        id: tetrisScene
        enabled: (state == current_scene) && (frameHandler.objectEdgesTracker.trackingQuality != TrackingQuality.Ugly)
        objectEdgesTracker: frameHandler.objectEdgesTracker
        textureReceiver: frameTextureReceiver
    }

    Rectangle {
        id: settings_panel

        anchors.margins: 50
        anchors.fill: parent

        color: "black"
        //color: Qt.rgba(0.0, 0.0, 0.0, 0.0)
        opacity: 0.3

        ColumnLayout {
            anchors.fill: parent
            spacing: 10
            anchors.margins: 50

            Text {
                Layout.fillWidth: true
                text: "Tracker parameters"
                font.pointSize: 12
                color: "white"
                horizontalAlignment: Text.AlignHCenter
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    Layout.fillWidth: true
                    text: "Binary threshold"
                    font.pointSize: 12
                    color: "white"
                }

                Slider {
                    Layout.fillWidth: true
                    from: 0.0
                    to: 255.0
                    value: settings.binaryThreshold
                    onValueChanged: {
                        settings.binaryThreshold = value
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    Layout.fillWidth: true
                    text: "Min blob area"
                    font.pointSize: 12
                    color: "white"
                }

                Slider {
                    Layout.fillWidth: true
                    from: 0.0
                    to: 100.0
                    value: settings.minBlobArea
                    onValueChanged: {
                        settings.minBlobArea = value
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    Layout.fillWidth: true
                    text: "Max blob circularity"
                    font.pointSize: 12
                    color: "white"
                }

                Slider {
                    Layout.fillWidth: true
                    from: 0.0
                    to: 1.0
                    value: settings.maxBlobCircularity
                    onValueChanged: {
                        settings.maxBlobCircularity = value
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                CheckBox {
                    text: "Use laplacian"
                    Layout.fillWidth: true
                    checkState: settings.useLaplacian ? Qt.Checked : Qt.Unchecked
                    onCheckStateChanged: {
                        settings.useLaplacian = (checkState === Qt.Checked)
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                CheckBox {
                    text: "Use adaptive binarization"
                    Layout.fillWidth: true
                    checkState: settings.useAdaptiveBinarization ? Qt.Checked : Qt.Unchecked
                    onCheckStateChanged: {
                        settings.useAdaptiveBinarization = (checkState === Qt.Checked)
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    Layout.fillWidth: true
                    text: "Adaptive binarization win size"
                    font.pointSize: 12
                    color: "white"
                }

                Slider {
                    Layout.fillWidth: true
                    from: 5
                    to: 51
                    stepSize: 2
                    value: settings.adaptiveBinarizationWinSize
                    onValueChanged: {
                        settings.adaptiveBinarizationWinSize = Math.floor(value)
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                CheckBox {
                    text: "Use dilate"
                    Layout.fillWidth: true
                    checkState: settings.useDilate ? Qt.Checked : Qt.Unchecked
                    onCheckStateChanged: {
                        settings.useDilate = (checkState === Qt.Checked)
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                CheckBox {
                    text: "Use erode"
                    Layout.fillWidth: true
                    checkState: settings.useErode ? Qt.Checked : Qt.Unchecked
                    onCheckStateChanged: {
                        settings.useErode = (checkState === Qt.Checked)
                    }
                }
            }
        }
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Left) {
            tetrisScene.moveFigureLeft()
        } else if (event.key === Qt.Key_Right) {
            tetrisScene.moveFigureRight()
        } else if (event.key === Qt.Key_Down) {
            tetrisScene.moveFigureDown()
        } else if (event.key === Qt.Key_Up) {
            if (!tetrisScene.started) {
                tetrisScene.start()
            } else {
                tetrisScene.rotateFigure()
            }
        }
    }

    MultiPointTouchArea {
        visible: parent.state != "settings"
        anchors.fill: parent
        onGestureStarted: {
            var p = gesture.touchPoints[0]
            var dX = p.x - p.startX
            var dY = p.y - p.startY
            if ((Math.abs(dX) + Math.abs(dY)) > 20) {
                if (Math.abs(dX) > Math.abs(dY)) {
                    if (dX > 0) {
                        tetrisScene.moveFigureRight()
                    } else {
                        tetrisScene.moveFigureLeft()
                    }
                } else if (dY > 0) {
                    tetrisScene.moveFigureDown()
                }
                gesture.grab()
            }
        }
        onReleased: {
            var p = touchPoints[0]
            var dX = p.x - p.startX
            var dY = p.y - p.startY
            if ((Math.abs(dX) + Math.abs(dY)) <= 20) {
                if ((!tetrisScene.started) &&
                        (frameHandler.objectEdgesTracker.trackingQuality == TrackingQuality.Good))
                {
                    tetrisScene.start();
                    return;
                }

                if (p.x < width * (1.0 / 3.0)) {
                    tetrisScene.moveFigureLeft()
                } else if (p.x > width * (2.0 / 3.0)) {
                    tetrisScene.moveFigureRight()
                } else {
                    tetrisScene.rotateFigure()
                }
            }
        }
    }

    /*ShineButton {
        enabled: (!tetrisScene.started) &&
                 (frameHandler.objectEdgesTracker.trackingQuality == TrackingQuality.Good)
        visible: enabled
        text: qsTr("Start")
        onClicked: {
            tetrisScene.start()
        }
        anchors.centerIn: parent
        width: 100
        height: 50
    }*/

    ShineButton {
        enabled: (tetrisScene.started) &&
                 (frameHandler.objectEdgesTracker.trackingQuality == TrackingQuality.Ugly)
        visible: enabled
        text: qsTr("Reset")
        onClicked: {
            tetrisScene.start()
        }
        anchors.centerIn: parent
        width: 100
        height: 50
    }

    ToolButton {
        width: 60
        height: 60
        x: parent.width - width
        y: 0
        text: " "
        background: null
        onClicked: {
            parent.state = (parent.state == "settings") ? current_scene : "settings"
        }
    }
}
