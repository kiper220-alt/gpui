#ifndef GPUI_SYSTEMDDEPENDENCYITEM_H
#define GPUI_SYSTEMDDEPENDENCYITEM_H
#include <mvvm/model/compounditem.h>
#include <QMetaType>

namespace preferences 
{

enum class DependencyState 
{
    Changed,
    PresenceChanged,
};

class SystemdDependencyItem : public ModelView::CompoundItem
{

//! Dependency item of systemd item representation for editor.

public:
    static inline const std::string TYPE        = "type";
    static inline const std::string PATH        = "path";


    SystemdDependencyItem();
    SystemdDependencyItem(const SystemdDependencyItem &other);
};

}

#endif // GPUI_SYSTEMDDEPENDENCYITEM_H