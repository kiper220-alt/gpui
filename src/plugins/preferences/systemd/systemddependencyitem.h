#ifndef GPUI_SYSTEMDDEPENDENCYITEM_H
#define GPUI_SYSTEMDDEPENDENCYITEM_H

#include <mvvm/model/compounditem.h>

namespace preferences
{

enum class SystemdDependencyMode
{
    Changed = 0,
    PresenceChanged = 1,
};

class SystemdDependencyItem : public ModelView::CompoundItem
{
public:
    static inline const std::string TYPE = "type";
    static inline const std::string PATH = "path";

    SystemdDependencyItem();
    SystemdDependencyItem(const SystemdDependencyItem &other);
};

} // namespace preferences

Q_DECLARE_METATYPE(::preferences::SystemdDependencyItem)

#endif // GPUI_SYSTEMDDEPENDENCYITEM_H
