# Myrlyn Help Center

This is the landing page for help topics around the Myrlyn package manager GUI.


## Permissions and Privileges

_[↗ Go to topic](permissions.md)_

- Why do I need root privileges for Myrlyn?

- Myrlyn without root privileges: Read-only mode
  - Exploring packages
  - Experimenting without danger
  - Limitations


### How to get root privileges

- sudo / myrlyn-sudo
- kdesu / gnomesu
- Plain su
  - Permissions for the X11 or Wayland display
- run0 / myrlyn-run0


### How to configure my normal user account to use sudo

- Needed packages
- Adding my user account to the wheel group
- Root password or my normal user account's password?

-------------------------


## Customization

_[↗ Go to topic](customization.md)_

### Changing desktop settings for the root user

- Why do I need that?
- No root password set?
- Logging into the root user's desktop
- Using qt6ct
  - Setting QT_QPA_PLATFORMTHEME
  - Using myrlyn-sudo.conf


### Customization FAQs

- Font size
- GUI scaling
- Dark theme

-------------------------

## Updates

- The difference between dup and up
- Packages from third party repos causing dependency problems
- Excluding third-party repos from dup / up


## Package search

- Search modes
- Auto search mode
- Default for auto search mode
- Searching in RPM provides, requires, recommends


## Patterns

- The idea behind patterns
- Invisible patterns as building blocks
- What is a pattern technically?
- How to inspect a pattern pseudo-package


## Installation Summary

- Managing huge updates
- Check for auto-added packages
- Check for auto-deleted packages


## Package Classification View

- Orphaned packages
- Unneeded packages
- Retracted packages


## Managing Repositories

- issue #106: Repository Management
- Community Repositories
  - Codecs: Packman and VideoLAN
  - NVidia proprietary drivers
- Working with third-party repositories
- Using a local directory as repository
- Why the need to restart


## Repositories View

- Filtering by one repository
- Protect all / un-protect all
- Secondary filters
  - All packages
  - Installed packages
  - Not installed packages
  - Search
  - By status

- Versions details view
  - Package versions
  - Repositories and vendors
  - Architectures
  - Multi-version packages


## Best Practices

- Do many small changes, not one huge change
- Be prepared to start over
- Handling dependency problems


## Misc

- Status taboo and protected
- Zypp history
