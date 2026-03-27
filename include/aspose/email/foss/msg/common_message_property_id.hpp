#pragma once

#include <cstdint>

namespace aspose::email::foss::msg
{

enum class common_message_property_id : std::uint16_t
{
    message_class = 26,
    transport_message_headers = 125,
    subject = 55,
    display_to = 3588,
    display_cc = 3587,
    display_bcc = 3586,
    internet_message_id = 4149,
    message_flags = 3591,
    internet_codepage = 16350,
    sender_name = 3098,
    sender_address_type = 3102,
    sender_email_address = 3103,
    message_delivery_time = 3590,
    store_support_mask = 13325,
    body = 4096,
    body_html = 4115,
    display_name = 12289,
    attach_method = 14085,
    attach_data_binary = 14081,
    attach_filename = 14084,
    attach_long_filename = 14087,
    attach_mime_tag = 14094,
    attach_content_id = 14098,
    schedule_info_delegate_names = 26692,
    schedule_info_months_busy = 26707,
    example_tag100_a = 4106,
    example_tag101_d = 4125
};

} // namespace aspose::email::foss::msg
