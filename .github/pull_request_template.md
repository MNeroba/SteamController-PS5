## Summary

<!-- What changes, and why? Keep this PR focused on one coherent issue. -->

## Related issue

<!-- Link an issue, or write "None". For significant features/ABI/security work, discuss scope in an issue first. -->

## Change type

- [ ] Bug fix
- [ ] Feature
- [ ] Refactor / architecture
- [ ] Security
- [ ] Documentation / tests
- [ ] Research only

## Evidence

Use the exact project labels that apply: `VERIFIED_CODE`, `VERIFIED_HOST_TEST`, `VERIFIED_PS5_BUILD`, `VERIFIED_HARDWARE`, `UPSTREAM_CLAIM`, `EXTERNAL_HARDWARE`, `HYPOTHESIS`, `BLOCKED`.

| Claim or behavior | Evidence label | Source / exact test / limitation |
|---|---|---|
|  |  |  |

## Tests and build

List exact commands, environment, and results. Do not report a cross-build or host mock as a PS5 runtime test.

```text
Command:
Environment:
Result:
```

- [ ] `make -C tests clean test`
- [ ] `make -C tests clean sanitize`
- [ ] Targeted ThreadSanitizer run for concurrency changes (command and result below)
- [ ] PS5 SDK cross-build for payload changes (SDK/compiler version and ELF SHA-256 below)
- [ ] `git diff --check`

Not run and why:

## Hardware, firmware, and transport

<!-- If tested, include PS5 model, firmware, controller/receiver model and firmware, transport, exact action, and result. Otherwise say NOT_RUN and explain the blocker. Never include serials, MAC addresses, tokens, or other secrets. -->

`NOT_RUN` / details:

## Security and safety review

- [ ] No stock Bluetooth radio/HCI operation, pairing change, firmware/system write, or irreversible console operation was performed.
- [ ] No anonymous input injection or unprotected state-changing endpoint was added.
- [ ] No secrets, serial numbers, Bluetooth addresses/bonding material, or user-identifying data are included.
- [ ] Any new game-hook/firmware-specific path is disabled by default, bounded, and has a documented rollback strategy.
- [ ] If this PR proposes an active BLE or other hardware experiment, an issue-approved evidence/safety plan exists; the experiment is not performed by this PR.

## Provenance and licensing

- [ ] No third-party code/assets were added; or
- [ ] `docs/PROVENANCE.md` records exact source URL/path, revision, license, copyright, destination, and changes.
- [ ] Existing upstream copyright and license notices are preserved.
- [ ] No confidential Sony SDK/firmware material, keys, symbols, or unlicensed assets are included.

## Review checklist

- [ ] The diff addresses one coherent change and avoids duplicate protocol/slot/web/BLE implementations.
- [ ] Reproducible bugs have a regression test that fails before the fix.
- [ ] Documentation and project memory are updated where the behavior, evidence, issue, decision, or blocker changed.
- [ ] Unsupported firmware or hardware compatibility is not claimed.
- [ ] This PR is marked Draft if implementation or required evidence is incomplete.
