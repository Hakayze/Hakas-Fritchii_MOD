// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QWidget>

class QQuickView;

class CinematicLibraryHost final : public QWidget {
    Q_OBJECT

public:
    explicit CinematicLibraryHost(QWidget* parent = nullptr);

    QString LoadScene();
    void FocusScene();

signals:
    void ReturnToClassic();
    void SceneFailed(const QString& message);

private:
    QQuickView* view = nullptr;
};
