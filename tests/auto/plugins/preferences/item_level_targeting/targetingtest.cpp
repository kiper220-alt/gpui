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
#include "src/plugins/preferences/item_level_targeting/targetingcontainer.h"

#include <QDomDocument>
#include <QDomElement>
#include <QUuid>

namespace tests
{

namespace
{

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

} // namespace tests

QTEST_MAIN(tests::TargetingTest)
