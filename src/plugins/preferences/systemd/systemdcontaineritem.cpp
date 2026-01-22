#include "systemdcontaineritem.h"
#include "common/commonitem.h"
#include "systemditem.h"
#include "common/defaultactions.h"

#include <mvvm/signals/itemmapper.h>
#include <iostream>

namespace preferences 
{

inline std::string stateName(int state) 
{
    switch (static_cast<SystemdState>(state))
    {
        case SystemdState::Enable:
            return "Enable";
        case SystemdState::Disable:
            return "Disable";
        case SystemdState::Mask:
            return "Mask";
        case SystemdState::Unmask:
            return "Unmask";
        default:
            return "Nothing";
    }
} 

SystemdContainerItem::SystemdContainerItem()
    : ModelView::CompoundItem("SystemdContainerItem")
{
    addProperty(NAME, "")->setDisplayName(
        QObject::tr("Name").toStdString()
    )->setEditable(false);
    addProperty(STATE, "")->setDisplayName(
        QObject::tr("Change Unit State").toStdString()
    )->setEditable(false);
    addProperty(EDIT, QObject::tr("No").toStdString())->setDisplayName(
        QObject::tr("Edit").toStdString()
    )->setEditable(false);
    addProperty(DEPENDENCY, QObject::tr("No").toStdString())->setDisplayName(
        QObject::tr("Dependencies").toStdString()
    )->setEditable(false);

    addProperty<CommonItem>(COMMON)->setVisible(false);
    addProperty<SystemdItem>(SYSTEMD)->setVisible(false);
}

CommonItem *SystemdContainerItem::getCommon() const
{
    return static_cast<CommonItem*>(children()[childrenCount() - 2]);
}

void SystemdContainerItem::setCommon(const CommonItem &item)
{
    setProperty(COMMON, item);
}

SystemdItem *SystemdContainerItem::getSystemds() const
{
    return static_cast<SystemdItem*>(children().back());
}

void SystemdContainerItem::setSystemds(const SystemdItem &item)
{
    setProperty(SYSTEMD, item);
}

void SystemdContainerItem::setupListeners()
{
    auto onChildPropertyChange = [&](SessionItem *item, std::string property) 
    {
        if (auto systemdItem = dynamic_cast<SystemdItem *>(item))
        {
            if (property == SystemdItem::UNIT)
            {
                setProperty(NAME, systemdItem->property<std::string>(SystemdItem::UNIT));
            }

            if (property == SystemdItem::STATE)
            {
                setProperty(STATE, stateName(systemdItem->property<int>(SystemdItem::STATE)));
            }

            if (property == SystemdItem::EDIT)
            {
                setProperty(EDIT, 
                    systemdItem->property<bool>(SystemdItem::EDIT)?
                    QObject::tr("Yes").toStdString():
                    QObject::tr("No").toStdString()
                );
            }

            if (property == SystemdItem::DEPENDENCY)
            {
                setProperty(DEPENDENCY, 
                    systemdItem->property<bool>(SystemdItem::DEPENDENCY)?
                    QObject::tr("Yes").toStdString():
                    QObject::tr("No").toStdString()
                );
            }
        }
    };

    this->mapper()->setOnChildPropertyChange(onChildPropertyChange, nullptr);
}

void SystemdContainerItem::retranslateStrings()
{
    // TODO: Implement retranslate
}

} // namespace preferences