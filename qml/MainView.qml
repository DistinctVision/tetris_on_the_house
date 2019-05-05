import QtQuick 2.3
import QtMultimedia 5.8
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import Qt.labs.settings 1.0
import mystuffs 1.0

Item {
    property string current_scene: "test_cube_scene"

    Settings {
        id: settings
        property double canny_thresholdA: 100
        property double canny_thresholdB: 150
    }

    states: [
        State {
            name: "test_cube_scene"
            PropertyChanges {
                target: testCubeScene
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
                target: testCubeScene
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

    state: current_scene

    Camera {
        id: camera

        captureMode: Camera.CaptureVideo

        deviceId: QtMultimedia.availableCameras[0].deviceId
    }

    /*MediaPlayer {
        id: player
        source: "D:\\1.mp4"
        autoPlay: true
    }*/

    FrameHandler {
        id: frameHandler
        maxFrameSize: "2000x2000"
        orientation: camera.orientation
        flipHorizontally: camera.position == Camera.BackFace
        objectEdgesTracker {
            debugEnabled: true
            cannyThresholdA: settings.canny_thresholdA
            cannyThresholdB: settings.canny_thresholdB
        }
    }

    VideoOutput {
        id: videoOutput
        fillMode: VideoOutput.PreserveAspectCrop
        source: camera
        anchors.fill: parent
        autoOrientation: true
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
        fillMode: gl_view.fillFrameMode
        debugImageObject: frameHandler.objectEdgesTracker
    }

    TestCubeScene {
        id: testCubeScene
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

        color: "black"
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
