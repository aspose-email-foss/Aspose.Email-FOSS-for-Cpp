add_executable(aspose_email_foss_example_create_msg_and_eml
    create_msg_and_eml.cpp
)

target_link_libraries(aspose_email_foss_example_create_msg_and_eml
    PRIVATE AsposeEmailFoss::AsposeEmailFoss
)

target_compile_features(aspose_email_foss_example_create_msg_and_eml PRIVATE cxx_std_17)

add_executable(aspose_email_foss_example_msg_reader
    msg_reader.cpp
)

target_link_libraries(aspose_email_foss_example_msg_reader
    PRIVATE AsposeEmailFoss::AsposeEmailFoss
)

target_compile_features(aspose_email_foss_example_msg_reader PRIVATE cxx_std_17)

add_executable(aspose_email_foss_example_msg_summary
    msg_summary.cpp
)

target_link_libraries(aspose_email_foss_example_msg_summary
    PRIVATE AsposeEmailFoss::AsposeEmailFoss
)

target_compile_features(aspose_email_foss_example_msg_summary PRIVATE cxx_std_17)
