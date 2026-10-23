# Public-source launch checklist

Reviewed: 2026-10-23. Repository: `MNeroba/SteamController-PS5`.

Opening the source is independent from establishing a stable, working PS5 runtime. The current experimental build remains `v0.1.0-dev.5` (source `382e4ecef4937356b72f9e1786917ae7147c0415`); `main` has later documentation commits. Do not call VDA/game input, account binding, USB teardown, console browser or native-DualSense coexistence physically verified.

## Source and exposure review

- Historical checkpoint M1-240 in [PUBLIC_READINESS.md](PUBLIC_READINESS.md) inspected all reachable commits then known (140 commits, 1,413 unique UTF-8 blobs, 63 binary-image objects) for targeted credential patterns and metadata. No confirmed credential leak was found. This was not an independent third-party secret scan.
- A new review of the **12 commits** after that checkpoint through `b45549639c2303338b8286196ba8b62b4ce26f90` inspected added patch lines for private-key headers, GitHub/AWS/Slack access tokens, long bearer literals, credential-bearing URLs, plausible hard-coded secret assignments and non-fixture email addresses. No matches. It is a signature-based review, **not proof that no secret exists**.
- **Nine selected** early, intermediate and recent GitHub Actions job logs were inspected for comparable secret patterns; no matches. Ordinary `/home/runner/` build paths occur in logs.
- GitHub reported **310 historical Actions runs** at this checkpoint. Their complete logs and downloadable artifacts have **not** all been inspected. Some are retained. A representative set of release, browser-evidence and cross-build artifacts was inventoried by name, size and expiration, not exhaustively unpacked or reviewed. These are residual exposure areas.
- The tracked tree contains code, documentation, test assets and third-party notices, without an obvious committed ELF, private-key filename or token-file filename. Publication does not hide historical branches, tags, old commits or prior release assets.
- Avoid adding actual console logs, screenshots containing account identifiers, pairing codes, browser bearer tokens, serial numbers, Bluetooth bonding data or user-specific paths. Existing public issue and SECURITY templates already direct users to sanitize evidence.

## Owner-only GitHub administration — required manual operations

The connected GitHub app cannot change repository visibility, set branch protection/rulesets or enable private vulnerability reporting. These settings require the repository administrator in GitHub.

1. **Review retained Actions history and artifacts before Public**, especially older failing jobs and archives; delete any individual runs/artifacts that expose secrets through GitHub's supported controls, and revoke exposed credentials if found. Selected scans above are incomplete. The existence of confidential data cannot be disproved solely by regex screening.
2. Verify the desired exposure of the complete reachable Git history, old PR discussions and `project-memory/` records, which include development decisions, local execution paths and build logs. Removing only today's file does not remove its history. Do not rewrite history without a separate plan, since published tags, source SHA evidence and release receipts depend on it.
3. Navigate to **Settings → General → Danger Zone → Change repository visibility → Public**, and complete GitHub's impact confirmation. GitHub says all prior Actions history/logs become publicly visible and push rulesets may be disabled by this conversion.
4. After the change, go to **Settings → Security and quality → Advanced Security**: enable and verify **Private vulnerability reporting**, plus available secret scanning/Dependabot security alerts and code scanning as applicable. Do not disclose vulnerabilities in public issues.
5. Configure **Settings → Rules → Rulesets** or **Settings → Branches** for `main`: disallow force-pushes and deletion, require checks for `host-tests` and `build` (verify exact check names in the GitHub UI), use PR review/conversation resolution for community contributions. Avoid breaking the deliberately separate, manually approved DEV prerelease workflow. Check settings again after switching visibility.
6. Review **Settings → Actions → General**: least-privileged default GITHUB_TOKEN, fork PR approval policy and permitted Actions; never allow write tokens or sensitive repository secrets in untrusted PR code. The Host test workflow now specifies `contents: read`.
7. Verify the repository and release links from a **signed-out** session, confirm `main` branch checks, issue templates, SECURITY.md and experimental dev.5 warnings, then announce the source as experimental.

## Runtime risks disclosed to contributors

- External dev.4 console logs support Steam Controller USB/Puck input decoding, **not** functional virtual DualSense game input. Dev.5 uses owner-authorized bounded system-event correlation after one VDA Add; this is not a proven ownership ABI.
- User assignment and native DualSense coexistence are not hardware-validated. A missing fresh `/dev/klog` / loopback 3232 source prevents a pending VDA attempt by design. No repeated Add/guessed handles are permitted.
- WebUI uses pairing and bearer authorization but plain HTTP on trusted LAN; operational storage failure may use an authenticated RAM-only token. Real console storage error remains unknown.
- Keep normal DualSense available, avoid simultaneous VDA payloads, do not repeatedly reload an unresolved registration and do not claim FW13.60 compatibility until tested.

**Outcome:** Source visibility is technically reasonable as an experimental project after the administrator is comfortable with the remaining Actions/history exposure and configures the post-public security controls. A stable functional release is **not** authorized by this checklist.
