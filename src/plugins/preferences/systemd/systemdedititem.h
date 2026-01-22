#ifndef GPUI_SYSTEMDEDITITEM_H
#define GPUI_SYSTEMDEDITITEM_H
#include <mvvm/model/compounditem.h>
#include <QMetaType>

namespace preferences 
{

class SystemdEditItem : public ModelView::CompoundItem
{

//! Edit item of systemd item representation for editor.

public:
    static inline const std::string TYPE        = "type";
    static inline const std::string SECTION     = "section";
    static inline const std::string KEY         = "key";
    static inline const std::string VALUE       = "value";


    SystemdEditItem();
    SystemdEditItem(const SystemdEditItem &other);
};

}

Q_DECLARE_METATYPE(::preferences::SystemdEditItem)

#endif // GPUI_SYSTEMDEDITITEM_H