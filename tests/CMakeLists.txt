add_executable(aspose_email_foss_cfb_tests
    test_cfb.cpp
)

target_link_libraries(aspose_email_foss_cfb_tests
    PRIVATE
        AsposeEmailFoss::AsposeEmailFoss
)

add_test(NAME aspose_email_foss_cfb_tests COMMAND aspose_email_foss_cfb_tests)

add_executable(aspose_email_foss_public_tests
    test_generated_headers.cpp
)

target_link_libraries(aspose_email_foss_public_tests
    PRIVATE
        AsposeEmailFoss::AsposeEmailFoss
)

add_test(NAME aspose_email_foss_public_tests COMMAND aspose_email_foss_public_tests)

add_executable(aspose_email_foss_msg_tests
    test_msg.cpp
)

target_link_libraries(aspose_email_foss_msg_tests
    PRIVATE
        AsposeEmailFoss::AsposeEmailFoss
)

add_test(NAME aspose_email_foss_msg_tests COMMAND aspose_email_foss_msg_tests)
