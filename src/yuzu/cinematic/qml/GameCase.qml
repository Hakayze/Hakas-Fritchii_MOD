// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick3D

Node {
    id: caseRoot

    property string gameKey: ""
    property string titleText: ""
    property url frontCover: ""
    property url spineCover: ""
    property url backCover: ""
    property real relativeOffset: 0.0
    property bool isCurrent: false
    property bool inspectOpen: false
    property bool dimmed: false

    readonly property real caseWidth: 250.0
    readonly property real caseHeight: 400.0
    readonly property real caseDepth: 30.0
    readonly property real spineWidth: 14.0
    readonly property real printEpsilon: 0.6

    readonly property bool hasFront: frontCover.toString().length > 0
    readonly property bool hasSpine: spineCover.toString().length > 0
    readonly property bool hasBack: backCover.toString().length > 0

    readonly property real seedHash: {
        const s = (gameKey.length > 0 ? gameKey : titleText)
        let h = 2166136261
        for (let i = 0; i < s.length; ++i) {
            h ^= s.charCodeAt(i)
            h = Math.imul(h, 16777619)
        }
        return h >>> 0
    }

    readonly property color fallbackFrontColor: Qt.hsla((seedHash % 360) / 360.0, 0.58, 0.34, 1.0)
    readonly property color fallbackSpineColor: Qt.hsla(((seedHash + 40) % 360) / 360.0, 0.62, 0.26, 1.0)
    readonly property color fallbackBackColor: Qt.hsla(((seedHash + 85) % 360) / 360.0, 0.48, 0.22, 1.0)
    readonly property real effectiveOpacity: dimmed ? 0.42 : 1.0

    Texture {
        id: frontTexture
        source: caseRoot.hasFront ? caseRoot.frontCover : ""
        generateMipmaps: true
        mipFilter: Texture.Linear
    }

    Texture {
        id: spineTexture
        source: caseRoot.hasSpine ? caseRoot.spineCover : ""
        generateMipmaps: true
        mipFilter: Texture.Linear
    }

    Texture {
        id: backTexture
        source: caseRoot.hasBack ? caseRoot.backCover : ""
        generateMipmaps: true
        mipFilter: Texture.Linear
    }

    PrincipledMaterial {
        id: plasticBodyMaterial
        baseColor: "#1a1e29"
        metalness: 0.05
        roughness: 0.24
        specularAmount: 0.85
        opacity: caseRoot.effectiveOpacity
    }

    PrincipledMaterial {
        id: clearSleeveMaterial
        baseColor: "#ffffff"
        metalness: 0.0
        roughness: 0.12
        specularAmount: 0.95
        opacity: caseRoot.dimmed ? 0.06 : 0.14
        alphaMode: PrincipledMaterial.Blend
    }

    PrincipledMaterial {
        id: frontPrintMaterial
        lighting: PrincipledMaterial.NoLighting
        baseColor: caseRoot.hasFront ? "#ffffff" : caseRoot.fallbackFrontColor
        baseColorMap: caseRoot.hasFront ? frontTexture : null
        opacity: caseRoot.effectiveOpacity
    }

    PrincipledMaterial {
        id: spinePrintMaterial
        lighting: PrincipledMaterial.NoLighting
        baseColor: caseRoot.hasSpine ? "#ffffff" : caseRoot.fallbackSpineColor
        baseColorMap: caseRoot.hasSpine ? spineTexture : null
        opacity: caseRoot.effectiveOpacity
    }

    PrincipledMaterial {
        id: backPrintMaterial
        lighting: PrincipledMaterial.NoLighting
        baseColor: caseRoot.hasBack ? "#ffffff" : caseRoot.fallbackBackColor
        baseColorMap: caseRoot.hasBack ? backTexture : null
        opacity: caseRoot.effectiveOpacity
    }

    PrincipledMaterial {
        id: cartridgeBodyMaterial
        baseColor: "#252934"
        metalness: 0.12
        roughness: 0.38
        opacity: caseRoot.effectiveOpacity
    }

    PrincipledMaterial {
        id: cartridgeContactsMaterial
        baseColor: "#c89d3c"
        metalness: 0.75
        roughness: 0.25
        opacity: caseRoot.effectiveOpacity
    }

    // Corpo traseiro (bandeja plástica)
    Model {
        id: backTrayMesh
        source: "#Cube"
        position: Qt.vector3d(4.0, 0.0, -7.0)
        scale: Qt.vector3d((caseRoot.caseWidth - 8.0) / 100.0,
                           caseRoot.caseHeight / 100.0,
                           (caseRoot.caseDepth * 0.52) / 100.0)
        materials: [ plasticBodyMaterial ]
        pickable: true
        property string caseGameKey: caseRoot.gameKey
        property bool caseIsCurrent: caseRoot.isCurrent
    }

    // Lombada plástica estrutural à esquerda
    Model {
        id: spinePlasticMesh
        source: "#Cube"
        position: Qt.vector3d(-caseRoot.caseWidth * 0.5 + caseRoot.spineWidth * 0.5, 0.0, 0.0)
        scale: Qt.vector3d(caseRoot.spineWidth / 100.0,
                           caseRoot.caseHeight / 100.0,
                           caseRoot.caseDepth / 100.0)
        materials: [ plasticBodyMaterial ]
        pickable: true
        property string caseGameKey: caseRoot.gameKey
        property bool caseIsCurrent: caseRoot.isCurrent
    }

    // Impressão da lombada afastada por printEpsilon contra z-fighting
    Model {
        id: spinePrintMesh
        source: "#Rectangle"
        position: Qt.vector3d(-caseRoot.caseWidth * 0.5 - caseRoot.printEpsilon, 0.0, 0.0)
        eulerRotation: Qt.vector3d(0.0, -90.0, 0.0)
        scale: Qt.vector3d((caseRoot.caseDepth - 4.0) / 100.0,
                           (caseRoot.caseHeight - 10.0) / 100.0,
                           1.0)
        materials: [ spinePrintMaterial ]
    }

    // Impressão da contracapa afastada em -Z por printEpsilon
    Model {
        id: backPrintMesh
        source: "#Rectangle"
        position: Qt.vector3d(2.0, 0.0, -caseRoot.caseDepth * 0.5 - caseRoot.printEpsilon)
        eulerRotation: Qt.vector3d(0.0, 180.0, 0.0)
        scale: Qt.vector3d((caseRoot.caseWidth - 14.0) / 100.0,
                           (caseRoot.caseHeight - 10.0) / 100.0,
                           1.0)
        materials: [ backPrintMaterial ]
    }

    // Cartucho geométrico interno fixado na bandeja traseira
    Node {
        id: cartridgeNode
        position: Qt.vector3d(18.0, -18.0, -1.5)

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.68, 0.96, 0.08)
            materials: [ cartridgeBodyMaterial ]
        }

        Model {
            source: "#Rectangle"
            position: Qt.vector3d(0.0, 4.0, 4.0 + caseRoot.printEpsilon)
            scale: Qt.vector3d(0.54, 0.66, 1.0)
            materials: [ frontPrintMaterial ]
        }

        Model {
            source: "#Rectangle"
            position: Qt.vector3d(0.0, -39.0, 4.0 + caseRoot.printEpsilon)
            scale: Qt.vector3d(0.48, 0.10, 1.0)
            materials: [ cartridgeContactsMaterial ]
        }
    }

    // Tampa frontal articulada com pivô na margem esquerda
    Node {
        id: frontLidHinge
        position: Qt.vector3d(-caseRoot.caseWidth * 0.5, 0.0, caseRoot.caseDepth * 0.5)
        eulerRotation.y: (caseRoot.isCurrent && caseRoot.inspectOpen) ? -165.0 : 0.0

        Behavior on eulerRotation.y {
            NumberAnimation {
                duration: 420
                easing.type: Easing.OutCubic
            }
        }

        // Placa plástica da tampa frontal
        Model {
            id: frontLidPlastic
            source: "#Cube"
            position: Qt.vector3d(caseRoot.caseWidth * 0.5, 0.0, -3.5)
            scale: Qt.vector3d(caseRoot.caseWidth / 100.0,
                               caseRoot.caseHeight / 100.0,
                               0.07)
            materials: [ plasticBodyMaterial ]
            pickable: true
            property string caseGameKey: caseRoot.gameKey
            property bool caseIsCurrent: caseRoot.isCurrent
        }

        // Arte impressa da capa frontal (gira solidária ao pivô da tampa)
        Model {
            id: frontPrintMesh
            source: "#Rectangle"
            position: Qt.vector3d(caseRoot.caseWidth * 0.5 + 2.0, 0.0, caseRoot.printEpsilon)
            scale: Qt.vector3d((caseRoot.caseWidth - 12.0) / 100.0,
                               (caseRoot.caseHeight - 10.0) / 100.0,
                               1.0)
            materials: [ frontPrintMaterial ]
        }

        // Película plástica translúcida sobre a capa para brilho especular
        Model {
            id: frontSleeveFilm
            source: "#Rectangle"
            position: Qt.vector3d(caseRoot.caseWidth * 0.5 + 2.0, 0.0, caseRoot.printEpsilon * 2.0)
            scale: Qt.vector3d((caseRoot.caseWidth - 10.0) / 100.0,
                               (caseRoot.caseHeight - 8.0) / 100.0,
                               1.0)
            materials: [ clearSleeveMaterial ]
        }
    }
}
