#include "systemdpreferencereader.h"

#include "schemas/systemdschema.h"
#include "systemdmodelbuilder.h"

namespace preferences 
{

SystemdPreferenceReader::SystemdPreferenceReader()
    : BasePreferenceReader("SystemdContainerItem")
{}

/**
 * @brief Reads a Systemd configuration from an input stream and returns a unique pointer to a PreferencesModel object.
 *
 * @param input The input stream from which to read the Systemd configuration.
 * @return A unique pointer to a PreferencesModel object containing the read configuration.
 */
std::unique_ptr<PreferencesModel> SystemdPreferenceReader::createModel(std::istream &input)
{
    auto schema       = Systemds_(input, ::xsd::cxx::tree::flags::dont_validate);
    auto modelBuilder = std::make_unique<SystemdModelBuilder>();
    return modelBuilder->schemaToModel(schema);
}

} // namespace preferences