#include "systemdpreferencereader.h"

#include "schemas/systemdschema.h"
#include "systemdmodelbuilder.h"

namespace preferences
{

SystemdPreferenceReader::SystemdPreferenceReader()
    : BasePreferenceReader("SystemdContainerItem")
{}

std::unique_ptr<PreferencesModel> SystemdPreferenceReader::createModel(std::istream &input)
{
    auto schema = Systemds_(input, ::xsd::cxx::tree::flags::dont_validate);
    auto modelBuilder = std::make_unique<SystemdModelBuilder>();
    return modelBuilder->schemaToModel(schema);
}

} // namespace preferences
