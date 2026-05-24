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

#include "sharespreferencewriter.h"

#include "item_level_targeting/common/filtersio.h"
#include "schemas/sharesschema.h"
#include "sharesmodelbuilder.h"

#include <sstream>

namespace preferences
{
SharesPreferenceWriter::SharesPreferenceWriter()
    : BasePreferenceWriter("SharesContainerItem")
{}

bool SharesPreferenceWriter::writeModel(std::ostream &output, const std::unique_ptr<PreferencesModel> &model)
{
    auto modelBuilder = std::make_unique<SharesModelBuilder>();
    auto shares       = modelBuilder->modelToSchema(const_cast<std::unique_ptr<PreferencesModel> &>(model));
    const ::xml_schema::NamespaceInfomap map;

    std::ostringstream tmp;
    NetworkShareSettings_(tmp, *shares.get(), map);

    const auto patched = FiltersIO::injectFilters(tmp.str(), model.get());
    output.write(patched.data(), static_cast<std::streamsize>(patched.size()));
    return true;
}

} // namespace preferences
