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

#include "basepreferencewriter.h"

#include "../../../src/plugins/storage/smb/smbfile.h"

#include "commonitem.h"
#include "item_level_targeting/filtersbridge.h"
#include "preferencesmodel.h"

#include <mvvm/model/sessionitem.h>

#include <fstream>
#include <sstream>
#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
#include <QMessageBox>

std::string preferences::BasePreferenceWriter::getType() const
{
    return writerType;
}

namespace
{

//! Replace every `<Filters>` subtree xsd-cxx emitted with the
//! byte-stable version derived from each `CommonItem` (see D2/D7 in
//! design.md). Run after `writeModel` and before writing to disk.
void rewriteFiltersInXml(std::string &xml, const preferences::PreferencesModel *model)
{
    if (!model || xml.empty())
    {
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(QByteArray(xml.data(), static_cast<int>(xml.size()))))
    {
        return;
    }

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

    bool touched = false;
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

        // Drop any <Filters> xsd-cxx already serialised (we re-emit below).
        QDomElement existing = item.firstChildElement(QStringLiteral("Filters"));
        while (!existing.isNull())
        {
            item.removeChild(existing);
            existing = item.firstChildElement(QStringLiteral("Filters"));
        }

        const auto container = it->second->filtersForSerialization();
        if (!container.filters().isEmpty())
        {
            preferences::appendContainerToXml(doc, item, container);
            touched = true;
        }

        // Task 6.3: mirror apply-once into the per-extension `image`
        // attribute (D1 in design.md). MSAD encodes run-once in both the
        // <FilterRunOnce> child and the element's image bits. The exact
        // semantics of `image` diverge across extensions and the project
        // has no MSAD fixture corpus yet to validate against, so we only
        // write image when it is currently unset or "0" — this avoids
        // clobbering any icon-index MSAD or another editor has set.
        if (it->second->applyOnce())
        {
            const QString current = item.attribute(QStringLiteral("image"));
            if (current.isEmpty() || current == QLatin1String("0"))
            {
                item.setAttribute(QStringLiteral("image"), QStringLiteral("7"));
                touched = true;
            }
        }
    }

    if (!touched)
    {
        return;
    }
    const QByteArray updated = doc.toByteArray();
    xml.assign(updated.constData(), static_cast<size_t>(updated.size()));
}

} // namespace

bool preferences::BasePreferenceWriter::write(const std::string path,
                                              const std::unique_ptr<preferences::PreferencesModel> &model)
{
    bool result = false;

    auto oss = std::make_unique<std::ostringstream>();

    result = writeModel(*oss, model);

    oss->flush();

    std::string xmlBuffer = oss->str();
    rewriteFiltersInXml(xmlBuffer, model.get());
    oss->str(xmlBuffer);
    oss->seekp(0, std::ios::end);

    qWarning() << "Current string values." << oss->str().c_str();

    bool ifShowError = false;

    auto showMessageFunction = [&path]() { Q_UNUSED(path); };

    try
    {
        if (QString::fromStdString(path).startsWith("smb://"))
        {
            gpui::smb::SmbFile smbLocationItemFile(QString::fromStdString(path));
            ifShowError = smbLocationItemFile.open(QFile::WriteOnly | QFile::Truncate);
            if (!ifShowError)
            {
                result = ifShowError = smbLocationItemFile.open(QFile::NewOnly | QFile::WriteOnly);
            }
            if (ifShowError && oss->str().size() > 0)
            {
                smbLocationItemFile.write(&oss->str().at(0), oss->str().size());
            }
            smbLocationItemFile.close();
        }
        else
        {
            QFile registryFile(QString::fromStdString(path));
            ifShowError = registryFile.open(QFile::WriteOnly | QFile::Truncate);
            if (!ifShowError)
            {
                result = ifShowError = registryFile.open(QFile::NewOnly | QFile::WriteOnly);
            }
            if (ifShowError && registryFile.isWritable() && oss->str().size() > 0)
            {
                registryFile.write(&oss->str().at(0), oss->str().size());
            }
            registryFile.close();
        }
    }
    catch (std::exception &e)
    {
        ifShowError = true;
        showMessageFunction();
    }

    if (!ifShowError)
    {
        showMessageFunction();
    }

    return result;
}

preferences::BasePreferenceWriter::BasePreferenceWriter(const std::string &type)
    : writerType(type)
{}
