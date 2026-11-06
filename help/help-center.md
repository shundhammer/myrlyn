# Myrlyn Help Center

_Author: Stefan Hundhammer (GitHub: @shundhammer)_


This is the landing page for help topics around the Myrlyn package manager GUI.

<img height="150" src="https://raw.githubusercontent.com/shundhammer/Myrlyn/refs/heads/master/src/artwork/Myrlyn.svg">


## Permissions and Privileges

_[↗ Go to topic](permissions.md)_

- Why do I need root privileges for Myrlyn?

- Myrlyn without root privileges: Read-only mode
  - Exploring packages
  - Experimenting without danger
  - Limitations


### How to Get Root Privileges

- sudo / myrlyn-sudo
  - sudo -E
  - Help menu entries / dropping privileges
- kdesu / gnomesu


### How to Configure Your Normal User Account to Use sudo

- Needed packages
- Adding your user account to the wheel group
- Root password or your normal user account's password?

-------------------------


## Customization

_[↗ Go to topic](customization.md)_

### Changing Desktop Settings for the Root User

- Why do I need that?
- No root password set?
- Logging into the root user's desktop
- Using qt6ct
  - Setting QT_QPA_PLATFORMTHEME
  - Using myrlyn-sudo.conf
- Checking the log


### Customization FAQs

- [↗ Font size](font-size.md)

- [↗ GUI scaling](gui-scaling.md)

- [↗ Dark theme](dark-theme.md)


-------------------------

## Updates

_[↗ Go to topic](updates.md)_

- What's in the package list?
- The difference between up and dup
  - Up
  - Dup
  - Point releases vs. Rolling releases
  - Update rules
  - Patches vs. Updates
- Checkmarks and sad faces
- Dependency resolver state


### Third Party Repos and Updates

_[↗ Go to topic](third-party-repos.md)_

- Packages from third-party repos causing dependency problems
- Excluding third-party repos from dup / up



## Package Search

_[↗ Go to topic](search.md)_

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

_[↗ Go to topic](best-practices.md)_

- Do many small changes, not one huge change
  - Updates
- Be prepared to start over
- Handling dependency problems


## Misc

- Status taboo and protected
- Zypp history


-----------------

## Feedback and Errors in this Document

Please use the GitHub issue tracker or a pull request.
For short comments, feedback and emojis, there is already
[issue #196](https://github.com/shundhammer/myrlyn/issues/169).

Please do not write personal mail.
