# GitHub Actions maintenance — PRs 3–7

2026-11-01 · M1-251 · CI-only maintenance; published dev.7 remains unchanged.

## Scope and source review

Integrate the five existing Dependabot commits on complete main f44da276c6672bb283d2b715f3a8729c2e598df8, preserving their original commit ancestry. The adjacent checkout/setup-node conflict in host-tests.yml is resolved with both upgraded versions. No payload source, SDK pin, profile/storage paths, firmware policy, release tag or downloadable ELF changes.

| PR | Dependency | Upgrade | Reviewed upstream reference |
|---|---|---|---|
| 3 | actions/upload-artifact | v6 → v7 | cf430e030ddbb5b0abf93d22962f4752f3646cd9 |
| 4 | actions/checkout | v4/v6 → v7 | 3d3c42e5aac5ba805825da76410c181273ba90b1 |
| 5 | actions/setup-node | v4 → v7 | 949feb2413d6458794dcd2491c4babbbce0c15c1 |
| 6 | actions/download-artifact | v6 → v8 | 9000827ccba6bdab643e8b6fd33ac0654aef8333 |
| 7 | actions/github-script | v8 → v9 | annotated tag373c709c69115d41ff229c7e5df9f8788daa9553 → commit3a2844b7e9c422d3c10d287c895573f7108da1b3 |

These are source-review pins, not a claim that the workflow major-version references are immutable. Dependency lockfiles/SHA-pinning policy remain a separate deferred task.

All selected actions use Node24 and run on GitHub-hosted Ubuntu. Checkout's privileged-event fork restriction is not bypassed; our events remain push, pull_request and workflow_dispatch. The repository's github-script bodies use injected github/context/core with no require('@actions/github') or getOctokit redeclaration, so the documented v9 ESM changes do not require a script rewrite. Setup-node still selects Node24; no package-manager manifest was introduced.

Upload steps explicitly retain archive:true for multi-file packages. Download steps explicitly use digest-mismatch:error. No warning override, archive:false/single-file mode, release-gate weakening or new publish permission is added.

## Non-publishing verification

`.github/workflows/ci-actions-smoke.yml` runs on workflow changes or manual dispatch. It uses only contents/read and actions/read, with bounded jobs and a one-day test artifact. It:

1. Checks out with checkout v7 and exercises the ref, compare and workflow-list APIs used by the DEV source/tag checks through github-script v9.
2. Uploads three ordinary fixture files (LICENSE, public source SHA, checksums) using upload-artifact v7/archive:true.
3. Downloads them in another job with download-artifact v8/digest-mismatch:error, verifies every checksum, layout/source SHA and exact LICENSE bytes.

It creates no tag, release or ELF and does not contact a console. Normal Host/ASan/UBSan/Chromium, SDKv0.43 and full-history secret-scan push workflows additionally validate the combined candidate.

## Evidence and next step

Compatibility is VERIFIED_CODE from the exact action.yml definitions/PR changes and VERIFIED_HOST_TEST from the combined candidate runs below. Do not infer successful DEV publication from the smoke check; the full existing manual release workflow remains mandatory for every future release.

Issue2 is now scoped to dev.7 with its exact release SHA/checksum and controlled VDA/user/native-DualSense/storage checklist; the original dev.5 request is retained under historical details. It remains open for real measured evidence. PR1 remains closed and superseded; its incomplete tree is not reused.


## M1-251 — verified CI Actions integration (2026-11-01)

All five Dependabot heads from PR3–7 are preserved in the combined ancestry on complete dev.7-era main. Candidate 44b9426a18e69b8d384cf505581903612e8ffd97 passed fresh push checks: [Host/ASan/UBSan/Chromium](https://github.com/MNeroba/SteamController-PS5/actions/runs/41305639575), [SDK v0.43](https://github.com/MNeroba/SteamController-PS5/actions/runs/41305639619), [secret scan](https://github.com/MNeroba/SteamController-PS5/actions/runs/41305639576), and [CI Actions smoke](https://github.com/MNeroba/SteamController-PS5/actions/runs/41305639653), all completed/success.

The smoke log confirms the reviewed checkout/upload/download/github-script revisions were executed; both upload and download jobs passed, every fixture checksum and exact LICENSE/source identity matched. All five upstream action licenses were reviewed as MIT at the referenced revisions (github-script uses LICENSE.md). Hosted Node24, explicit archive:true and digest-mismatch:error are compatible. No privileged-event fork-checkout bypass or write permission was added to the smoke.

Payload, tests, scripts, config, SDK/version/policy and released dev.7 bytes are unchanged. This is CI maintenance, not a new payload release or hardware result. Issue2 now requests exact dev.7 evidence and remains open; its dev.5 text is preserved. PR1 stays closed/superseded. Future workflow updates should run this smoke check; future DEV publication still requires its separate manual full-gate procedure and owner approval. Next product task remains controlled physical dev.7 verification.
