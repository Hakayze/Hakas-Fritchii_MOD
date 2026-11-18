// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QMetaObject>
#include <QPointer>
#include <QStandardItemModel>
#include <QString>
#include <QUrl>
#include <QVector>

namespace FritchiiUi {

class CinematicGameModel : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(int gameCount READ gameCount NOTIFY gameCountChanged)
    Q_PROPERTY(QString selectedKey READ selectedKey NOTIFY selectedKeyChanged)
    Q_PROPERTY(QString selectedTitle READ selectedTitle NOTIFY selectedTitleChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(
        bool favoritesOnly READ favoritesOnly WRITE setFavoritesOnly NOTIFY favoritesOnlyChanged)

public:
    struct SourceRoles {
        int typeRole;
        int titleRole;
        int pathRole;
        int programIdRole;
        int gameType;
        int favoritesType;
    };

    enum Role {
        GameKeyRole = Qt::UserRole + 1,
        TitleTextRole,
        FrontCoverRole,
        SpineCoverRole,
        BackCoverRole,
        RelativeOffsetRole,
        IsCurrentRole,
        ProgramIdHexRole,
        FavoriteRole,
    };
    Q_ENUM(Role)

    static constexpr int kWindowRadius = 7;

    explicit CinematicGameModel(SourceRoles roles, QObject* parent = nullptr);
    CinematicGameModel(SourceRoles roles, QStandardItemModel* sourceModel,
                       QObject* parent = nullptr);
    ~CinematicGameModel() override;

    QStandardItemModel* sourceModel() const;
    void setSourceModel(QStandardItemModel* model);

    int gameCount() const;
    QString selectedKey() const;
    QString selectedTitle() const;

    QString searchText() const;
    void setSearchText(const QString& text);

    bool favoritesOnly() const;
    void setFavoritesOnly(bool favoritesOnly);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void navigate(int delta);
    Q_INVOKABLE void select(const QString& key);
    Q_INVOKABLE void requestLaunch(const QString& key);

signals:
    void sourceModelChanged();
    void gameCountChanged();
    void selectedKeyChanged();
    void selectedTitleChanged();
    void searchTextChanged();
    void favoritesOnlyChanged();
    void launchGameRequested(const QString& path, qulonglong programId);

private:
    struct GameEntry {
        QString gameKey;
        QString titleText;
        QString rawPath;
        QString normalizedPath;
        quint64 programId = 0;
        QString programIdHex;
        bool favorite = false;
    };

    struct WindowEntry {
        QString gameKey;
        QString titleText;
        QString programIdHex;
        bool favorite = false;
        qreal relativeOffset = 0.0;
        bool isCurrent = false;
    };

    static QString normalizePath(const QString& rawPath);
    static QString formatProgramIdHex(quint64 programId);
    static QString makeGameKey(const QString& programIdHex, const QString& normalizedPath);

    int itemTypeOf(const QStandardItem* item) const;
    void disconnectSource();
    void onSourceDestroyed();
    void scheduleRebuild();
    void rebuildNow();
    void collectFromItem(const QStandardItem* parentItem, bool inFavorites,
                         QVector<GameEntry>& outEntries,
                         QHash<QString, qsizetype>& keyToIndex) const;
    void applyFiltersAndSync();
    void applyFilteredEntries(QVector<GameEntry> nextFiltered);
    void applySelectionIndex(int nextIndex);
    void syncWindow();

    SourceRoles m_roles;
    QPointer<QStandardItemModel> m_sourceModel;
    QVector<QMetaObject::Connection> m_sourceConnections;

    QVector<GameEntry> m_allEntries;
    QVector<GameEntry> m_filtered;
    QVector<WindowEntry> m_window;

    int m_selectedIndex = -1;
    QString m_selectedKey;
    QString m_selectedTitle;
    QString m_searchText;
    bool m_favoritesOnly = false;
    bool m_rebuildScheduled = false;
};

} // namespace FritchiiUi
