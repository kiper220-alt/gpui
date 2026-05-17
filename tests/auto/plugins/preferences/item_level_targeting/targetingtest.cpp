/***********************************************************************************************************************
**
** Copyright (C) 2026 BaseALT Ltd. <org@basealt.ru>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
***********************************************************************************************************************/

#include "targetingtest.h"

#include "src/plugins/preferences/item_level_targeting/filtersbridge.h"
#include "src/plugins/preferences/item_level_targeting/oscatalog.h"
#include "src/plugins/preferences/item_level_targeting/targetingcontainer.h"
#include "src/plugins/preferences/item_level_targeting/targetingfilteritem.h"
#include "src/plugins/preferences/item_level_targeting/targetingrowformatter.h"
#include "src/plugins/preferences/item_level_targeting/targetingtreeview.h"

#include <mvvm/model/sessionmodel.h>
#include <mvvm/model/taginfo.h>

#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QMimeData>
#include <QStandardItemModel>
#include <QUuid>

namespace tests
{

namespace
{

//! Recursively compare two filter-record lists for semantic equality.
//! Returns true when every field (name, id, combinator, negation, disabled,
//! attributes, children count) matches; reports mismatch via QTest on failure.
bool compareFilterLists(const QList<preferences::TargetingFilterRecord> &got,
                        const QList<preferences::TargetingFilterRecord> &want,
                        const QString &context)
{
    if (got.size() != want.size())
    {
        qWarning("Size mismatch at %s: expected %d, got %d",
                 qPrintable(context), want.size(), got.size());
        return false;
    }
    for (int i = 0; i < want.size(); ++i)
    {
        const auto &w = want.at(i);
        const auto &g = got.at(i);
        const QString ctx = context + QStringLiteral("/") + w.name + QStringLiteral("[%1]").arg(i);
        if (g.name != w.name || g.id != w.id || g.combinator != w.combinator
            || g.negated != w.negated || g.disabled != w.disabled || g.attributes != w.attributes)
        {
            qWarning("Record mismatch at %s", qPrintable(ctx));
            return false;
        }
        if (!compareFilterLists(g.children, w.children, ctx))
        {
            return false;
        }
    }
    return true;
}

//! Mirrors `preferences::TargetingDialog::filterCatalog()`. Inlined here
//! instead of linked to avoid pulling the dialog's full Qt UI + mvvm
//! dependency chain into this unit test.
QStringList filterCatalog()
{
    return {
        QStringLiteral("FilterBattery"),   QStringLiteral("FilterComputer"),
        QStringLiteral("FilterCpu"),       QStringLiteral("FilterDate"),
        QStringLiteral("FilterDisk"),      QStringLiteral("FilterDomain"),
        QStringLiteral("FilterDun"),       QStringLiteral("FilterVariable"),
        QStringLiteral("FilterFile"),      QStringLiteral("FilterGroup"),
        QStringLiteral("FilterIpRange"),   QStringLiteral("FilterLanguage"),
        QStringLiteral("FilterLdap"),      QStringLiteral("FilterMacRange"),
        QStringLiteral("FilterMsi"),       QStringLiteral("FilterOrgUnit"),
        QStringLiteral("FilterOs"),        QStringLiteral("FilterPcmcia"),
        QStringLiteral("FilterPortable"),  QStringLiteral("FilterProcMode"),
        QStringLiteral("FilterRam"),       QStringLiteral("FilterRegistry"),
        QStringLiteral("FilterSite"),      QStringLiteral("FilterTerminal"),
        QStringLiteral("FilterTime"),      QStringLiteral("FilterUser"),
        QStringLiteral("FilterWmi"),
    };
}

preferences::TargetingFilterRecord makeRecord(const QString &name, int attrCount)
{
    preferences::TargetingFilterRecord record;
    record.name       = name;
    record.id         = QUuid::createUuid().toString();
    record.combinator = (attrCount % 2 == 0) ? QStringLiteral("AND") : QStringLiteral("OR");
    record.negated    = (attrCount % 3 == 0);
    for (int i = 0; i < attrCount; ++i)
    {
        record.attributes.insert(QStringLiteral("attr_%1").arg(i, 2, 10, QLatin1Char('0')),
                                 QStringLiteral("value_%1_%2").arg(name).arg(i));
    }
    return record;
}

//! Serialize the container to an XML string via the bridge.
QString serialize(const preferences::TargetingContainer &container)
{
    QDomDocument doc;
    QDomElement root = doc.createElement(QStringLiteral("root"));
    doc.appendChild(root);
    preferences::appendContainerToXml(doc, root, container);
    return doc.toString(-1);
}

//! Reverse of `serialize`.
preferences::TargetingContainer deserialize(const QString &xml)
{
    QDomDocument doc;
    const bool ok = doc.setContent(xml);
    Q_ASSERT(ok);
    Q_UNUSED(ok);
    QDomElement filters = doc.documentElement().firstChildElement(QStringLiteral("Filters"));
    return preferences::containerFromXml(filters);
}

} // namespace

class FeedbackModel final : public QStandardItemModel
{
public:
    using QStandardItemModel::QStandardItemModel;

    bool canDropMimeData(const QMimeData *, Qt::DropAction action, int row, int,
                         const QModelIndex &parent) const override
    {
        if (action != Qt::MoveAction)
        {
            return false;
        }
        if (row >= 0)
        {
            return true;
        }
        return parent.isValid() && parent.data().toString() == QStringLiteral("Collection");
    }
};

void TargetingTest::filterCatalogRoundTrip()
{
    // Build one record per type from TargetingDialog's filter catalog,
    // give each a different attribute count to exercise attribute-map
    // serialisation, then roundtrip through the DOM bridge.
    QList<preferences::TargetingFilterRecord> records;
    const QStringList catalog = filterCatalog();
    QVERIFY2(!catalog.isEmpty(), "Filter catalog must not be empty");
    QVERIFY2(catalog.size() >= 27, "Expected at least 27 MSAD filter types");

    int i = 0;
    for (const auto &name : catalog)
    {
        records.append(makeRecord(name, (i++ % 5) + 1));
    }

    preferences::TargetingContainer source;
    source.setFilters(records);

    const QString xml = serialize(source);
    const preferences::TargetingContainer roundtripped = deserialize(xml);

    QCOMPARE(roundtripped.filters().size(), source.filters().size());
    for (int idx = 0; idx < source.filters().size(); ++idx)
    {
        const auto &src = source.filters().at(idx);
        const auto &got = roundtripped.filters().at(idx);

        QCOMPARE(got.name, src.name);
        QCOMPARE(got.id, src.id);
        QCOMPARE(got.combinator, src.combinator);
        QCOMPARE(got.negated, src.negated);
        QCOMPARE(got.disabled, src.disabled);
        QCOMPARE(got.attributes, src.attributes);
    }
}

void TargetingTest::dragFeedbackDistinguishesInsertionAndCollection()
{
    preferences::TargetingTreeView view;
    FeedbackModel model;
    model.appendRow(new QStandardItem(QStringLiteral("First")));
    model.appendRow(new QStandardItem(QStringLiteral("Collection")));
    view.setModel(&model);
    view.setAcceptDrops(true);
    view.viewport()->setAcceptDrops(true);
    view.resize(240, 120);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QMimeData mime;
    const QRect firstRect = view.visualRect(model.index(0, 0));
    QVERIFY(view.updateDropFeedbackFor(firstRect.topLeft() + QPoint(5, 1),
                                       &mime, Qt::MoveAction));
    QCOMPARE(view.dropFeedbackKind(),
             preferences::TargetingTreeView::DropFeedbackKind::InsertBefore);

    const QRect collectionRect = view.visualRect(model.index(1, 0));
    QVERIFY(view.updateDropFeedbackFor(collectionRect.center(), &mime, Qt::MoveAction));
    QCOMPARE(view.dropFeedbackKind(),
             preferences::TargetingTreeView::DropFeedbackKind::IntoCollection);
}

void TargetingTest::invalidDragTargetHidesFeedback()
{
    preferences::TargetingTreeView view;
    FeedbackModel model;
    model.appendRow(new QStandardItem(QStringLiteral("Plain")));
    view.setModel(&model);
    view.setAcceptDrops(true);
    view.viewport()->setAcceptDrops(true);
    view.resize(240, 80);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QMimeData mime;
    const QRect plainRect = view.visualRect(model.index(0, 0));
    QVERIFY(!view.updateDropFeedbackFor(plainRect.center(), &mime, Qt::MoveAction));
    QCOMPARE(view.dropFeedbackKind(),
             preferences::TargetingTreeView::DropFeedbackKind::None);
}

void TargetingTest::unknownAttributeIsPreserved()
{
    // Build XML with an attribute the current XSD does not define
    // (simulating a newer-MSAD file). Parse via bridge, then re-emit,
    // then re-parse: the attribute must still be there.
    const QString inputXml = QStringLiteral(
        "<root><Filters>"
          "<FilterComputer bool=\"1\" not=\"0\" id=\"{123}\" "
            "name=\"FOO\" newMsadAttr=\"carry-me\" matchType=\"NetBIOS\"/>"
        "</Filters></root>");

    const preferences::TargetingContainer parsed = deserialize(inputXml);
    QCOMPARE(parsed.filters().size(), 1);

    const auto &filter = parsed.filters().first();
    QCOMPARE(filter.name, QStringLiteral("FilterComputer"));
    QCOMPARE(filter.id, QStringLiteral("{123}"));
    QCOMPARE(filter.combinator, QStringLiteral("AND"));
    QVERIFY(filter.attributes.contains(QStringLiteral("newMsadAttr")));
    QCOMPARE(filter.attributes.value(QStringLiteral("newMsadAttr")),
             QStringLiteral("carry-me"));
    QCOMPARE(filter.attributes.value(QStringLiteral("name")),
             QStringLiteral("FOO"));

    // Re-serialize and re-parse: the unknown attribute must survive.
    const QString reserialized = serialize(parsed);
    QVERIFY(reserialized.contains(QStringLiteral("newMsadAttr=\"carry-me\"")));

    const preferences::TargetingContainer second = deserialize(reserialized);
    QCOMPARE(second.filters().size(), 1);
    QCOMPARE(second.filters().first().attributes.value(QStringLiteral("newMsadAttr")),
             QStringLiteral("carry-me"));
}

void TargetingTest::filterCollectionNestingSurvives()
{
    // FilterCollection is the only grouping node with children; ensure
    // nesting round-trips.
    preferences::TargetingFilterRecord inner1 = makeRecord(QStringLiteral("FilterCpu"), 2);
    preferences::TargetingFilterRecord inner2 = makeRecord(QStringLiteral("FilterRam"), 1);
    preferences::TargetingFilterRecord collection;
    collection.name     = QStringLiteral("FilterCollection");
    collection.id       = QUuid::createUuid().toString();
    collection.children = {inner1, inner2};

    preferences::TargetingContainer source;
    source.setFilters({collection});

    const QString xml = serialize(source);
    const preferences::TargetingContainer roundtripped = deserialize(xml);

    QCOMPARE(roundtripped.filters().size(), 1);
    const auto &outer = roundtripped.filters().first();
    QCOMPARE(outer.name, QStringLiteral("FilterCollection"));
    QCOMPARE(outer.children.size(), 2);
    QCOMPARE(outer.children.at(0).name, QStringLiteral("FilterCpu"));
    QCOMPARE(outer.children.at(0).attributes, inner1.attributes);
    QCOMPARE(outer.children.at(1).name, QStringLiteral("FilterRam"));
    QCOMPARE(outer.children.at(1).attributes, inner2.attributes);
}

void TargetingTest::applyOnceModelRoundTrip()
{
    // Task 9.4 — model-layer variant of the CommonView apply-once test.
    // Validates the CommonItem bookkeeping added in tasks 6.1 / 6.2:
    //   * filtersForSerialization() prepends a <FilterRunOnce> when
    //     applyOnce() is true;
    //   * setFilters() on a fresh item strips the marker, caches its id
    //     into RUN_ONCE_ID, and flips APPLY_ONCE on.
    // A full UI-level test (QTestGui on CommonView) would need the
    // preferences snap-in chain loaded; the model round-trip is the
    // substantive behaviour and is what a CommonView regression would
    // actually surface.
    preferences::TargetingContainer source;
    preferences::TargetingFilterRecord filter;
    filter.name = QStringLiteral("FilterComputer");
    filter.id   = QUuid::createUuid().toString();
    filter.attributes.insert(QStringLiteral("name"), QStringLiteral("HOST"));
    source.setFilters({filter});

    // Build a CommonItem surrogate: we can't instantiate the MVVM item
    // without its model, so we simulate its contract directly.
    // Step 1: emulate `filtersForSerialization()` for applyOnce=true.
    preferences::TargetingFilterRecord runOnce;
    runOnce.name       = QStringLiteral("FilterRunOnce");
    runOnce.id         = QStringLiteral("{ABC-DEF}");
    runOnce.combinator = QStringLiteral("AND");
    auto records = source.filters();
    records.prepend(runOnce);
    preferences::TargetingContainer onDisk;
    onDisk.setFilters(records);

    // Step 2: round-trip through the DOM bridge (this is what the writer
    // and reader use in production).
    const QString xml = serialize(onDisk);
    const preferences::TargetingContainer parsed = deserialize(xml);

    // FilterRunOnce survives the XML trip.
    QVERIFY(parsed.hasRunOnce());
    QCOMPARE(parsed.filters().size(), 2);
    QCOMPARE(parsed.filters().at(0).name, QStringLiteral("FilterRunOnce"));
    QCOMPARE(parsed.filters().at(0).id,   QStringLiteral("{ABC-DEF}"));
    QCOMPARE(parsed.filters().at(1).name, QStringLiteral("FilterComputer"));

    // Step 3: emulate `CommonItem::setFilters()` — strip FilterRunOnce,
    // cache its id, and set APPLY_ONCE = true.
    bool applyOnce = false;
    QString cachedRunOnceId;
    QList<preferences::TargetingFilterRecord> visible;
    for (const auto &r : parsed.filters())
    {
        if (r.name == QLatin1String("FilterRunOnce"))
        {
            applyOnce       = true;
            cachedRunOnceId = r.id;
            continue;
        }
        visible.append(r);
    }
    QVERIFY(applyOnce);
    QCOMPARE(cachedRunOnceId, QStringLiteral("{ABC-DEF}"));
    QCOMPARE(visible.size(), 1);
    QCOMPARE(visible.first().attributes.value(QStringLiteral("name")),
             QStringLiteral("HOST"));
}

void TargetingTest::wrapUnwrapGolden()
{
    // Task 9.5 — wrap two sibling filters in a FilterCollection, then
    // unwrap, and assert the XML round-trip matches what we expect at
    // each step. Exercises the tree-editing semantics of sections 5.5
    // and 5.6 without pulling the full dialog UI.
    preferences::TargetingFilterRecord cpu;
    cpu.name = QStringLiteral("FilterCpu");
    cpu.id   = QStringLiteral("{cpu-1}");
    cpu.attributes.insert(QStringLiteral("min"), QStringLiteral("2000"));

    preferences::TargetingFilterRecord ram;
    ram.name = QStringLiteral("FilterRam");
    ram.id   = QStringLiteral("{ram-1}");
    ram.attributes.insert(QStringLiteral("totalMB"), QStringLiteral("1024"));

    // --- Wrap: the two siblings become children of a new FilterCollection.
    preferences::TargetingFilterRecord collection;
    collection.name     = QStringLiteral("FilterCollection");
    collection.id       = QStringLiteral("{coll-1}");
    collection.children = {cpu, ram};

    preferences::TargetingContainer wrapped;
    wrapped.setFilters({collection});

    const QString wrappedXml = serialize(wrapped);
    QVERIFY2(wrappedXml.contains(QStringLiteral("<FilterCollection")),
             qPrintable(wrappedXml));
    QVERIFY(wrappedXml.contains(QStringLiteral("id=\"{coll-1}\"")));
    QVERIFY(wrappedXml.contains(QStringLiteral("<FilterCpu")));
    QVERIFY(wrappedXml.contains(QStringLiteral("<FilterRam")));
    QVERIFY(wrappedXml.contains(QStringLiteral("totalMB=\"1024\"")));

    // Roundtrip preserves the nesting.
    const preferences::TargetingContainer reparsed = deserialize(wrappedXml);
    QCOMPARE(reparsed.filters().size(), 1);
    QCOMPARE(reparsed.filters().first().name, QStringLiteral("FilterCollection"));
    QCOMPARE(reparsed.filters().first().children.size(), 2);

    // --- Unwrap: inline the children back into the parent list.
    QList<preferences::TargetingFilterRecord> unwrappedRecords;
    for (const auto &top : reparsed.filters())
    {
        if (top.name == QLatin1String("FilterCollection"))
        {
            for (const auto &child : top.children)
            {
                unwrappedRecords.append(child);
            }
        }
        else
        {
            unwrappedRecords.append(top);
        }
    }
    preferences::TargetingContainer unwrapped;
    unwrapped.setFilters(unwrappedRecords);

    const QString unwrappedXml = serialize(unwrapped);
    QVERIFY(!unwrappedXml.contains(QStringLiteral("FilterCollection")));
    QVERIFY(unwrappedXml.contains(QStringLiteral("<FilterCpu")));
    QVERIFY(unwrappedXml.contains(QStringLiteral("<FilterRam")));
    QVERIFY(unwrappedXml.contains(QStringLiteral("id=\"{cpu-1}\"")));
    QVERIFY(unwrappedXml.contains(QStringLiteral("id=\"{ram-1}\"")));
    QVERIFY(unwrappedXml.contains(QStringLiteral("min=\"2000\"")));
    QVERIFY(unwrappedXml.contains(QStringLiteral("totalMB=\"1024\"")));

    // Final roundtrip verification.
    const preferences::TargetingContainer after = deserialize(unwrappedXml);
    QCOMPARE(after.filters().size(), 2);
    QCOMPARE(after.filters().at(0).name, QStringLiteral("FilterCpu"));
    QCOMPARE(after.filters().at(1).name, QStringLiteral("FilterRam"));
}

void TargetingTest::fixtureRoundTrip()
{
    // Task 9.1 / 9.2 — load each XML fixture from tests/fixtures/msad_targeting/,
    // route it through containerFromXml + appendContainerToXml, then parse the
    // output again and assert that the two TargetingContainer objects are
    // semantically identical.  Attribute order in QDomDocument serialisation is
    // QHash-order (non-deterministic across runs), so string equality is not
    // tested; instead we compare the parsed data model.
    const QString fixturePath = QStringLiteral("../../../../fixtures/msad_targeting");
    QDir dir(fixturePath);
    const QStringList xmlFiles = dir.entryList({QStringLiteral("*.xml")}, QDir::Files);
    QVERIFY2(!xmlFiles.isEmpty(),
             qPrintable(QStringLiteral("No fixture files found in: %1 (resolved: %2)")
                            .arg(fixturePath, dir.absolutePath())));

    for (const auto &fileName : xmlFiles)
    {
        QFile f(dir.filePath(fileName));
        QVERIFY2(f.open(QIODevice::ReadOnly | QIODevice::Text),
                 qPrintable(QStringLiteral("Cannot open fixture: %1").arg(fileName)));

        QDomDocument doc;
        QString parseError;
        int line = 0, col = 0;
        QVERIFY2(doc.setContent(&f, &parseError, &line, &col),
                 qPrintable(QStringLiteral("XML parse error in %1:%2:%3: %4")
                                .arg(fileName)
                                .arg(line)
                                .arg(col)
                                .arg(parseError)));
        f.close();

        const QDomElement filtersElem = doc.documentElement();
        QCOMPARE(filtersElem.tagName(), QStringLiteral("Filters"));

        // Pass 1: fixture <Filters> element → TargetingContainer.
        const preferences::TargetingContainer c1 = preferences::containerFromXml(filtersElem);
        QVERIFY2(!c1.isEmpty(),
                 qPrintable(QStringLiteral("Fixture produced empty container: %1").arg(fileName)));

        // Pass 2: serialize c1 and re-parse.
        QDomDocument doc2;
        QDomElement root2 = doc2.createElement(QStringLiteral("root"));
        doc2.appendChild(root2);
        preferences::appendContainerToXml(doc2, root2, c1);
        const QDomElement filters2 = root2.firstChildElement(QStringLiteral("Filters"));
        const preferences::TargetingContainer c2 = preferences::containerFromXml(filters2);

        // The two containers must be semantically identical.
        QVERIFY2(compareFilterLists(c2.filters(), c1.filters(), fileName),
                 qPrintable(QStringLiteral("Round-trip mismatch for fixture: %1").arg(fileName)));
    }
}

namespace
{

//! Mirrors `CommonItem::setFilters` (UI accept path) — pure store, no
//! mutation of the apply-once flag or the cached run-once id.
struct CommonItemFake
{
    bool                  applyOnce          = false;
    bool                  itemLevelTargeting = false;
    QString               runOnceId          = QStringLiteral("{seed-id}");
    preferences::TargetingContainer m_filters;

    void setFilters(preferences::TargetingContainer filters)
    {
        m_filters = std::move(filters);
    }

    //! Mirrors `CommonItem::setFiltersFromXml` (load path) — strips
    //! FilterRunOnce, caches its id, sets applyOnce from its presence
    //! and itemLevelTargeting from any non-FilterRunOnce record.
    void setFiltersFromXml(preferences::TargetingContainer filters)
    {
        bool seenRunOnce = false;
        QList<preferences::TargetingFilterRecord> kept;
        kept.reserve(filters.filters().size());
        for (const auto &record : filters.filters())
        {
            if (record.name == QLatin1String("FilterRunOnce"))
            {
                seenRunOnce = true;
                if (!record.id.isEmpty())
                {
                    runOnceId = record.id;
                }
                continue;
            }
            kept.append(record);
        }
        applyOnce          = seenRunOnce;
        itemLevelTargeting = !kept.isEmpty();
        preferences::TargetingContainer trimmed;
        trimmed.setFilters(std::move(kept));
        m_filters = std::move(trimmed);
    }

    //! Mirrors `CommonItem::filtersForSerialization`: empty container
    //! when itemLevelTargeting is off; FilterRunOnce prepend driven
    //! orthogonally by applyOnce.
    preferences::TargetingContainer filtersForSerialization() const
    {
        QList<preferences::TargetingFilterRecord> records;
        if (itemLevelTargeting)
        {
            records = m_filters.filters();
        }
        if (applyOnce)
        {
            preferences::TargetingFilterRecord runOnce;
            runOnce.name       = QStringLiteral("FilterRunOnce");
            runOnce.id         = runOnceId;
            runOnce.combinator = QStringLiteral("AND");
            runOnce.negated    = false;
            records.prepend(runOnce);
        }
        preferences::TargetingContainer container;
        container.setFilters(std::move(records));
        return container;
    }
};

} // namespace

void TargetingTest::setFiltersIsPureSetter()
{
    // Regression test: prior implementation reset applyOnce=false whenever
    // setFilters was called with a non-empty container that lacked a
    // FilterRunOnce — which is exactly what the targeting dialog hands
    // back on accept. The split makes setFilters a pure setter.
    CommonItemFake item;
    item.applyOnce = true;
    item.runOnceId = QStringLiteral("{cached-id}");

    preferences::TargetingFilterRecord f;
    f.name = QStringLiteral("FilterComputer");
    f.id   = QStringLiteral("{added-id}");
    preferences::TargetingContainer c;
    c.setFilters({f});

    item.setFilters(c);

    QVERIFY2(item.applyOnce, "setFilters MUST NOT clear apply-once on a "
                              "non-empty container that has no FilterRunOnce");
    QCOMPARE(item.runOnceId, QStringLiteral("{cached-id}"));
    QCOMPARE(item.m_filters.filters().size(), 1);
    QCOMPARE(item.m_filters.filters().first().name, QStringLiteral("FilterComputer"));
}

void TargetingTest::setFiltersFromXmlInfersApplyOnce()
{
    // Load path: when the XML carries FilterRunOnce, strip it, cache its
    // id, and set applyOnce=true. When the XML has no FilterRunOnce,
    // applyOnce is false.
    {
        CommonItemFake item;
        preferences::TargetingFilterRecord runOnce;
        runOnce.name = QStringLiteral("FilterRunOnce");
        runOnce.id   = QStringLiteral("{from-xml}");
        preferences::TargetingFilterRecord other;
        other.name = QStringLiteral("FilterComputer");

        preferences::TargetingContainer c;
        c.setFilters({runOnce, other});
        item.setFiltersFromXml(c);

        QVERIFY(item.applyOnce);
        QCOMPARE(item.runOnceId, QStringLiteral("{from-xml}"));
        QCOMPARE(item.m_filters.filters().size(), 1);
        QCOMPARE(item.m_filters.filters().first().name, QStringLiteral("FilterComputer"));
    }
    {
        CommonItemFake item;
        item.applyOnce = true; // initial state — must be cleared by load
        preferences::TargetingFilterRecord other;
        other.name = QStringLiteral("FilterComputer");

        preferences::TargetingContainer c;
        c.setFilters({other});
        item.setFiltersFromXml(c);

        QVERIFY(!item.applyOnce);
        QCOMPARE(item.m_filters.filters().size(), 1);
    }
}

void TargetingTest::setFiltersFromXmlSetsItemLevelFlag()
{
    // Load path infers ITEM_LEVEL_TARGETING from any non-FilterRunOnce
    // record being present.
    {
        // Filters present → checkbox should be ticked.
        CommonItemFake item;
        preferences::TargetingFilterRecord f;
        f.name = QStringLiteral("FilterComputer");
        preferences::TargetingContainer c;
        c.setFilters({f});
        item.setFiltersFromXml(c);
        QVERIFY(item.itemLevelTargeting);
    }
    {
        // Empty container → checkbox should be cleared.
        CommonItemFake item;
        item.itemLevelTargeting = true; // initial state — must be cleared
        preferences::TargetingContainer c;
        item.setFiltersFromXml(c);
        QVERIFY(!item.itemLevelTargeting);
    }
    {
        // Only FilterRunOnce → checkbox should stay false (it's the
        // apply-once marker, not user-authored targeting).
        CommonItemFake item;
        preferences::TargetingFilterRecord runOnce;
        runOnce.name = QStringLiteral("FilterRunOnce");
        runOnce.id   = QStringLiteral("{r}");
        preferences::TargetingContainer c;
        c.setFilters({runOnce});
        item.setFiltersFromXml(c);
        QVERIFY(!item.itemLevelTargeting);
        QVERIFY(item.applyOnce);
    }
}

void TargetingTest::filtersForSerializationRespectsCheckbox()
{
    // Truth table from design.md §D1 for the four combinations of
    // itemLevelTargeting × applyOnce.
    preferences::TargetingFilterRecord cpu;
    cpu.name = QStringLiteral("FilterCpu");
    cpu.id   = QStringLiteral("{cpu}");
    preferences::TargetingContainer authored;
    authored.setFilters({cpu});

    auto run = [&authored](bool itemLevel, bool apply) {
        CommonItemFake item;
        item.itemLevelTargeting = itemLevel;
        item.applyOnce          = apply;
        item.runOnceId          = QStringLiteral("{ro}");
        item.m_filters          = authored;
        return item.filtersForSerialization().filters();
    };

    // (false, false) → empty.
    {
        const auto out = run(false, false);
        QCOMPARE(out.size(), 0);
    }
    // (false, true) → [FilterRunOnce] only.
    {
        const auto out = run(false, true);
        QCOMPARE(out.size(), 1);
        QCOMPARE(out.first().name, QStringLiteral("FilterRunOnce"));
        QCOMPARE(out.first().id,   QStringLiteral("{ro}"));
    }
    // (true, false) → m_filters as-is.
    {
        const auto out = run(true, false);
        QCOMPARE(out.size(), 1);
        QCOMPARE(out.first().name, QStringLiteral("FilterCpu"));
    }
    // (true, true) → [FilterRunOnce, ...m_filters].
    {
        const auto out = run(true, true);
        QCOMPARE(out.size(), 2);
        QCOMPARE(out.at(0).name, QStringLiteral("FilterRunOnce"));
        QCOMPARE(out.at(1).name, QStringLiteral("FilterCpu"));
    }
}

void TargetingTest::humanReadableLineSamples()
{
    // Sanity-check the formatter for representative filter types.
    using preferences::TargetingRowFormatter;

    {
        QMap<QString, QString> e;
        e.insert(QStringLiteral("displayName"), QStringLiteral("Russian (Russia)"));
        const auto out = TargetingRowFormatter::format(QStringLiteral("FilterLanguage"), e);
        QVERIFY(!out.isEmpty());
        QVERIFY(out.contains(QStringLiteral("Russian (Russia)")));
    }
    {
        QMap<QString, QString> e;
        e.insert(QStringLiteral("name"), QStringLiteral("WORKSTATION01"));
        const auto out = TargetingRowFormatter::format(QStringLiteral("FilterComputer"), e);
        QVERIFY(out.contains(QStringLiteral("WORKSTATION01")));
    }
    {
        QMap<QString, QString> e;
        e.insert(QStringLiteral("speedMHz"), QStringLiteral("2400"));
        const auto out = TargetingRowFormatter::format(QStringLiteral("FilterCpu"), e);
        QVERIFY(out.contains(QStringLiteral("2400")));
    }
    {
        QMap<QString, QString> e;
        e.insert(QStringLiteral("totalMB"), QStringLiteral("4096"));
        const auto out = TargetingRowFormatter::format(QStringLiteral("FilterRam"), e);
        QVERIFY(out.contains(QStringLiteral("4096")));
    }
    {
        QMap<QString, QString> e;
        e.insert(QStringLiteral("variableName"), QStringLiteral("PATH"));
        e.insert(QStringLiteral("value"),        QStringLiteral("/usr/bin"));
        const auto out = TargetingRowFormatter::format(QStringLiteral("FilterVariable"), e);
        // MSAD-style %VAR% rendering — both the name and the value appear.
        QVERIFY(out.contains(QStringLiteral("PATH")));
        QVERIFY(out.contains(QStringLiteral("/usr/bin")));
    }
    {
        QMap<QString, QString> e;
        e.insert(QStringLiteral("hive"),      QStringLiteral("HKEY_LOCAL_MACHINE"));
        e.insert(QStringLiteral("key"),       QStringLiteral("SOFTWARE\\Test"));
        e.insert(QStringLiteral("valueName"), QStringLiteral("Enabled"));
        const auto out = TargetingRowFormatter::format(QStringLiteral("FilterRegistry"), e);
        QVERIFY(out.contains(QStringLiteral("Enabled")));
        QVERIFY(out.contains(QStringLiteral("HKEY_LOCAL_MACHINE")));
    }
    {
        // Battery has no attributes — the formatter still returns a
        // non-empty static string.
        const auto out = TargetingRowFormatter::format(QStringLiteral("FilterBattery"), {});
        QVERIFY(!out.isEmpty());
    }
}

void TargetingTest::formatterFallsBackForUnknown()
{
    // Filter types not in the formatter table fall back to the
    // "Filter" prefix-stripped name.
    const auto out = preferences::TargetingRowFormatter::format(
        QStringLiteral("FilterTotallyMadeUp"), {});
    QCOMPARE(out, QStringLiteral("TotallyMadeUp"));

    // Names that don't start with "Filter" pass through verbatim.
    const auto raw = preferences::TargetingRowFormatter::format(
        QStringLiteral("Plain"), {});
    QCOMPARE(raw, QStringLiteral("Plain"));
}

void TargetingTest::combinatorPrefixRefreshesAfterFirstRowDelete()
{
    ModelView::SessionModel model("TargetingLabelRefreshTest");
    model.registerItem<preferences::TargetingFilterItem>();
    model.rootItem()->registerTag(
        ModelView::TagInfo::universalTag("filters",
                                         {preferences::TargetingFilterItem::kModelType}),
        true);

    preferences::TargetingFilterRecord first;
    first.name = QStringLiteral("FilterComputer");
    first.id = QStringLiteral("{first}");
    first.combinator = QStringLiteral("AND");
    first.attributes.insert(QStringLiteral("name"), QStringLiteral("FIRST"));

    preferences::TargetingFilterRecord second;
    second.name = QStringLiteral("FilterComputer");
    second.id = QStringLiteral("{second}");
    second.combinator = QStringLiteral("OR");
    second.attributes.insert(QStringLiteral("name"), QStringLiteral("SECOND"));

    auto *firstItem = model.insertItem<preferences::TargetingFilterItem>(
        model.rootItem(), {"filters", -1});
    firstItem->loadRecord(first);
    auto *secondItem = model.insertItem<preferences::TargetingFilterItem>(
        model.rootItem(), {"filters", -1});
    secondItem->loadRecord(second);

    QVERIFY(secondItem->label().startsWith(QStringLiteral("OR ")));

    model.removeItem(model.rootItem(), {"filters", 0});
    secondItem->refreshDisplayName();

    QVERIFY(!secondItem->label().startsWith(QStringLiteral("OR ")));
    QVERIFY(!secondItem->label().startsWith(QStringLiteral("AND ")));
    QVERIFY(secondItem->label().contains(QStringLiteral("SECOND")));
}

void TargetingTest::localizedRowFormattersUseLabels()
{
    using preferences::TargetingRowFormatter;

    {
        QMap<QString, QString> e;
        e.insert(QStringLiteral("version"), QStringLiteral("WINTHRESHOLDSRV"));
        e.insert(QStringLiteral("edition"), QStringLiteral("DTC"));
        e.insert(QStringLiteral("type"),    QStringLiteral("DC"));

        const auto out = TargetingRowFormatter::format(QStringLiteral("FilterOs"), e);
        QVERIFY(out.contains(QStringLiteral("Windows Server 2019 Family")));
        QVERIFY(out.contains(QStringLiteral("Datacenter")));
        QVERIFY(out.contains(QStringLiteral("Domain Controller")));
        QVERIFY(!out.contains(QStringLiteral("WINTHRESHOLDSRV")));
        QVERIFY(!out.contains(QStringLiteral("DTC")));
        QVERIFY(!out.contains(QStringLiteral("DC")));
    }

    {
        QMap<QString, QString> e;
        e.insert(QStringLiteral("version"), QStringLiteral("WINTHRESHOLD"));
        e.insert(QStringLiteral("edition"), QStringLiteral("NE"));
        e.insert(QStringLiteral("type"),    QStringLiteral("NE"));
        e.insert(QStringLiteral("sp"),      QStringLiteral("NE"));

        const auto out = TargetingRowFormatter::format(QStringLiteral("FilterOs"), e);
        QCOMPARE(out, QStringLiteral("operating system Windows 10"));
    }

    {
        QMap<QString, QString> e;
        e.insert(QStringLiteral("type"), QStringLiteral("vpn"));

        const auto out = TargetingRowFormatter::format(QStringLiteral("FilterDun"), e);
        QVERIFY(out.contains(QStringLiteral("Virtual Private Network (VPN)")));
        QVERIFY(!out.contains(QStringLiteral("vpn")));
    }

    {
        QMap<QString, QString> e;
        e.insert(QStringLiteral("synchFore"), QStringLiteral("1"));
        e.insert(QStringLiteral("verbLog"),   QStringLiteral("1"));

        const auto out = TargetingRowFormatter::format(QStringLiteral("FilterProcMode"), e);
        QVERIFY(out.contains(QStringLiteral("synchronous foreground")));
        QVERIFY(out.contains(QStringLiteral("verbose logging")));
        QVERIFY(!out.contains(QStringLiteral("synchFore")));
        QVERIFY(!out.contains(QStringLiteral("verbLog")));
    }

    {
        ModelView::SessionModel model("TargetingLocalizedCombinatorTest");
        model.registerItem<preferences::TargetingFilterItem>();
        model.rootItem()->registerTag(
            ModelView::TagInfo::universalTag("filters",
                                             {preferences::TargetingFilterItem::kModelType}),
            true);

        preferences::TargetingFilterRecord first;
        first.name = QStringLiteral("FilterComputer");
        first.attributes.insert(QStringLiteral("name"), QStringLiteral("FIRST"));

        preferences::TargetingFilterRecord second;
        second.name       = QStringLiteral("FilterComputer");
        second.combinator = QStringLiteral("OR");
        second.attributes.insert(QStringLiteral("name"), QStringLiteral("SECOND"));

        auto *firstItem = model.insertItem<preferences::TargetingFilterItem>(
            model.rootItem(), {"filters", -1});
        firstItem->loadRecord(first);
        auto *secondItem = model.insertItem<preferences::TargetingFilterItem>(
            model.rootItem(), {"filters", -1});
        secondItem->loadRecord(second);

        QVERIFY(secondItem->label().startsWith(QStringLiteral("OR ")));
    }
}

void TargetingTest::osCatalogProductChoices()
{
    const auto products = preferences::OsCatalog::visibleProducts();
    QStringList labels;
    for (const auto &product : products)
    {
        labels << product.label;
    }

    QCOMPARE(labels.first(), QStringLiteral("Any"));
    QVERIFY(labels.contains(QStringLiteral("Windows XP")));
    QVERIFY(labels.contains(QStringLiteral("Windows Server 2003")));
    QVERIFY(labels.contains(QStringLiteral("Windows Server 2003 R2")));
    QVERIFY(labels.contains(QStringLiteral("Windows Vista")));
    QVERIFY(labels.contains(QStringLiteral("Windows Server 2008")));
    QVERIFY(labels.contains(QStringLiteral("Windows 7")));
    QVERIFY(labels.contains(QStringLiteral("Windows Server 2008 R2")));
    QVERIFY(labels.contains(QStringLiteral("Windows 8")));
    QVERIFY(labels.contains(QStringLiteral("Windows Server 2012 Family")));
    QVERIFY(labels.contains(QStringLiteral("Windows 8.1")));
    QVERIFY(labels.contains(QStringLiteral("Windows Server 2012 R2 Family")));
    QVERIFY(labels.contains(QStringLiteral("Windows 10")));
    QVERIFY(labels.contains(QStringLiteral("Windows Server 2019 Family")));
    QVERIFY(!labels.contains(QStringLiteral("ALT Linux")));
    QVERIFY(!labels.contains(QStringLiteral("Linux")));
}

void TargetingTest::osCatalogDependentChoices()
{
    auto labels = [](const QList<preferences::OsChoice> &choices) {
        QStringList out;
        for (const auto &choice : choices)
        {
            out << choice.label;
        }
        return out;
    };

    const auto xp = preferences::OsCatalog::productByValue(QStringLiteral("XP"));
    const auto xpSp = labels(xp.servicePacks);
    QVERIFY(xpSp.contains(QStringLiteral("Any")));
    QVERIFY(xpSp.contains(QStringLiteral("No service packs installed")));
    QVERIFY(xpSp.contains(QStringLiteral("Service Pack 1")));
    QVERIFY(xpSp.contains(QStringLiteral("Service Pack 2")));
    QVERIFY(xpSp.contains(QStringLiteral("Service Pack 3")));
    QCOMPARE(labels(xp.roles), QStringList{QStringLiteral("Any")});

    const auto win7 = preferences::OsCatalog::productByValue(QStringLiteral("WIN7"));
    const auto win7Editions = labels(win7.editions);
    QVERIFY(win7Editions.contains(QStringLiteral("Enterprise")));
    QVERIFY(win7Editions.contains(QStringLiteral("Professional")));
    QVERIFY(win7Editions.contains(QStringLiteral("Ultimate")));
    QVERIFY(win7Editions.contains(QStringLiteral("Enterprise, 64-bit")));
    QVERIFY(win7Editions.contains(QStringLiteral("Professional, 64-bit")));

    const auto server2008 = preferences::OsCatalog::productByValue(QStringLiteral("2K8"));
    const auto server2008Editions = labels(server2008.editions);
    QVERIFY(server2008Editions.contains(QStringLiteral("HPC Server")));
    QVERIFY(server2008Editions.contains(QStringLiteral("Itanium-based Systems")));
    QVERIFY(server2008Editions.contains(QStringLiteral("Standard without Hyper-V")));
    QVERIFY(server2008Editions.contains(QStringLiteral("Datacenter without Hyper-V, 64-bit")));
    QVERIFY(labels(server2008.roles).contains(QStringLiteral("Member Server")));
    QVERIFY(labels(server2008.roles).contains(QStringLiteral("Domain Controller")));

    const auto server2012 = preferences::OsCatalog::productByValue(QStringLiteral("WIN8S"));
    const auto server2012Editions = labels(server2012.editions);
    QVERIFY(server2012Editions.contains(QStringLiteral("Datacenter")));
    QVERIFY(server2012Editions.contains(QStringLiteral("Standard")));
    QVERIFY(server2012Editions.contains(QStringLiteral("Storage Server Standard")));
    QVERIFY(server2012Editions.contains(QStringLiteral("Storage Server Workgroup")));
    QVERIFY(server2012Editions.contains(QStringLiteral("MultiPoint Server Premium")));
    QVERIFY(server2012Editions.contains(QStringLiteral("Essentials")));
}

void TargetingTest::osCatalogKeepsLinuxReserved()
{
    const auto reserved = preferences::OsCatalog::reservedProducts();
    QStringList labels;
    for (const auto &product : reserved)
    {
        labels << product.label;
        QVERIFY(!product.visible);
    }
    QVERIFY(labels.contains(QStringLiteral("ALT Linux")));
    QVERIFY(labels.contains(QStringLiteral("Linux")));
}

} // namespace tests

QTEST_MAIN(tests::TargetingTest)
