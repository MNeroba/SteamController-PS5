# Project memory — current continuation entry

## M1-249 — dev.6 candidate preparation (2026-10-31)

The owner requested preparation of v0.1.0-dev.6. The published dev.5 ELF/tag/assets remain unchanged. Candidate scope is the M1-248 token-storage and VDA-observer fixes, not a new Add ABI or controller-ownership method. The tester's shown logs are from dev.5: token setup fails at file-inspect/EINVAL and VDA log initialization at EILSEQ before AddDevice; exact firmware/ELF SHA and reader source are unknown. Those logs do not test M1-248.

Version header, changelog, Payload Manager metadata, DEV workflow default and packaged loading guide are being aligned to dev.6. The candidate must pass exact-SHA Host/SDK workflows; DEV workflow dispatch and publication require separate explicit approval. No console/firmware/radio/tile operation. Current main before version bump: 4bd783c1b8dc1082942b85a7fcee72a0e0c45632; see CURRENT_STATUS, TEST_LOG and SESSION_LOG for CI evidence and next action.

## M1-248 — source fixes now on main

Source commit e03f2c428e98c8e76334539f33e1f336d3b6ae49 is on current main ancestry. Host and SDK runs 41245027045/41245027061 plus secret scan 41245027077 passed. Documentation descendant 4bd783c1b8dc1082942b85a7fcee72a0e0c45632 passed Host/SDK 41245148448/41245148430. This code is not in the published dev.5 ELF. The current local checkout remains an inherited dirty overlay and must not be committed/published wholesale.
