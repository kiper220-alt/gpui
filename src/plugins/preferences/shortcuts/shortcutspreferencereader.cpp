/***********************************************************************************************************************
**
** Copyright (C) 2022 BaseALT Ltd. <org@basealt.ru>
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

#include "shortcutspreferencereader.h"

#include "item_level_targeting/common/filtersio.h"
#include "schemas/shortcutsschema.h"
#include "shortcutsmodelbuilder.h"

#include <sstream>

namespace preferences
{
ShortcutsPreferenceReader::ShortcutsPreferenceReader()
    : BasePreferenceReader("ShortcutsContainerItem")
{}

std::unique_ptr<PreferencesModel> ShortcutsPreferenceReader::createModel(std::istream &input)
{
    auto stripped = FiltersIO::stripFilters(input);
    std::istringstream cleaned(stripped.cleanedXml);

    auto schema       = Shortcuts_(cleaned, ::xsd::cxx::tree::flags::dont_validate);
    auto modelBuilder = std::make_unique<ShortcutsModelBuilder>();
    auto model        = modelBuilder->schemaToModel(schema);

    FiltersIO::applyToModel(model.get(), stripped.filters);
    return model;
}

} // namespace preferences
