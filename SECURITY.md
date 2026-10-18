# Security Policy

BearWave is currently in a public beta phase. Security reports are very welcome and should be handled carefully, especially since BearWave processes external internet radio streams and metadata.

## Build and release security

This Omarchy edition is built locally from source. Forgejo Actions builds and
tests changes; only a successful `main` build is mirrored to the public GitHub
repository. Review the source and build it with the documented CMake workflow
when installing from a checkout.

BearWave processes untrusted station records, artwork links, redirects, and
stream metadata. The current code rejects artwork requests to non-public
network targets, renders external metadata as plain text, and stores persisted
stream URLs in owner-only configuration data. Please preserve these controls
when contributing changes.

## Supported Versions

| Version | Supported |
|---------|-----------|
| **1.3.x** (latest) | Yes |
| 1.2.x | Prefer upgrade to 1.3.x |
| &lt; 1.2 | No |

During the public beta phase, fixes primarily target the latest code on `main`
and the most recent published release. Users should update after a security fix
is published.

## Reporting a Vulnerability

Please **do not** open a public issue for a vulnerability before it has been reviewed.

Preferred reporting path:

Contact the maintainer through an agreed private channel. Do not place secrets,
access tokens, personal data, or unredacted system logs in this working tree.

Include as much detail as possible:

- Affected BearWave version or commit SHA
- Operating system and desktop environment
- Steps to reproduce the issue
- Potential impact of the vulnerability
