#include "systemdedititem.h"

namespace preferences 
{

SystemdEditItem::SystemdEditItem() 
    : ModelView::CompoundItem("SystemdEditItem")
{
    addProperty(TYPE, 0);
    addProperty(SECTION, "");
    addProperty(KEY, "");
    addProperty(VALUE, "");
}
SystemdEditItem::SystemdEditItem(const SystemdEditItem &other) 
    : ModelView::CompoundItem("SystemdEditItem")
{
    addProperty(TYPE, other.property<int>(TYPE));
    addProperty(SECTION, other.property<QString>(SECTION));
    addProperty(KEY, other.property<QString>(KEY));
    addProperty(VALUE, other.property<QString>(VALUE));
}

} // namespace preferences