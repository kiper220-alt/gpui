#include "systemddependencyitem.h"

namespace preferences
{

SystemdDependencyItem::SystemdDependencyItem()
    : ModelView::CompoundItem("SystemdDependencyItem")
{
    addProperty(TYPE, static_cast<int>(SystemdDependencyMode::Changed));
    addProperty(PATH, "");
}

SystemdDependencyItem::SystemdDependencyItem(const SystemdDependencyItem &other)
    : ModelView::CompoundItem("SystemdDependencyItem")
{
    addProperty(TYPE, other.property<int>(TYPE));
    addProperty(PATH, other.property<std::string>(PATH));
}

} // namespace preferences
