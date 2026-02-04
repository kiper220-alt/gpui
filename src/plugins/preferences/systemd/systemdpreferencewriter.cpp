#include "systemdpreferencewriter.h"

#include "systemdmodelbuilder.h"

namespace preferences
{

SystemdPreferenceWriter::SystemdPreferenceWriter()
    : BasePreferenceWriter("SystemdContainerItem")
{}

bool SystemdPreferenceWriter::writeModel(std::ostream &output, const std::unique_ptr<PreferencesModel> &model)
{
    auto modelBuilder = std::make_unique<SystemdModelBuilder>();
    auto systemds = modelBuilder->modelToSchema(const_cast<std::unique_ptr<PreferencesModel> &>(model));

    const ::xml_schema::NamespaceInfomap map;
    Systemds_(output, *systemds, map);

    return true;
}

} // namespace preferences
