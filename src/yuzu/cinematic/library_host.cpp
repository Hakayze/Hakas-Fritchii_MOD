// SPDX-License-Identifier: GPL-3.0-or-later

#include <QColor>
#include <QQmlError>
#include <QQuickItem>
#include <QQuickView>
#include <QSGRendererInterface>
#include <QStringList>
#include <QUrl>
#include <QVBoxLayout>

#include "yuzu/cinematic/library_host.h"

CinematicLibraryHost::CinematicLibraryHost(QWidget* parent) : QWidget{parent} {
    view = new QQuickView;
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->setColor(QColor{QStringLiteral("#121217")});
    view->setPersistentGraphics(false);
    view->setPersistentSceneGraph(false);

    connect(view, &QQuickView::statusChanged, this, [this](QQuickView::Status status) {
        if (status != QQuickView::Error) {
            return;
        }
        QStringList errors;
        for (const auto& error : view->errors()) {
            errors.append(error.toString());
        }
        emit SceneFailed(errors.join(QLatin1Char('\n')));
    });
    connect(view, &QQuickWindow::sceneGraphError, this,
            [this](QQuickWindow::SceneGraphError, const QString& message) {
                emit SceneFailed(message);
            });

    auto* container = QWidget::createWindowContainer(view, this);
    container->setFocusPolicy(Qt::StrongFocus);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(container);
    setFocusProxy(container);
}

QString CinematicLibraryHost::LoadScene() {
    if (QQuickWindow::sceneGraphBackend() == QStringLiteral("software") ||
        QQuickWindow::graphicsApi() == QSGRendererInterface::Software) {
        return tr("Qt Quick 3D is unavailable with the software backend. Use the Classic view.");
    }

    view->setSource(QUrl{QStringLiteral("qrc:/qt/qml/Hakas/Cinematic/CalibrationScene.qml")});
    if (view->status() == QQuickView::Error) {
        QStringList errors;
        for (const auto& error : view->errors()) {
            errors.append(error.toString());
        }
        return errors.join(QLatin1Char('\n'));
    }
    if (!view->rootObject()) {
        return tr("The cinematic prototype did not create its root item.");
    }
    if (!connect(view->rootObject(), SIGNAL(returnToClassic()), this, SIGNAL(ReturnToClassic()))) {
        return tr("The cinematic prototype could not connect its return action.");
    }
    return {};
}

void CinematicLibraryHost::FocusScene() {
    if (!isVisible()) {
        return;
    }
    setFocus(Qt::OtherFocusReason);
    view->requestActivate();
    if (view->rootObject()) {
        view->rootObject()->forceActiveFocus(Qt::OtherFocusReason);
    }
}
