# Myrlyn Help: Anatomy of a Pattern Pseudo-Package

_Author: Stefan Hundhammer (GitHub: @shundhammer)_

_Up: [↗ Patterns](patterns.md)_

-------------------------


**This is for nerds.**
But hey, we are Linux people. Aren't we all more or less nerdy?


## In Myrlyn

- Switch to the "Search" view
- In "Auto" mode, search for `patterns-*`
  or switch to "Starts With" mode and search for `patterns-`
- Select any of the search results in the package list
- Switch to the "Dependencies" details view below the package list
- Scroll down to see
  - Provides
  - Requires
  - Recommends
  - Suggests
  - Supplements (sometimes)
  - Enhances (sometimes)


## With zypper

```console

sh@meteor:~> zypper patterns -i

S  | Name          | Version       | Repository           | Dependency
---+---------------+---------------+----------------------+-----------
i  | base          | 20241218-34.1 | Slowroll OSS Repo    |
...
i+ | console       | 20241218-34.1 | Slowroll OSS Repo    |
i+ | devel_basis   | 20170319-13.4 | Slowroll OSS Repo    |
i+ | devel_C_C++   | 20170319-4.10 | Slowroll OSS Repo    |
...
i+ | enhanced_base | 20241218-34.1 | Slowroll OSS Repo    |
i+ | fonts         | 20170319-13.2 | Slowroll OSS Repo    |
i+ | imaging       | 20201106-5.3  | Slowroll OSS Repo    |
i+ | laptop        | 20201106-5.3  | Slowroll OSS Repo    |
i  | minimal_base  | 20241218-34.1 | Slowroll OSS Repo    |
...
i+ | xfce          | 20230212-13.1 | Slowroll Update Repo |
i+ | xfce_extra    | 20230212-13.1 | Slowroll Update Repo |

sh@meteor:~> zypper info --provides --requires --recommends --suggests \
  --supplements --enhances patterns-base-base

...
Information for package patterns-base-base:
-------------------------------------------
Repository     : Slowroll OSS Repo
Name           : patterns-base-base
...
Upstream URL   : https://github.com/openSUSE/patterns
Summary        : Base System
Description    :
    This is the base runtime system.  It contains only a basic multiuser
    booting system. For running on real hardware, you need to add additional
    packages and pattern to make this pattern useful on its own.

Provides       : [9]
    pattern-visible()
    pattern-category() = Base%20Technologies
    pattern() = base
    pattern-icon() = pattern-basis
    patterns-base-base = 20241218-34.1
    pattern-order() = 1030
    ...
Requires       : [28]
    coreutils
    aaa_base
    systemd
    glibc
    bash
    util-linux
    shadow
    rpm
    procps
    ca-certificates-mozilla
    timezone
    pam
    libnss_usrfiles2
    pam-config
    terminfo-base
    glibc-locale-base
    user(nobody)
    coreutils-systemd
    wtmpdb
    ntp-daemon
    pattern() = minimal_base
    ...
Recommends     : [16]
    (busybox-static if (product(openSUSE) or product(SLES) or product(SLES-SAP)))
    ...
    (service(network) if (product(openSUSE) or product(SLES) or product(SLES-SAP)))
    (system-group-trusted if (product(openSUSE) or product(SLES) or product(SLES-SAP)))
    (system-group-wheel if (product(openSUSE) or product(SLES) or product(SLES-SAP)))
    ...
    (zypper if (product(openSUSE) or product(SLES) or product(SLES-SAP)))
Suggests       : [12]
    kernel-default
    NetworkManager
    tar
    xz
    openssl
    psmisc
    postfix
    pipewire-pulseaudio
    chrony
    libcurl4
    libjack-devel
    openssl-1_1
Supplements    : ---
Enhances       : ---
```

Some interesting parts:

```
    Summary : Base System
```

This is the user-visible name in the pattern list.


```
    pattern-visible()
```

This indicates that this pattern is visible to the user in the "Patterns" view.
Invisible patterns simply don't provide this symbol.


```
    pattern-category() = Base%20Technologies
```

This is the pattern _category_, the headline under which this is displayed in
the pattern list in the "Patterns" view. Note that the blank in the name is
HTML-escaped with `%20` to avoid a syntax error in the RPM "provides"
dependency.

```
    pattern() = base
```

This _looks_ like a user-visible name in the pattern list, but it's not:
The RPM "summary" field is used for that purpose.

This field is the _internal_ pattern name that can be used for "requires:" or
"recommends:" in other patterns like this: `Requires: pattern() = base`.


```
    pattern-icon() = pattern-basis
```

This is the name of the icon for this pattern in the pattern list. Myrlyn has a
number of built-in icons for this purpose as a fallback, but it uses an icon
from the icon theme if there is one.

```
    patterns-base-base = 20241218-34.1
```

This is the exact version of the pattern in case this is needed for some
"Obsoletes" dependency or for another pattern's "Requires" dependency.


```
    pattern-order() = 1030
```

This is the sort order of the pattern within its pattern category in the
pattern list.

```
Requires       : [28]
    coreutils
    aaa_base
    systemd
    glibc
    bash
    ...
```

A real list of packages that are required for this pattern. Since this is one
of the very fundamental building blocks, this contains more than an average
number of packages. But...

```
    pattern() = minimal_base
```

...it's not the most fundamental one: It requires pattern "minimal_base".

```
Recommends     : [16]
    (busybox-static if (product(openSUSE) or product(SLES) or product(SLES-SAP)))
    ...
    (service(network) if (product(openSUSE) or product(SLES) or product(SLES-SAP)))
    (system-group-trusted if (product(openSUSE) or product(SLES) or product(SLES-SAP)))
    (system-group-wheel if (product(openSUSE) or product(SLES) or product(SLES-SAP)))
    ...
    (zypper if (product(openSUSE) or product(SLES) or product(SLES-SAP)))
```

Here it pulls in some more packages, but only as "recommended" packages,
i.e., they are not absolutely required. We can see that this depends on what
product is installed.

Note that `system-group-*` packages ensure that certain user groups are
created in `/etc/group`, and the real action happens in the
pre-/post-transaction scripts:

```console
sh@meteor:~> rpm -ql system-group-wheel
/usr/lib/sysusers.d/system-group-wheel.conf
sh@meteor:~> cat /usr/lib/sysusers.d/system-group-wheel.conf
# Type Name ID GECOS [HOME]
g wheel -
sh@meteor:~> rpm -q --scripts system-group-wheel
preinstall scriptlet (using /bin/sh):
/usr/sbin/sysusers2shadow system-group-wheel.conf <<"EOF"
g wheel -
EOF
```

But let's continue with the pattern:

```
Suggests       : [12]
    kernel-default
    NetworkManager
    tar
    xz
    openssl
```

Isn't it surprising that `kernel-default` is only _suggested_? You'd think that
something as fundamental as the Linux kernel would be a hard requirement. No
kernel - no booting - no fun.

But you might want to install `kernel-longterm` instead. Or a custom kernel
that one of the kernel developers gave you via Bugzilla to see if that one
solves a hardware problem that you have on your brand new laptop. Or you might
want to build your own.


## With an rpm Command Line

This is pretty much the same as with `zypper`, but it's limited to the patterns
that you actually have installed because the `rpm` command knows nothing about
your active repos, only about installed packages that are in the (local!) RPM
database.


```console
sh@meteor:~> rpm -qa "patterns-*"
patterns-base-base-20241218-34.1.x86_64
patterns-base-basesystem-20241218-34.1.x86_64
patterns-base-console-20241218-34.1.x86_64
patterns-base-enhanced_base-20241218-34.1.x86_64
patterns-base-minimal_base-20241218-34.1.x86_64
patterns-base-sw_management-20241218-34.1.x86_64
patterns-devel-C-C++-devel_C_C++-20170319-4.10.noarch
patterns-xfce-xfce-20230212-13.1.x86_64
patterns-xfce-xfce_extra-20230212-13.1.x86_64
...
sh@meteor:~> rpm -q --provides patterns-base-base
pattern() = base
pattern-category() = Base%20Technologies
pattern-icon() = pattern-basis
pattern-order() = 1030
pattern-visible()
patterns-base-base = 20241218-34.1
patterns-base-base(x86-64) = 20241218-34.1
patterns-openSUSE-base = 20241218
patterns-openSUSE-minimal = 20241218

sh@meteor:~> rpm -q --requires patterns-base-base
...
aaa_base
bash
coreutils
glibc
...
sh@meteor:~> rpm -q --recommends patterns-base-base
(busybox-static if (product(openSUSE) or product(SLES) or product(SLES-SAP)))
...
(service(network) if (product(openSUSE) or product(SLES) or product(SLES-SAP)))
(system-group-trusted if (product(openSUSE) or product(SLES) or product(SLES-SAP)))
(system-group-wheel if (product(openSUSE) or product(SLES) or product(SLES-SAP)))
...
(zypper if (product(openSUSE) or product(SLES) or product(SLES-SAP)))
sh@meteor:~> rpm -q --suggests  patterns-base-base
kernel-default
NetworkManager
tar
xz
openssl
...
sh@meteor:~> rpm -q --enhances  patterns-base-base
sh@meteor:~>
```

-------------------------

_Up: [↗ Patterns](patterns.md)_
