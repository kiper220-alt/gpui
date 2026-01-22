#include "systemdpreferencewriter.h"

#include "systemdmodelbuilder.h"

namespace preferences 
{

SystemdPreferenceWriter::SystemdPreferenceWriter()
    : BasePreferenceWriter("SystemdContainerItem")
{}

/**
 * Writes the given model to the output stream in the format of Systemd configuration files.
 *
 * @param output The output stream to write the model to.
 * @param model The model to write to the output stream.
 * @return true if the model was written successfully, false otherwise.
 */
bool SystemdPreferenceWriter::writeModel(std::ostream &output, const std::unique_ptr<PreferencesModel> &model)
{
    auto modelBuilder = std::make_unique<SystemdModelBuilder>();
    auto systemds    = modelBuilder->modelToSchema(const_cast<std::unique_ptr<PreferencesModel> &>(model));
    const ::xml_schema::NamespaceInfomap map;
    Systemds_(output, *systemds.get(), map);
    return true;
}

} // namespace preferences