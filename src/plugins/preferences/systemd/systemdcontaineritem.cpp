#include "systemdcontaineritem.h"

#include <cassert>

#include <mvvm/signals/itemmapper.h>

#include "common/commonitem.h"
#include "systemditem.h"

namespace
{

std::string yesNoText(bool value)
{
    return value ? QObject::tr("Yes").toStdString() : QObject::tr("No").toStdString();
}

std::string stateToText(int state)
{
    switch (static_cast<preferences::SystemdState>(state))
    {
    case preferences::SystemdState::Enable:
        return QObject::tr("Enable").toStdString();
    case preferences::SystemdState::Disable:
        return QObject::tr("Disable").toStdString();
    case preferences::SystemdState::Mask:
        return QObject::tr("Mask").toStdString();
    case preferences::SystemdState::Unmask:
        return QObject::tr("Unmask").toStdString();
    case preferences::SystemdState::Preset:
        return QObject::tr("Preset").toStdString();
    case preferences::SystemdState::AsIs:
    default:
        return QObject::tr("As is").toStdString();
    }
}

std::string unitTypeToText(int unitType)
{
    using preferences::SystemdUnitType;

    switch (static_cast<SystemdUnitType>(unitType))
    {
    case SystemdUnitType::Service:
        return QObject::tr("Service").toStdString();
    case SystemdUnitType::Socket:
        return QObject::tr("Socket").toStdString();
    case SystemdUnitType::Timer:
        return QObject::tr("Timer").toStdString();
    case SystemdUnitType::Path:
        return QObject::tr("Path").toStdString();
    case SystemdUnitType::Mount:
        return QObject::tr("Mount").toStdString();
    case SystemdUnitType::Automount:
        return QObject::tr("Automount").toStdString();
    case SystemdUnitType::Swap:
        return QObject::tr("Swap").toStdString();
    case SystemdUnitType::Target:
        return QObject::tr("Target").toStdString();
    case SystemdUnitType::Device:
        return QObject::tr("Device").toStdString();
    case SystemdUnitType::Slice:
        return QObject::tr("Slice").toStdString();
    case SystemdUnitType::Scope:
        return QObject::tr("Scope").toStdString();
    default:
        return QObject::tr("Unit").toStdString();
    }
}

} // namespace

namespace preferences
{

SystemdContainerItem::SystemdContainerItem()
    : SystemdContainerItem("SystemdContainerItem", static_cast<int>(SystemdUnitType::Service))
{}

SystemdContainerItem::SystemdContainerItem(const std::string &typeName, int unitType)
    : ModelView::CompoundItem(typeName)
{
    addProperty(NAME, "")->setDisplayName(QObject::tr("Name").toStdString())->setEditable(false);
    addProperty(TYPE, unitTypeToText(static_cast<int>(SystemdUnitType::Service)))
        ->setDisplayName(QObject::tr("Type").toStdString())
        ->setEditable(false);
    addProperty(STATE, stateToText(static_cast<int>(SystemdState::AsIs)))
        ->setDisplayName(QObject::tr("State").toStdString())
        ->setEditable(false);
    addProperty(EDIT, QObject::tr("No").toStdString())
        ->setDisplayName(QObject::tr("Edit").toStdString())
        ->setEditable(false);
    addProperty(DEPENDENCY, QObject::tr("No").toStdString())
        ->setDisplayName(QObject::tr("Dependencies").toStdString())
        ->setEditable(false);

    addProperty<CommonItem>(COMMON)->setVisible(false);
    addProperty<SystemdItem>(SYSTEMD)->setVisible(false);

    auto *systemdItem = getSystemd();
    systemdItem->setProperty(SystemdItem::UNIT_TYPE, unitType);
    systemdItem->setProperty(SystemdItem::HAS_DEPENDENCIES, SystemdItem::unitTypeHasFileDependencies(unitType));
}

CommonItem *SystemdContainerItem::getCommon() const
{
    assert(childrenCount() >= 2);
    return static_cast<CommonItem *>(children()[childrenCount() - 2]);
}

void SystemdContainerItem::setCommon(const CommonItem &item)
{
    setProperty(COMMON, item);
}

SystemdItem *SystemdContainerItem::getSystemd() const
{
    assert(childrenCount() >= 1);
    return static_cast<SystemdItem *>(children().back());
}

void SystemdContainerItem::setSystemd(const SystemdItem &item)
{
    setProperty(SYSTEMD, item);
}

void SystemdContainerItem::setupListeners()
{
    auto onChildPropertyChange = [&](SessionItem *item, std::string property) {
        auto systemdItem = dynamic_cast<SystemdItem *>(item);
        if (!systemdItem)
        {
            return;
        }

        if (property == SystemdItem::UNIT)
        {
            setProperty(NAME, systemdItem->property<std::string>(SystemdItem::UNIT));
        }

        if (property == SystemdItem::UNIT_TYPE)
        {
            setProperty(TYPE, unitTypeToText(systemdItem->property<int>(SystemdItem::UNIT_TYPE)));
        }

        if (property == SystemdItem::STATE)
        {
            setProperty(STATE, stateToText(systemdItem->property<int>(SystemdItem::STATE)));
        }

        if (property == SystemdItem::EDIT)
        {
            setProperty(EDIT, yesNoText(systemdItem->property<bool>(SystemdItem::EDIT)));
        }

        if (property == SystemdItem::DEPENDENCY || property == SystemdItem::HAS_DEPENDENCIES)
        {
            if (!systemdItem->property<bool>(SystemdItem::HAS_DEPENDENCIES))
            {
                setProperty(DEPENDENCY, QObject::tr("N/A").toStdString());
            }
            else
            {
                setProperty(DEPENDENCY, yesNoText(systemdItem->property<bool>(SystemdItem::DEPENDENCY)));
            }
        }
    };

    mapper()->setOnChildPropertyChange(onChildPropertyChange, nullptr);

    // Synchronize summary columns for default values too (e.g. Service type).
    auto *systemdItem = getSystemd();
    setProperty(NAME, systemdItem->property<std::string>(SystemdItem::UNIT));
    setProperty(TYPE, unitTypeToText(systemdItem->property<int>(SystemdItem::UNIT_TYPE)));
    setProperty(STATE, stateToText(systemdItem->property<int>(SystemdItem::STATE)));
    setProperty(EDIT, yesNoText(systemdItem->property<bool>(SystemdItem::EDIT)));

    if (!systemdItem->property<bool>(SystemdItem::HAS_DEPENDENCIES))
    {
        setProperty(DEPENDENCY, QObject::tr("N/A").toStdString());
    }
    else
    {
        setProperty(DEPENDENCY, yesNoText(systemdItem->property<bool>(SystemdItem::DEPENDENCY)));
    }
}

void SystemdContainerItem::retranslateStrings()
{
    assert(childrenCount() >= 5);
    children()[0]->setDisplayName(QObject::tr("Name").toStdString());
    children()[1]->setDisplayName(QObject::tr("Type").toStdString());
    children()[2]->setDisplayName(QObject::tr("State").toStdString());
    children()[3]->setDisplayName(QObject::tr("Edit").toStdString());
    children()[4]->setDisplayName(QObject::tr("Dependencies").toStdString());

    auto *systemdItem = getSystemd();
    setProperty(TYPE, unitTypeToText(systemdItem->property<int>(SystemdItem::UNIT_TYPE)));
    setProperty(STATE, stateToText(systemdItem->property<int>(SystemdItem::STATE)));
    setProperty(EDIT, yesNoText(systemdItem->property<bool>(SystemdItem::EDIT)));

    if (!systemdItem->property<bool>(SystemdItem::HAS_DEPENDENCIES))
    {
        setProperty(DEPENDENCY, QObject::tr("N/A").toStdString());
    }
    else
    {
        setProperty(DEPENDENCY, yesNoText(systemdItem->property<bool>(SystemdItem::DEPENDENCY)));
    }
}

SystemdServiceContainerItem::SystemdServiceContainerItem()
    : SystemdContainerItem("SystemdServiceContainerItem", static_cast<int>(SystemdUnitType::Service))
{}

SystemdSocketContainerItem::SystemdSocketContainerItem()
    : SystemdContainerItem("SystemdSocketContainerItem", static_cast<int>(SystemdUnitType::Socket))
{}

SystemdTimerContainerItem::SystemdTimerContainerItem()
    : SystemdContainerItem("SystemdTimerContainerItem", static_cast<int>(SystemdUnitType::Timer))
{}

SystemdPathContainerItem::SystemdPathContainerItem()
    : SystemdContainerItem("SystemdPathContainerItem", static_cast<int>(SystemdUnitType::Path))
{}

SystemdMountContainerItem::SystemdMountContainerItem()
    : SystemdContainerItem("SystemdMountContainerItem", static_cast<int>(SystemdUnitType::Mount))
{}

SystemdAutomountContainerItem::SystemdAutomountContainerItem()
    : SystemdContainerItem("SystemdAutomountContainerItem", static_cast<int>(SystemdUnitType::Automount))
{}

SystemdSwapContainerItem::SystemdSwapContainerItem()
    : SystemdContainerItem("SystemdSwapContainerItem", static_cast<int>(SystemdUnitType::Swap))
{}

SystemdTargetContainerItem::SystemdTargetContainerItem()
    : SystemdContainerItem("SystemdTargetContainerItem", static_cast<int>(SystemdUnitType::Target))
{}

SystemdDeviceContainerItem::SystemdDeviceContainerItem()
    : SystemdContainerItem("SystemdDeviceContainerItem", static_cast<int>(SystemdUnitType::Device))
{}

SystemdSliceContainerItem::SystemdSliceContainerItem()
    : SystemdContainerItem("SystemdSliceContainerItem", static_cast<int>(SystemdUnitType::Slice))
{}

SystemdScopeContainerItem::SystemdScopeContainerItem()
    : SystemdContainerItem("SystemdScopeContainerItem", static_cast<int>(SystemdUnitType::Scope))
{}

} // namespace preferences
