// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick3D

Item {
    id: root
    objectName: "cinematicCalibrationScene"
    width: 1100
    height: 680
    focus: true

    signal returnToClassic()

    property int selectedIndex: 1
    property real turnAngle: -12
    readonly property var titles: ["ORBITAL", "PRISMA", "HORIZONTE"]
    readonly property var coverColors: ["#226b82", "#c32e49", "#7358b5"]

    component CardPart: Model {
        required property int cardIndex
    }

    function selectCard(index: int) {
        selectedIndex = Math.max(0, Math.min(2, index));
        turnAngle = -12;
        forceActiveFocus();
    }

    Keys.onPressed: function(event) {
        if (event.modifiers & (Qt.AltModifier | Qt.ControlModifier | Qt.MetaModifier)) {
            event.accepted = false;
            return;
        }
        switch (event.key) {
        case Qt.Key_Left:
            selectCard(selectedIndex - 1);
            break;
        case Qt.Key_Right:
            selectCard(selectedIndex + 1);
            break;
        case Qt.Key_Up:
            turnAngle = Math.min(55, turnAngle + 8);
            break;
        case Qt.Key_Down:
            turnAngle = Math.max(-55, turnAngle - 8);
            break;
        case Qt.Key_Escape:
            if (!event.isAutoRepeat)
                returnToClassic();
            break;
        default:
            event.accepted = false;
            return;
        }
        event.accepted = true;
    }

    Rectangle {
        anchors.fill: parent
        color: "#121217"
    }

    Column {
        x: 30
        y: 24
        spacing: 5

        Text {
            text: "HAKAS"
            color: "#f4f1f4"
            font.pixelSize: 28
            font.bold: true
            font.letterSpacing: 5
        }
        Text {
            text: qsTr("Protótipo 3D · caixas de teste")
            color: "#b3aeb9"
            font.pixelSize: 13
        }
    }

    Rectangle {
        id: returnButton
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 24
        width: 152
        height: 38
        radius: 5
        color: returnMouse.containsMouse ? "#3a202d" : "#231c26"
        border.width: activeFocus ? 2 : 1
        border.color: activeFocus ? "#f2a7b9" : "#8c334d"
        activeFocusOnTab: true
        Accessible.role: Accessible.Button
        Accessible.name: returnText.text
        Keys.onReturnPressed: function(event) {
            if (!event.isAutoRepeat)
                root.returnToClassic();
        }
        Keys.onSpacePressed: function(event) {
            if (!event.isAutoRepeat)
                root.returnToClassic();
        }

        Text {
            id: returnText
            anchors.centerIn: parent
            text: qsTr("Voltar ao Clássico")
            color: "#f1dce4"
            font.pixelSize: 13
        }
        MouseArea {
            id: returnMouse
            anchors.fill: parent
            hoverEnabled: true
            onClicked: root.returnToClassic()
        }
    }

    View3D {
        id: shelf
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 100
        anchors.bottom: footer.top
        camera: shelfCamera

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "#121217"
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.Medium
        }

        // Near-plane bounds use the same pixel scale as the 1800-unit focal distance.
        FrustumCamera {
            id: shelfCamera
            z: 1800
            clipNear: 10
            clipFar: 5000
            left: -0.5 * Math.max(1, shelf.width) * clipNear / 1800
            right: 0.5 * Math.max(1, shelf.width) * clipNear / 1800
            top: 0.42 * Math.max(1, shelf.height) * clipNear / 1800
            bottom: -0.58 * Math.max(1, shelf.height) * clipNear / 1800
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(-25, -30, 0)
            brightness: 1.2
            ambientColor: "#777780"
        }

        Node {
            readonly property real fit: Math.min(shelf.width / 960, shelf.height / 440, 1.3)
            scale: Qt.vector3d(fit, fit, fit)

            Model {
                source: "#Cube"
                y: -176
                z: -35
                scale: Qt.vector3d(9.5, 0.06, 3)
                materials: PrincipledMaterial {
                    baseColor: "#25212b"
                    roughness: 0.8
                }
            }

            Repeater3D {
                model: 3
                delegate: Node {
                    id: card
                    required property int index
                    readonly property bool selected: index === root.selectedIndex
                    x: (index - root.selectedIndex) * 244
                    y: selected ? 5 : -12
                    z: selected ? 85 : -75
                    eulerRotation.y: selected ? root.turnAngle
                                              : (index < root.selectedIndex ? 24 : -24)

                    Behavior on x { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
                    Behavior on y { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
                    Behavior on z { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
                    Behavior on eulerRotation.y {
                        NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
                    }

                    CardPart {
                        source: "#Cube"
                        scale: Qt.vector3d(1.9, 2.7, 0.35)
                        pickable: true
                        cardIndex: card.index
                        materials: PrincipledMaterial {
                            baseColor: card.selected ? "#af2843" : "#562438"
                            roughness: 0.3
                            metalness: 0
                        }
                    }

                    CardPart {
                        source: "#Rectangle"
                        z: 17.8
                        scale: Qt.vector3d(1.78, 2.58, 1)
                        pickable: true
                        cardIndex: card.index
                        materials: PrincipledMaterial {
                            lighting: PrincipledMaterial.NoLighting
                            baseColorMap: Texture {
                                sourceItem: Rectangle {
                                    width: 356
                                    height: 516
                                    color: root.coverColors[card.index]

                                    Rectangle {
                                        x: 24
                                        y: 24
                                        width: 308
                                        height: 30
                                        color: "#16131f"
                                        Text {
                                            anchors.centerIn: parent
                                            text: "HAKAS / " + (card.index + 1)
                                            color: "#f9eaf0"
                                            font.pixelSize: 15
                                            font.letterSpacing: 4
                                        }
                                    }
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 150
                                        height: 150
                                        rotation: 45 + card.index * 15
                                        color: "transparent"
                                        border.color: "#e8cbd8"
                                        border.width: 12
                                    }
                                    Text {
                                        x: 24
                                        y: 406
                                        width: 308
                                        text: root.titles[card.index]
                                        color: "#fff3f7"
                                        font.pixelSize: 30
                                        font.bold: true
                                        fontSizeMode: Text.HorizontalFit
                                        minimumPixelSize: 18
                                    }
                                    Text {
                                        x: 24
                                        y: 461
                                        text: qsTr("ARTE ORIGINAL · TESTE M1")
                                        color: "#ead6e1"
                                        font.pixelSize: 12
                                        font.letterSpacing: 2
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: function(mouse) {
                root.forceActiveFocus();
                const result = shelf.pick(mouse.x, mouse.y);
                const cardPart = result.objectHit as CardPart;
                if (cardPart)
                    root.selectCard(cardPart.cardIndex);
            }
        }
    }

    Column {
        id: footer
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24
        spacing: 8

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.titles[root.selectedIndex]
            color: "#f6e9ef"
            font.pixelSize: 19
            font.bold: true
            font.letterSpacing: 3
        }
        Text {
            width: parent.width - 32
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("← → selecionar · ↑ ↓ girar · clique para selecionar · Esc voltar")
            color: "#ada6b3"
            font.pixelSize: 13
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }
}
