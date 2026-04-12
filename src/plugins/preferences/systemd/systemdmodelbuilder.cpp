#include "systemdmodelbuilder.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <type_traits>

#include "common/commonitem.h"
#include "systemdcontaineritem.h"
#include "systemditem.h"

namespace
{

const std::string GENERATED_EDITS_HEADER = "# GPUI-GENERATED: edits-from-table";

std::string unitSuffix(preferences::SystemdUnitType type)
{
    using preferences::SystemdUnitType;

    switch (type)
    {
    case SystemdUnitType::Service:
        return ".service";
    case SystemdUnitType::Socket:
        return ".socket";
    case SystemdUnitType::Timer:
        return ".timer";
    case SystemdUnitType::Path:
        return ".path";
    case SystemdUnitType::Mount:
        return ".mount";
    case SystemdUnitType::Automount:
        return ".automount";
    case SystemdUnitType::Swap:
        return ".swap";
    case SystemdUnitType::Target:
        return ".target";
    case SystemdUnitType::Device:
        return ".device";
    case SystemdUnitType::Slice:
        return ".slice";
    case SystemdUnitType::Scope:
        return ".scope";
    default:
        return "";
    }
}

bool endsWith(const std::string &value, const std::string &suffix)
{
    return value.size() >= suffix.size()
        && value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string trimUnitSuffix(const std::string &fullName, preferences::SystemdUnitType unitType)
{
    const auto suffix = unitSuffix(unitType);
    if (suffix.empty() || !endsWith(fullName, suffix))
    {
        return fullName;
    }

    return fullName.substr(0, fullName.size() - suffix.size());
}

std::string addUnitSuffixIfNeeded(const std::string &name, preferences::SystemdUnitType unitType)
{
    const auto suffix = unitSuffix(unitType);
    if (suffix.empty() || endsWith(name, suffix))
    {
        return name;
    }

    return name + suffix;
}

UnitState_t toSchemaState(int state)
{
    switch (static_cast<preferences::SystemdState>(state))
    {
    case preferences::SystemdState::Enable:
        return UnitState_t(UnitState_t::enable);
    case preferences::SystemdState::Disable:
        return UnitState_t(UnitState_t::disable);
    case preferences::SystemdState::Mask:
        return UnitState_t(UnitState_t::mask);
    case preferences::SystemdState::Unmask:
        return UnitState_t(UnitState_t::unmask);
    case preferences::SystemdState::Preset:
        return UnitState_t(UnitState_t::preset);
    case preferences::SystemdState::AsIs:
    default:
        return UnitState_t(UnitState_t::as_is);
    }
}

int fromSchemaState(const UnitState_t &state)
{
    return static_cast<int>(static_cast<UnitState_t::Value>(state));
}

UnitApplyMode_t toSchemaApplyMode(int applyMode)
{
    switch (static_cast<preferences::SystemdApplyMode>(applyMode))
    {
    case preferences::SystemdApplyMode::IfExists:
        return UnitApplyMode_t(UnitApplyMode_t::if_exists);
    case preferences::SystemdApplyMode::IfMissing:
        return UnitApplyMode_t(UnitApplyMode_t::if_missing);
    case preferences::SystemdApplyMode::Always:
    default:
        return UnitApplyMode_t(UnitApplyMode_t::always);
    }
}

int fromSchemaApplyMode(const UnitApplyMode_t &mode)
{
    return static_cast<int>(static_cast<UnitApplyMode_t::Value>(mode));
}

UnitPolicyTarget_t toSchemaPolicyTarget(int policyTarget)
{
    switch (static_cast<preferences::SystemdPolicyTarget>(policyTarget))
    {
    case preferences::SystemdPolicyTarget::User:
        return UnitPolicyTarget_t(UnitPolicyTarget_t::user);
    case preferences::SystemdPolicyTarget::Machine:
    default:
        return UnitPolicyTarget_t(UnitPolicyTarget_t::machine);
    }
}

int fromSchemaPolicyTarget(const UnitPolicyTarget_t &policyTarget)
{
    return static_cast<int>(static_cast<UnitPolicyTarget_t::Value>(policyTarget));
}

int editModeFromApplyMode(int applyMode)
{
    switch (static_cast<preferences::SystemdApplyMode>(applyMode))
    {
    case preferences::SystemdApplyMode::IfExists:
        return static_cast<int>(preferences::SystemdEditMode::Override);
    case preferences::SystemdApplyMode::IfMissing:
        return static_cast<int>(preferences::SystemdEditMode::Create);
    case preferences::SystemdApplyMode::Always:
    default:
        return static_cast<int>(preferences::SystemdEditMode::CreateOrOverride);
    }
}

UnitFileMode_t toSchemaUnitFileMode(int mode)
{
    switch (static_cast<preferences::SystemdUnitFileMode>(mode))
    {
    case preferences::SystemdUnitFileMode::Text:
        return UnitFileMode_t(UnitFileMode_t::text);
    case preferences::SystemdUnitFileMode::Table:
    default:
        return UnitFileMode_t(UnitFileMode_t::table);
    }
}

int fromSchemaUnitFileMode(const UnitFileMode_t &mode)
{
    return static_cast<int>(static_cast<UnitFileMode_t::Value>(mode));
}

std::string trim(const std::string &value)
{
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos)
    {
        return "";
    }

    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

std::string buildGeneratedUnitFile(const std::vector<preferences::SystemdEditItem *> &editItems)
{
    struct EditLine
    {
        std::string section;
        std::string key;
        std::string value;
        preferences::SystemdConflictStrategy strategy;
        size_t sourceIndex;
    };

    std::ostringstream fileText;
    fileText << GENERATED_EDITS_HEADER << "\n";

    std::vector<EditLine> lines;
    lines.reserve(editItems.size());

    for (size_t i = 0; i < editItems.size(); ++i)
    {
        const auto *editItem = editItems.at(i);
        const auto section = trim(editItem->property<std::string>(preferences::SystemdEditItem::SECTION));
        const auto key = trim(editItem->property<std::string>(preferences::SystemdEditItem::KEY));
        if (section.empty() || key.empty())
        {
            continue;
        }

        const auto value = editItem->property<std::string>(preferences::SystemdEditItem::VALUE);
        const auto strategy = static_cast<preferences::SystemdConflictStrategy>(
            editItem->property<int>(preferences::SystemdEditItem::STRATEGY));
        lines.push_back(EditLine{section, key, value, strategy, i});
    }

    std::sort(lines.begin(), lines.end(), [](const EditLine &lhs, const EditLine &rhs) {
        if (lhs.section != rhs.section)
        {
            return lhs.section < rhs.section;
        }
        if (lhs.key != rhs.key)
        {
            return lhs.key < rhs.key;
        }
        return lhs.sourceIndex < rhs.sourceIndex;
    });

    std::string currentSection;
    bool firstSection = true;
    for (const auto &line : lines)
    {
        if (line.section != currentSection)
        {
            currentSection = line.section;
            if (!firstSection)
            {
                fileText << "\n";
            }
            fileText << "[" << currentSection << "]\n";
            firstSection = false;
        }

        switch (line.strategy)
        {
        case preferences::SystemdConflictStrategy::ReplaceValue:
            fileText << line.key << "=\n";
            if (!line.value.empty())
            {
                fileText << line.key << "=" << line.value << "\n";
            }
            break;
        case preferences::SystemdConflictStrategy::ResetKey:
            fileText << line.key << "=\n";
            break;
        case preferences::SystemdConflictStrategy::AddValue:
        default:
            fileText << line.key << "=" << line.value << "\n";
            break;
        }
    }

    return fileText.str();
}

void appendParsedEdits(preferences::SystemdItem *item, const std::string &unitFileText)
{
    struct ParsedLine
    {
        ParsedLine()
            : section()
            , key()
            , value()
        {
        }

        std::string section;
        std::string key;
        std::string value;
    };

    std::istringstream stream(unitFileText);
    std::string currentSection;
    std::vector<ParsedLine> parsedLines;

    std::string line;
    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        const auto normalizedLine = trim(line);
        if (normalizedLine.empty() || normalizedLine.rfind("#", 0) == 0 || normalizedLine.rfind(";", 0) == 0)
        {
            continue;
        }

        if (normalizedLine.front() == '[' && normalizedLine.back() == ']')
        {
            currentSection = trim(normalizedLine.substr(1, normalizedLine.size() - 2));
            continue;
        }

        const auto equalsPos = normalizedLine.find('=');
        if (equalsPos == std::string::npos || currentSection.empty())
        {
            continue;
        }

        ParsedLine parsed;
        parsed.section = currentSection;
        parsed.key = trim(normalizedLine.substr(0, equalsPos));
        parsed.value = normalizedLine.substr(equalsPos + 1);
        if (!parsed.key.empty())
        {
            parsedLines.push_back(parsed);
        }
    }

    for (size_t i = 0; i < parsedLines.size();)
    {
        const auto &current = parsedLines.at(i);
        auto *editItem = item->insertItem<preferences::SystemdEditItem>(preferences::SystemdItem::EDIT_ITEM_TAG);
        editItem->setProperty(preferences::SystemdEditItem::SECTION, current.section);
        editItem->setProperty(preferences::SystemdEditItem::KEY, current.key);

        if (current.value.empty())
        {
            const bool hasReplaceSecondLine = (i + 1) < parsedLines.size()
                && parsedLines.at(i + 1).section == current.section
                && parsedLines.at(i + 1).key == current.key;

            if (hasReplaceSecondLine)
            {
                editItem->setProperty(preferences::SystemdEditItem::VALUE, parsedLines.at(i + 1).value);
                editItem->setProperty(preferences::SystemdEditItem::STRATEGY,
                                      static_cast<int>(preferences::SystemdConflictStrategy::ReplaceValue));
                i += 2;
                continue;
            }

            editItem->setProperty(preferences::SystemdEditItem::VALUE, std::string());
            editItem->setProperty(preferences::SystemdEditItem::STRATEGY,
                                  static_cast<int>(preferences::SystemdConflictStrategy::ResetKey));
            ++i;
            continue;
        }

        editItem->setProperty(preferences::SystemdEditItem::VALUE, current.value);
        editItem->setProperty(preferences::SystemdEditItem::STRATEGY,
                              static_cast<int>(preferences::SystemdConflictStrategy::AddValue));
        ++i;
    }
}

FileDependencyMode_t toSchemaDependencyMode(int mode)
{
    switch (static_cast<preferences::SystemdDependencyMode>(mode))
    {
    case preferences::SystemdDependencyMode::PresenceChanged:
        return FileDependencyMode_t(FileDependencyMode_t::presence_changed);
    case preferences::SystemdDependencyMode::Changed:
    default:
        return FileDependencyMode_t(FileDependencyMode_t::changed);
    }
}

int fromSchemaDependencyMode(const FileDependencyMode_t &mode)
{
    return static_cast<int>(static_cast<FileDependencyMode_t::Value>(mode));
}

template<typename PropertiesType>
void fillModelFromUnitProperties(preferences::SystemdItem *item,
                                 const PropertiesType &properties,
                                 preferences::SystemdUnitType unitType)
{
    item->setProperty(preferences::SystemdItem::UNIT, trimUnitSuffix(properties.unit().c_str(), unitType));
    item->setProperty(preferences::SystemdItem::STATE, fromSchemaState(properties.state()));
    item->setProperty(preferences::SystemdItem::STATE_NOW, static_cast<bool>(properties.now()));
    item->setProperty(preferences::SystemdItem::APPLY_MODE, fromSchemaApplyMode(properties.applyMode()));
    item->setProperty(preferences::SystemdItem::POLICY_TARGET, fromSchemaPolicyTarget(properties.policyTarget()));
    item->setProperty(preferences::SystemdItem::EDIT_MODE,
                      editModeFromApplyMode(fromSchemaApplyMode(properties.applyMode())));
    item->setProperty(preferences::SystemdItem::DROP_IN_NAME, properties.dropInName().present()
                                                            ? properties.dropInName().get().c_str()
                                                            : std::string());
}

template<typename PropertiesType>
void fillModelEdits(preferences::SystemdItem *item, const PropertiesType &properties)
{
    if (!properties.UnitFile().present())
    {
        item->setProperty(preferences::SystemdItem::EDIT, false);
        item->setProperty(preferences::SystemdItem::UNIT_FILE_TEXT, std::string());
        item->setProperty(preferences::SystemdItem::UNIT_FILE_MODE,
                          static_cast<int>(preferences::SystemdUnitFileMode::Table));
        return;
    }

    item->setProperty(preferences::SystemdItem::EDIT, true);
    const auto &unitFile = properties.UnitFile().get();
    item->setProperty(preferences::SystemdItem::UNIT_FILE_MODE, fromSchemaUnitFileMode(unitFile.mode()));
    item->setProperty(preferences::SystemdItem::UNIT_FILE_TEXT, static_cast<std::string>(unitFile));
    item->editLength(0);
    appendParsedEdits(item, static_cast<std::string>(unitFile));
}

void fillModelDependencies(preferences::SystemdItem *item, const UnitWithEditsAndFileDeps_t &properties)
{
    if (!properties.FileDependencies().present())
    {
        item->setProperty(preferences::SystemdItem::DEPENDENCY, false);
        return;
    }

    item->setProperty(preferences::SystemdItem::DEPENDENCY, true);

    for (const auto &depSchema : properties.FileDependencies().get().Dependency())
    {
        auto *depItem = item->insertItem<preferences::SystemdDependencyItem>(preferences::SystemdItem::DEPENDENCY_ITEM_TAG);
        depItem->setProperty(preferences::SystemdDependencyItem::TYPE, fromSchemaDependencyMode(depSchema.mode()));
        depItem->setProperty(preferences::SystemdDependencyItem::PATH, depSchema.path().c_str());
    }
}

template<typename PolicyType, typename PropertiesType, typename CommonSetter>
void appendPoliciesToModel(const xsd::cxx::tree::sequence<PolicyType> &policies,
                           preferences::SystemdUnitType unitType,
                           std::unique_ptr<preferences::PreferencesModel> &model,
                           CommonSetter setCommonData)
{
    for (const auto &policy : policies)
    {
        auto *container = model->insertItem<preferences::SystemdContainerItem>(model->rootItem());
        container->setupListeners();

        auto *systemdItem = container->getSystemd();
        const auto hasDependencies = preferences::SystemdItem::unitTypeHasFileDependencies(static_cast<int>(unitType));
        systemdItem->setProperty(preferences::SystemdItem::UNIT_TYPE, static_cast<int>(unitType));
        systemdItem->setProperty(preferences::SystemdItem::HAS_DEPENDENCIES, hasDependencies);

        const auto &properties = policy.Properties();
        fillModelFromUnitProperties(systemdItem, properties, unitType);
        fillModelEdits(systemdItem, properties);

        if constexpr (std::is_same_v<PropertiesType, UnitWithEditsAndFileDeps_t>)
        {
            fillModelDependencies(systemdItem, properties);
        }
        else
        {
            systemdItem->setProperty(preferences::SystemdItem::DEPENDENCY, false);
        }

        auto *common = container->getCommon();
        setCommonData(common, policy);
    }
}

UnitFile_t createSchemaUnitFile(const preferences::SystemdItem *systemdItem)
{
    const auto mode = static_cast<preferences::SystemdUnitFileMode>(
        systemdItem->property<int>(preferences::SystemdItem::UNIT_FILE_MODE));
    const auto unitFileText = mode == preferences::SystemdUnitFileMode::Text
                            ? systemdItem->property<std::string>(preferences::SystemdItem::UNIT_FILE_TEXT)
                            : buildGeneratedUnitFile(systemdItem->editItems());
    UnitFile_t unitFile(unitFileText);
    unitFile.mode(toSchemaUnitFileMode(static_cast<int>(mode)));
    return unitFile;
}

FileDependencies_t createSchemaDependencies(const preferences::SystemdItem *systemdItem)
{
    FileDependencies_t dependencies;

    for (const auto *depItem : systemdItem->depItems())
    {
        const auto path = depItem->property<std::string>(preferences::SystemdDependencyItem::PATH);
        if (path.empty())
        {
            continue;
        }

        FileDependence_t dependency(
            toSchemaDependencyMode(depItem->property<int>(preferences::SystemdDependencyItem::TYPE)),
            path
        );

        dependencies.Dependency().push_back(dependency);
    }

    return dependencies;
}

UnitWithEdits_t createEditsOnlyProperties(const preferences::SystemdItem *systemdItem, const std::string &unit)
{
    UnitWithEdits_t properties(unit, toSchemaState(systemdItem->property<int>(preferences::SystemdItem::STATE)));
    properties.now(systemdItem->property<bool>(preferences::SystemdItem::STATE_NOW));
    properties.applyMode(toSchemaApplyMode(systemdItem->property<int>(preferences::SystemdItem::APPLY_MODE)));
    properties.policyTarget(toSchemaPolicyTarget(systemdItem->property<int>(preferences::SystemdItem::POLICY_TARGET)));
    const auto dropInName = systemdItem->property<std::string>(preferences::SystemdItem::DROP_IN_NAME);
    if (!dropInName.empty())
    {
        properties.dropInName(dropInName);
    }

    if (systemdItem->property<bool>(preferences::SystemdItem::EDIT))
    {
        properties.UnitFile(createSchemaUnitFile(systemdItem));
    }

    return properties;
}

UnitWithEditsAndFileDeps_t createFileDepsProperties(const preferences::SystemdItem *systemdItem,
                                                     const std::string &unit)
{
    UnitWithEditsAndFileDeps_t properties(unit, toSchemaState(systemdItem->property<int>(preferences::SystemdItem::STATE)));
    properties.now(systemdItem->property<bool>(preferences::SystemdItem::STATE_NOW));
    properties.applyMode(toSchemaApplyMode(systemdItem->property<int>(preferences::SystemdItem::APPLY_MODE)));
    properties.policyTarget(toSchemaPolicyTarget(systemdItem->property<int>(preferences::SystemdItem::POLICY_TARGET)));
    const auto dropInName = systemdItem->property<std::string>(preferences::SystemdItem::DROP_IN_NAME);
    if (!dropInName.empty())
    {
        properties.dropInName(dropInName);
    }

    if (systemdItem->property<bool>(preferences::SystemdItem::EDIT))
    {
        properties.UnitFile(createSchemaUnitFile(systemdItem));
    }

    if (systemdItem->property<bool>(preferences::SystemdItem::DEPENDENCY))
    {
        properties.FileDependencies(createSchemaDependencies(systemdItem));
    }

    return properties;
}

} // namespace

namespace preferences
{

SystemdModelBuilder::SystemdModelBuilder()
    : BaseModelBuilder()
{}

std::unique_ptr<PreferencesModel> SystemdModelBuilder::schemaToModel(std::unique_ptr<Systemds> &systemds)
{
    auto model = std::make_unique<PreferencesModel>();

    auto setCommonData = [this](auto *common, const auto &policy) { this->setCommonItemData(common, policy); };

    appendPoliciesToModel<PolicyWithFileDeps_t, UnitWithEditsAndFileDeps_t>(
        systemds->Service(), SystemdUnitType::Service, model, setCommonData);
    appendPoliciesToModel<PolicyWithFileDeps_t, UnitWithEditsAndFileDeps_t>(
        systemds->Socket(), SystemdUnitType::Socket, model, setCommonData);
    appendPoliciesToModel<PolicyWithFileDeps_t, UnitWithEditsAndFileDeps_t>(
        systemds->Timer(), SystemdUnitType::Timer, model, setCommonData);
    appendPoliciesToModel<PolicyWithFileDeps_t, UnitWithEditsAndFileDeps_t>(
        systemds->Path(), SystemdUnitType::Path, model, setCommonData);
    appendPoliciesToModel<PolicyWithFileDeps_t, UnitWithEditsAndFileDeps_t>(
        systemds->Mount(), SystemdUnitType::Mount, model, setCommonData);
    appendPoliciesToModel<PolicyWithFileDeps_t, UnitWithEditsAndFileDeps_t>(
        systemds->Automount(), SystemdUnitType::Automount, model, setCommonData);
    appendPoliciesToModel<PolicyWithFileDeps_t, UnitWithEditsAndFileDeps_t>(
        systemds->Swap(), SystemdUnitType::Swap, model, setCommonData);

    appendPoliciesToModel<PolicyEditsOnly_t, UnitWithEdits_t>(
        systemds->Target(), SystemdUnitType::Target, model, setCommonData);
    appendPoliciesToModel<PolicyEditsOnly_t, UnitWithEdits_t>(
        systemds->Device(), SystemdUnitType::Device, model, setCommonData);
    appendPoliciesToModel<PolicyEditsOnly_t, UnitWithEdits_t>(
        systemds->Slice(), SystemdUnitType::Slice, model, setCommonData);
    appendPoliciesToModel<PolicyEditsOnly_t, UnitWithEdits_t>(
        systemds->Scope(), SystemdUnitType::Scope, model, setCommonData);

    return model;
}

std::unique_ptr<Systemds> SystemdModelBuilder::modelToSchema(const std::unique_ptr<PreferencesModel> &model)
{
    auto systemds = std::make_unique<Systemds>("{48ABBD05-0169-4696-8F2C-E1E42A499ADF}");

    for (const auto *containerItem : model->topItems())
    {
        auto *systemdContainer = dynamic_cast<const SystemdContainerItem *>(containerItem);
        if (!systemdContainer)
        {
            continue;
        }

        auto *systemdModel = systemdContainer->getSystemd();
        auto *commonModel = systemdContainer->getCommon();

        const auto unitType = static_cast<SystemdUnitType>(systemdModel->property<int>(SystemdItem::UNIT_TYPE));
        const auto unitName = addUnitSuffixIfNeeded(systemdModel->property<std::string>(SystemdItem::UNIT), unitType);

        const auto clsidKey = CommonItem::propertyToString(CommonItem::CLSID);
        const auto nameKey = CommonItem::propertyToString(CommonItem::NAME);
        const auto uidKey = CommonItem::propertyToString(CommonItem::UID);
        const auto changedKey = CommonItem::propertyToString(CommonItem::CHANGED);

        if (commonModel->property<std::string>(uidKey).empty())
        {
            continue;
        }

        if (commonModel->property<std::string>(clsidKey).empty())
        {
            commonModel->setProperty(clsidKey, "{613BDB77-EC4A-4EA1-BA1D-42FD47762D0E}");
        }

        auto setCommon = [&](auto &policy) {
            commonModel->setProperty(changedKey, createDateOfChange());
            commonModel->setProperty(nameKey, unitName);
            setCommonModelData(policy, commonModel);
        };

        switch (unitType)
        {
        case SystemdUnitType::Service:
        {
            auto properties = createFileDepsProperties(systemdModel, unitName);
            PolicyWithFileDeps_t policy(commonModel->property<std::string>(clsidKey),
                                        commonModel->property<std::string>(nameKey),
                                        commonModel->property<std::string>(uidKey),
                                        properties);
            setCommon(policy);
            systemds->Service().push_back(policy);
            break;
        }
        case SystemdUnitType::Socket:
        {
            auto properties = createFileDepsProperties(systemdModel, unitName);
            PolicyWithFileDeps_t policy(commonModel->property<std::string>(clsidKey),
                                        commonModel->property<std::string>(nameKey),
                                        commonModel->property<std::string>(uidKey),
                                        properties);
            setCommon(policy);
            systemds->Socket().push_back(policy);
            break;
        }
        case SystemdUnitType::Timer:
        {
            auto properties = createFileDepsProperties(systemdModel, unitName);
            PolicyWithFileDeps_t policy(commonModel->property<std::string>(clsidKey),
                                        commonModel->property<std::string>(nameKey),
                                        commonModel->property<std::string>(uidKey),
                                        properties);
            setCommon(policy);
            systemds->Timer().push_back(policy);
            break;
        }
        case SystemdUnitType::Path:
        {
            auto properties = createFileDepsProperties(systemdModel, unitName);
            PolicyWithFileDeps_t policy(commonModel->property<std::string>(clsidKey),
                                        commonModel->property<std::string>(nameKey),
                                        commonModel->property<std::string>(uidKey),
                                        properties);
            setCommon(policy);
            systemds->Path().push_back(policy);
            break;
        }
        case SystemdUnitType::Mount:
        {
            auto properties = createFileDepsProperties(systemdModel, unitName);
            PolicyWithFileDeps_t policy(commonModel->property<std::string>(clsidKey),
                                        commonModel->property<std::string>(nameKey),
                                        commonModel->property<std::string>(uidKey),
                                        properties);
            setCommon(policy);
            systemds->Mount().push_back(policy);
            break;
        }
        case SystemdUnitType::Automount:
        {
            auto properties = createFileDepsProperties(systemdModel, unitName);
            PolicyWithFileDeps_t policy(commonModel->property<std::string>(clsidKey),
                                        commonModel->property<std::string>(nameKey),
                                        commonModel->property<std::string>(uidKey),
                                        properties);
            setCommon(policy);
            systemds->Automount().push_back(policy);
            break;
        }
        case SystemdUnitType::Swap:
        {
            auto properties = createFileDepsProperties(systemdModel, unitName);
            PolicyWithFileDeps_t policy(commonModel->property<std::string>(clsidKey),
                                        commonModel->property<std::string>(nameKey),
                                        commonModel->property<std::string>(uidKey),
                                        properties);
            setCommon(policy);
            systemds->Swap().push_back(policy);
            break;
        }
        case SystemdUnitType::Target:
        {
            auto properties = createEditsOnlyProperties(systemdModel, unitName);
            PolicyEditsOnly_t policy(commonModel->property<std::string>(clsidKey),
                                     commonModel->property<std::string>(nameKey),
                                     commonModel->property<std::string>(uidKey),
                                     properties);
            setCommon(policy);
            systemds->Target().push_back(policy);
            break;
        }
        case SystemdUnitType::Device:
        {
            auto properties = createEditsOnlyProperties(systemdModel, unitName);
            PolicyEditsOnly_t policy(commonModel->property<std::string>(clsidKey),
                                     commonModel->property<std::string>(nameKey),
                                     commonModel->property<std::string>(uidKey),
                                     properties);
            setCommon(policy);
            systemds->Device().push_back(policy);
            break;
        }
        case SystemdUnitType::Slice:
        {
            auto properties = createEditsOnlyProperties(systemdModel, unitName);
            PolicyEditsOnly_t policy(commonModel->property<std::string>(clsidKey),
                                     commonModel->property<std::string>(nameKey),
                                     commonModel->property<std::string>(uidKey),
                                     properties);
            setCommon(policy);
            systemds->Slice().push_back(policy);
            break;
        }
        case SystemdUnitType::Scope:
        {
            auto properties = createEditsOnlyProperties(systemdModel, unitName);
            PolicyEditsOnly_t policy(commonModel->property<std::string>(clsidKey),
                                     commonModel->property<std::string>(nameKey),
                                     commonModel->property<std::string>(uidKey),
                                     properties);
            setCommon(policy);
            systemds->Scope().push_back(policy);
            break;
        }
        }
    }

    return systemds;
}

} // namespace preferences
