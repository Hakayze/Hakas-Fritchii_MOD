// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick3D

Item {
    id: sceneRoot

    property var gameModel: null
    property bool detailsOpen: false
    property bool inspecting: false
    property bool inputEnabled: true

    signal caseClicked(string gameKey, bool isCurrent)

    // Picked meshes intentionally carry dynamic metadata supplied by GameCase.
    function selectPickedObject(mesh: var) {
        if (mesh && typeof mesh.caseGameKey === "string" && mesh.caseGameKey.length > 0)
            sceneRoot.caseClicked(mesh.caseGameKey, Boolean(mesh.caseIsCurrent))
    }

    readonly property real safeW: Math.max(1.0, width)
    readonly property real safeH: Math.max(1.0, height)
    readonly property real cameraDistance: 1800.0
    readonly property real nearPlane: 10.0
    readonly property real responsiveScale: Math.min(1.0, Math.max(0.72, safeW / 1600.0))

    View3D {
        id: view3d
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#0a0c12"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        FrustumCamera {
            id: shelfCamera
            position: Qt.vector3d(0.0, 0.0, sceneRoot.cameraDistance)
            eulerRotation: Qt.vector3d(0.0, 0.0, 0.0)
            clipNear: sceneRoot.nearPlane
            clipFar: 5500.0
            left: -0.50 * sceneRoot.safeW * clipNear / sceneRoot.cameraDistance
            right: 0.50 * sceneRoot.safeW * clipNear / sceneRoot.cameraDistance
            top: 0.42 * sceneRoot.safeH * clipNear / sceneRoot.cameraDistance
            bottom: -0.58 * sceneRoot.safeH * clipNear / sceneRoot.cameraDistance
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(-28.0, -22.0, 0.0)
            brightness: 1.35
            color: "#f8f9ff"
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(18.0, 145.0, 0.0)
            brightness: 0.65
            color: "#8ab4f8"
        }

        PointLight {
            position: Qt.vector3d(-120.0, 180.0, 520.0)
            brightness: 1.15
            color: "#ffffff"
            quadraticFade: 0.00018
        }

        // Base escura discreta da estante
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(0.0, -230.0, -180.0)
            eulerRotation: Qt.vector3d(-90.0, 0.0, 0.0)
            scale: Qt.vector3d(38.0, 14.0, 1.0)
            materials: [
                PrincipledMaterial {
                    baseColor: "#0d1018"
                    metalness: 0.2
                    roughness: 0.35
                }
            ]
        }

        Node {
            id: shelfGroup
            scale: Qt.vector3d(sceneRoot.responsiveScale,
                               sceneRoot.responsiveScale,
                               sceneRoot.responsiveScale)
            x: sceneRoot.detailsOpen ? -Math.min(280.0, sceneRoot.safeW * 0.18) : 0.0
            y: -24.0

            Behavior on x {
                NumberAnimation {
                    duration: 280
                    easing.type: Easing.OutCubic
                }
            }

            Repeater3D {
                id: shelfRepeater
                model: sceneRoot.gameModel

                delegate: Node {
                    id: delegateNode

                    required property string gameKey
                    required property string titleText
                    required property url frontCover
                    required property url spineCover
                    required property url backCover
                    required property real relativeOffset
                    required property bool isCurrent

                    property real smoothOffset: relativeOffset

                    Behavior on smoothOffset {
                        NumberAnimation {
                            duration: 210
                            easing.type: Easing.OutCubic
                        }
                    }

                    readonly property real absOff: Math.abs(smoothOffset)
                    readonly property real signOff: smoothOffset < 0.0 ? -1.0 : 1.0

                    x: {
                        if (isCurrent) {
                            return sceneRoot.inspecting ? 110.0 : 0.0
                        }
                        return signOff * (235.0 + (absOff - 1.0) * 148.0)
                    }

                    y: isCurrent ? 8.0 : -12.0

                    z: {
                        if (isCurrent) {
                            return sceneRoot.inspecting ? 310.0 : (sceneRoot.detailsOpen ? 160.0 : 130.0)
                        }
                        const detailRecess = sceneRoot.detailsOpen ? -95.0 : 0.0
                        return -125.0 - (absOff * 44.0) + detailRecess
                    }

                    eulerRotation.y: {
                        if (isCurrent) {
                            if (sceneRoot.inspecting)
                                return 14.0
                            return sceneRoot.detailsOpen ? 18.0 : 0.0
                        }
                        return smoothOffset < 0.0 ? 46.0 : -46.0
                    }

                    Behavior on x {
                        NumberAnimation { duration: 240; easing.type: Easing.OutCubic }
                    }
                    Behavior on y {
                        NumberAnimation { duration: 220; easing.type: Easing.OutCubic }
                    }
                    Behavior on z {
                        NumberAnimation { duration: 260; easing.type: Easing.OutCubic }
                    }
                    Behavior on eulerRotation.y {
                        NumberAnimation { duration: 260; easing.type: Easing.OutCubic }
                    }

                    GameCase {
                        gameKey: delegateNode.gameKey
                        titleText: delegateNode.titleText
                        frontCover: delegateNode.frontCover
                        spineCover: delegateNode.spineCover
                        backCover: delegateNode.backCover
                        relativeOffset: delegateNode.relativeOffset
                        isCurrent: delegateNode.isCurrent
                        inspectOpen: delegateNode.isCurrent && sceneRoot.inspecting
                        dimmed: !delegateNode.isCurrent && sceneRoot.detailsOpen
                    }
                }
            }
        }
    }

    MouseArea {
        id: pickMouseArea
        anchors.fill: parent
        enabled: sceneRoot.inputEnabled
        acceptedButtons: Qt.LeftButton

        onClicked: function(mouse) {
            if (!sceneRoot.inputEnabled)
                return
            sceneRoot.selectPickedObject(view3d.pick(mouse.x, mouse.y).objectHit)
        }
    }
}
