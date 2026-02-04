#include "systemditem.h"

#include <mvvm/model/taginfo.h>

namespace preferences
{

SystemdItem::SystemdItem()
    : ModelView::CompoundItem("SystemdItem")
{
    addProperty(UNIT, "");
    addProperty(UNIT_TYPE, static_cast<int>(SystemdUnitType::Service));
    addProperty(STATE, static_cast<int>(SystemdState::AsIs));
    addProperty(STATE_NOW, false);
    addProperty(APPLY_MODE, static_cast<int>(SystemdApplyMode::Always));
    addProperty(POLICY_TARGET, static_cast<int>(SystemdPolicyTarget::Machine));
    addProperty(IDEMPOTENT, true);

    addProperty(EDIT, false);
    addProperty(EDIT_MODE, static_cast<int>(SystemdEditMode::Override));
    addProperty(DROP_IN_NAME, "50-gpo.conf");
    addProperty(CONFLICT_STRATEGY, static_cast<int>(SystemdConflictStrategy::Replace));

    addProperty(DEPENDENCY, false);
    addProperty(HAS_DEPENDENCIES, true);

    registerTag(ModelView::TagInfo::universalTag(EDIT_ITEM_TAG), false);
    registerTag(ModelView::TagInfo::universalTag(DEPENDENCY_ITEM_TAG), false);
}

SystemdItem::SystemdItem(const SystemdItem &other)
    : ModelView::CompoundItem("SystemdItem")
{
    addProperty(UNIT, other.property<std::string>(UNIT));
    addProperty(UNIT_TYPE, other.property<int>(UNIT_TYPE));
    addProperty(STATE, other.property<int>(STATE));
    addProperty(STATE_NOW, other.property<bool>(STATE_NOW));
    addProperty(APPLY_MODE, other.property<int>(APPLY_MODE));
    addProperty(POLICY_TARGET, other.property<int>(POLICY_TARGET));
    addProperty(IDEMPOTENT, other.property<bool>(IDEMPOTENT));

    addProperty(EDIT, other.property<bool>(EDIT));
    addProperty(EDIT_MODE, other.property<int>(EDIT_MODE));
    addProperty(DROP_IN_NAME, other.property<std::string>(DROP_IN_NAME));
    addProperty(CONFLICT_STRATEGY, other.property<int>(CONFLICT_STRATEGY));

    addProperty(DEPENDENCY, other.property<bool>(DEPENDENCY));
    addProperty(HAS_DEPENDENCIES, other.property<bool>(HAS_DEPENDENCIES));

    registerTag(ModelView::TagInfo::universalTag(EDIT_ITEM_TAG), false);
    registerTag(ModelView::TagInfo::universalTag(DEPENDENCY_ITEM_TAG), false);

    for (const auto child : other.editItems())
    {
        if (child)
        {
            insertItem(new SystemdEditItem(*child), EDIT_ITEM_TAG);
        }
    }

    for (const auto child : other.depItems())
    {
        if (child)
        {
            insertItem(new SystemdDependencyItem(*child), DEPENDENCY_ITEM_TAG);
        }
    }
}

size_t SystemdItem::editLength() const
{
    return editItems().size();
}

std::vector<SystemdEditItem *> SystemdItem::editItems() const
{
    return items<SystemdEditItem>(EDIT_ITEM_TAG);
}

void SystemdItem::editLength(size_t size)
{
    while (editItems().size() > size)
    {
        auto current = editItems();
        takeItem({EDIT_ITEM_TAG, static_cast<int>(current.size() - 1)});
    }

    while (editItems().size() < size)
    {
        insertItem(new SystemdEditItem(), EDIT_ITEM_TAG);
    }
}

size_t SystemdItem::depLength() const
{
    return depItems().size();
}

std::vector<SystemdDependencyItem *> SystemdItem::depItems() const
{
    return items<SystemdDependencyItem>(DEPENDENCY_ITEM_TAG);
}

void SystemdItem::depLength(size_t size)
{
    while (depItems().size() > size)
    {
        auto current = depItems();
        takeItem({DEPENDENCY_ITEM_TAG, static_cast<int>(current.size() - 1)});
    }

    while (depItems().size() < size)
    {
        insertItem(new SystemdDependencyItem(), DEPENDENCY_ITEM_TAG);
    }
}

bool SystemdItem::unitTypeHasFileDependencies(int unitType)
{
    return unitType >= static_cast<int>(SystemdUnitType::Service)
        && unitType <= static_cast<int>(SystemdUnitType::Swap);
}

} // namespace preferences
