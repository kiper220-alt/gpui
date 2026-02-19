find_package(Qt5 COMPONENTS Core Gui Widgets Test REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCES
    systemd/systemdwidgettest.cpp
)

set(MOC_HEADERS
    systemd/systemdwidgettest.h
)

qt5_wrap_cpp(MOC_SOURCES ${MOC_HEADERS})

add_executable(systemdwidgettest ${SOURCES} ${MOC_SOURCES})
target_include_directories(systemdwidgettest PRIVATE
    ${PROJECT_SOURCE_DIR}/src/plugins/preferences
)
target_link_libraries(systemdwidgettest ${GPUI_LIBRARIES} preferences_static Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Test)
add_gpui_test(plugins.systemdwidgettest systemdwidgettest)
