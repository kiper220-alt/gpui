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
inline DependencyState getUnitDependencyState(const std::string& state) 
{
    if (state.empty() || state.length() != 2) 
    {
        return DependencyState::Changed;
    }

    return state == "PC" ? DependencyState::PresenceChanged : DependencyState::Changed;
}

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
