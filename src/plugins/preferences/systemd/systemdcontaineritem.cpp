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
/**
 * Returns the CommonItem of this SystemdContainerItem.
 *
 * This item will be used to represent the unit in the editor.
 *
 * The item will contain the name, order, and action of the unit.
 *
 * @return The CommonItem of this unit.
 */
CommonItem *SystemdContainerItem::getCommon() const
{
    return static_cast<CommonItem*>(children()[childrenCount() - 2]);
}
/**
 * Sets the CommonItem of this SystemdContainerItem.
 *
 * @param item The CommonItem to be set.
 *
 * This method sets the CommonItem of this SystemdContainerItem. The CommonItem
 * contains the name, order, and action of the unit.
 *
 * The item will be used to represent the unit in the editor.
 */
void SystemdContainerItem::setCommon(const CommonItem &item)
{
    setProperty(COMMON, item);
}
/**
 * Returns the SystemdItem of this SystemdContainerItem.
 *
 * This item will be used to represent the systemd item in the editor.
 *
 * The item will contain the unit name, state, and edit state of the systemd item.
 *
 * @return The SystemdItem of this systemd item.
 */
SystemdItem *SystemdContainerItem::getSystemds() const
{
    return static_cast<SystemdItem*>(children().back());
}
/**
 * Sets the SystemdItem of this SystemdContainerItem.
 *
 * @param item The Systemditem to be set.
 *
 * This method sets the Systemditem of this SystemdContainerItem. The Systemditem
 * contains the unit name, state, and edit state of the systemd item.
 *
 * The item will be used to represent the systemd item in the editor.
 */
void SystemdContainerItem::setSystemds(const SystemdItem &item)
{
    setProperty(SYSTEMD, item);
}
/**
 * Sets up listeners for the SystemdContainerItem.
 *
 * This method sets up listeners for the CommonItem and SystemdItem of this
 * SystemdContainerItem. The listeners will be called when the CommonItem or
 * SystemdItem changes. The listeners are used to update the display of the
 * systemd item in the editor.
 *
 * This method is called on construction of the SystemdContainerItem.
 */
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

/** 
 * @brief Translates the strings of this SystemdContainerItem.
 *
 * This method translates the action, name, state, and edit state of the SystemdContainerItem.
 *
 * The method is called on construction of the SystemdContainerItem.
 *
 * The translated strings will be used to display the SystemdContainerItem in the editor.
 */ 
void SystemdContainerItem::retranslateStrings()
{
    // TODO: Implement retranslate
}

} // namespace preferences