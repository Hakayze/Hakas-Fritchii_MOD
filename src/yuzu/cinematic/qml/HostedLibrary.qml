// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

CinematicLibrary {
    required property var adapter

    gameModel: adapter
    gameCount: adapter ? adapter.gameCount : 0
    selectedKey: adapter ? adapter.selectedKey : ""
    selectedTitle: adapter ? adapter.selectedTitle : ""
    inputEnabled: Window.window !== null && Window.window.visible && Window.active
    onNavigateRequested: delta => adapter.navigate(delta)
    onSelectRequested: key => adapter.select(key)
    onLaunchRequested: key => adapter.requestLaunch(key)
}
