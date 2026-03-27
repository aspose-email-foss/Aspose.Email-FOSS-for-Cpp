# Changelog

All notable changes to this package will be documented in this file.

## [0.1.0] - 2026-03-18

### Added

- Initial C++17 package scaffold.
- CMake install/export package configuration.
- Foundation for deterministic KB-driven header generation.
- Dependency-free CFB reader, mutable document model, and deterministic writer.
- Low-level MSG reader, document model, and writer on top of the CFB layer.
- High-level `mapi_message` API with recipients, attachments, and embedded-message attachments.
- Internal MIME reader, writer, and EML bridge APIs for `mapi_message`.
- Public self-contained tests for CFB, MSG, and EML workflows.
- Internal KB-alignment, MSG corpus, and EML corpus verification tests.
