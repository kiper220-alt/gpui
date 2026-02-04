#ifndef GPUI_SYSTEMDCONTAINERITEM_H
#define GPUI_SYSTEMDCONTAINERITEM_H

#include <mvvm/model/compounditem.h>

#include "interfaces/containeriteminterface.h"

namespace preferences
{

class CommonItem;
class SystemdItem;

class SystemdContainerItem : public ModelView::CompoundItem, public ContainerItemInterface
{
public:
    static inline const std::string NAME = "name";
    static inline const std::string TYPE = "type";
    static inline const std::string STATE = "state";
    static inline const std::string EDIT = "edit";
    static inline const std::string DEPENDENCY = "dependency";

    static inline const std::string COMMON = "common";
    static inline const std::string SYSTEMD = "systemd";

    explicit SystemdContainerItem();

    CommonItem *getCommon() const;
    void setCommon(const CommonItem &item);

    SystemdItem *getSystemd() const;
    void setSystemd(const SystemdItem &item);

    void setupListeners() override;
    void retranslateStrings() override;

protected:
    SystemdContainerItem(const std::string &typeName, int unitType);
};

class SystemdServiceContainerItem : public SystemdContainerItem
{
public:
    SystemdServiceContainerItem();
};

class SystemdSocketContainerItem : public SystemdContainerItem
{
public:
    SystemdSocketContainerItem();
};

class SystemdTimerContainerItem : public SystemdContainerItem
{
public:
    SystemdTimerContainerItem();
};

class SystemdPathContainerItem : public SystemdContainerItem
{
public:
    SystemdPathContainerItem();
};

class SystemdMountContainerItem : public SystemdContainerItem
{
public:
    SystemdMountContainerItem();
};

class SystemdAutomountContainerItem : public SystemdContainerItem
{
public:
    SystemdAutomountContainerItem();
};

class SystemdSwapContainerItem : public SystemdContainerItem
{
public:
    SystemdSwapContainerItem();
};

class SystemdTargetContainerItem : public SystemdContainerItem
{
public:
    SystemdTargetContainerItem();
};

class SystemdDeviceContainerItem : public SystemdContainerItem
{
public:
    SystemdDeviceContainerItem();
};

class SystemdSliceContainerItem : public SystemdContainerItem
{
public:
    SystemdSliceContainerItem();
};

class SystemdScopeContainerItem : public SystemdContainerItem
{
public:
    SystemdScopeContainerItem();
};

} // namespace preferences

#endif // GPUI_SYSTEMDCONTAINERITEM_H
