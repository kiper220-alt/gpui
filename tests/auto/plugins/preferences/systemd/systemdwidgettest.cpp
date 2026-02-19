#include "systemdwidgettest.h"

#include <memory>
#include <QMessageBox>
#include <QTimer>

#include "../../../../../src/plugins/preferences/systemd/systemditem.h"
#include "../../../../../src/plugins/preferences/systemd/systemdwidget.h"
#include "../../../../../src/plugins/preferences/preferencessnapinprivate.h"

namespace gpui
{
void PreferencesSnapInPrivate::onDataSave() {}
}

namespace
{

using preferences::SystemdEditMode;
using preferences::SystemdItem;
using preferences::SystemdUnitType;
using preferences::SystemdWidget;

struct Fixture
{
    std::unique_ptr<SystemdItem> item;
    std::unique_ptr<SystemdWidget> widget;
};

Fixture createFixture(SystemdEditMode mode, SystemdUnitType unitType = SystemdUnitType::Service)
{
    Fixture fixture;
    fixture.item = std::make_unique<SystemdItem>();
    fixture.item->setProperty(SystemdItem::EDIT, true);
    fixture.item->setProperty(SystemdItem::EDIT_MODE, static_cast<int>(mode));
    fixture.item->setProperty(SystemdItem::UNIT_TYPE, static_cast<int>(unitType));
    fixture.item->setProperty(SystemdItem::HAS_DEPENDENCIES, SystemdItem::unitTypeHasFileDependencies(static_cast<int>(unitType)));
    fixture.item->setProperty(SystemdItem::UNIT, std::string("demo"));

    fixture.widget = std::make_unique<SystemdWidget>(nullptr, fixture.item.get());
    fixture.widget->setItem(fixture.item.get());
    return fixture;
}

QTableWidget *actionTable(const Fixture &fixture)
{
    return fixture.widget->findChild<QTableWidget *>("actionsTableWidget");
}

QTableWidget *dependencyTable(const Fixture &fixture)
{
    return fixture.widget->findChild<QTableWidget *>("dependenciesTableWidget");
}

int findRow(const QTableWidget *table, const QString &section, const QString &key)
{
    for (int row = 0; row < table->rowCount(); ++row)
    {
        const auto *sectionItem = table->item(row, 1);
        const auto *keyItem = table->item(row, 2);
        if (!sectionItem || !keyItem)
        {
            continue;
        }

        if (sectionItem->text().trimmed().compare(section, Qt::CaseInsensitive) == 0
            && keyItem->text().trimmed().compare(key, Qt::CaseInsensitive) == 0)
        {
            return row;
        }
    }

    return -1;
}

QComboBox *strategyCombo(const QTableWidget *table, int row)
{
    return qobject_cast<QComboBox *>(table->cellWidget(row, 0));
}

QString keyTextAt(const QTableWidget *table, int row)
{
    const auto *item = table ? table->item(row, 2) : nullptr;
    return item ? item->text().trimmed() : QString();
}

void closeMessageBoxesAsync()
{
    QTimer::singleShot(0, []() {
        for (auto *widget : QApplication::topLevelWidgets())
        {
            if (auto *messageBox = qobject_cast<QMessageBox *>(widget))
            {
                messageBox->accept();
            }
        }
    });
}

QComboBox *createEditorForCell(QTableWidget *table, int row, int column)
{
    if (!table)
    {
        return nullptr;
    }

    const QModelIndex index = table->model()->index(row, column);
    if (!index.isValid())
    {
        return nullptr;
    }

    auto *delegate = table->itemDelegateForColumn(column);
    if (!delegate)
    {
        return nullptr;
    }

    auto *editor = qobject_cast<QComboBox *>(delegate->createEditor(table, QStyleOptionViewItem(), index));
    if (!editor)
    {
        return nullptr;
    }

    delegate->setEditorData(editor, index);
    return editor;
}

} // namespace

namespace tests
{

void SystemdWidgetTest::scaffoldCreateModes()
{
    {
        auto fixture = createFixture(SystemdEditMode::Create);
        auto *table = actionTable(fixture);
        QVERIFY(table);

        const int execStartRow = findRow(table, "Service", "ExecStart");
        QVERIFY(execStartRow >= 0);

        auto *combo = strategyCombo(table, execStartRow);
        QVERIFY(combo);
        QCOMPARE(combo->count(), 2);
        QCOMPARE(combo->findText("Reset key"), -1);
    }

    {
        auto fixture = createFixture(SystemdEditMode::CreateOrOverride);
        auto *table = actionTable(fixture);
        QVERIFY(table);

        const int execStartRow = findRow(table, "Service", "ExecStart");
        QVERIFY(execStartRow >= 0);

        auto *combo = strategyCombo(table, execStartRow);
        QVERIFY(combo);
        QCOMPARE(combo->count(), 2);
    }
}

void SystemdWidgetTest::noScaffoldInOverride()
{
    auto fixture = createFixture(SystemdEditMode::Override);
    auto *table = actionTable(fixture);
    QVERIFY(table);
    QCOMPARE(table->rowCount(), 0);
}

void SystemdWidgetTest::autocompleteEditorsUseCatalog()
{
    auto fixture = createFixture(SystemdEditMode::Override);
    auto *table = actionTable(fixture);
    QVERIFY(table);

    fixture.widget->on_actionAddButton_clicked();
    QVERIFY(table->rowCount() > 0);

    std::unique_ptr<QComboBox> sectionEditor(createEditorForCell(table, 0, 1));
    QVERIFY(sectionEditor);
    QVERIFY(sectionEditor->findText("Service") >= 0);
    QVERIFY(sectionEditor->findText("Unit") >= 0);

    table->item(0, 1)->setText("Service");
    std::unique_ptr<QComboBox> keyEditor(createEditorForCell(table, 0, 2));
    QVERIFY(keyEditor);
    QVERIFY(keyEditor->findText("ExecStart") >= 0);
}

void SystemdWidgetTest::strictMandatoryKeyLocked()
{
    auto fixture = createFixture(SystemdEditMode::Create, SystemdUnitType::Mount);
    auto *table = actionTable(fixture);
    QVERIFY(table);

    const int whatRow = findRow(table, "Mount", "What");
    const int whereRow = findRow(table, "Mount", "Where");
    QVERIFY(whatRow >= 0);
    QVERIFY(whereRow >= 0);

    const auto *whatSectionItem = table->item(whatRow, 1);
    const auto *whatKeyItem = table->item(whatRow, 2);
    QVERIFY(whatSectionItem);
    QVERIFY(whatKeyItem);
    QVERIFY(!(whatSectionItem->flags() & Qt::ItemIsEditable));
    QVERIFY(!(whatKeyItem->flags() & Qt::ItemIsEditable));
}

void SystemdWidgetTest::oneOfScaffoldKeySelectableWithinGroupOnly()
{
    auto fixture = createFixture(SystemdEditMode::Create, SystemdUnitType::Service);
    auto *table = actionTable(fixture);
    QVERIFY(table);

    const int row = findRow(table, "Service", "ExecStart");
    QVERIFY(row >= 0);

    const auto *sectionItem = table->item(row, 1);
    const auto *keyItem = table->item(row, 2);
    QVERIFY(sectionItem);
    QVERIFY(keyItem);
    QVERIFY(!(sectionItem->flags() & Qt::ItemIsEditable));
    QVERIFY(keyItem->flags() & Qt::ItemIsEditable);

    std::unique_ptr<QComboBox> keyEditor(createEditorForCell(table, row, 2));
    QVERIFY(keyEditor);
    QVERIFY(keyEditor->findText("ExecStart") >= 0);
    QVERIFY(keyEditor->findText("ExecStop") >= 0);
    QCOMPARE(keyEditor->findText("Description"), -1);

    auto *delegate = table->itemDelegateForColumn(2);
    QVERIFY(delegate);
    const QModelIndex keyIndex = table->model()->index(row, 2);
    QVERIFY(keyIndex.isValid());

    const QString originalKey = keyTextAt(table, row);
    keyEditor->setEditText("Description");
    delegate->setModelData(keyEditor.get(), table->model(), keyIndex);
    QCOMPARE(keyTextAt(table, row), originalKey);

    keyEditor->setEditText("ExecStop");
    delegate->setModelData(keyEditor.get(), table->model(), keyIndex);
    QCOMPARE(keyTextAt(table, row), QString("ExecStop"));
}

void SystemdWidgetTest::removeMandatoryRows()
{
    auto fixture = createFixture(SystemdEditMode::Create);
    auto *table = actionTable(fixture);
    QVERIFY(table);

    int execStartRow = findRow(table, "Service", "ExecStart");
    QVERIFY(execStartRow >= 0);

    table->clearSelection();
    table->selectRow(execStartRow);
    closeMessageBoxesAsync();
    fixture.widget->on_actionRemoveButton_clicked();
    QCOMPARE(table->rowCount(), 1);

    fixture.widget->on_actionAddButton_clicked();
    const int addedRow = table->rowCount() - 1;
    QVERIFY(table->item(addedRow, 1));
    QVERIFY(table->item(addedRow, 2));
    QVERIFY(table->item(addedRow, 3));
    table->item(addedRow, 1)->setText("Service");
    table->item(addedRow, 2)->setText("ExecStop");
    table->item(addedRow, 3)->setText("/bin/true");

    execStartRow = findRow(table, "Service", "ExecStart");
    QVERIFY(execStartRow >= 0);
    table->clearSelection();
    table->selectRow(execStartRow);
    closeMessageBoxesAsync();
    fixture.widget->on_actionRemoveButton_clicked();
    QCOMPARE(table->rowCount(), 2);
    QVERIFY(findRow(table, "Service", "ExecStart") >= 0);

    table->clearSelection();
    table->selectRow(0);
    closeMessageBoxesAsync();
    fixture.widget->on_actionRemoveButton_clicked();
    QCOMPARE(table->rowCount(), 2);

    fixture.widget->on_actionsClearButton_clicked();
    QVERIFY(table->rowCount() >= 1);
    QVERIFY(findRow(table, "Service", "ExecStart") >= 0);
}

void SystemdWidgetTest::validationForMandatoryRows()
{
    auto fixture = createFixture(SystemdEditMode::Create);
    auto *table = actionTable(fixture);
    QVERIFY(table);

    closeMessageBoxesAsync();
    QVERIFY(!fixture.widget->validate());

    const int execStartRow = findRow(table, "Service", "ExecStart");
    QVERIFY(execStartRow >= 0);
    QVERIFY(table->item(execStartRow, 3));
    table->item(execStartRow, 3)->setText("/bin/true");

    QVERIFY(fixture.widget->validate());
}

void SystemdWidgetTest::moveActionsRowsNoCrash()
{
    auto fixture = createFixture(SystemdEditMode::Override, SystemdUnitType::Service);
    auto *table = actionTable(fixture);
    QVERIFY(table);

    fixture.widget->on_actionAddButton_clicked();
    fixture.widget->on_actionAddButton_clicked();
    fixture.widget->on_actionAddButton_clicked();
    QCOMPARE(table->rowCount(), 3);

    table->item(0, 3)->setText("v0");
    table->item(1, 3)->setText("v1");
    table->item(2, 3)->setText("v2");

    table->clearSelection();
    table->selectRow(1);
    QVERIFY(QMetaObject::invokeMethod(fixture.widget.get(), "on_actionMoveUpButton_clicked"));
    QCOMPARE(table->item(0, 3)->text(), QString("v1"));
    QCOMPARE(table->item(1, 3)->text(), QString("v0"));
    QCOMPARE(table->item(2, 3)->text(), QString("v2"));

    table->clearSelection();
    table->selectRow(0);
    QVERIFY(QMetaObject::invokeMethod(fixture.widget.get(), "on_actionMoveDownButton_clicked"));
    QCOMPARE(table->item(0, 3)->text(), QString("v0"));
    QCOMPARE(table->item(1, 3)->text(), QString("v1"));
    QCOMPARE(table->item(2, 3)->text(), QString("v2"));
}

void SystemdWidgetTest::moveDependencyRowsNoCrash()
{
    auto fixture = createFixture(SystemdEditMode::Override, SystemdUnitType::Service);
    auto *table = dependencyTable(fixture);
    QVERIFY(table);

    fixture.widget->on_dependAddButton_clicked();
    fixture.widget->on_dependAddButton_clicked();
    fixture.widget->on_dependAddButton_clicked();
    QCOMPARE(table->rowCount(), 3);

    auto *combo0 = qobject_cast<QComboBox *>(table->cellWidget(0, 0));
    auto *combo1 = qobject_cast<QComboBox *>(table->cellWidget(1, 0));
    auto *combo2 = qobject_cast<QComboBox *>(table->cellWidget(2, 0));
    QVERIFY(combo0);
    QVERIFY(combo1);
    QVERIFY(combo2);
    combo0->setCurrentIndex(0);
    combo1->setCurrentIndex(1);
    combo2->setCurrentIndex(0);

    table->setItem(0, 1, new QTableWidgetItem("d0"));
    table->setItem(1, 1, new QTableWidgetItem("d1"));
    table->setItem(2, 1, new QTableWidgetItem("d2"));

    table->clearSelection();
    table->selectRow(1);
    QVERIFY(QMetaObject::invokeMethod(fixture.widget.get(), "on_dependMoveUpButton_clicked"));
    QCOMPARE(table->item(0, 1)->text(), QString("d1"));
    QCOMPARE(table->item(1, 1)->text(), QString("d0"));
    QCOMPARE(qobject_cast<QComboBox *>(table->cellWidget(0, 0))->currentIndex(), 1);
    QCOMPARE(qobject_cast<QComboBox *>(table->cellWidget(1, 0))->currentIndex(), 0);

    table->clearSelection();
    table->selectRow(0);
    QVERIFY(QMetaObject::invokeMethod(fixture.widget.get(), "on_dependMoveDownButton_clicked"));
    QCOMPARE(table->item(0, 1)->text(), QString("d0"));
    QCOMPARE(table->item(1, 1)->text(), QString("d1"));
    QCOMPARE(qobject_cast<QComboBox *>(table->cellWidget(0, 0))->currentIndex(), 0);
    QCOMPARE(qobject_cast<QComboBox *>(table->cellWidget(1, 0))->currentIndex(), 1);
}

} // namespace tests

QTEST_MAIN(tests::SystemdWidgetTest)
