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
        property double canny_thresholdA: 100
        property double canny_thresholdB: 150
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
        source: "file:///D:/1.mp4"
        autoPlay: true
        loops: MediaPlayer.Infinite
    }

    FrameHandler {
        id: frameHandler
        maxFrameSize: "600x600"
        //orientation: camera.orientation
        orientation: 270
        flipHorizontally: false
        //flipHorizontally: (camera.position != Camera.FrontFace)
        focalLength: Qt.vector2d(2.5, 2.5)
        opticalCenter: Qt.vector2d(0.5, 0.5)
        objectEdgesTracker {
            debugEnabled: true
            cannyThresholdA: settings.canny_thresholdA
            cannyThresholdB: settings.canny_thresholdB
        }
        gl_view: gl_view
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

        //color: "black"
        color: Qt.rgba(0.0, 0.0, 0.0, 0.0)
        opacity: 0.3

        ColumnLayout {
            anchors.fill: parent
            spacing: 10
            anchors.margins: 50

            Text {
                Layout.fillWidth: true
                text: "Canny"
                font.pointSize: 23
                color: "white"
                horizontalAlignment: Text.AlignHCenter
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    Layout.fillWidth: true
                    text: "Threshold A"
                    font.pointSize: 23
                    color: "white"
                }

                Slider {
                    Layout.fillWidth: true
                    from: 0
                    to: 500
                    value: settings.canny_thresholdA
                    onValueChanged: {
                        settings.canny_thresholdA = value
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    Layout.fillWidth: true
                    text: "Threshold B"
                    font.pointSize: 23
                    color: "white"
                }

                Slider {
                    id: sliderB
                    Layout.fillWidth: true
                    from: 0
                    to: 500
                    value: settings.canny_thresholdB
                    onValueChanged: {
                        settings.canny_thresholdB = value
                    }
                }
            }
        }
    }
}
