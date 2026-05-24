find_package(Qt5 COMPONENTS Core Gui Widgets Test Xml REQUIRED)
find_package(MVVM REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(TARGETING_SRC_DIR ${CMAKE_SOURCE_DIR}/src/plugins/preferences/item_level_targeting)

# Focused unit test: compile the helper/model sources it exercises directly.
# The explicit dependency below also builds the production preferences-plugin
# target, covering the dialog and per-filter widget sources that are easy to
# miss in a narrow unit-test build.
set(SOURCES
    targetingtest.cpp
    ${TARGETING_SRC_DIR}/common/filtersbridge.cpp
    ${TARGETING_SRC_DIR}/common/targetingfilterdefaults.cpp
    ${TARGETING_SRC_DIR}/common/targetingreader.cpp
    ${TARGETING_SRC_DIR}/common/targetingwriter.cpp
    ${TARGETING_SRC_DIR}/os/oscatalog.cpp
    ${TARGETING_SRC_DIR}/common/targetingcontainer.cpp
    ${TARGETING_SRC_DIR}/common/targetingfilteritem.cpp
    ${TARGETING_SRC_DIR}/common/targetingrowformatter.cpp
    ${TARGETING_SRC_DIR}/common/targetingselection.cpp
    ${TARGETING_SRC_DIR}/common/targetingtreeview.cpp
)

set(MOC_HEADERS
    targetingtest.h
)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${TARGETING_SRC_DIR})
include_directories(${TARGETING_SRC_DIR}/common)

add_executable(targetingtest ${SOURCES} ${MOC_HEADERS})
target_compile_definitions(targetingtest PRIVATE
    TARGETING_FIXTURES_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures/msad_targeting"
    TARGETING_SOURCE_DIR="${TARGETING_SRC_DIR}"
)
if(TARGET preferences-plugin)
    add_dependencies(targetingtest preferences-plugin)
endif()
target_link_libraries(targetingtest
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Test
    Qt5::Xml
    ${MVVM_LIBRARIES}
)
add_gpui_test(plugins.preferences.targetingtest targetingtest)
