#ifndef GPUI_SYSTEMDCONTAINERITEM_H
#define GPUI_SYSTEMDCONTAINERITEM_H

#include "interfaces/containeriteminterface.h"

#include <mvvm/model/compounditem.h>

namespace preferences
{

class CommonItem;
class SystemdItem;

class SystemdContainerItem: public ModelView::CompoundItem, public ContainerItemInterface
{

//! Systemd item representation for editor

public:
    static inline const std::string NAME    = "name";
    static inline const std::string STATE   = "state";
    static inline const std::string EDIT    = "edit";
    static inline const std::string DEPENDENCY = "dependency";

    static inline const std::string COMMON  = "common";
    static inline const std::string SYSTEMD = "systemd";

    explicit SystemdContainerItem();

    CommonItem *getCommon() const;
    void setCommon(const CommonItem &item);

    SystemdItem *getSystemds() const;
    void setSystemds(const SystemdItem &item);

    void setupListeners() override;
    void retranslateStrings() override;
};

} // namespace preferences

#endif // GPUI_SYSTEMDCONTAINERITEM_H
