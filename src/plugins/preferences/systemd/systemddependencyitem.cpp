#include "systemditem.h"
#include "systemddependencyitem.h"

namespace preferences 
{

SystemdDependencyItem::SystemdDependencyItem() 
    : ModelView::CompoundItem("SystemdDependencyItem")
{
    addProperty(TYPE, 0);
    addProperty(PATH, "");
}
SystemdDependencyItem::SystemdDependencyItem(const SystemdDependencyItem &other) 
    : ModelView::CompoundItem("SystemdDependencyItem")
{
    addProperty(TYPE, other.property<int>(TYPE));
    addProperty(PATH, other.property<QString>(PATH));
}

} // namespace preferences