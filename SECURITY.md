# Security policy

SteamController-PS5 is experimental software for a development console. Host tests and SDK builds do not establish safe operation or compatibility on physical PS5 hardware. There is no hardware-verified stable release.

## Versions and updates

The current experimental version is **v1.5.0-experimental**. Report reproducible defects against current `main` or identify the exact release/source SHA. Superseded DEV builds were archived during the 1.0.0 consolidation and are not supported downloads; historical receipts remain in project memory. There is no promise of maintenance branches or backported security fixes. Review the [changelog](CHANGELOG.md) and [release notes](https://github.com/MNeroba/SteamController-PS5/releases) before updating.

## Report a vulnerability privately

Do **not** put exploit details, tokens, pairing codes, private captures or console identifiers in a public issue or PR.

1. Use GitHub **Security → Report a vulnerability** to send a private report. Private vulnerability reporting is enabled for this public repository.
2. If the private form is unavailable, contact the maintainer through a private channel you already know. If no private channel is available, open an issue containing only **“Private security contact requested”** and wait for a private route before sharing technical details. Do not put a proof of concept or sensitive files in that issue.

Provide the affected release/source SHA, area, expected/observed behavior, minimal sanitized reproducer, impact and actual test environment. State explicitly whether the issue was observed on a host fixture or a physical console. Retain error codes and timing, but remove credentials, serials, account names, Bluetooth addresses and bonding keys.

This is a solo-maintained project. No response deadline, bug bounty or coordinated-disclosure date is promised. Agree on disclosure privately before publishing an exploitable issue.

## Runtime boundaries

- The payload holds elevated kernel credentials (an elevated authid and the maximum capability mask) for its whole lifetime because the SDK's kernel/MBus/hook calls need them; they are restored on shutdown. The loopback WebUI and all input parsers therefore run in that privileged process. There is no sandbox between them; keep untrusted software off the console and review this code before loading it.
- The current WebUI binds only PS5 IPv4 loopback. It has no pairing code or user authentication. Host/Origin checks, no-sniff and frame denial reduce browser attacks; local console processes can access it.
- Older releases used pairing codes and bearer tokens; never share those credentials.
- Keep the native DualSense available. Game hooks, bridge probing and DualSense handoff remain disabled by default; native Bluetooth LE is not enabled.
- The current first-run worker attempts Media tile registration automatically only if the exact title path is absent. The tile does not start the payload after reboot.
- Do not obtain reproduction evidence by taking over the stock radio, flashing firmware, invoking unknown side-effecting APIs or making unreviewed system changes.
- RAM-only fallback uses a new session credential after eligible operational storage failures; unsafe existing credentials still fail closed. Profile persistence is independent of this fallback.
- Pending VDA recovery uses temporal event correlation, which is not exclusive ownership proof. Keep other virtual-controller payloads stopped; native coexistence still needs PS5 testing.
- Diagnostic reports are bounded and filtered, but review profile names, USB paths and contextual details before sharing them publicly.

[Loading and recovery](docs/development/LOADING.md) · [Contribution rules](CONTRIBUTING.md) · [Hardware test plan](docs/TEST_PLAN.md)
