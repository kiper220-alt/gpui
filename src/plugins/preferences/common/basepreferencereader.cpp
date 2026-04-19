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

#include "basepreferencereader.h"

#include <fstream>
#include <sstream>
#include <QDebug>
#include <QDomDocument>
#include <QDomElement>

#include "../../../src/plugins/storage/smb/smbfile.h"

#include "commonitem.h"
#include "item_level_targeting/filtersbridge.h"
#include "preferencesmodel.h"

#include <mvvm/model/sessionitem.h>

std::string preferences::BasePreferenceReader::getType() const
{
    return readerType;
}

namespace
{

//! Walk the file DOM and copy every `<Filters>` subtree into the matching
//! `CommonItem` (located by `uid` attribute). See D2 in design.md —
//! per-snapin readers pass the filter tree through an opaque container
//! rather than typed xsd-cxx accessors, which preserves element order and
//! unknown attributes byte-for-byte.
void ingestFiltersFromXml(const QByteArray &xml, preferences::PreferencesModel *model)
{
    if (!model || xml.isEmpty())
    {
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(xml))
    {
        return;
    }

    // Index CommonItem by uid once; each item's Filters land in one place.
    // Per the convention used by every container item in the preferences
    // plugin, the embedded CommonItem is registered under tag "common".
    std::map<std::string, preferences::CommonItem *> byUid;
    for (auto *top : model->rootItem()->children())
    {
        auto *commonRaw = top->getItem("common");
        auto *common    = dynamic_cast<preferences::CommonItem *>(commonRaw);
        if (!common)
        {
            continue;
        }
        const auto uid = common->property<std::string>(
            preferences::CommonItem::propertyToString(preferences::CommonItem::UID));
        if (uid.empty())
        {
            continue;
        }
        byUid.emplace(uid, common);
    }

    // Top-level items sit at docElement().firstChildElement() depth; but
    // xsd-cxx XML sometimes nests the per-type element one level deeper
    // (e.g. <Drives><Drive uid="..">). Walk every descendant and process
    // elements carrying a uid attribute.
    QDomNodeList all = doc.elementsByTagName(QStringLiteral("*"));
    for (int i = 0; i < all.count(); ++i)
    {
        QDomElement item = all.at(i).toElement();
        if (item.isNull() || !item.hasAttribute(QStringLiteral("uid")))
        {
            continue;
        }
        const auto it = byUid.find(item.attribute(QStringLiteral("uid")).toStdString());
        if (it == byUid.end())
        {
            continue;
        }
        const QDomElement filters = item.firstChildElement(QStringLiteral("Filters"));
        if (filters.isNull())
        {
            continue;
        }
        it->second->setFilters(preferences::containerFromXml(filters));
    }
}

} // namespace

std::unique_ptr<preferences::PreferencesModel> preferences::BasePreferenceReader::read(const std::string path)
{
    std::unique_ptr<PreferencesModel> result;

    auto stringvalues = std::make_unique<std::string>();

    try
    {
        QString qtPath = QString::fromStdString(path);

        if (qtPath.startsWith("smb://"))
        {
            gpui::smb::SmbFile smbFile(qtPath);
            smbFile.open(QFile::ReadOnly);
            stringvalues->resize(smbFile.size(), 0);
            smbFile.read(&stringvalues->at(0), smbFile.size());
            smbFile.close();
        }
        else
        {
            QFile registryFile(qtPath);
            registryFile.open(QFile::ReadOnly);
            stringvalues->resize(registryFile.size(), 0);
            registryFile.read(&stringvalues->at(0), registryFile.size());
            registryFile.close();
        }

        auto iss = std::make_unique<std::istringstream>(*stringvalues);

        result = createModel(*iss.get());

        ingestFiltersFromXml(QByteArray::fromStdString(*stringvalues), result.get());
    }
    catch (const std::exception &e)
    {
        qWarning() << e.what();
    }

    return result;
}

preferences::BasePreferenceReader::BasePreferenceReader(const std::string &type)
{
    readerType = type;
}
