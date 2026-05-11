![build workflow](https://github.com/august-alt/gpui/actions/workflows/main.yml/badge.svg)
![tests workflow](https://github.com/august-alt/gpui/actions/workflows/test.yml/badge.svg)

# GPUI

GPUI — is an open source utility for group policy management on ALT-Linux.

![Screenshot](https://github.com/mchernigin/gpui/assets/59616661/951b3715-96f0-4c8f-b044-33ab4b94a75f)

## Building and installation

For building and installation instructions refer to [INSTALL.md](INSTALL.md).

## Basic usage

```
Usage: gpui-main [options]

Options:
  -p <path>      The full path of policy to edit.
  -b <path>      The full path of policy bundle to load.
  -h, --help     Displays help on commandline options.
  -n <name>      This options left for compatibility with ADMC. Currently it
                 does nothing.
  -v, --version  Displays version information.
```

## Group Policy Preferences

The Preferences editor supports the "Common" tab controls that every MSAD
Group Policy Preferences item exposes:

- **Apply once and do not reapply** — run the preference exactly once per
  client, matching the MSAD `FilterRunOnce` semantics.
- **Item-level targeting** — restrict when a preference item is applied by
  combining the full MSAD filter catalog (Battery Present, Computer Name,
  CPU Speed, Date Match, Disk Space, Domain, Environment Variable, File
  Match, IP Address Range, Language, LDAP Query, MAC Address Range, MSI
  Query, Network Connection, Operating System, Organizational Unit,
  PCMCIA, Portable Computer, Processing Mode, RAM, Registry Match,
  Security Group, Site, Terminal Session, Time Range, User, WMI Query,
  Dial-Up Networking) with AND/OR/NOT combinators and parenthesized
  `FilterCollection` groups.

The `<Filters>` subtree is persisted byte-for-byte compatibly with MSAD:
filter ordering, `id` attributes and unknown attributes (e.g. from newer
MSAD releases) round-trip through gpui unchanged.

