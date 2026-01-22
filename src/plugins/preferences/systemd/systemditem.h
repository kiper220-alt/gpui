#ifndef GPUI_SYSTEMDITEM_H
#define GPUI_SYSTEMDITEM_H

#include <mvvm/model/compounditem.h>
#include <QMetaType>
#include "systemdedititem.h"
#include "systemddependencyitem.h"

namespace preferences 
{

enum class SystemdState {
    Nothing,
    Enable,
    Disable,
    Mask,
    Unmask,
};

class SystemdItem : public ModelView::CompoundItem
{

//! Systemd item representation for editor.

public:
    static inline const std::string UNIT        = "unit";
    static inline const std::string STATE       = "state";
    static inline const std::string STATE_NOW   = "state_now";
    static inline const std::string EDIT        = "edit";
    static inline const std::string DEPENDENCY     = "dependency";

    static inline const std::string EDIT_ITEM_TAG    = "SytemdItemEditTag";
    static inline const std::string DEPENDENCY_ITEM_TAG = "SystemdItemDependencyTag";

    SystemdItem();
    SystemdItem(const SystemdItem &other);

    size_t editLength();
    std::vector<SystemdEditItem *> editItems();
    void editLength(size_t);

    size_t depLength();
    std::vector<SystemdDependencyItem *> depItems();
    void depLength(size_t);
};

} // namespace preferences

Q_DECLARE_METATYPE(::preferences::SystemdDependencyItem)
Q_DECLARE_METATYPE(::preferences::SystemdItem)

#endif // GPUI_SYSTEMDITEM_H
