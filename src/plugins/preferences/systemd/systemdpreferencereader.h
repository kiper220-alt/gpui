#ifndef GPUI_SYSTEMDPREFERENCEREADER_H
#define GPUI_SYSTEMDPREFERENCEREADER_H

#include "common/basepreferencereader.h"

namespace preferences 
{

class SystemdPreferenceReader : public BasePreferenceReader
{
public:
    SystemdPreferenceReader();

private:
    std::unique_ptr<PreferencesModel> createModel(std::istream &input) override;
};

} // namespace preferences

#endif // GPUI_SYSTEMDPREFERENCEREADER_H
