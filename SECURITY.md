# Security Policy

BearWave is currently in a public beta phase. Security reports are very welcome and should be handled carefully, especially since BearWave processes external internet radio streams and metadata.

## Local Build Security

This Omarchy edition is built and used locally. No prebuilt binary repository
or automated publishing channel is part of this project. Review the local
source and build it with the documented CMake workflow.

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
