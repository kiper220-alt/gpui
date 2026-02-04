#ifndef GPUI_SYSTEMDITEM_H
#define GPUI_SYSTEMDITEM_H

#include <mvvm/model/compounditem.h>

#include "systemddependencyitem.h"
#include "systemdedititem.h"

namespace preferences
{

enum class SystemdUnitType
{
    Service = 0,
    Socket,
    Timer,
    Path,
    Mount,
    Automount,
    Swap,
    Target,
    Device,
    Slice,
    Scope,
};

enum class SystemdState
{
    AsIs = 0,
    Enable,
    Disable,
    Mask,
    Unmask,
    Preset,
};

enum class SystemdApplyMode
{
    Always = 0,
    IfExists,
    IfMissing,
};

enum class SystemdPolicyTarget
{
    Machine = 0,
    User,
};

enum class SystemdEditMode
{
    Create = 0,
    Override,
    CreateOrOverride,
};

enum class SystemdConflictStrategy
{
    Replace = 0,
    Append,
    Merge,
    IgnoreIfExists,
};

class SystemdItem : public ModelView::CompoundItem
{
public:
    static inline const std::string UNIT = "unit";
    static inline const std::string UNIT_TYPE = "unit_type";
    static inline const std::string STATE = "state";
    static inline const std::string STATE_NOW = "state_now";
    static inline const std::string APPLY_MODE = "apply_mode";
    static inline const std::string POLICY_TARGET = "policy_target";
    static inline const std::string IDEMPOTENT = "idempotent";

    static inline const std::string EDIT = "edit";
    static inline const std::string EDIT_MODE = "edit_mode";
    static inline const std::string DROP_IN_NAME = "drop_in_name";
    static inline const std::string CONFLICT_STRATEGY = "conflict_strategy";

    static inline const std::string DEPENDENCY = "dependency";
    static inline const std::string HAS_DEPENDENCIES = "has_dependencies";

    static inline const std::string EDIT_ITEM_TAG = "SystemdEditItemTag";
    static inline const std::string DEPENDENCY_ITEM_TAG = "SystemdDependencyItemTag";

    SystemdItem();
    SystemdItem(const SystemdItem &other);

    size_t editLength() const;
    std::vector<SystemdEditItem *> editItems() const;
    void editLength(size_t size);

    size_t depLength() const;
    std::vector<SystemdDependencyItem *> depItems() const;
    void depLength(size_t size);

    static bool unitTypeHasFileDependencies(int unitType);
};

} // namespace preferences

Q_DECLARE_METATYPE(::preferences::SystemdItem)

#endif // GPUI_SYSTEMDITEM_H
