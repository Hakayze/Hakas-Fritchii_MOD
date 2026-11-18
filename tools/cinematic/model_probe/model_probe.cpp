// SPDX-License-Identifier: GPL-3.0-or-later

#include "cinematic_game_model.h"

#include <limits>

#include <QAbstractItemModelTester>
#include <QCoreApplication>
#include <QMetaType>
#include <QSignalSpy>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QUrl>
#include <QtTest>

using FritchiiUi::CinematicGameModel;

namespace {

// Verified constants matching src/qt_common/game_list/game_list_p.h
constexpr int kTypeRole = Qt::UserRole + 1;
constexpr int kSortRole = Qt::UserRole + 2;
constexpr int kTitleRole = kSortRole + 1;
constexpr int kFullPathRole = kSortRole + 2;
constexpr int kProgramIdRole = kSortRole + 3;

constexpr int kGameType = QStandardItem::UserType + 1;
constexpr int kCustomDirType = QStandardItem::UserType + 2;
constexpr int kSdmcDirType = QStandardItem::UserType + 3;
constexpr int kAddDirType = QStandardItem::UserType + 6;
constexpr int kFavoritesType = QStandardItem::UserType + 7;

CinematicGameModel::SourceRoles makeVerifiedRoles() {
    return CinematicGameModel::SourceRoles{
        .typeRole = kTypeRole,
        .titleRole = kTitleRole,
        .pathRole = kFullPathRole,
        .programIdRole = kProgramIdRole,
        .gameType = kGameType,
        .favoritesType = kFavoritesType,
    };
}

QStandardItem* makeFolderItem(int folderType, const QString& label) {
    auto* item = new QStandardItem(label);
    item->setData(folderType, kTypeRole);
    return item;
}

QList<QStandardItem*> makeGameRow(const QString& title, const QString& path, quint64 programId) {
    auto* col0 = new QStandardItem(title);
    col0->setData(kGameType, kTypeRole);
    col0->setData(title, kSortRole);
    col0->setData(title, kTitleRole);
    col0->setData(path, kFullPathRole);
    col0->setData(static_cast<qulonglong>(programId), kProgramIdRole);

    // Extra columns simulating GameListModel::COLUMN_COUNT to prove column > 0 is ignored.
    auto* col1 = new QStandardItem(QStringLiteral("NSP"));
    col1->setData(kGameType, kTypeRole);
    col1->setData(QStringLiteral("Decoy Column 1"), kTitleRole);
    col1->setData(QStringLiteral("/decoy/col1.nsp"), kFullPathRole);
    col1->setData(static_cast<qulonglong>(0xDEADBEEFULL), kProgramIdRole);

    auto* col2 = new QStandardItem(QStringLiteral("1.5 GiB"));
    return {col0, col1, col2};
}

} // namespace

class CinematicModelProbe : public QObject {
    Q_OBJECT

private slots:
    void testTreeAndFlatWithNonGameItems();
    void testFavoritesDeduplicationAndTranslatedFolder();
    void testSameProgramIdDistinctPathsAndZeroId();
    void testSearchAndFavoritesFilter();
    void testNavigation500GamesWindowAndSignals();
    void testSelectionSurvivesReorderAndReset();
    void testRemovalAndStaleLaunchRejection();
    void testSourceModelDestruction();
};

void CinematicModelProbe::testTreeAndFlatWithNonGameItems() {
    QStandardItemModel source;
    source.setColumnCount(3);

    auto* customDir = makeFolderItem(kCustomDirType, QStringLiteral("My Games"));
    customDir->appendRow(
        makeGameRow(QStringLiteral("Ocarina"), QStringLiteral("/roms/oot.nsp"), 0x01007EF00011E000ULL));
    customDir->appendRow(
        makeGameRow(QStringLiteral("Metroid"), QStringLiteral("/roms/metroid.xci"), 0x010093801237C000ULL));

    // Malformed game item with empty path should be ignored.
    auto* brokenGame = new QStandardItem(QStringLiteral("Broken"));
    brokenGame->setData(kGameType, kTypeRole);
    brokenGame->setData(QStringLiteral("Broken"), kTitleRole);
    customDir->appendRow({brokenGame, new QStandardItem(), new QStandardItem()});

    auto* addDir = makeFolderItem(kAddDirType, QStringLiteral("Add New Game Directory"));
    auto* sdmcDir = makeFolderItem(kSdmcDirType, QStringLiteral("SDMC"));

    source.appendRow({customDir, new QStandardItem(), new QStandardItem()});
    source.appendRow({sdmcDir, new QStandardItem(), new QStandardItem()});
    source.appendRow({addDir, new QStandardItem(), new QStandardItem()});

    CinematicGameModel model(makeVerifiedRoles(), &source);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

    QCOMPARE(model.gameCount(), 2);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.selectedTitle(), QStringLiteral("Ocarina"));

    // Verify exact required QML role names and types.
    const QHash<int, QByteArray> roles = model.roleNames();
    QCOMPARE(roles.value(CinematicGameModel::GameKeyRole), QByteArrayLiteral("gameKey"));
    QCOMPARE(roles.value(CinematicGameModel::TitleTextRole), QByteArrayLiteral("titleText"));
    QCOMPARE(roles.value(CinematicGameModel::FrontCoverRole), QByteArrayLiteral("frontCover"));
    QCOMPARE(roles.value(CinematicGameModel::SpineCoverRole), QByteArrayLiteral("spineCover"));
    QCOMPARE(roles.value(CinematicGameModel::BackCoverRole), QByteArrayLiteral("backCover"));
    QCOMPARE(roles.value(CinematicGameModel::RelativeOffsetRole), QByteArrayLiteral("relativeOffset"));
    QCOMPARE(roles.value(CinematicGameModel::IsCurrentRole), QByteArrayLiteral("isCurrent"));

    const QModelIndex idx0 = model.index(0, 0);
    QCOMPARE(model.data(idx0, CinematicGameModel::TitleTextRole).toString(),
             QStringLiteral("Ocarina"));
    QCOMPARE(model.data(idx0, CinematicGameModel::FrontCoverRole).toUrl(), QUrl());
    QCOMPARE(model.data(idx0, CinematicGameModel::SpineCoverRole).toUrl(), QUrl());
    QCOMPARE(model.data(idx0, CinematicGameModel::BackCoverRole).toUrl(), QUrl());
    QCOMPARE(model.data(idx0, CinematicGameModel::RelativeOffsetRole).toReal(), 0.0);
    QCOMPARE(model.data(idx0, CinematicGameModel::IsCurrentRole).toBool(), true);

    const QVariant hexVar = model.data(idx0, CinematicGameModel::ProgramIdHexRole);
    QCOMPARE(hexVar.typeId(), QMetaType::QString);
    QCOMPARE(hexVar.toString(), QStringLiteral("01007EF00011E000"));
    QCOMPARE(hexVar.toString().size(), 16);

    // Switch to a flat source model (games directly under root + non-game item).
    QStandardItemModel flatSource;
    flatSource.setColumnCount(3);
    flatSource.appendRow(makeGameRow(QStringLiteral("Flat A"), QStringLiteral("/flat/a.nsp"), 1ULL));
    flatSource.appendRow({makeFolderItem(kAddDirType, QStringLiteral("Add Dir")),
                          new QStandardItem(), new QStandardItem()});
    flatSource.appendRow(makeGameRow(QStringLiteral("Flat B"), QStringLiteral("/flat/b.nsp"), 2ULL));

    model.setSourceModel(&flatSource);
    QCOMPARE(model.gameCount(), 2);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.data(model.index(0, 0), CinematicGameModel::TitleTextRole).toString(),
             QStringLiteral("Flat A"));
    QCOMPARE(model.data(model.index(1, 0), CinematicGameModel::TitleTextRole).toString(),
             QStringLiteral("Flat B"));
}

void CinematicModelProbe::testFavoritesDeduplicationAndTranslatedFolder() {
    QStandardItemModel source;
    source.setColumnCount(3);

    // Translated / arbitrary label; favorites detection must rely solely on favoritesType ancestry.
    auto* favFolder = makeFolderItem(kFavoritesType, QStringLiteral("お気に入り / Favoritos"));
    favFolder->appendRow(makeGameRow(QStringLiteral("Celeste"),
                                     QStringLiteral("C:\\Games\\.\\Celeste.nsp"),
                                     0x01002B30028F6000ULL));

    auto* dirFolder = makeFolderItem(kCustomDirType, QStringLiteral("Library"));
    dirFolder->appendRow(makeGameRow(QStringLiteral("Celeste"),
                                     QStringLiteral("C:/Games/Celeste.nsp"),
                                     0x01002B30028F6000ULL));
    dirFolder->appendRow(makeGameRow(QStringLiteral("Hollow Knight"),
                                     QStringLiteral("C:/Games/HollowKnight.nsp"),
                                     0x0100633007D48000ULL));

    // Also test a folder appearing before a second favorites entry.
    auto* favFolder2 = makeFolderItem(kFavoritesType, QStringLiteral("Starred"));
    auto* nestedGroup = makeFolderItem(kCustomDirType, QStringLiteral("Nested"));
    nestedGroup->appendRow(makeGameRow(QStringLiteral("Hollow Knight"),
                                       QStringLiteral("C:\\Games\\HollowKnight.nsp"),
                                       0x0100633007D48000ULL));
    favFolder2->appendRow({nestedGroup, new QStandardItem(), new QStandardItem()});

    dirFolder->appendRow(makeGameRow(QStringLiteral("Hades"),
                                     QStringLiteral("C:/Games/Hades.nsp"),
                                     0x0100535012974000ULL));

    source.appendRow({favFolder, new QStandardItem(), new QStandardItem()});
    source.appendRow({dirFolder, new QStandardItem(), new QStandardItem()});
    source.appendRow({favFolder2, new QStandardItem(), new QStandardItem()});

    CinematicGameModel model(makeVerifiedRoles(), &source);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

    QCOMPARE(model.gameCount(), 3);
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.data(model.index(0, 0), CinematicGameModel::TitleTextRole).toString(),
             QStringLiteral("Celeste"));
    QCOMPARE(model.data(model.index(0, 0), CinematicGameModel::FavoriteRole).toBool(), true);
    QCOMPARE(model.data(model.index(1, 0), CinematicGameModel::TitleTextRole).toString(),
             QStringLiteral("Hollow Knight"));
    QCOMPARE(model.data(model.index(1, 0), CinematicGameModel::FavoriteRole).toBool(), true);
    QCOMPARE(model.data(model.index(2, 0), CinematicGameModel::TitleTextRole).toString(),
             QStringLiteral("Hades"));
    QCOMPARE(model.data(model.index(2, 0), CinematicGameModel::FavoriteRole).toBool(), false);
}

void CinematicModelProbe::testSameProgramIdDistinctPathsAndZeroId() {
    QStandardItemModel source;
    source.setColumnCount(3);

    const quint64 sharedId = 0x01007EF00011E000ULL;
    source.appendRow(makeGameRow(QStringLiteral("BOTW Base"), QStringLiteral("/roms/v1/botw.nsp"), sharedId));
    source.appendRow(makeGameRow(QStringLiteral("BOTW Dump"), QStringLiteral("/roms/v2/botw.xci"), sharedId));
    source.appendRow(makeGameRow(QStringLiteral("Homebrew One"), QStringLiteral("/hb/one.nro"), 0ULL));
    source.appendRow(makeGameRow(QStringLiteral("Homebrew Two"), QStringLiteral("/hb/two.nro"), 0ULL));

    CinematicGameModel model(makeVerifiedRoles(), &source);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

    QCOMPARE(model.gameCount(), 4);
    QCOMPARE(model.rowCount(), 4);

    const QString key0 = model.data(model.index(0, 0), CinematicGameModel::GameKeyRole).toString();
    const QString key1 = model.data(model.index(1, 0), CinematicGameModel::GameKeyRole).toString();
    const QString key2 = model.data(model.index(2, 0), CinematicGameModel::GameKeyRole).toString();
    const QString key3 = model.data(model.index(3, 0), CinematicGameModel::GameKeyRole).toString();

    QVERIFY(key0 != key1);
    QVERIFY(key2 != key3);
    QCOMPARE(model.data(model.index(0, 0), CinematicGameModel::ProgramIdHexRole).toString(),
             QStringLiteral("01007EF00011E000"));
    QCOMPARE(model.data(model.index(1, 0), CinematicGameModel::ProgramIdHexRole).toString(),
             QStringLiteral("01007EF00011E000"));
    QCOMPARE(model.data(model.index(2, 0), CinematicGameModel::ProgramIdHexRole).toString(),
             QStringLiteral("0000000000000000"));
    QCOMPARE(model.data(model.index(3, 0), CinematicGameModel::ProgramIdHexRole).toString(),
             QStringLiteral("0000000000000000"));
}

void CinematicModelProbe::testSearchAndFavoritesFilter() {
    QStandardItemModel source;
    source.setColumnCount(3);

    auto* favDir = makeFolderItem(kFavoritesType, QStringLiteral("Favorites"));
    favDir->appendRow(makeGameRow(QStringLiteral("Super Mario Odyssey"),
                                  QStringLiteral("/roms/smo.nsp"), 0x0100000000010000ULL));

    auto* mainDir = makeFolderItem(kCustomDirType, QStringLiteral("Games"));
    mainDir->appendRow(makeGameRow(QStringLiteral("Super Mario Odyssey"),
                                   QStringLiteral("/roms/smo.nsp"), 0x0100000000010000ULL));
    mainDir->appendRow(makeGameRow(QStringLiteral("Mario Kart 8"),
                                   QStringLiteral("/roms/mk8.nsp"), 0x0100152000022000ULL));
    mainDir->appendRow(makeGameRow(QStringLiteral("Xenoblade Chronicles"),
                                   QStringLiteral("/roms/xbc.nsp"), 0x0100FF500E34A000ULL));

    source.appendRow({favDir, new QStandardItem(), new QStandardItem()});
    source.appendRow({mainDir, new QStandardItem(), new QStandardItem()});

    CinematicGameModel model(makeVerifiedRoles(), &source);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

    QCOMPARE(model.gameCount(), 3);

    // Select Mario Kart 8, then apply search "mario" -> selection must survive.
    model.navigate(1);
    const QString mk8Key = model.selectedKey();
    QCOMPARE(model.selectedTitle(), QStringLiteral("Mario Kart 8"));

    model.setSearchText(QStringLiteral("MARIO"));
    QCOMPARE(model.gameCount(), 2);
    QCOMPARE(model.selectedKey(), mk8Key);

    // Toggle favoritesOnly -> MK8 is not favorite, so fallback selects Super Mario Odyssey.
    model.setFavoritesOnly(true);
    QCOMPARE(model.gameCount(), 1);
    QCOMPARE(model.selectedTitle(), QStringLiteral("Super Mario Odyssey"));

    // Search with no match -> empty state.
    model.setSearchText(QStringLiteral("0"));
    QCOMPARE(model.gameCount(), 0);
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(model.selectedKey().isEmpty());
    QVERIFY(model.selectedTitle().isEmpty());

    // Clear filters -> full list restored with valid selection.
    model.setFavoritesOnly(false);
    model.setSearchText(QString());
    QCOMPARE(model.gameCount(), 3);
    QVERIFY(!model.selectedKey().isEmpty());
}

void CinematicModelProbe::testNavigation500GamesWindowAndSignals() {
    QStandardItemModel source;
    source.setColumnCount(3);
    for (int i = 0; i < 500; ++i) {
        const QString title = QStringLiteral("Game %1").arg(i, 3, 10, QLatin1Char('0'));
        const QString path = QStringLiteral("/roms/game_%1.nsp").arg(i);
        source.appendRow(makeGameRow(title, path, 0x0100000000000000ULL + static_cast<quint64>(i)));
    }

    CinematicGameModel model(makeVerifiedRoles(), &source);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

    QCOMPARE(model.gameCount(), 500);
    QCOMPARE(model.rowCount(), 8);
    QCOMPARE(model.selectedTitle(), QStringLiteral("Game 000"));

    QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);
    QSignalSpy insertedSpy(&model, &QAbstractItemModel::rowsInserted);
    QSignalSpy removedSpy(&model, &QAbstractItemModel::rowsRemoved);

    // Step forward 20 times and validate incremental window invariants.
    for (int step = 1; step <= 20; ++step) {
        model.navigate(1);
        QVERIFY(model.rowCount() <= 15);
        if (step >= 7) {
            QCOMPARE(model.rowCount(), 15);
        }
        int currentCount = 0;
        for (int r = 0; r < model.rowCount(); ++r) {
            const QModelIndex idx = model.index(r, 0);
            const bool isCur = model.data(idx, CinematicGameModel::IsCurrentRole).toBool();
            const qreal offset = model.data(idx, CinematicGameModel::RelativeOffsetRole).toReal();
            if (isCur) {
                ++currentCount;
                QCOMPARE(offset, 0.0);
                QCOMPARE(model.data(idx, CinematicGameModel::GameKeyRole).toString(),
                         model.selectedKey());
            }
            QVERIFY(offset >= -7.0 && offset <= 7.0);
        }
        QCOMPARE(currentCount, 1);
    }

    QCOMPARE(model.selectedTitle(), QStringLiteral("Game 020"));
    QCOMPARE(resetSpy.count(), 0);
    QVERIFY(insertedSpy.count() > 0);
    QVERIFY(removedSpy.count() > 0);

    // Extreme delta overflow checks.
    model.navigate(std::numeric_limits<int>::max());
    QCOMPARE(model.selectedTitle(), QStringLiteral("Game 499"));
    QCOMPARE(model.rowCount(), 8);
    QCOMPARE(model.data(model.index(7, 0), CinematicGameModel::IsCurrentRole).toBool(), true);
    QCOMPARE(model.data(model.index(0, 0), CinematicGameModel::RelativeOffsetRole).toReal(), -7.0);

    model.navigate(std::numeric_limits<int>::min());
    QCOMPARE(model.selectedTitle(), QStringLiteral("Game 000"));
    QCOMPARE(model.rowCount(), 8);
    QCOMPARE(model.data(model.index(0, 0), CinematicGameModel::IsCurrentRole).toBool(), true);
    QCOMPARE(model.data(model.index(7, 0), CinematicGameModel::RelativeOffsetRole).toReal(), 7.0);
    QCOMPARE(resetSpy.count(), 0);
}

void CinematicModelProbe::testSelectionSurvivesReorderAndReset() {
    QStandardItemModel source;
    source.setColumnCount(3);
    source.setSortRole(kSortRole);

    for (int i = 0; i < 20; ++i) {
        const QString title = QStringLiteral("Title %1").arg(i, 2, 10, QLatin1Char('0'));
        source.appendRow(makeGameRow(title, QStringLiteral("/roms/t_%1.nsp").arg(i),
                                     0x0100100000000000ULL + static_cast<quint64>(i)));
    }

    CinematicGameModel model(makeVerifiedRoles(), &source);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

    model.navigate(5);
    const QString targetKey = model.selectedKey();
    QCOMPARE(model.selectedTitle(), QStringLiteral("Title 05"));

    QSignalSpy titleSpy(&model, &CinematicGameModel::selectedTitleChanged);
    QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);

    // Burst mutations: update title + sort descending (emits dataChanged and layoutChanged).
    source.item(5, 0)->setData(QStringLiteral("Title 05 Updated"), kTitleRole);
    source.item(5, 0)->setData(QStringLiteral("Title 05 Updated"), kSortRole);
    source.sort(0, Qt::DescendingOrder);

    QTRY_COMPARE(model.selectedTitle(), QStringLiteral("Title 05 Updated"));
    QCOMPARE(model.selectedKey(), targetKey);
    QCOMPARE(titleSpy.count(), 1);
    QCOMPARE(resetSpy.count(), 0);

    // Full reset of source model where targetKey reappears at a different position.
    source.clear();
    source.setColumnCount(3);
    source.appendRow(makeGameRow(QStringLiteral("New First"), QStringLiteral("/roms/new_first.nsp"), 0x11ULL));
    source.appendRow(makeGameRow(QStringLiteral("Title 05 Reset"),
                                 QStringLiteral("/roms/t_5.nsp"), 0x0100100000000005ULL));
    source.appendRow(makeGameRow(QStringLiteral("New Last"), QStringLiteral("/roms/new_last.nsp"), 0x22ULL));

    QTRY_COMPARE(model.gameCount(), 3);
    QCOMPARE(model.selectedKey(), targetKey);
    QCOMPARE(model.selectedTitle(), QStringLiteral("Title 05 Reset"));
}

void CinematicModelProbe::testRemovalAndStaleLaunchRejection() {
    QStandardItemModel source;
    source.setColumnCount(3);
    source.appendRow(makeGameRow(QStringLiteral("Alpha"), QStringLiteral("/roms/a.nsp"), 0xAULL));
    source.appendRow(makeGameRow(QStringLiteral("Beta"), QStringLiteral("/roms/b.nsp"), 0xBULL));
    source.appendRow(makeGameRow(QStringLiteral("Gamma"), QStringLiteral("/roms/c.nsp"), 0xCULL));

    CinematicGameModel model(makeVerifiedRoles(), &source);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

    const QString keyAlpha = model.selectedKey();
    model.navigate(1);
    const QString keyBeta = model.selectedKey();
    QCOMPARE(model.selectedTitle(), QStringLiteral("Beta"));

    QSignalSpy launchSpy(&model, &CinematicGameModel::launchGameRequested);

    // Valid launch of currently selected item.
    model.requestLaunch(keyBeta);
    QCOMPARE(launchSpy.count(), 1);
    QCOMPARE(launchSpy.at(0).at(0).toString(), QStringLiteral("/roms/b.nsp"));
    QCOMPARE(launchSpy.at(0).at(1).toULongLong(), static_cast<qulonglong>(0xBULL));

    // Non-current or unknown keys must not launch.
    model.requestLaunch(keyAlpha);
    model.requestLaunch(QStringLiteral("0000000000000000|/does/not/exist.nsp"));
    QCOMPARE(launchSpy.count(), 1);

    // Remove Beta from source; wait for queued rebuild and verify neighbor Gamma is selected.
    source.removeRow(1);
    QTRY_COMPARE(model.gameCount(), 2);
    QCOMPARE(model.selectedTitle(), QStringLiteral("Gamma"));
    const QString keyGamma = model.selectedKey();

    // Old removed key must be rejected.
    model.requestLaunch(keyBeta);
    QCOMPARE(launchSpy.count(), 1);

    // Remove Gamma and immediately attempt launch BEFORE event loop spins (stale-cache race guard).
    source.removeRow(1);
    model.requestLaunch(keyGamma);
    QCOMPARE(launchSpy.count(), 1);
    QCOMPARE(model.gameCount(), 1);
    QCOMPARE(model.selectedTitle(), QStringLiteral("Alpha"));

    // Remove final item -> empty selection and no launch possible.
    source.removeRow(0);
    QTRY_COMPARE(model.gameCount(), 0);
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(model.selectedKey().isEmpty());
    QVERIFY(model.selectedTitle().isEmpty());
    model.requestLaunch(keyAlpha);
    QCOMPARE(launchSpy.count(), 1);
}

void CinematicModelProbe::testSourceModelDestruction() {
    auto* source = new QStandardItemModel();
    source->setColumnCount(3);
    source->appendRow(makeGameRow(QStringLiteral("Transient"),
                                  QStringLiteral("/roms/transient.nsp"), 0x1234ULL));

    CinematicGameModel model(makeVerifiedRoles(), source);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

    const QString oldKey = model.selectedKey();
    QCOMPARE(model.gameCount(), 1);
    QVERIFY(!oldKey.isEmpty());

    QSignalSpy launchSpy(&model, &CinematicGameModel::launchGameRequested);
    QSignalSpy sourceChangedSpy(&model, &CinematicGameModel::sourceModelChanged);

    // Trigger a pending queued rebuild and immediately destroy source before it runs.
    source->appendRow(makeGameRow(QStringLiteral("Pending"), QStringLiteral("/roms/p.nsp"), 0x5678ULL));
    delete source;
    QCoreApplication::processEvents();

    QCOMPARE(model.sourceModel(), nullptr);
    QCOMPARE(model.gameCount(), 0);
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(model.selectedKey().isEmpty());
    QVERIFY(model.selectedTitle().isEmpty());
    QCOMPARE(sourceChangedSpy.count(), 1);

    model.requestLaunch(oldKey);
    QCOMPARE(launchSpy.count(), 0);
}

QTEST_GUILESS_MAIN(CinematicModelProbe)
#include "model_probe.moc"
