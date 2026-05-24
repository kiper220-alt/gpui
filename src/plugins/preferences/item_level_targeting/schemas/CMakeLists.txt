set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

set(TARGETING_XSD_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/batteryschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/computernameschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/cpuschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/datematchschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/diskspacechema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/domainschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/environmentschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/filematchschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/iprangeschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/languageschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/ldapschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/macrangeschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/msischema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/networkschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/osschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/ouschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/pcmciaschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/portableschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/processingschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/ramschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/registryschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/securitygroupschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/siteschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/terminalschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/timerangeschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/userschema.xsd"
    "${CMAKE_CURRENT_SOURCE_DIR}/wmischema.xsd"
)

add_xsd_library(targeting_filters_xsd_library ${TARGETING_XSD_FILES})
