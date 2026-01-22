#include <memory>

#include "systemdmodelbuilder.h"
#include "systemditem.h"
#include "systemdcontaineritem.h"
#include <schemas/systemdschema.h>

namespace preferences
{
SystemdModelBuilder::SystemdModelBuilder() 
    : BaseModelBuilder() 
{}

/**
 * Returns a pair containing the systemd state and whether the state is 'now'.
 *
 * If the state string is empty, the function will return {SystemdState::Nothing, false}.
 *
 * The systemd state is determined by the first character of the state string.
 * The bool value is determined by whether the state string has at least two characters and the second character is 'N'.
 *
 * @param state The systemd state string.
 * @return A pair containing the systemd state and whether the state is 'now'. 
 */
inline QPair<SystemdState, bool> getUnitStateState(const std::string& state) 
{
    if (state.empty()) 
    {
        return {SystemdState::Nothing, false};
    }

    SystemdState s_state;

    switch (state[0]) 
    {
        case 'E':
            s_state = SystemdState::Enable;
            break;
        case 'D':
            s_state = SystemdState::Disable;
            break;
        case 'M':
            s_state = SystemdState::Mask;
            break;
        case 'U':
            s_state = SystemdState::Unmask;
            break;
        default:
            return {SystemdState::Nothing, false};
    }

    return {s_state, state.size() >= 2 && state[1] == 'N'};
}

/**
 * Returns a string representing the systemd state.
 *
 * If the state is 'now', the returned string will have 'N' appended to it.
 * If the state is not 'now', the returned string will be the single character representing the state.
 *
 * @param state The systemd state.
 * @param now Whether the state is 'now'.
 * @return A string representing the systemd state.
 */
inline std::string getUnitStateModel(const SystemdState state, const bool now) 
{
    switch (state) 
    {
        case SystemdState::Enable:
            return now ? "EN" : "E";
        case SystemdState::Disable:
            return now ? "DN" : "D";
        case SystemdState::Mask:
            return now ? "MN" : "M";
        case SystemdState::Unmask:
            return now ? "UN" : "U";
        default:
            return "N";
    }
}
/**
 * Returns the unit dependency state based on the given string.
 *
 * If the state string is empty or has a length of two, the function returns DependencyState::Changed.
 * If the state string is "PC", the function returns DependencyState::PresenceChanged.
 * Otherwise, the function returns DependencyState::Changed.
 * 
 * @param state The unit dependency state string.
 * @return The unit dependency state.
 */
inline DependencyState getUnitDependencyState(const std::string& state) 
{
    if (state.empty() || state.length() != 2) 
    {
        return DependencyState::Changed;
    }

    return state == "PC" ? DependencyState::PresenceChanged : DependencyState::Changed;
}

/**
 * Returns a string representing the unit dependency state.
 *
 * DependencyState::PresenceChanged corresponds to "PC".
 * DependencyState::Changed corresponds to "C".
 *
 * @param state The unit dependency state.
 * @return A string representing the unit dependency state.
 */
inline std::string getUnitDependencyModel(const DependencyState state) 
{
    switch (state) 
    {
        case DependencyState::PresenceChanged:
            return "PC";
        default:
            return "C";
    }
}


/**
 * @brief Converts a systemd schema to a preferences model.
 * 
 * The function goes through each systemd item in the given schema and 
 * creates a corresponding item in the preferences model. The created item is 
 * a SystemdContainerItem with the unit name, state, and state now 
 * properties set. If the systemd item has edit items, the function 
 * creates SystemdEditItem for each edit item and sets the type, section, 
 * key, and value properties. If the systemd item has dependency items, 
 * the function creates SystemdDependencyItem for each dependency item and sets 
 * the type and path properties.
 * 
 * @param systemds The systemd schema to be converted.
 * @return The converted preferences model.
 */
std::unique_ptr<PreferencesModel> SystemdModelBuilder::schemaToModel(std::unique_ptr<Systemds> &systemds) 
{
    auto model = std::make_unique<PreferencesModel>();

    for (auto &systemdSchema: systemds->Systemd()) 
    {
        auto properties = systemdSchema.Properties();
        for (auto &currentProperties: properties) 
        {
            auto actionState = getUnitStateState(
                currentProperties.state().c_str()
            );
            auto _editUnitProperties = currentProperties.Actions().Properties();
            auto _dependUnitProperties = currentProperties.Dependencies().Properties();

            auto sessionItem = model->insertItem<SystemdContainerItem>(model->rootItem());
            sessionItem->setupListeners();

            auto systemds = sessionItem->getSystemds();
            
            if (_editUnitProperties.size() > 0)
            {
                systemds->setProperty(
                    SystemdContainerItem::EDIT, 
                    true
                );

                for (auto &_editUnitProperties: _editUnitProperties) 
                {
                    auto _editAction = systemds->insertItem<SystemdEditItem>(
                        SystemdItem::EDIT_ITEM_TAG
                    );
                    _editAction->setProperty(
                        SystemdEditItem::TYPE, 
                        getActionCheckboxState(_editUnitProperties.type().c_str())
                    );
                    _editAction->setProperty(
                        SystemdEditItem::SECTION, 
                        _editUnitProperties.section().c_str()
                    );
                    _editAction->setProperty(
                        SystemdEditItem::KEY, 
                        _editUnitProperties.key().c_str()
                    );
                    _editAction->setProperty(
                        SystemdEditItem::VALUE,
                        _editUnitProperties.value().c_str()
                    );
                }
            }
            
            if (_dependUnitProperties.size() > 0)
            {
                systemds->setProperty(
                    SystemdContainerItem::DEPENDENCY, 
                    true
                );
                for (auto &_depUnitProperties: _dependUnitProperties) 
                {
                    auto _depItem = systemds->insertItem<SystemdDependencyItem>(
                        SystemdItem::DEPENDENCY_ITEM_TAG
                    );
                    _depItem->setProperty(
                        SystemdDependencyItem::TYPE, 
                        static_cast<int>(getUnitDependencyState(_depUnitProperties.type().c_str()))
                    );
                    _depItem->setProperty(
                        SystemdDependencyItem::PATH, 
                        _depUnitProperties.path().c_str()
                    );
                }
            }

            systemds->setProperty(
                SystemdItem::UNIT, 
                currentProperties.unit().c_str()
            );
            systemds->setProperty(
                SystemdItem::STATE, 
                static_cast<int>(actionState.first)
            );
            systemds->setProperty(
                SystemdItem::STATE_NOW, 
                actionState.second
            );

            auto common = sessionItem->getCommon();
            setCommonItemData(common, systemdSchema);
        }
    }

    return model;
}
/**
 * @brief Converts a preferences model to a systemd schema.
 * 
 * The function goes through each SystemdContainerItem in the given preferences model and 
 * creates a corresponding item in the systemd schema. The created item is a Systemd_t with 
 * the unit name, state, and state now properties set. If the SystemdContainerItem 
 * has edit items, the function creates SystemdAction_t for each edit item and sets the 
 * type, section, key, and value properties. If the SystemdContainerItem has dependency 
 * items, the function creates SystemdDependency_t for each dependency item and sets the type 
 * and path properties.
 * 
 * @param model The preferences model to be converted.
 * @return The converted systemd schema.
 */
std::unique_ptr<Systemds> SystemdModelBuilder::modelToSchema(std::unique_ptr<PreferencesModel> &model) 
{
    auto systemds = std::make_unique<Systemds>("{48ABBD05-0169-4696-8F2C-E1E42A499ADF}");

    for (const auto &containerItem : model->topItems()) 
    {
        if (auto systemdContainer = dynamic_cast<SystemdContainerItem *>
            (containerItem); systemdContainer) 
        {
            auto systemdModel = systemdContainer->getSystemds();
            auto commonModel = systemdContainer->getCommon();
            
            std::string state = getUnitStateModel(
                static_cast<SystemdState>(systemdModel->property<int>(SystemdItem::STATE)), 
                systemdModel->property<bool>(SystemdItem::STATE_NOW)
            );

            auto systemd = Systemd_t("", "", "");
            commonModel->setProperty(
                CommonItem::propertyToString(CommonItem::CLSID),
                "{613BDB77-EC4A-4EA1-BA1D-42FD47762D0E}"
            );
            commonModel->setProperty(
                CommonItem::propertyToString(CommonItem::CHANGED), 
                createDateOfChange()
            );
            commonModel->setProperty(
                CommonItem::propertyToString(CommonItem::NAME),
                systemdModel->property<std::string>(SystemdItem::UNIT)
            );
            
            auto edit = SystemdActions_t();
            auto dependency = SystemdDependencies_t();

            for (auto &item: systemdModel->editItems()) 
            {
                auto propertie = SystemdAction_t (
                    getActionCheckboxModel(item->property<int>(SystemdEditItem::TYPE)),
                    item->property<std::string>(SystemdEditItem::SECTION),
                    item->property<std::string>(SystemdEditItem::KEY),
                    item->property<std::string>(SystemdEditItem::VALUE)
                );
                edit.Properties().push_back(propertie);
            }
            
            for (auto &item: systemdModel->depItems()) 
            {
                auto propertie = SystemdDependence_t (
                    getUnitDependencyModel(
                        static_cast<DependencyState>(item->property<int>(SystemdDependencyItem::TYPE))
                    ),
                    item->property<std::string>(SystemdDependencyItem::PATH)
                );
                dependency.Properties().push_back(propertie);
            }

            auto properties = SystemdProperties_t(edit, dependency, systemdModel->property<std::string>(SystemdItem::UNIT), state);
            properties.state(state);
            setCommonModelData(systemd, commonModel);
            systemd.Properties().push_back(properties);

            systemds->Systemd().push_back(systemd);
        }
    }

    return systemds;
}
} // namespace preferences
