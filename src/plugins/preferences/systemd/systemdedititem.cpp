#include "systemdedititem.h"

#include "systemditem.h"

namespace preferences
{

SystemdEditItem::SystemdEditItem()
    : ModelView::CompoundItem("SystemdEditItem")
{
    addProperty(SECTION, "");
    addProperty(KEY, "");
    addProperty(VALUE, "");
    addProperty(STRATEGY, static_cast<int>(SystemdConflictStrategy::ReplaceValue));
}

SystemdEditItem::SystemdEditItem(const SystemdEditItem &other)
    : ModelView::CompoundItem("SystemdEditItem")
{
    addProperty(SECTION, other.property<std::string>(SECTION));
    addProperty(KEY, other.property<std::string>(KEY));
    addProperty(VALUE, other.property<std::string>(VALUE));
    addProperty(STRATEGY, other.property<int>(STRATEGY));
}

} // namespace preferences
