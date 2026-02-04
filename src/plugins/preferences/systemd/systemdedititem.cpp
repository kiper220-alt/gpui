#include "systemdedititem.h"

namespace preferences
{

SystemdEditItem::SystemdEditItem()
    : ModelView::CompoundItem("SystemdEditItem")
{
    addProperty(SECTION, "");
    addProperty(KEY, "");
    addProperty(VALUE, "");
}

SystemdEditItem::SystemdEditItem(const SystemdEditItem &other)
    : ModelView::CompoundItem("SystemdEditItem")
{
    addProperty(SECTION, other.property<std::string>(SECTION));
    addProperty(KEY, other.property<std::string>(KEY));
    addProperty(VALUE, other.property<std::string>(VALUE));
}

} // namespace preferences
