import QtQuick 2.3
import QtMultimedia 5.8
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import Qt.labs.settings 1.0
import mystuffs 1.0

Item {
    width: 9 * 60
    height: 16 * 60

    property string current_scene: "tetris_scene"

    Settings {
        id: settings
        property double binaryThreshold: 100.0
        property double minBlobArea: 30.0
        property double maxBlobCircularity: 0.25
    }

    states: [
        State {
            name: "tetris_scene"
            PropertyChanges {
                target: tetrisScene
                enabled: true
            }
            PropertyChanges {
                target: debugImageScene
                enabled: false
            }
            PropertyChanges {
                target: settings_panel
                enabled: false
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
        },
        State {
            name: "settings"
            PropertyChanges {
                target: tetrisScene
                enabled: false
            }
            PropertyChanges {
                target: debugImageScene
                enabled: true
            }
            PropertyChanges {
                target: settings_panel
                enabled: true
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
        }
    ]

    state: "settings" //current_scene

    /*Camera {
        id: camera

        captureMode: Camera.CaptureVideo

        deviceId: QtMultimedia.availableCameras[0].deviceId
    }*/

    MediaPlayer {
        id: player
        source: "file:///D:/1/3.mp4"
        autoPlay: true
        loops: MediaPlayer.Infinite
    }

    TextureReceiver {
        id: frameTextureReceiver
    }

    FrameHandler {
        id: frameHandler
        maxFrameSize: "600x600"
        orientation: 270
        flipHorizontally: false
        //orientation: camera.orientation
        //flipHorizontally: (camera.position != Camera.FrontFace)
        focalLength: Qt.vector2d(1.5, 1.5)
        opticalCenter: Qt.vector2d(0.5, 0.5)
        objectEdgesTracker {
            debugEnabled: true
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
        source: player
        anchors.fill: parent
        //autoOrientation: true
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
        objectEdgesTracker: frameHandler.objectEdgesTracker
        textureReceiver: frameTextureReceiver
    }

    ToolButton {
        width: 60
        height: 60
        x: parent.width - width
        y: 0
        text: "*"
        background: null
        onClicked: {
            parent.state = (parent.state == "settings") ? current_scene : "settings"
        }
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
        }
    }
}
