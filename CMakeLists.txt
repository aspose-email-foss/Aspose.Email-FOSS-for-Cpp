cmake_minimum_required(VERSION 3.26)

project(AsposeEmailFoss VERSION 0.1.0 LANGUAGES CXX)

option(ASPOSE_EMAIL_FOSS_BUILD_TESTS "Build public tests" ON)
option(ASPOSE_EMAIL_FOSS_BUILD_EXAMPLES "Build examples" OFF)
option(ASPOSE_EMAIL_FOSS_BUILD_SHARED "Build shared library" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(ASPOSE_EMAIL_FOSS_BUILD_SHARED)
    set(BUILD_SHARED_LIBS ON)
endif()

add_library(AsposeEmailFoss
    src/library.cpp
    src/cfb/cfb_document.cpp
    src/cfb/cfb_node.cpp
    src/cfb/cfb_reader.cpp
    src/cfb/cfb_storage.cpp
    src/cfb/cfb_stream.cpp
    src/cfb/cfb_value_types.cpp
    src/cfb/cfb_writer.cpp
    src/cfb/detail.cpp
    src/msg/msg_document.cpp
    src/msg/mapi_attachment.cpp
    src/msg/mapi_message.cpp
    src/msg/mapi_property.cpp
    src/msg/mapi_property_collection.cpp
    src/msg/mime/eml_message_mapper.cpp
    src/msg/mime/mime_model.cpp
    src/msg/mime/mime_reader.cpp
    src/msg/mime/mime_writer.cpp
    src/msg/mime/transfer_encoding.cpp
    src/msg/msg_reader.cpp
    src/msg/msg_storage.cpp
    src/msg/msg_stream.cpp
    src/msg/msg_writer.cpp
)

add_library(AsposeEmailFoss::AsposeEmailFoss ALIAS AsposeEmailFoss)

target_compile_features(AsposeEmailFoss PUBLIC cxx_std_17)

target_include_directories(AsposeEmailFoss
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

set_target_properties(AsposeEmailFoss PROPERTIES
    EXPORT_NAME AsposeEmailFoss
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/AsposeEmailFossConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/AsposeEmailFossConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/AsposeEmailFossConfig.cmake"
    INSTALL_DESTINATION lib/cmake/AsposeEmailFoss
)

install(TARGETS AsposeEmailFoss
    EXPORT AsposeEmailFossTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT AsposeEmailFossTargets
    FILE AsposeEmailFossTargets.cmake
    NAMESPACE AsposeEmailFoss::
    DESTINATION lib/cmake/AsposeEmailFoss
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/AsposeEmailFossConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/AsposeEmailFossConfigVersion.cmake"
    DESTINATION lib/cmake/AsposeEmailFoss
)

if(ASPOSE_EMAIL_FOSS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(ASPOSE_EMAIL_FOSS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
