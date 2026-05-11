find_package(Qt5 COMPONENTS Core Gui Widgets Test Xml REQUIRED)
find_package(MVVM REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(TARGETING_SRC_DIR ${CMAKE_SOURCE_DIR}/src/plugins/preferences/item_level_targeting)

# Focused unit test — compile only the sources we actually exercise.
# Linking preferences_static pulls in widgets whose symbols live in
# the plugin (preferences-plugin) so it cannot resolve as a bare static.
set(SOURCES
    targetingtest.cpp
    ${TARGETING_SRC_DIR}/filtersbridge.cpp
    ${TARGETING_SRC_DIR}/oscatalog.cpp
    ${TARGETING_SRC_DIR}/targetingcontainer.cpp
    ${TARGETING_SRC_DIR}/targetingfilteritem.cpp
    ${TARGETING_SRC_DIR}/targetingrowformatter.cpp
    # targetingdialog.cpp is compiled only for access to the static
    # `filterCatalog()`; its Qt UI members are unused in this test.
)

set(MOC_HEADERS
    targetingtest.h
)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${TARGETING_SRC_DIR})

add_executable(targetingtest ${SOURCES} ${MOC_HEADERS})
target_link_libraries(targetingtest
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Test
    Qt5::Xml
    ${MVVM_LIBRARIES}
)
add_gpui_test(plugins.preferences.targetingtest targetingtest)
