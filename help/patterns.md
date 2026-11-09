# Myrlyn Help: Patterns

_Author: Stefan Hundhammer (GitHub: @shundhammer)_

_Up: [↗ Help Center](help-center.md)_

 <img width="1000" src="https://raw.githubusercontent.com/shundhammer/myrlyn/refs/heads/master/screenshots/Myrlyn-13-patterns-view.png">

-------------------------


This is about Myrlyn's "Patterns" view. If you don't see it, use the "View"
button at the left of the open tabs, then "Patterns".


## The Idea Behind Patterns

In the openSUSE / libzypp context, a (software) _pattern_ is a collection of
software packages that provides your machine with a higher-level capability. To
name just a few:

- A graphical desktop environment like KDE Plasma / GNOME / Xfce etc.

- Some kind of server functionality, such as
  - file server
  - print server
  - web server

- A software development environment for:
  - KDE Plasma development (C++, Qt, Kf libs and header files)
  - Java development
  - Python development


## Building Blocks

A pattern consists of all needed packages for its task, and it often suggests
some more packages that may be useful in that context.

In most cases, a pattern includes not only packages, but also a number of other
patterns as building blocks.

For example, a C++ development environment is useful by itself, but it is also
needed for developing KDE Plasma software, and also for pure Qt software
development.

Some of those building block patterns may not be visible in the user interface,
so their packages may not be visible in any of the higher-level patterns that
are user-visible.


## What is a Pattern Technically?

A pattern is really just a package that does not have any real content that you
would find in the filesystem; it consists only of dependencies such as
"requires", "recommends", "suggests", and "enhances", and some
"provides" dependencies that are specific to patterns.

By convention, a pattern pseudo-package has a name that starts with
"patterns-", so you can easily find them all (even the invisible ones) with a
simple package search for packages with a name starting with `patterns-`. Try
that and switch to the "Dependencies" details view below the package list.


## Anatomy of a Pattern Pseudo-Package

(For nerds)

_[↗ Go to topic](pattern-anatomy.md)_

- In Myrlyn
- With zypper
- With an rpm command line


-------------------------

_Next: [↗ Installation Summary](inst-summary.md)_
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
_Previous: [↗ Package Search](search.md)_
