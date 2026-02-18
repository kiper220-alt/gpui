#ifndef GPUI_SYSTEMDEDITITEM_H
#define GPUI_SYSTEMDEDITITEM_H

#include <mvvm/model/compounditem.h>

namespace preferences
{

class SystemdEditItem : public ModelView::CompoundItem
{
public:
    static inline const std::string SECTION = "section";
    static inline const std::string KEY = "key";
    static inline const std::string VALUE = "value";
    static inline const std::string STRATEGY = "strategy";

    SystemdEditItem();
    SystemdEditItem(const SystemdEditItem &other);
};

} // namespace preferences

Q_DECLARE_METATYPE(::preferences::SystemdEditItem)

#endif // GPUI_SYSTEMDEDITITEM_H
