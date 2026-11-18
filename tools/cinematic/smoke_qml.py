#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Exercise the isolated cinematic QML contract with Qt 6.8.3 and synthetic data.

The null backend validates objects/input. Quick 3D cannot render with that backend
and prints that limitation; graphics, performance and Windows hosting are untested.
Requires the development-only dependency PySide6==6.8.3.
"""
import os
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from pathlib import Path
import sys
from PySide6.QtCore import QAbstractListModel, QCoreApplication, QEvent, QModelIndex, QObject, QPoint, Qt, QUrl, qVersion
from PySide6.QtGui import QGuiApplication
from PySide6.QtQuick import QQuickView, QQuickWindow, QSGRendererInterface
from PySide6.QtTest import QTest


class WindowModel(QAbstractListModel):
    names = ("gameKey", "titleText", "frontCover", "spineCover", "backCover", "relativeOffset", "isCurrent")

    def __init__(self):
        super().__init__()
        self.size = 15

    def rowCount(self, parent=QModelIndex()):
        return 0 if parent.isValid() else self.size

    def roleNames(self):
        return {Qt.UserRole + i + 1: name.encode() for i, name in enumerate(self.names)}

    def data(self, index, role):
        if not index.isValid() or not Qt.UserRole < role <= Qt.UserRole + len(self.names):
            return None
        row = index.row()
        values = (f"game-{row}", f"Synthetic game {row}", QUrl(), QUrl(), QUrl(), float(row - 7), row == 7)
        return values[role - Qt.UserRole - 1]


def main():
    assert qVersion() == "6.8.3", f"Expected Qt 6.8.3, found {qVersion()}"
    QQuickWindow.setGraphicsApi(QSGRendererInterface.GraphicsApi.Null)
    app = QGuiApplication(sys.argv)
    model = WindowModel()
    view = QQuickView()
    warnings = []
    view.engine().warnings.connect(lambda items: warnings.extend(e.toString() for e in items))
    view.setResizeMode(QQuickView.SizeRootObjectToView)
    view.resize(1100, 680)
    view.setInitialProperties(dict(gameModel=model, gameCount=500, selectedKey="game-7", selectedTitle="Synthetic game 7"))
    path = Path(__file__).resolve().parents[2] / "src/yuzu/cinematic/qml/CinematicLibrary.qml"
    view.setSource(QUrl.fromLocalFile(str(path)))
    assert view.status() == QQuickView.Ready, [e.toString() for e in view.errors()]
    root = view.rootObject()
    assert root.objectName() == "cinematicLibrary"
    events = {"navigate": [], "select": [], "launch": [], "classic": []}
    root.navigateRequested.connect(lambda value: events["navigate"].append(value))
    root.selectRequested.connect(lambda value: events["select"].append(value))
    root.launchRequested.connect(lambda value: events["launch"].append(value))
    root.returnToClassic.connect(lambda: events["classic"].append(True))
    view.show()
    view.requestActivate()
    root.forceActiveFocus()
    QTest.qWait(100)
    cases = [o for o in root.findChildren(QObject) if o.metaObject().className().startswith("GameCase_QMLTYPE")]
    assert len(cases) == 15, f"Expected 15 cases, found {len(cases)}"
    selected_case = next(o for o in cases if o.property("isCurrent"))
    camera = next(o for o in root.findChildren(QObject) if o.metaObject().className() == "QQuick3DFrustumCamera")
    assert abs(camera.property("top") - 0.42 * 680 * 10 / 1800) < 0.0001
    assert abs(camera.property("bottom") + 0.58 * 680 * 10 / 1800) < 0.0001

    def key(value, modifier=Qt.NoModifier):
        QTest.keyClick(view, value, modifier)
        app.processEvents()

    key(Qt.Key_Right)
    assert events["navigate"] == [1]
    key(Qt.Key_Right, Qt.ControlModifier)
    assert events["navigate"] == [1]
    key(Qt.Key_Return)
    assert root.property("detailsOpen") and not events["launch"]
    key(Qt.Key_Down)
    key(Qt.Key_Return)
    assert root.property("inspecting")
    assert selected_case.property("inspectOpen")
    key(Qt.Key_Escape)
    assert not root.property("inspecting") and root.property("detailsOpen")
    assert not selected_case.property("inspectOpen")
    key(Qt.Key_Escape)
    assert not root.property("detailsOpen")
    key(Qt.Key_Escape)
    assert len(events["classic"]) == 1

    key(Qt.Key_Return)
    key(Qt.Key_Return)
    assert events["launch"] == ["game-7"]
    root.setProperty("inputEnabled", False)
    before = {name: list(values) for name, values in events.items()}
    for value in (Qt.Key_Left, Qt.Key_Right, Qt.Key_Return, Qt.Key_Escape):
        key(value)
    QTest.mouseClick(view, Qt.LeftButton, Qt.NoModifier, QPoint(988, 27))
    app.processEvents()
    assert events == before, "Disabled input emitted a command"

    root.setProperty("inputEnabled", True)
    root.setProperty("loading", True)
    assert not root.property("detailsOpen")
    key(Qt.Key_Return)
    assert events["launch"] == ["game-7"]
    root.setProperty("loading", False)
    model.beginResetModel()
    model.size = 0
    model.endResetModel()
    QCoreApplication.sendPostedEvents(None, QEvent.DeferredDelete)
    root.setProperty("gameCount", 0)
    root.setProperty("selectedKey", "")
    key(Qt.Key_Return)
    assert not root.property("detailsOpen")
    assert not [o for o in root.findChildren(QObject) if o.metaObject().className().startswith("GameCase_QMLTYPE")]
    root.setWidth(0)
    root.setHeight(0)
    app.processEvents()
    assert abs(camera.property("top") - 0.42 * 10 / 1800) < 0.0001
    assert not warnings, "\n".join(warnings)
    print("PASS Qt 6.8.3: load, 15-case window, keyboard, details/inspection/Escape, launch key, disabled input, loading, empty/reset and zero viewport")
    view.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
