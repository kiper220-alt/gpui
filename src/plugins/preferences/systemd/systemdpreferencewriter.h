#ifndef GPUI_SYSTEMDPREFERENCEWRITER_H
#define GPUI_SYSTEMDPREFERENCEWRITER_H

#include "common/basepreferencewriter.h"

namespace preferences 
{

class SystemdPreferenceWriter: public BasePreferenceWriter
{
public:
    SystemdPreferenceWriter();

protected:
    bool writeModel(std::ostream &input, const std::unique_ptr<PreferencesModel> &model) override;
};

} // namespace preferences

#endif // GPUI_SYSTEMDPREFERENCEWRITER_H
