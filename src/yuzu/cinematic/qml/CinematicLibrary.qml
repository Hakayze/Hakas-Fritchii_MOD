// SPDX-License-Identifier: GPL-3.0-or-later
pragma ComponentBehavior: Bound

import QtQuick

FocusScope {
    id: root
    objectName: "cinematicLibrary"
    focus: true
    implicitWidth: 1280
    implicitHeight: 720

    property var gameModel: null
    property int gameCount: 0
    property string selectedKey: ""
    property string selectedTitle: ""
    property bool inputEnabled: true
    property bool loading: false

    signal navigateRequested(int delta)
    signal selectRequested(string gameKey)
    signal launchRequested(string gameKey)
    signal returnToClassic()

    property bool detailsOpen: false
    property bool inspecting: false
    property int activeButtonIndex: 0

    readonly property bool hasGames: !loading && gameCount > 0 && selectedKey.length > 0

    function openDetailsPanel() {
        if (!root.inputEnabled || !root.hasGames)
            return
        root.detailsOpen = true
        root.inspecting = false
        root.activeButtonIndex = 0
        keyHandler.forceActiveFocus()
    }

    function closeDetailsPanel() {
        root.inspecting = false
        root.detailsOpen = false
        root.activeButtonIndex = 0
        keyHandler.forceActiveFocus()
    }

    function triggerFocusedPanelAction() {
        if (!root.inputEnabled || !root.detailsOpen || !root.hasGames)
            return
        if (root.activeButtonIndex === 0) {
            root.launchRequested(root.selectedKey)
        } else if (root.activeButtonIndex === 1) {
            root.inspecting = !root.inspecting
        } else if (root.activeButtonIndex === 2) {
            root.closeDetailsPanel()
        }
    }

    onSelectedKeyChanged: {
        if (root.selectedKey.length === 0) {
            root.inspecting = false
            root.detailsOpen = false
        } else {
            root.inspecting = false
        }
    }

    onGameCountChanged: {
        if (root.gameCount <= 0)
            root.closeDetailsPanel()
    }

    onLoadingChanged: {
        if (root.loading)
            root.closeDetailsPanel()
    }

    onInputEnabledChanged: {
        if (root.inputEnabled)
            keyHandler.forceActiveFocus()
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#080a10" }
            GradientStop { position: 0.6; color: "#0b0e17" }
            GradientStop { position: 1.0; color: "#06070c" }
        }
    }

    ShelfScene {
        id: shelfScene
        anchors.fill: parent
        gameModel: root.gameModel
        detailsOpen: root.detailsOpen
        inspecting: root.inspecting
        inputEnabled: root.inputEnabled && !root.loading && root.gameCount > 0

        onCaseClicked: function(gameKey, isCurrent) {
            if (!root.inputEnabled || root.loading)
                return
            if (isCurrent) {
                if (!root.detailsOpen)
                    root.openDetailsPanel()
                else
                    root.inspecting = !root.inspecting
            } else {
                root.inspecting = false
                root.selectRequested(gameKey)
            }
            keyHandler.forceActiveFocus()
        }
    }

    // Atenuação lateral mantendo os vizinhos visíveis sob o painel
    Rectangle {
        anchors.fill: parent
        color: "#06080e"
        opacity: root.detailsOpen ? 0.46 : 0.0
        visible: opacity > 0.01

        Behavior on opacity {
            NumberAnimation { duration: 220; easing.type: Easing.OutQuad }
        }
    }

    // HUD Superior Discreto
    Rectangle {
        id: topHud
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 54
        color: "#88090c14"

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 28
            anchors.verticalCenter: parent.verticalCenter
            text: "FRITCHII CINEBOX  •  " + root.gameCount + (root.gameCount === 1 ? " JOGO" : " JOGOS")
            color: "#d5deef"
            font.pixelSize: 14
            font.bold: true
        }

        Rectangle {
            id: classicButton
            anchors.right: parent.right
            anchors.rightMargin: 28
            anchors.verticalCenter: parent.verticalCenter
            width: 168
            height: 34
            radius: 6
            color: classicMouse.containsMouse && root.inputEnabled ? "#263044" : "#181e2c"
            border.width: 1
            border.color: "#3b4866"
            Accessible.role: Accessible.Button
            Accessible.name: "Retornar ao modo clássico"
            Accessible.onPressAction: {
                if (root.inputEnabled)
                    root.returnToClassic()
            }

            Text {
                anchors.centerIn: parent
                text: "Modo Clássico (Esc)"
                color: "#e4ecfa"
                font.pixelSize: 13
            }

            MouseArea {
                id: classicMouse
                anchors.fill: parent
                hoverEnabled: true
                enabled: root.inputEnabled
                onClicked: {
                    if (root.inputEnabled)
                        root.returnToClassic()
                }
            }
        }
    }

    // Rodapé com título selecionado e dicas de comando na estante
    Item {
        id: shelfFooter
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24
        height: 86
        visible: root.hasGames && !root.detailsOpen

        Text {
            id: mainTitleLabel
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            width: parent.width * 0.82
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            text: root.selectedTitle.length > 0 ? root.selectedTitle : root.selectedKey
            textFormat: Text.PlainText
            color: "#f4f7ff"
            font.pixelSize: Math.max(22, Math.min(32, Math.round(root.width * 0.022)))
            font.bold: true
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            text: "[◀/▶] Navegar    [Enter] Detalhes    [Esc] Modo Clássico"
            color: "#8c9bb5"
            font.pixelSize: 13
        }
    }

    // Painel Lateral de Detalhes e Inspeção
    Rectangle {
        id: detailsPanel
        anchors.top: topHud.bottom
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 28
        width: Math.max(340, Math.min(440, Math.round(root.width * 0.30)))
        radius: 12
        color: "#d9101522"
        border.width: 1
        border.color: "#313d59"
        visible: root.detailsOpen && root.hasGames
        opacity: root.detailsOpen ? 1.0 : 0.0

        Behavior on opacity {
            NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
        }

        // Prevent clicks in blank panel space from selecting a case behind it.
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
        }

        Column {
            anchors.fill: parent
            anchors.margins: 26
            spacing: 16

            Text {
                text: "DETALHES DO TÍTULO"
                color: "#7f95bc"
                font.pixelSize: 12
                font.bold: true
            }

            Text {
                width: parent.width
                text: root.selectedTitle.length > 0 ? root.selectedTitle : "Título sem nome"
                textFormat: Text.PlainText
                color: "#ffffff"
                font.pixelSize: 24
                font.bold: true
                wrapMode: Text.WordWrap
                maximumLineCount: 3
                elide: Text.ElideRight
            }

            Text {
                width: parent.width
                text: "ID: " + root.selectedKey
                textFormat: Text.PlainText
                color: "#9aa8c2"
                font.pixelSize: 13
                elide: Text.ElideMiddle
            }

            Rectangle {
                width: parent.width
                height: 1
                color: "#28324a"
            }

            Text {
                width: parent.width
                text: root.inspecting
                      ? "Inspeção ativa: tampa aberta a -165° exibindo o cartucho interno."
                      : "Vizinhos preservados na estante. Inicie o jogo ou inspecione o interior da caixa."
                color: "#c5d1e8"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
            }

            Repeater {
                id: actionRepeater
                model: [
                    { label: "Jogar Agora", roleDesc: "Iniciar jogo selecionado" },
                    { label: root.inspecting ? "Fechar Caixa 3D" : "Inspecionar Caixa 3D", roleDesc: "Alternar abertura da tampa 3D" },
                    { label: "Voltar à Estante", roleDesc: "Fechar painel de detalhes" }
                ]

                delegate: Rectangle {
                    id: actionButton
                    required property int index
                    required property var modelData

                    width: detailsPanel.width - 52
                    height: 44
                    radius: 8
                    readonly property bool isFocused: root.activeButtonIndex === index
                    color: {
                        if (index === 0)
                            return isFocused ? "#d83440" : "#aa2530"
                        return isFocused ? "#2b3752" : "#1a2234"
                    }
                    border.width: isFocused ? 2 : 1
                    border.color: isFocused ? "#f2f6ff" : "#384666"

                    Accessible.role: Accessible.Button
                    Accessible.name: modelData.label
                    Accessible.description: modelData.roleDesc
                    Accessible.focused: isFocused
                    Accessible.onPressAction: {
                        if (!root.inputEnabled)
                            return
                        root.activeButtonIndex = index
                        root.triggerFocusedPanelAction()
                    }

                    Text {
                        anchors.centerIn: parent
                        text: actionButton.modelData.label
                        color: "#ffffff"
                        font.pixelSize: 15
                        font.bold: actionButton.isFocused
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        enabled: root.inputEnabled && root.detailsOpen
                        onEntered: {
                            if (root.inputEnabled)
                                root.activeButtonIndex = actionButton.index
                        }
                        onClicked: {
                            if (!root.inputEnabled)
                                return
                            root.activeButtonIndex = actionButton.index
                            root.triggerFocusedPanelAction()
                            keyHandler.forceActiveFocus()
                        }
                    }
                }
            }
        }
    }

    // Estados de Carregamento e Biblioteca Vazia
    Rectangle {
        anchors.centerIn: parent
        width: 440
        height: 130
        radius: 10
        color: "#cc101521"
        border.width: 1
        border.color: "#2b3650"
        visible: root.loading || root.gameCount <= 0

        Column {
            anchors.centerIn: parent
            spacing: 10

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: root.loading ? "Carregando biblioteca..." : "Nenhum jogo disponível na estante"
                color: "#eef3ff"
                font.pixelSize: 19
                font.bold: true
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: root.loading
                      ? "Sincronizando metadados e capas do modelo C++."
                      : "Verifique o diretório configurado no modo Clássico (Esc)."
                color: "#98a6c0"
                font.pixelSize: 13
            }
        }
    }

    Item {
        id: keyHandler
        focus: true

        Keys.onPressed: function(event) {
            if (!root.inputEnabled) {
                event.accepted = true
                return
            }

            if ((event.modifiers & (Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier)) !== 0) {
                event.accepted = false
                return
            }

            if (root.loading) {
                if (!event.isAutoRepeat && (event.key === Qt.Key_Escape || event.key === Qt.Key_Back))
                    root.returnToClassic()
                event.accepted = true
                return
            }

            if (event.key === Qt.Key_Left || event.key === Qt.Key_Right) {
                if (!root.detailsOpen && !root.inspecting && root.gameCount > 0) {
                    root.navigateRequested(event.key === Qt.Key_Left ? -1 : 1)
                }
                event.accepted = true
                return
            }

            if (event.isAutoRepeat) {
                event.accepted = true
                return
            }

            if (event.key === Qt.Key_Up || event.key === Qt.Key_Backtab) {
                if (root.detailsOpen)
                    root.activeButtonIndex = (root.activeButtonIndex + 2) % 3
                event.accepted = true
                return
            }

            if (event.key === Qt.Key_Down || event.key === Qt.Key_Tab) {
                if (root.detailsOpen)
                    root.activeButtonIndex = (root.activeButtonIndex + 1) % 3
                event.accepted = true
                return
            }

            if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
                if (!root.detailsOpen) {
                    if (root.hasGames)
                        root.openDetailsPanel()
                } else {
                    root.triggerFocusedPanelAction()
                }
                event.accepted = true
                return
            }

            if (event.key === Qt.Key_I && root.detailsOpen && root.hasGames) {
                root.inspecting = !root.inspecting
                event.accepted = true
                return
            }

            if (event.key === Qt.Key_Escape || event.key === Qt.Key_Back) {
                if (root.inspecting) {
                    root.inspecting = false
                    root.activeButtonIndex = 1
                    keyHandler.forceActiveFocus()
                } else if (root.detailsOpen) {
                    root.closeDetailsPanel()
                } else {
                    root.returnToClassic()
                }
                event.accepted = true
                return
            }
        }
    }
}
