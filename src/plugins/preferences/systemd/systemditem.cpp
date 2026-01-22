#include "systemditem.h"
#include "systemdedititem.h"
#include "systemddependencyitem.h"

namespace preferences 
{

SystemdItem::SystemdItem()
    : ModelView::CompoundItem("SystemdItem")
{
    addProperty(UNIT, "");
    addProperty(STATE, 0);
    addProperty(STATE_NOW, false);
    addProperty(EDIT, false);
    addProperty(DEPENDENCY, false);
    registerTag(ModelView::TagInfo::universalTag(SystemdItem::EDIT_ITEM_TAG), false);
    registerTag(ModelView::TagInfo::universalTag(SystemdItem::DEPENDENCY_ITEM_TAG), false);
}

SystemdItem::SystemdItem(const SystemdItem &other)
    : ModelView::CompoundItem("SystemdItem")
{
    addProperty(UNIT, other.property<std::string>(UNIT));
    addProperty(STATE, other.property<int>(STATE));
    addProperty(STATE_NOW, other.property<bool>(STATE_NOW));
    addProperty(EDIT, other.property<bool>(EDIT));
    addProperty(DEPENDENCY, other.property<bool>(DEPENDENCY));
    registerTag(ModelView::TagInfo::universalTag(EDIT_ITEM_TAG), false);
    registerTag(ModelView::TagInfo::universalTag(DEPENDENCY_ITEM_TAG), false);

    for (auto child : other.items<SystemdEditItem>(EDIT)) 
    {
        if (child) 
        {
            insertItem(new SystemdEditItem(*child), EDIT);
        }
    }
    for (auto child : other.items<SystemdDependencyItem>(DEPENDENCY))
    {
        if (child) 
        {
            insertItem(new SystemdDependencyItem(*child), DEPENDENCY);
        }
    }
}

size_t SystemdItem::editLength() 
{
    return items(EDIT_ITEM_TAG).size();
}
std::vector<SystemdEditItem *> SystemdItem::editItems() 
{
    return items<SystemdEditItem>(EDIT_ITEM_TAG);
}
void SystemdItem::editLength(size_t size)
{
    const auto length = static_cast<ptrdiff_t>(editItems().size());
    if (length > size)
    {
        for (ptrdiff_t i = length - 1; i >= static_cast<ptrdiff_t>(size); --i)
        {
            takeItem({EDIT_ITEM_TAG, static_cast<int>(i)});
        }
    }
    if (size > length)
    {
        for (ptrdiff_t i = length; i < static_cast<ptrdiff_t>(size); ++i)
        {
            insertItem(new SystemdEditItem(), EDIT_ITEM_TAG);
        }
    }
}

size_t SystemdItem::depLength()
{
    return items(DEPENDENCY_ITEM_TAG).size();
}
std::vector<SystemdDependencyItem *> SystemdItem::depItems()
{
    return items<SystemdDependencyItem>(DEPENDENCY_ITEM_TAG);
}
void SystemdItem::depLength(size_t size)
{
    const auto length = depItems().size();
    if (length > size)
    {
        for (ptrdiff_t i = length - 1; i >= static_cast<ptrdiff_t>(size); --i)
        {
            takeItem({DEPENDENCY_ITEM_TAG, static_cast<int>(i)});
        }
    }
    if (size > length)
    {
        for (ptrdiff_t i = length; i < static_cast<ptrdiff_t>(size); ++i)
        {
            insertItem(new SystemdDependencyItem(), DEPENDENCY_ITEM_TAG);
        }
    }
}

} // namespace preferences