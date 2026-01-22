#ifndef GPUI_SYSTEMD_MODEL_BUILDER_H
#define GPUI_SYSTEMD_MODEL_BUILDER_H

#include <memory>

#include "common/basemodelbuilder.h"
#include "common/preferencesmodel.h"
#include "schemas/systemdschema.h"

namespace preferences
{
class SystemdModelBuilder : public BaseModelBuilder
{
public:
    SystemdModelBuilder();

    std::unique_ptr<PreferencesModel> schemaToModel(std::unique_ptr<Systemds> &shortcuts);

    std::unique_ptr<Systemds> modelToSchema(std::unique_ptr<PreferencesModel> &model);
};
} // namespace preferences

#endif // GPUI_SYSTEMD_MODEL_BUILDER_H
