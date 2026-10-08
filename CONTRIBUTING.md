# Contributing to SteamController-PS5

Thanks for helping improve the project. Contributions should make the Steam Controller 2026 path safer, easier to test, and easier to continue. This repository is still in development; read the status and hardware limits in [README.md](README.md) before proposing runtime claims.

## Before you start

1. Read [AGENTS.md](AGENTS.md), the relevant section of [CODEX_TASK.md](CODEX_TASK.md), and the related architecture, evidence, test, and project-memory documents.
2. Search existing issues and pull requests. For a new feature, firmware-specific behavior, protocol change, security change, or architectural change, open an issue first to agree on scope and the evidence needed.
3. Keep the change focused. One pull request should address one coherent problem; separate unrelated refactors, UI changes, and features.
4. For a reproduced bug, add a regression that fails on the old behavior before fixing it. Do not write a test that only duplicates the implementation's assumptions.

## Safety and project boundaries

- USB-C and the official Valve Puck are the current hardware priorities. Native PS5 Bluetooth LE is an evidence-gated research track.
- Do not reset, scan, claim, pair, or take over the stock PS5 Bluetooth radio. Do not send raw HCI commands. Any active radio experiment requires an explicit issue-approved lab plan and must preserve native DualSense operation.
- Do not write firmware/system resources, invoke unknown ABI calls with side effects, run unreviewed game-memory patches, or perform irreversible operations on a physical console.
- Keep the standard virtual DualSense path as the default. Edge identity and ShellUI changes are separate research experiments.
- Preserve the complete Steam input model. Device-specific decoding belongs in Steam protocol/device adapters; transport code must not own button mappings; mapping code must not depend on the PS5 ABI; portal code must not mutate driver state directly.
- Protect LAN controls. No anonymous input injection, unprotected state-changing endpoints, credentials in logs, or secrets in fixtures.

If a change would cross one of these boundaries, explain the need and evidence in an issue before implementing it. Do not perform a risky experiment to obtain evidence for a PR.

## Evidence and test reporting

Use the project's evidence labels exactly and apply them narrowly:

- `VERIFIED_CODE` — a fact visible in a pinned source revision.
- `VERIFIED_HOST_TEST` — a test actually executed on the host; include the exact command and result.
- `VERIFIED_PS5_BUILD` — a successful cross-build with SDK/toolchain versions and artifact identity. This is compile/link evidence, not runtime evidence.
- `VERIFIED_HARDWARE` — a test actually run on the listed physical hardware and firmware.
- `UPSTREAM_CLAIM` — a claim made by an upstream project's author, not independently reproduced here.
- `EXTERNAL_HARDWARE` — evidence from hardware or a report outside this project.
- `HYPOTHESIS` — a reasoned but unverified explanation.
- `BLOCKED` — the check cannot be done in the available environment; state what is missing.

Before opening a PR, run the relevant host checks and report their actual outcomes:

```sh
make -C tests clean test
make -C tests clean sanitize
git diff --check
```

For concurrency changes, run the targeted ThreadSanitizer command documented for that suite in [`project-memory/TEST_LOG.md`](project-memory/TEST_LOG.md). For payload changes, follow [docs/BUILDING.md](docs/BUILDING.md) and report the SDK version, compiler version, build result, and ELF SHA-256. Do not describe CI as passing unless the current PR head has a completed successful workflow run.

If a check cannot run, mark it `NOT_RUN` and state why (for example, no PS5 or Steam Controller available). Do not convert a host mock, source inspection, cross-build, upstream claim, or external report into a hardware result. Add hardware results only with the exact console model, firmware, controller/receiver and firmware, transport, action performed, and observed result. Redact serials, Bluetooth addresses, keys, tokens, usernames, and other identifying data.

## Third-party code, assets, and licensing

This repository is GPL-3.0-or-later and derives from PuckbridgePS5. Before proposing third-party source or assets:

1. Identify the exact repository, file/path, pinned commit or release, copyright notice, and applicable license.
2. Add or update a row in [`docs/PROVENANCE.md`](docs/PROVENANCE.md) **before** importing the material.
3. Retain all existing notices and explain what changed in the PR.
4. Do not submit Sony confidential SDK headers, proprietary firmware, keys, dumps, private symbols, or material obtained under an NDA.
5. Do not import BTstack code without first resolving its noncommercial terms against this project's license and distribution goals. A public repository or a compatible repository-level license does not automatically clear each asset or source file.

Do not commit generated payload binaries, downloaded SDK files, private test captures, or local credentials. Build caches and ELF output belong in the existing ignored paths.

## Branch and pull request process

1. Fork the repository and branch from the current `main` (unless an issue or maintainer asks for another base). Use a short descriptive branch name such as `fix/session-neutral-state`, `feat/steam-touch-mapping`, or `docs/build-guide`.
2. Make a small set of commits with clear messages. Keep generated files and unrelated formatting changes out of the diff.
3. Open a pull request against `main`. Use **Draft** while implementation, tests, or required evidence are incomplete. Link the issue and explain the user-visible or safety impact.
4. Fill every applicable section of the [pull request template](.github/pull_request_template.md). Include exact commands, environment, test results, evidence labels, licensing/provenance, and anything not run.
5. Respond to review with focused follow-up commits. If requirements change, update the issue or PR description so the evidence and remaining work stay clear.

Do not mark a PR ready or describe it as release-ready while it depends on unverified hardware, firmware behavior, unsupported BLE, or an unresolved safety gate. Maintainers decide when a PR has enough evidence to merge.

### Good pull requests

- Fix one reproducible defect and include an expected-red regression plus the passing result.
- Add a portable module with a narrow contract and show how the existing adapter uses it.
- Improve docs with commands and compatibility statements verified against the source tree.
- Add third-party code only after recording its exact provenance and legal review.

### Pull requests that need more work

- A large drop-in rewrite or copied subsystem without source comparison, license details, or tests.
- A claim of “works on PS5” based only on compilation or a mock.
- A Bluetooth, firmware, system-resource, game-hook, or loader experiment without prior evidence and a safe review plan.
- A test report that omits failures, unavailable hardware, sanitizer limitations, or the tested commit.
- Logs containing tokens, private addresses, serial numbers, personal information, or console secrets.

## Further reading

The repository contribution flow follows GitHub's guidance for [contributor guidelines](https://docs.github.com/en/communities/setting-up-your-project-for-healthy-contributions/setting-guidelines-for-repository-contributors) and [pull request templates](https://docs.github.com/en/communities/using-templates-to-encourage-useful-issues-and-pull-requests/about-issue-and-pull-request-templates). The template collects scope, evidence, tests, and review requirements in a consistent place.
