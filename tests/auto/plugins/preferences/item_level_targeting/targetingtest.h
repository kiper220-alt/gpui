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

#ifndef GPUI_TESTS_TARGETING_TEST_H
#define GPUI_TESTS_TARGETING_TEST_H

#include <QObject>
#include <QtTest>

namespace tests
{

//! Covers tasks 9.3 and 9.6 from
//! openspec/changes/add-preferences-common-targeting/tasks.md.
//!
//! 9.3 — roundtrip one of every filter type (catalog from
//!       `TargetingDialog::filterCatalog()`) through the DOM bridge and
//!       assert every attribute, id, combinator and negation survives.
//! 9.6 — negative test: an unknown attribute on a filter loads without
//!       error and is preserved verbatim on the return trip.
class TargetingTest : public QObject
{
    Q_OBJECT

private slots:
    void filterCatalogRoundTrip();
    void documentedDefaultExpectations();
    void createdFiltersSerializeDefaults();
    void uiTemplateDefaultsMatchDocumentedExpectations();
    void translationCatalogHasNoKnownGaps();
    void rsatShapedFiltersRoundTrip();
    void dateEveryYearOmitsYear();
    void unknownAttributeIsPreserved();
    void filterCollectionNestingSurvives();
    void applyOnceModelRoundTrip();
    void wrapUnwrapGolden();
    void fixtureRoundTrip();
    void setFiltersIsPureSetter();
    void setFiltersFromXmlInfersApplyOnce();
    void setFiltersFromXmlSetsItemLevelFlag();
    void filtersForSerializationRespectsCheckbox();
    void humanReadableLineSamples();
    void formatterFallsBackForUnknown();
    void collectionLabelReflectsLoadedChildren();
    void combinatorPrefixRefreshesAfterFirstRowDelete();
    void localizedRowFormattersUseLabels();
    void osCatalogProductChoices();
    void osCatalogDependentChoices();
    void osCatalogKeepsLinuxReserved();
    void dragFeedbackDistinguishesInsertionAndCollection();
    void nonCollectionCenterShowsInsertionFeedback();
    void selectionNormalizationDropsDescendants();
    void nestedSelectionDeleteRemovesAncestorOnly();
    void nestedSelectionCutCopiesAncestorOnly();
    void internalDragMimeAcceptsSameToken();
    void internalDragMimePreservesTreeOrder();
    void internalDragWorksForFiltersLoadedWithoutXmlIds();
    void dropRowAdjustmentCountsAllOriginalRows();
    void internalDragMimeRejectsForeignToken();
    void internalDragMimeRejectsStaleIds();
};

} // namespace tests

#endif // GPUI_TESTS_TARGETING_TEST_H
