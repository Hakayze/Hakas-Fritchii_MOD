// SPDX-License-Identifier: GPL-3.0-or-later

#include "cinematic_game_model.h"

#include <algorithm>
#include <utility>

#include <QDir>
#include <QMetaObject>
#include <QSet>
#include <QStandardItem>

namespace FritchiiUi {

CinematicGameModel::CinematicGameModel(SourceRoles roles, QObject* parent)
    : QAbstractListModel(parent), m_roles(roles) {}

CinematicGameModel::CinematicGameModel(SourceRoles roles, QStandardItemModel* sourceModel,
                                       QObject* parent)
    : QAbstractListModel(parent), m_roles(roles) {
    setSourceModel(sourceModel);
}

CinematicGameModel::~CinematicGameModel() {
    disconnectSource();
}

QStandardItemModel* CinematicGameModel::sourceModel() const {
    return m_sourceModel.data();
}

void CinematicGameModel::setSourceModel(QStandardItemModel* model) {
    if (m_sourceModel.data() == model) {
        return;
    }

    disconnectSource();
    m_sourceModel = model;
    m_rebuildScheduled = false;

    if (m_sourceModel) {
        m_sourceConnections.reserve(7);
        m_sourceConnections.push_back(connect(m_sourceModel.data(), &QAbstractItemModel::modelReset,
                                              this, [this]() { scheduleRebuild(); }));
        m_sourceConnections.push_back(
            connect(m_sourceModel.data(), &QAbstractItemModel::layoutChanged, this,
                    [this]() { scheduleRebuild(); }));
        m_sourceConnections.push_back(
            connect(m_sourceModel.data(), &QAbstractItemModel::rowsInserted, this,
                    [this]() { scheduleRebuild(); }));
        m_sourceConnections.push_back(
            connect(m_sourceModel.data(), &QAbstractItemModel::rowsRemoved, this,
                    [this]() { scheduleRebuild(); }));
        m_sourceConnections.push_back(connect(m_sourceModel.data(), &QAbstractItemModel::rowsMoved,
                                              this, [this]() { scheduleRebuild(); }));
        m_sourceConnections.push_back(connect(
            m_sourceModel.data(), &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>&) {
                if (!topLeft.isValid() || !bottomRight.isValid() || topLeft.column() == 0) {
                    scheduleRebuild();
                }
            }));
        m_sourceConnections.push_back(connect(m_sourceModel.data(), &QObject::destroyed, this,
                                              [this]() { onSourceDestroyed(); }));
    }

    rebuildNow();
    emit sourceModelChanged();
}

int CinematicGameModel::gameCount() const {
    return static_cast<int>(m_filtered.size());
}

QString CinematicGameModel::selectedKey() const {
    return m_selectedKey;
}

QString CinematicGameModel::selectedTitle() const {
    return m_selectedTitle;
}

QString CinematicGameModel::searchText() const {
    return m_searchText;
}

void CinematicGameModel::setSearchText(const QString& text) {
    if (m_searchText == text) {
        return;
    }
    m_searchText = text;
    emit searchTextChanged();

    if (m_rebuildScheduled) {
        rebuildNow();
    } else {
        applyFiltersAndSync();
    }
}

bool CinematicGameModel::favoritesOnly() const {
    return m_favoritesOnly;
}

void CinematicGameModel::setFavoritesOnly(bool favoritesOnly) {
    if (m_favoritesOnly == favoritesOnly) {
        return;
    }
    m_favoritesOnly = favoritesOnly;
    emit favoritesOnlyChanged();

    if (m_rebuildScheduled) {
        rebuildNow();
    } else {
        applyFiltersAndSync();
    }
}

int CinematicGameModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_window.size());
}

QVariant CinematicGameModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.parent().isValid() || index.column() != 0) {
        return {};
    }
    const int row = index.row();
    if (row < 0 || row >= m_window.size()) {
        return {};
    }

    const WindowEntry& entry = m_window[row];
    switch (role) {
    case Qt::DisplayRole:
    case TitleTextRole:
        return entry.titleText;
    case GameKeyRole:
        return entry.gameKey;
    case FrontCoverRole:
    case SpineCoverRole:
    case BackCoverRole:
        return QUrl();
    case RelativeOffsetRole:
        return entry.relativeOffset;
    case IsCurrentRole:
        return entry.isCurrent;
    case ProgramIdHexRole:
        return entry.programIdHex;
    case FavoriteRole:
        return entry.favorite;
    default:
        return {};
    }
}

QHash<int, QByteArray> CinematicGameModel::roleNames() const {
    return {
        {GameKeyRole, QByteArrayLiteral("gameKey")},
        {TitleTextRole, QByteArrayLiteral("titleText")},
        {FrontCoverRole, QByteArrayLiteral("frontCover")},
        {SpineCoverRole, QByteArrayLiteral("spineCover")},
        {BackCoverRole, QByteArrayLiteral("backCover")},
        {RelativeOffsetRole, QByteArrayLiteral("relativeOffset")},
        {IsCurrentRole, QByteArrayLiteral("isCurrent")},
        {ProgramIdHexRole, QByteArrayLiteral("programIdHex")},
        {FavoriteRole, QByteArrayLiteral("favorite")},
    };
}

void CinematicGameModel::navigate(int delta) {
    if (m_rebuildScheduled) {
        rebuildNow();
    }
    const qint64 count = m_filtered.size();
    if (count <= 0) {
        return;
    }
    const qint64 current = (m_selectedIndex >= 0 && m_selectedIndex < count) ? m_selectedIndex : 0;
    const qint64 clamped =
        std::clamp<qint64>(current + static_cast<qint64>(delta), 0, count - 1);
    const int target = static_cast<int>(clamped);
    if (target == m_selectedIndex && m_selectedKey == m_filtered[target].gameKey) {
        return;
    }
    applySelectionIndex(target);
}

void CinematicGameModel::select(const QString& key) {
    if (m_rebuildScheduled) {
        rebuildNow();
    }
    if (key.isEmpty()) {
        return;
    }
    for (qsizetype i = 0; i < m_filtered.size(); ++i) {
        if (m_filtered[i].gameKey == key) {
            applySelectionIndex(static_cast<int>(i));
            return;
        }
    }
}

void CinematicGameModel::requestLaunch(const QString& key) {
    if (m_rebuildScheduled) {
        rebuildNow();
    }
    if (key.isEmpty() || !m_sourceModel) {
        return;
    }
    if (m_selectedIndex < 0 || m_selectedIndex >= m_filtered.size()) {
        return;
    }

    const GameEntry& current = m_filtered[m_selectedIndex];
    if (current.gameKey != key || current.gameKey != m_selectedKey) {
        return;
    }
    if (current.rawPath.trimmed().isEmpty()) {
        return;
    }

    emit launchGameRequested(current.rawPath, static_cast<qulonglong>(current.programId));
}

QString CinematicGameModel::normalizePath(const QString& rawPath) {
    QString normalized = rawPath.trimmed();
    normalized.replace(QLatin1Char('\\'), QLatin1Char('/'));
    normalized = QDir::cleanPath(normalized);
#ifdef Q_OS_WIN
    normalized = normalized.toLower();
#endif
    return normalized;
}

QString CinematicGameModel::formatProgramIdHex(quint64 programId) {
    return QStringLiteral("%1").arg(programId, 16, 16, QLatin1Char('0')).toUpper();
}

QString CinematicGameModel::makeGameKey(const QString& programIdHex,
                                        const QString& normalizedPath) {
    return programIdHex + QLatin1Char('|') + normalizedPath;
}

int CinematicGameModel::itemTypeOf(const QStandardItem* item) const {
    if (!item) {
        return -1;
    }
    const QVariant typeVar = item->data(m_roles.typeRole);
    if (typeVar.isValid()) {
        bool ok = false;
        const int parsed = typeVar.toInt(&ok);
        if (ok) {
            return parsed;
        }
    }
    return item->type();
}

void CinematicGameModel::disconnectSource() {
    for (const QMetaObject::Connection& conn : m_sourceConnections) {
        disconnect(conn);
    }
    m_sourceConnections.clear();
}

void CinematicGameModel::onSourceDestroyed() {
    m_sourceConnections.clear();
    m_sourceModel = nullptr;
    m_rebuildScheduled = false;
    m_allEntries.clear();
    applyFilteredEntries({});
    emit sourceModelChanged();
}

void CinematicGameModel::scheduleRebuild() {
    if (m_rebuildScheduled) {
        return;
    }
    m_rebuildScheduled = true;
    QMetaObject::invokeMethod(
        this,
        [this]() {
            if (!m_rebuildScheduled) {
                return;
            }
            rebuildNow();
        },
        Qt::QueuedConnection);
}

void CinematicGameModel::rebuildNow() {
    m_rebuildScheduled = false;
    m_allEntries.clear();

    if (m_sourceModel) {
        QHash<QString, qsizetype> keyToIndex;
        collectFromItem(m_sourceModel->invisibleRootItem(), false, m_allEntries, keyToIndex);
    }

    applyFiltersAndSync();
}

void CinematicGameModel::collectFromItem(const QStandardItem* parentItem, bool inFavorites,
                                         QVector<GameEntry>& outEntries,
                                         QHash<QString, qsizetype>& keyToIndex) const {
    if (!parentItem) {
        return;
    }

    const int rows = parentItem->rowCount();
    for (int row = 0; row < rows; ++row) {
        const QStandardItem* item = parentItem->child(row, 0);
        if (!item) {
            continue;
        }

        const int type = itemTypeOf(item);
        if (type == m_roles.gameType) {
            const QString rawPath = item->data(m_roles.pathRole).toString();
            if (rawPath.trimmed().isEmpty()) {
                continue;
            }

            const QVariant idVar = item->data(m_roles.programIdRole);
            bool idOk = false;
            const quint64 parsedId = idVar.isValid() ? idVar.toULongLong(&idOk) : 0ULL;
            const quint64 safeProgramId = idOk ? parsedId : 0ULL;

            const QString normalizedPath = normalizePath(rawPath);
            const QString hexId = formatProgramIdHex(safeProgramId);
            const QString key = makeGameKey(hexId, normalizedPath);

            QString title = item->data(m_roles.titleRole).toString();
            if (title.isEmpty()) {
                title = item->text();
            }

            const auto existingIt = keyToIndex.constFind(key);
            if (existingIt != keyToIndex.constEnd()) {
                GameEntry& existing = outEntries[existingIt.value()];
                existing.favorite = existing.favorite || inFavorites;
                if (existing.titleText.isEmpty() && !title.isEmpty()) {
                    existing.titleText = title;
                }
                continue;
            }

            GameEntry entry;
            entry.gameKey = key;
            entry.titleText = title;
            entry.rawPath = rawPath;
            entry.normalizedPath = normalizedPath;
            entry.programId = safeProgramId;
            entry.programIdHex = hexId;
            entry.favorite = inFavorites;

            keyToIndex.insert(key, outEntries.size());
            outEntries.push_back(std::move(entry));
        } else {
            const bool childInFavorites = inFavorites || (type == m_roles.favoritesType);
            if (item->rowCount() > 0) {
                collectFromItem(item, childInFavorites, outEntries, keyToIndex);
            }
        }
    }
}

void CinematicGameModel::applyFiltersAndSync() {
    const QString needle = m_searchText.trimmed();
    QVector<GameEntry> nextFiltered;
    nextFiltered.reserve(m_allEntries.size());

    for (const GameEntry& entry : m_allEntries) {
        if (m_favoritesOnly && !entry.favorite) {
            continue;
        }
        if (!needle.isEmpty() && !entry.titleText.contains(needle, Qt::CaseInsensitive)) {
            continue;
        }
        nextFiltered.push_back(entry);
    }

    applyFilteredEntries(std::move(nextFiltered));
}

void CinematicGameModel::applyFilteredEntries(QVector<GameEntry> nextFiltered) {
    const int oldGameCount = static_cast<int>(m_filtered.size());
    const int oldSelectedIndex = m_selectedIndex;
    const QString oldSelectedKey = m_selectedKey;
    const QString oldSelectedTitle = m_selectedTitle;

    m_filtered = std::move(nextFiltered);

    int nextSelectedIndex = -1;
    if (!m_filtered.isEmpty()) {
        if (!oldSelectedKey.isEmpty()) {
            for (qsizetype i = 0; i < m_filtered.size(); ++i) {
                if (m_filtered[i].gameKey == oldSelectedKey) {
                    nextSelectedIndex = static_cast<int>(i);
                    break;
                }
            }
        }
        if (nextSelectedIndex < 0) {
            const int fallback = (oldSelectedIndex >= 0) ? oldSelectedIndex : 0;
            nextSelectedIndex =
                std::clamp(fallback, 0, static_cast<int>(m_filtered.size()) - 1);
        }
    }

    m_selectedIndex = nextSelectedIndex;
    if (m_selectedIndex >= 0) {
        m_selectedKey = m_filtered[m_selectedIndex].gameKey;
        m_selectedTitle = m_filtered[m_selectedIndex].titleText;
    } else {
        m_selectedKey.clear();
        m_selectedTitle.clear();
    }

    syncWindow();

    if (static_cast<int>(m_filtered.size()) != oldGameCount) {
        emit gameCountChanged();
    }
    if (m_selectedKey != oldSelectedKey) {
        emit selectedKeyChanged();
    }
    if (m_selectedTitle != oldSelectedTitle) {
        emit selectedTitleChanged();
    }
}

void CinematicGameModel::applySelectionIndex(int nextIndex) {
    if (nextIndex < 0 || nextIndex >= m_filtered.size()) {
        return;
    }

    const QString oldSelectedKey = m_selectedKey;
    const QString oldSelectedTitle = m_selectedTitle;

    m_selectedIndex = nextIndex;
    m_selectedKey = m_filtered[m_selectedIndex].gameKey;
    m_selectedTitle = m_filtered[m_selectedIndex].titleText;

    syncWindow();

    if (m_selectedKey != oldSelectedKey) {
        emit selectedKeyChanged();
    }
    if (m_selectedTitle != oldSelectedTitle) {
        emit selectedTitleChanged();
    }
}

void CinematicGameModel::syncWindow() {
    QVector<WindowEntry> targetWindow;
    if (!m_filtered.isEmpty() && m_selectedIndex >= 0 && m_selectedIndex < m_filtered.size()) {
        const int first = std::max(0, m_selectedIndex - kWindowRadius);
        const int last =
            std::min(static_cast<int>(m_filtered.size()) - 1, m_selectedIndex + kWindowRadius);
        targetWindow.reserve(last - first + 1);

        for (int i = first; i <= last; ++i) {
            const GameEntry& src = m_filtered[i];
            WindowEntry win;
            win.gameKey = src.gameKey;
            win.titleText = src.titleText;
            win.programIdHex = src.programIdHex;
            win.favorite = src.favorite;
            win.relativeOffset = static_cast<qreal>(i - m_selectedIndex);
            win.isCurrent = (i == m_selectedIndex);
            targetWindow.push_back(std::move(win));
        }
    }

    // Phase 1: Remove rows no longer present in the target window (back-to-front contiguous runs).
    QSet<QString> targetKeys;
    targetKeys.reserve(targetWindow.size());
    for (const WindowEntry& entry : targetWindow) {
        targetKeys.insert(entry.gameKey);
    }

    for (qsizetype i = m_window.size() - 1; i >= 0;) {
        if (targetKeys.contains(m_window[i].gameKey)) {
            --i;
            continue;
        }
        const qsizetype removeEnd = i;
        while (i > 0 && !targetKeys.contains(m_window[i - 1].gameKey)) {
            --i;
        }
        const qsizetype removeStart = i;

        beginRemoveRows(QModelIndex(), static_cast<int>(removeStart), static_cast<int>(removeEnd));
        m_window.remove(removeStart, removeEnd - removeStart + 1);
        endRemoveRows();
        --i;
    }

    // Phase 2: Reorder surviving rows to match targetWindow order using beginMoveRows.
    QSet<QString> survivingKeys;
    survivingKeys.reserve(m_window.size());
    for (const WindowEntry& entry : m_window) {
        survivingKeys.insert(entry.gameKey);
    }

    QVector<QString> desiredSurvivingOrder;
    desiredSurvivingOrder.reserve(m_window.size());
    for (const WindowEntry& entry : targetWindow) {
        if (survivingKeys.contains(entry.gameKey)) {
            desiredSurvivingOrder.push_back(entry.gameKey);
        }
    }

    for (qsizetype dest = 0; dest < desiredSurvivingOrder.size(); ++dest) {
        qsizetype src = dest;
        while (src < m_window.size() && m_window[src].gameKey != desiredSurvivingOrder[dest]) {
            ++src;
        }
        if (src < m_window.size() && src != dest) {
            beginMoveRows(QModelIndex(), static_cast<int>(src), static_cast<int>(src),
                          QModelIndex(), static_cast<int>(dest));
            WindowEntry moved = m_window.takeAt(src);
            m_window.insert(dest, std::move(moved));
            endMoveRows();
        }
    }

    // Phase 3: Insert newly visible rows from targetWindow in contiguous runs.
    qsizetype targetIdx = 0;
    while (targetIdx < targetWindow.size()) {
        if (targetIdx < m_window.size() &&
            m_window[targetIdx].gameKey == targetWindow[targetIdx].gameKey) {
            ++targetIdx;
            continue;
        }

        const qsizetype runStart = targetIdx;
        qsizetype runEnd = runStart;
        if (runStart < m_window.size()) {
            const QString& nextExistingKey = m_window[runStart].gameKey;
            while (runEnd + 1 < targetWindow.size() &&
                   targetWindow[runEnd + 1].gameKey != nextExistingKey) {
                ++runEnd;
            }
        } else {
            runEnd = targetWindow.size() - 1;
        }

        beginInsertRows(QModelIndex(), static_cast<int>(runStart), static_cast<int>(runEnd));
        for (qsizetype i = runStart; i <= runEnd; ++i) {
            m_window.insert(i, targetWindow[i]);
        }
        endInsertRows();

        targetIdx = runEnd + 1;
    }

    // Phase 4: Apply all modified role values coherently before emitting dataChanged.
    struct RowChange {
        int row;
        QList<int> roles;
    };
    QVector<RowChange> changes;
    changes.reserve(m_window.size());

    for (qsizetype i = 0; i < m_window.size(); ++i) {
        const WindowEntry& oldEntry = m_window[i];
        const WindowEntry& newEntry = targetWindow[i];
        QList<int> roles;

        if (oldEntry.titleText != newEntry.titleText) {
            roles.append(TitleTextRole);
            roles.append(Qt::DisplayRole);
        }
        if (oldEntry.programIdHex != newEntry.programIdHex) {
            roles.append(ProgramIdHexRole);
        }
        if (oldEntry.favorite != newEntry.favorite) {
            roles.append(FavoriteRole);
        }
        if (oldEntry.relativeOffset != newEntry.relativeOffset) {
            roles.append(RelativeOffsetRole);
        }
        if (oldEntry.isCurrent != newEntry.isCurrent) {
            roles.append(IsCurrentRole);
        }

        if (!roles.isEmpty()) {
            m_window[i] = newEntry;
            changes.push_back(RowChange{static_cast<int>(i), std::move(roles)});
        }
    }

    qsizetype c = 0;
    while (c < changes.size()) {
        const int startRow = changes[c].row;
        int endRow = startRow;
        QList<int> mergedRoles = changes[c].roles;
        ++c;

        while (c < changes.size() && changes[c].row == endRow + 1) {
            endRow = changes[c].row;
            for (int r : changes[c].roles) {
                if (!mergedRoles.contains(r)) {
                    mergedRoles.append(r);
                }
            }
            ++c;
        }

        emit dataChanged(index(startRow, 0), index(endRow, 0), mergedRoles);
    }
}

} // namespace FritchiiUi
