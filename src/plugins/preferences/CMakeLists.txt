find_package(Qt5 COMPONENTS Widgets Qml Xml LinguistTools REQUIRED)

find_package(GPUI COMPONENTS core io REQUIRED)

find_package(MVVM REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_subdirectory(schemas)
add_subdirectory(item_level_targeting/schemas)

set(HEADERS
    attributes/attributesitem.h

    common/basemodelbuilder.h
    common/basepreferenceitem.h
    common/basepreferencewidget.h
    common/basepreferencereader.h
    common/basepreferencewriter.h
    common/commonitem.h
    common/commonutils.h
    common/commonview.h
    common/defaultactions.h
    common/modeltype.h
    common/preferencecategoryitem.h
    common/preferencesdialog.h
    common/preferencesmodel.h
    common/preferencestreeview.h
    common/propertieswidget.h
    common/propertieswidgetdelegate.h
    common/selectvariabledialog.h
    common/shortcutlineedit.h
    common/tabledetailswidget.h
    common/variablesmodel.h
    common/widgetfactory.h

    datasource/datasourcecontaineritem.h
    datasource/datasourceitem.h
    datasource/datasourcemodelbuilder.h
    datasource/datasourcewidget.h

    device/deviceitem.h
    device/devicecontaineritem.h
    device/devicewidget.h

    drives/drivesitem.h
    drives/drivescontaineritem.h
    drives/drivesmodelbuilder.h
    drives/drivespreferencereader.h
    drives/drivespreferencewriter.h
    drives/driveswidget.h

    folders/foldercontaineritem.h
    folders/folderpreferencereader.h
    folders/folderpreferencewriter.h
    folders/folderitem.h
    folders/foldermodelbuilder.h
    folders/folderwidget.h

    folder_options/folderoptionscontaineritem.h
    folder_options/folderoptionscontaineritem.inl
    folder_options/folderoptionswidget.h
    folder_options/folderoptionsitem.h
    folder_options/folderoptionsmodelbuilder.h
    folder_options/openwithitem.h
    folder_options/openwithwidget.h
    folder_options/fileextensionactionitem.h
    folder_options/fileextensioncontaineritem.h
    folder_options/fileextensionitem.h
    folder_options/fileextensionwidget.h

    files/filesitem.h
    files/filespreferencereader.h
    files/filespreferencewriter.h
    files/filescontaineritem.h
    files/filesmodelbuilder.h
    files/fileswidget.h

    ini/iniitem.h
    ini/inipreferencereader.h
    ini/inipreferencewriter.h
    ini/inicontaineritem.h
    ini/inimodelbuilder.h
    ini/iniwidget.h

    interfaces/preferencesreaderinterface.h
    interfaces/preferenceswriterinterface.h

    item_level_targeting/common/filtersbridge.h
    item_level_targeting/common/filtersio.h
    item_level_targeting/common/targetingcontainer.h
    item_level_targeting/common/targetingfilterdefaults.h
    item_level_targeting/common/targetingreader.h
    item_level_targeting/common/targetingrowformatter.h
    item_level_targeting/common/targetingselection.h
    item_level_targeting/common/targetingwriter.h
    item_level_targeting/common/targetingdialog.h
    item_level_targeting/common/targetingtreeview.h
    item_level_targeting/common/targetingfilteritem.h
    item_level_targeting/common/targetingfilterwidget.h
    item_level_targeting/common/targetingicons.h
    item_level_targeting/common/targetingmodel.h
    item_level_targeting/common/targetingwidgetfactory.h
    item_level_targeting/battery/batterypresentwidget.h
    item_level_targeting/battery/batteryreader.h
    item_level_targeting/battery/batterywriter.h
    item_level_targeting/computername/computernamereader.h
    item_level_targeting/computername/computernamewriter.h
    item_level_targeting/cpu/cpureader.h
    item_level_targeting/cpu/cpuwriter.h
    item_level_targeting/datematch/datematchreader.h
    item_level_targeting/datematch/datematchwriter.h
    item_level_targeting/diskspace/diskspacereader.h
    item_level_targeting/diskspace/diskspacewriter.h
    item_level_targeting/domain/domainreader.h
    item_level_targeting/domain/domainwriter.h
    item_level_targeting/environment/environmentreader.h
    item_level_targeting/environment/environmentwriter.h
    item_level_targeting/filematch/filematchreader.h
    item_level_targeting/filematch/filematchwriter.h
    item_level_targeting/iprange/iprangereader.h
    item_level_targeting/iprange/iprangewriter.h
    item_level_targeting/language/languagereader.h
    item_level_targeting/language/languagewriter.h
    item_level_targeting/ldap/ldapreader.h
    item_level_targeting/ldap/ldapwriter.h
    item_level_targeting/macrange/macrangereader.h
    item_level_targeting/macrange/macrangewriter.h
    item_level_targeting/msi/msireader.h
    item_level_targeting/msi/msiwriter.h
    item_level_targeting/network/networkreader.h
    item_level_targeting/network/networkwriter.h
    item_level_targeting/os/osreader.h
    item_level_targeting/os/oswriter.h
    item_level_targeting/ou/oureader.h
    item_level_targeting/ou/ouwriter.h
    item_level_targeting/pcmcia/pcmciareader.h
    item_level_targeting/pcmcia/pcmciawriter.h
    item_level_targeting/portable/portablereader.h
    item_level_targeting/portable/portablewriter.h
    item_level_targeting/processing/processingreader.h
    item_level_targeting/processing/processingwriter.h
    item_level_targeting/ram/ramreader.h
    item_level_targeting/ram/ramwriter.h
    item_level_targeting/registry/registryreader.h
    item_level_targeting/registry/registrywriter.h
    item_level_targeting/securitygroup/securitygroupreader.h
    item_level_targeting/securitygroup/securitygroupwriter.h
    item_level_targeting/site/sitereader.h
    item_level_targeting/site/sitewriter.h
    item_level_targeting/terminal/terminalreader.h
    item_level_targeting/terminal/terminalwriter.h
    item_level_targeting/timerange/timerangereader.h
    item_level_targeting/timerange/timerangewriter.h
    item_level_targeting/user/userreader.h
    item_level_targeting/user/userwriter.h
    item_level_targeting/wmi/wmireader.h
    item_level_targeting/wmi/wmiwriter.h
    item_level_targeting/pcmcia/pcmciapresentwidget.h
    item_level_targeting/site/sitefilterwidget.h
    item_level_targeting/cpu/cpuspeedwidget.h
    item_level_targeting/ram/ramfilterwidget.h
    item_level_targeting/computername/computernamewidget.h
    item_level_targeting/domain/domainfilterwidget.h
    item_level_targeting/environment/environmentvariablewidget.h
    item_level_targeting/iprange/ipaddressrangewidget.h
    item_level_targeting/macrange/macaddressrangewidget.h
    item_level_targeting/timerange/timerangefilterwidget.h
    item_level_targeting/user/userfilterwidget.h
    item_level_targeting/ou/organizationunitwidget.h
    item_level_targeting/securitygroup/securitygroupwidget.h
    item_level_targeting/diskspace/diskspacewidget.h
    item_level_targeting/portable/portablecomputerwidget.h
    item_level_targeting/processing/processingmodewidget.h
    item_level_targeting/ldap/ldapfilterwidget.h
    item_level_targeting/wmi/wmiquerywidget.h
    item_level_targeting/language/languagewidget.h
    item_level_targeting/network/networkconnectionwidget.h
    item_level_targeting/os/operatingsystemwidget.h
    item_level_targeting/os/oscatalog.h
    item_level_targeting/datematch/datematchwidget.h
    item_level_targeting/filematch/filematchwidget.h
    item_level_targeting/msi/msiquerywidget.h
    item_level_targeting/registry/registrymatchwidget.h
    item_level_targeting/terminal/terminalsessionwidget.h

    local_users_and_groups/baselocalitem.h
    local_users_and_groups/groupmemberdialog.h
    local_users_and_groups/groupmemberitem.h
    local_users_and_groups/groupmodelbuilder.h
    local_users_and_groups/groupmemberswidget.h
    local_users_and_groups/groupmemberscontaineritem.h
    local_users_and_groups/localgroupcontaineritem.h
    local_users_and_groups/localgroupcontaineritem.inl
    local_users_and_groups/localgroupitem.h
    local_users_and_groups/localgroupwidget.h
    local_users_and_groups/localuseritem.h
    local_users_and_groups/localuserwidget.h

    network_options/basenetworkitem.h
    network_options/dialupitem.h
    network_options/dialupwidget.h
    network_options/networkmodelbuilder.h
    network_options/networkcontaineritem.h
    network_options/networkcontaineritem.inl
    network_options/vpnitem.h
    network_options/vpnnetworkingwidget.h
    network_options/vpnoptionswidget.h
    network_options/vpnsecuritywidget.h
    network_options/vpnwidget.h

    interfaces/containeriteminterface.h
    interfaces/preferenceswidgetinterface.h
    interfaces/widgetfactoryinterface.h

    power_options/powercontaineritem.h
    power_options/poweroptionsitem.h
    power_options/poweroptionsitem.inl
    power_options/powermodelbuilder.h
    power_options/poweroptionswidget.h
    power_options/powerplanitem.h
    power_options/powerplanitem.inl
    power_options/powerplanwidget.h
    power_options/powerschemeitem.h
    power_options/powerschemeitem.inl
    power_options/powerschemewidget.h

    printers/localprinteritem.h
    printers/localprinterwidget.h
    printers/printersmodelbuilder.h
    printers/printercontaineritem.h
    printers/printercontaineritem.inl
    printers/sharedprinteritem.h
    printers/sharedprinterwidget.h
    printers/tcpprintergeneralwidget.h
    printers/tcpprinteritem.h
    printers/tcpprinterportwidget.h

    registry/registrycontaineritem.h
    registry/registrypreferencereader.h
    registry/registrypreferencewriter.h
    registry/registryitem.h
    registry/registrymodelbuilder.h
    registry/registrywidget.h

    scheduled_tasks/immediateactionwin7widget.h
    scheduled_tasks/immediateconditionswin7widget.h
    scheduled_tasks/immediatesettingswidget.h
    scheduled_tasks/immediatesettingswin7widget.h
    scheduled_tasks/immediatetaskswidget.h
    scheduled_tasks/immediatewin7widget.h
    scheduled_tasks/scheduleactionwin7widget.h
    scheduled_tasks/scheduleconditionswin7widget.h
    scheduled_tasks/scheduledtaskswidget.h
    scheduled_tasks/scheduleschedulewidget.h
    scheduled_tasks/schedulesettingswidget.h
    scheduled_tasks/schedulesettingswin7widget.h
    scheduled_tasks/scheduletriggerswin7widget.h
    scheduled_tasks/schedulewin7widget.h

    start_menu/startadvancedwinxpwidget.h
    start_menu/startclassicvistawidget.h
    start_menu/startclassicwinxpwidget.h
    start_menu/startvistawidget.h
    start_menu/startwinxpwidget.h
    start_menu/startmenuwinvistaitem.h
    start_menu/startmenuwinxpitem.h

    shares/sharesitem.h
    shares/sharespreferencereader.h
    shares/sharespreferencewriter.h
    shares/sharesmodelbuilder.h
    shares/sharescontaineritem.h
    shares/shareswidget.h

    shortcuts/keysequenceencoder.h
    shortcuts/shortcutsitem.h
    shortcuts/shortcutspreferencereader.h
    shortcuts/shortcutspreferencewriter.h
    shortcuts/shortcutsmodelbuilder.h
    shortcuts/shortcutscontaineritem.h
    shortcuts/shortcutswidget.h

    variables/variablesitem.h
    variables/variablespreferencereader.h
    variables/variablespreferencewriter.h
    variables/variablesmodelbuilder.h
    variables/variablescontaineritem.h
    variables/variableswidget.h

    common/preferencestreemodel.h
    common/preferencestreeproxymodel.h

    common/preferenceswidget.h
)

set(SOURCES
    attributes/attributesitem.cpp

    common/basemodelbuilder.cpp
    common/basepreferencewidget.cpp
    common/basepreferencereader.cpp
    common/basepreferencewriter.cpp
    common/commonitem.cpp
    common/commonutils.cpp
    common/commonview.cpp
    common/defaultactions.cpp
    common/preferencecategoryitem.cpp
    common/preferencesdialog.cpp
    common/preferencesmodel.cpp
    common/preferencestreeview.cpp
    common/propertieswidget.cpp
    common/propertieswidgetdelegate.cpp
    common/selectvariabledialog.cpp
    common/shortcutlineedit.cpp
    common/tabledetailswidget.cpp
    common/variablesmodel.cpp
    common/widgetfactory.cpp

    datasource/datasourceitem.cpp
    datasource/datasourcecontaineritem.cpp
    datasource/datasourcemodelbuilder.cpp
    datasource/datasourcewidget.cpp
    datasource/datasourcewidgetslots.cpp

    device/deviceitem.cpp
    device/devicecontaineritem.cpp
    drives/drivesmodelbuilder.cpp
    device/devicewidget.cpp

    drives/drivesitem.cpp
    drives/drivescontaineritem.cpp
    drives/driveswidget.cpp
    drives/drivespreferencereader.cpp
    drives/drivespreferencewriter.cpp
    drives/driveswidgetslots.cpp

    folders/foldercontaineritem.cpp
    folders/folderpreferencereader.cpp
    folders/folderpreferencewriter.cpp
    folders/folderitem.cpp
    folders/foldermodelbuilder.cpp
    folders/folderwidget.cpp
    folders/folderviewslots.cpp

    folder_options/folderoptionswidget.cpp
    folder_options/folderoptionsitem.cpp
    folder_options/folderoptionsmodelbuilder.cpp
    folder_options/openwithitem.cpp
    folder_options/openwithwidget.cpp
    folder_options/openwithwidgetslots.cpp
    folder_options/fileextensionactionitem.cpp
    folder_options/fileextensioncontaineritem.cpp
    folder_options/fileextensionitem.cpp
    folder_options/fileextensionwidget.cpp

    files/filesitem.cpp
    files/filespreferencereader.cpp
    files/filespreferencewriter.cpp
    files/filescontaineritem.cpp
    files/filesmodelbuilder.cpp
    files/fileswidget.cpp
    files/fileswidgetslots.cpp

    ini/iniitem.cpp
    ini/inipreferencereader.cpp
    ini/inipreferencewriter.cpp
    ini/inicontaineritem.cpp
    ini/inimodelbuilder.cpp
    ini/iniwidget.cpp
    ini/iniwidgetslots.cpp

    item_level_targeting/common/filtersbridge.cpp
    item_level_targeting/common/filtersio.cpp
    item_level_targeting/common/targetingcontainer.cpp
    item_level_targeting/common/targetingfilterdefaults.cpp
    item_level_targeting/common/targetingreader.cpp
    item_level_targeting/common/targetingrowformatter.cpp
    item_level_targeting/common/targetingselection.cpp
    item_level_targeting/common/targetingwriter.cpp
    item_level_targeting/common/targetingdialog.cpp
    item_level_targeting/common/targetingtreeview.cpp
    item_level_targeting/common/targetingfilteritem.cpp
    item_level_targeting/common/targetingfilterwidget.cpp
    item_level_targeting/common/targetingicons.cpp
    item_level_targeting/common/targetingmodel.cpp
    item_level_targeting/common/targetingwidgetfactory.cpp
    item_level_targeting/battery/batterypresentwidget.cpp
    item_level_targeting/battery/batteryreader.cpp
    item_level_targeting/battery/batterywriter.cpp
    item_level_targeting/computername/computernamereader.cpp
    item_level_targeting/computername/computernamewriter.cpp
    item_level_targeting/cpu/cpureader.cpp
    item_level_targeting/cpu/cpuwriter.cpp
    item_level_targeting/datematch/datematchreader.cpp
    item_level_targeting/datematch/datematchwriter.cpp
    item_level_targeting/diskspace/diskspacereader.cpp
    item_level_targeting/diskspace/diskspacewriter.cpp
    item_level_targeting/domain/domainreader.cpp
    item_level_targeting/domain/domainwriter.cpp
    item_level_targeting/environment/environmentreader.cpp
    item_level_targeting/environment/environmentwriter.cpp
    item_level_targeting/filematch/filematchreader.cpp
    item_level_targeting/filematch/filematchwriter.cpp
    item_level_targeting/iprange/iprangereader.cpp
    item_level_targeting/iprange/iprangewriter.cpp
    item_level_targeting/language/languagereader.cpp
    item_level_targeting/language/languagewriter.cpp
    item_level_targeting/ldap/ldapreader.cpp
    item_level_targeting/ldap/ldapwriter.cpp
    item_level_targeting/macrange/macrangereader.cpp
    item_level_targeting/macrange/macrangewriter.cpp
    item_level_targeting/msi/msireader.cpp
    item_level_targeting/msi/msiwriter.cpp
    item_level_targeting/network/networkreader.cpp
    item_level_targeting/network/networkwriter.cpp
    item_level_targeting/os/osreader.cpp
    item_level_targeting/os/oswriter.cpp
    item_level_targeting/ou/oureader.cpp
    item_level_targeting/ou/ouwriter.cpp
    item_level_targeting/pcmcia/pcmciareader.cpp
    item_level_targeting/pcmcia/pcmciawriter.cpp
    item_level_targeting/portable/portablereader.cpp
    item_level_targeting/portable/portablewriter.cpp
    item_level_targeting/processing/processingreader.cpp
    item_level_targeting/processing/processingwriter.cpp
    item_level_targeting/ram/ramreader.cpp
    item_level_targeting/ram/ramwriter.cpp
    item_level_targeting/registry/registryreader.cpp
    item_level_targeting/registry/registrywriter.cpp
    item_level_targeting/securitygroup/securitygroupreader.cpp
    item_level_targeting/securitygroup/securitygroupwriter.cpp
    item_level_targeting/site/sitereader.cpp
    item_level_targeting/site/sitewriter.cpp
    item_level_targeting/terminal/terminalreader.cpp
    item_level_targeting/terminal/terminalwriter.cpp
    item_level_targeting/timerange/timerangereader.cpp
    item_level_targeting/timerange/timerangewriter.cpp
    item_level_targeting/user/userreader.cpp
    item_level_targeting/user/userwriter.cpp
    item_level_targeting/wmi/wmireader.cpp
    item_level_targeting/wmi/wmiwriter.cpp
    item_level_targeting/pcmcia/pcmciapresentwidget.cpp
    item_level_targeting/site/sitefilterwidget.cpp
    item_level_targeting/cpu/cpuspeedwidget.cpp
    item_level_targeting/ram/ramfilterwidget.cpp
    item_level_targeting/computername/computernamewidget.cpp
    item_level_targeting/domain/domainfilterwidget.cpp
    item_level_targeting/environment/environmentvariablewidget.cpp
    item_level_targeting/iprange/ipaddressrangewidget.cpp
    item_level_targeting/macrange/macaddressrangewidget.cpp
    item_level_targeting/timerange/timerangefilterwidget.cpp
    item_level_targeting/user/userfilterwidget.cpp
    item_level_targeting/ou/organizationunitwidget.cpp
    item_level_targeting/securitygroup/securitygroupwidget.cpp
    item_level_targeting/diskspace/diskspacewidget.cpp
    item_level_targeting/portable/portablecomputerwidget.cpp
    item_level_targeting/processing/processingmodewidget.cpp
    item_level_targeting/ldap/ldapfilterwidget.cpp
    item_level_targeting/wmi/wmiquerywidget.cpp
    item_level_targeting/language/languagewidget.cpp
    item_level_targeting/network/networkconnectionwidget.cpp
    item_level_targeting/os/operatingsystemwidget.cpp
    item_level_targeting/os/oscatalog.cpp
    item_level_targeting/datematch/datematchwidget.cpp
    item_level_targeting/filematch/filematchwidget.cpp
    item_level_targeting/msi/msiquerywidget.cpp
    item_level_targeting/registry/registrymatchwidget.cpp
    item_level_targeting/terminal/terminalsessionwidget.cpp

    local_users_and_groups/baselocalitem.cpp
    local_users_and_groups/groupmemberdialog.cpp
    local_users_and_groups/groupmemberitem.cpp
    local_users_and_groups/groupmodelbuilder.cpp
    local_users_and_groups/groupmemberswidget.cpp
    local_users_and_groups/groupmemberswidgetslots.cpp
    local_users_and_groups/localgroupitem.cpp
    local_users_and_groups/localgroupwidget.cpp
    local_users_and_groups/localgroupwidgetslots.cpp
    local_users_and_groups/localuseritem.cpp
    local_users_and_groups/localuserwidget.cpp
    local_users_and_groups/localuserwidgetslots.cpp

    network_options/basenetworkitem.cpp
    network_options/dialupitem.cpp
    network_options/dialupwidget.cpp
    network_options/dialupwidgetslots.cpp
    network_options/networkmodelbuilder.cpp
    network_options/vpnitem.cpp
    network_options/vpnnetworkingwidget.cpp
    network_options/vpnoptionswidget.cpp
    network_options/vpnsecuritywidget.cpp
    network_options/vpnwidget.cpp
    network_options/vpnwidgetslots.cpp

    power_options/powercontaineritem.inl
    power_options/poweroptionsitem.cpp
    power_options/powermodelbuilder.cpp
    power_options/poweroptionswidget.cpp
    power_options/powerplanitem.cpp
    power_options/powerplanwidget.cpp
    power_options/powerschemeitem.cpp
    power_options/powerschemewidget.cpp

    printers/localprinteritem.cpp
    printers/localprinterwidget.cpp
    printers/printersmodelbuilder.cpp
    printers/sharedprinteritem.cpp
    printers/sharedprinterwidget.cpp
    printers/tcpprintergeneralwidget.cpp
    printers/tcpprinteritem.cpp
    printers/tcpprinterportwidget.cpp

    scheduled_tasks/immediateactionwin7widget.cpp
    scheduled_tasks/immediateconditionswin7widget.cpp
    scheduled_tasks/immediatesettingswidget.cpp
    scheduled_tasks/immediatesettingswin7widget.cpp
    scheduled_tasks/immediatetaskswidget.cpp
    scheduled_tasks/immediatewin7widget.cpp
    scheduled_tasks/scheduleactionwin7widget.cpp
    scheduled_tasks/scheduleconditionswin7widget.cpp
    scheduled_tasks/scheduledtasksmodelbuilder.cpp
    scheduled_tasks/scheduledtaskswidget.cpp
    scheduled_tasks/scheduleschedulewidget.cpp
    scheduled_tasks/schedulesettingswidget.cpp
    scheduled_tasks/schedulesettingswin7widget.cpp
    scheduled_tasks/scheduletriggerswin7widget.cpp
    scheduled_tasks/schedulewin7widget.cpp

    start_menu/startadvancedwinxpwidget.cpp
    start_menu/startclassicvistawidget.cpp
    start_menu/startclassicwinxpwidget.cpp
    start_menu/startvistawidget.cpp
    start_menu/startwinxpwidget.cpp
    start_menu/startmenuwinvistaitem.cpp
    start_menu/startmenuwinxpitem.cpp

    registry/registrycontaineritem.cpp
    registry/registrypreferencereader.cpp
    registry/registrypreferencewriter.cpp
    registry/registryitem.cpp
    registry/registrymodelbuilder.cpp
    registry/registrywidget.cpp
    registry/registrywidgetslots.cpp

    shares/sharesitem.cpp
    shares/sharespreferencereader.cpp
    shares/sharespreferencewriter.cpp
    shares/sharescontaineritem.cpp
    shares/sharesmodelbuilder.cpp
    shares/shareswidget.cpp
    shares/shareswidgetslots.cpp

    shortcuts/keysequenceencoder.cpp
    shortcuts/shortcutsitem.cpp
    shortcuts/shortcutspreferencereader.cpp
    shortcuts/shortcutspreferencewriter.cpp
    shortcuts/shortcutscontaineritem.cpp
    shortcuts/shortcutsmodelbuilder.cpp
    shortcuts/shortcutswidget.cpp
    shortcuts/shortcutswidgetslots.cpp

    variables/variablesitem.cpp
    variables/variablespreferencereader.cpp
    variables/variablespreferencewriter.cpp
    variables/variablescontaineritem.cpp
    variables/variablesmodelbuilder.cpp
    variables/variableswidget.cpp
    variables/variableswidgetslots.cpp

    common/preferencestreemodel.cpp
    common/preferencestreeproxymodel.cpp

    common/preferenceswidget.cpp
)

set(UI_FORMS
    common/commonview.ui
    common/preferencesdialog.ui
    common/propertieswidget.ui
    common/selectvariabledialog.ui
    common/tabledetailswidget.ui

    datasource/datasourcewidget.ui

    drives/driveswidget.ui

    device/devicewidget.ui

    folders/folderswidget.ui

    folder_options/folderoptionswidget.ui
    folder_options/openwithwidget.ui
    folder_options/fileextensionwidget.ui

    files/fileswidget.ui

    ini/iniwidget.ui

    item_level_targeting/common/targetingdialog.ui
    item_level_targeting/battery/batterypresentwidget.ui
    item_level_targeting/pcmcia/pcmciawidget.ui
    item_level_targeting/site/sitewidget.ui
    item_level_targeting/cpu/cpuspeedwidget.ui
    item_level_targeting/ram/ramwidget.ui
    item_level_targeting/computername/computernamewidget.ui
    item_level_targeting/domain/domainwidget.ui
    item_level_targeting/environment/environmentwidget.ui
    item_level_targeting/iprange/ipaddressrangewidget.ui
    item_level_targeting/macrange/macaddressrangewidget.ui
    item_level_targeting/timerange/timerangewidget.ui
    item_level_targeting/user/userwidget.ui
    item_level_targeting/ou/organizationuniitwidget.ui
    item_level_targeting/securitygroup/securitygroupwidget.ui
    item_level_targeting/diskspace/diskspacewidget.ui
    item_level_targeting/portable/portablecomputerwidget.ui
    item_level_targeting/processing/processingmodewidget.ui
    item_level_targeting/ldap/ldapwidget.ui
    item_level_targeting/wmi/wmiquerywidget.ui
    item_level_targeting/language/languagewidget.ui
    item_level_targeting/network/networkconnectionwidget.ui
    item_level_targeting/os/operatingsystemwidget.ui
    item_level_targeting/datematch/datematchwidget.ui
    item_level_targeting/filematch/filematchwidget.ui
    item_level_targeting/msi/msiquerywidget.ui
    item_level_targeting/registry/registrymatchwidget.ui
    item_level_targeting/terminal/terminalsessionwidget.ui

    local_users_and_groups/groupmemberswidget.ui
    local_users_and_groups/groupmemberdialog.ui
    local_users_and_groups/localuserwidget.ui
    local_users_and_groups/localgroupwidget.ui

    network_options/dialupwidget.ui
    network_options/vpnnetworkingwidget.ui
    network_options/vpnoptionswidget.ui
    network_options/vpnsecuritywidget.ui
    network_options/vpnwidget.ui

    power_options/poweroptionswidget.ui
    power_options/powerplanwidget.ui
    power_options/powerschemewidget.ui

    printers/localprinterwidget.ui
    printers/sharedprinterwidget.ui
    printers/tcpprintergeneralwidget.ui
    printers/tcpprinterportwidget.ui

    registry/registrywidget.ui

    scheduled_tasks/immediateactionwin7widget.ui
    scheduled_tasks/immediateconditionswin7widget.ui
    scheduled_tasks/immediatesettingswidget.ui
    scheduled_tasks/immediatesettingswin7widget.ui
    scheduled_tasks/immediatetaskswidget.ui
    scheduled_tasks/immediatewin7widget.ui
    scheduled_tasks/scheduleactionwin7widget.ui
    scheduled_tasks/scheduleconditionswin7widget.ui
    scheduled_tasks/scheduledtaskswidget.ui
    scheduled_tasks/scheduleschedulewidget.ui
    scheduled_tasks/schedulesettingswidget.ui
    scheduled_tasks/schedulesettingswin7widget.ui
    scheduled_tasks/scheduletriggerswin7widget.ui
    scheduled_tasks/schedulewin7widget.ui

    start_menu/startadvancedwinxpwidget.ui
    start_menu/startclassicvistawidget.ui
    start_menu/startclassicwinxpwidget.ui
    start_menu/startvistawidget.ui
    start_menu/startwinxpwidget.ui

    shares/shareswidget.ui

    shortcuts/shortcutswidget.ui

    variables/variableswidget.ui

    common/preferenceswidget.ui
)

set(PLUGIN_HEADERS
    modelcreator.h
    modelwriter.h
    preferencessnapin.h
    preferencessnapinprivate.h
)

set(PLUGIN_SOURCES
    modelcreator.cpp
    modelwriter.cpp
    preferencesplugin.cpp
    preferencessnapin.cpp
    preferencessnapinprivate.cpp
)

file(GLOB_RECURSE TS_FILES ${CMAKE_CURRENT_SOURCE_DIR} *.ts)

set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR})

add_translation(QM_FILES ${TS_FILES})

add_translation_resource(RESOURCES_SRC "preferences" ${QM_FILES})

qt5_add_resources(LIB_RESOURCES ${RESOURCES_SRC} ${ICON_RESOURCES})

set(PLUGIN_SOURCES ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

set(SOURCES ${SOURCES} ${HEADERS} ${UI_FORMS})

include_directories(${CMAKE_SOURCE_DIR}/src)

set(PLUGIN_NAME preferences-plugin)

add_library(preferences_static STATIC ${SOURCES})
target_include_directories(preferences_static PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/item_level_targeting/common
    ${CMAKE_CURRENT_SOURCE_DIR}/item_level_targeting/schemas
)
target_link_libraries(preferences_static Qt5::Core Qt5::Widgets Qt5::Xml)
target_link_libraries(preferences_static ${GPUI_LIBRARIES})
target_link_libraries(preferences_static ${MVVM_LIBRARIES})

target_link_libraries(preferences_static static_xsd_library)
target_link_libraries(preferences_static targeting_filters_xsd_library)
target_link_libraries(preferences_static xerces-c)

add_gpui_plugin(${PLUGIN_NAME} ${PLUGIN_SOURCES} ${LIB_RESOURCES})
target_link_libraries(${PLUGIN_NAME} preferences_static)
