# Examples

This file maps common tasks to example source files.

## Quick Index

- Create a new `.msg`, read it back, and save `.eml`:
  - [create_msg_and_eml.cpp](create_msg_and_eml.cpp)
- Read low-level MSG and CFB structure:
  - [msg_reader.cpp](msg_reader.cpp)
- Read a message through the high-level API and print a summary:
  - [msg_summary.cpp](msg_summary.cpp)

## Example Files

### [create_msg_and_eml.cpp](create_msg_and_eml.cpp)

Use this example to:
- create a new `mapi_message`
- set common properties through `common_message_property_id`
- add recipients
- add attachments
- save as `.msg`
- load the `.msg` back
- save the loaded message as `.eml`

Build with:

```powershell
cmake -S cpp -B cpp/out/build/examples -DASPOSE_EMAIL_FOSS_BUILD_EXAMPLES=ON -DASPOSE_EMAIL_FOSS_BUILD_TESTS=OFF
cmake --build cpp/out/build/examples --config RelWithDebInfo --target aspose_email_foss_example_create_msg_and_eml
```

### [msg_reader.cpp](msg_reader.cpp)

Use this example to:
- open a `.msg` through `msg_reader`
- inspect CFB geometry
- inspect top-level storages and streams
- list recipient and attachment storages
- dump the CFB tree

Build with:

```powershell
cmake --build cpp/out/build/examples --config RelWithDebInfo --target aspose_email_foss_example_msg_reader
```

### [msg_summary.cpp](msg_summary.cpp)

Use this example to:
- open a `.msg` through `mapi_message`
- print high-level message metadata
- print transport headers
- print body preview
- list recipients and attachments
- inspect an arbitrary MAPI property

Build with:

```powershell
cmake --build cpp/out/build/examples --config RelWithDebInfo --target aspose_email_foss_example_msg_summary
```

## Recommended Starting Points

- If you want a complete end-to-end workflow, start with [create_msg_and_eml.cpp](create_msg_and_eml.cpp).
- If you want to inspect container internals, start with [msg_reader.cpp](msg_reader.cpp).
- If you want a high-level projection of an existing message, start with [msg_summary.cpp](msg_summary.cpp).
