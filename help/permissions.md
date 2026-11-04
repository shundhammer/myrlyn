# Myrlyn Help: Permissions and Privileges

_Up: [↗ Help Center](help-center.md)_

<img height="150" src="https://raw.githubusercontent.com/shundhammer/Myrlyn/refs/heads/master/src/artwork/Myrlyn.svg">

-------------------------


## Why do I Need Root Privileges for Myrlyn?

For any serious package management, you need _root_ privileges, because you
will change the system:

- Load and refresh the software repositories when Myrlyn starts

- Install or remove packages or whole software _patterns_

- Keep your system up to date with daily / weekly updates

- Add, modify or remove software repositories



##  Myrlyn Without Root Privileges (Read-Only Mode)

You can use Myrlyn as a normal user without _root_ user privileges, but only in
a _read-only_ mode, i.e. without being able to change anything. This is not as
pointless as it may sound: You can explore and experiment.


### Exploring Packages

You can explore and inspect your system and the packages that are available in
the active repositories. To name just a few:

- See what packages are installed

- Search for available packages (installed or not)

- See the details for each package:

  - Package description
  - Technical details
  - Dependencies
  - Versions
  - File list (installed packages only)
  - Change log (installed packages only)

- See what packages each active repository provides


### Experimenting Without Danger

You can even mark packages for installation, update or removal and see what
that would mean: The package dependencies will be resolved, you will see any
dependency problems, and you can choose solutions for them.

You can see what it would mean to remove packages that a whole lot of other
packages depend on; you can choose even radical destructive solutions of the
dependency resolver that would remove whole subsystems (such as all of KDE or
all of Qt) without fear of wrecking your system.

When you are done with the destructive experiment, switch to the "Installation
Summary" view and see the result: What packages would be removed, and what
packages would remain untouched.

Not only can this be very educational, you could use it as a preparation to
migrate to a different desktop, or to reduce your system to a very minimalistic
one.


### Limitations

In read-only, you cannot actually change anything on your system: The "Accept"
button is disabled. You can look, you can touch, but you cannot perform any
changes.

Likewise, when starting Myrlyn in _read-only_ mode (as a non-privileged user),
the active repos are loaded, but not refreshed; because refreshing would
already mean changing the system (libzypp's package metadata cache).

Another annoying, but sadly real life limitation is that you also cannot view
the zypp history: Its parent directory has very restrictive permissions
(see [issue #136](https://github.com/shundhammer/myrlyn/issues/136).



## How to Get Root Privileges

There are many different methods to get _root_ privileges on a modern Linux
system, but there are very few that allow you to run a graphical application
like Myrlyn with _root_ privileges.


### sudo / myrlyn-sudo

The preferred way is using `myrlyn-sudo`. This is what the provided "Myrlyn" /
"Full root access (via sudo)" (`myrlyn-sudo.desktop`) launcher / starter menu
entry uses.

The `sudo` package is installed by default on openSUSE distributions (Leap,
Tumbleweed, Slowroll). If you chose not to install it, you need to use
something else (see below).

Prior to Leap 16.0, the `sudo` configuration on openSUSE required you to enter
the _root_ password. From Leap 16.0 on, your first user account on a new
installation is added to the _wheel_ user group, and then you need to enter
_your own_ password, not the _root_ password.


[`myrlyn-sudo`](https://github.com/shundhammer/myrlyn/blob/master/src/myrlyn-sudo)
is a simple shell script that keeps certain environment variables from your
normal user alive across the `sudo` call. They keep the connection to your X11
or Wayland display alive, and also certain desktop settings.

But remember that Myrlyn with elevated permissions is running as the _root_
user, and you may have to configure desktop settings such as the widget theme
(light or dark), font sizes, GUI scaling for the _root_ user to match your
preferences.

See also [↗ Customization](customization.md).


#### sudo -E

If `myrlyn-sudo` does not work for you, try `sudo -E myrlyn`. The `-E`
parameter keeps _all_ environment variables; but your `sudo` configuration
might prohibit this (in rare extreme cases).


#### Help Menu Entries / Dropping Privileges

Using `sudo` / `myrlyn-sudo` also brings another benefit: They make it possible
to _drop_ the root privileges again, because `sudo` saves a lot of settings in
special `SUDO_*` environment variables:

```console
sh@meteor:~> sudo bash
meteor:/work/home/sh # env | grep '^SUDO_'
SUDO_GID=100
SUDO_COMMAND=/usr/bin/bash
SUDO_USER=sh
SUDO_TTY=/dev/pts/5
SUDO_HOME=/work/home/sh
SUDO_UID=1000
```

Dropping the privileges and restoring a shell environment to match the one of
the previous non-privileged user makes it possible to call `xdg-open` with a
URL which will then be opened in the user's standard browser.

This does not work without dropping the privileges and restoring that
environment. `sudo` lets us do this; `su`, `kdesu`, `gnomesu` etc. don't
preserve that environment.

So, if you don't see many useful items in Myrlyn's "Help" menu, either start it
as a normal unprivileged user, or use `sudo` / `myrlyn-sudo`.


### kdesu / gnomesu

`kdesu` and `gnomesu` are the classic ways on KDE Plasma (`kdesu`) and GNOME
(`gnomesu`) to start graphical applications with _root_ privileges. They both
use the `su` command internally, and you need to enter the _root_ password.

They both keep the connection to the X11 or Wayland display alive, and they
should also both make the started application use the desktop settings of the
normal user like widget theme (light or dark), fonts, GUI scaling.

But as mentioned above, `su` does not keep those environment variables alive
that would allow Myrlyn to drop privileges for an `xdg-open` call, so you will
find the "Help" menu quite empty.

But of course you can always add a bookmark to your web browser for Myrlyn's
Help Center:

```
https://github.com/shundhammer/myrlyn/blob/master/help/help-center.md
```


## How to Configure my Normal User Account to Use sudo

The `sudo` package is installed by default on a standard openSUSE installation.

Starting with Leap 16.0, the first user created during the installation is
added by default to the _wheel_ user group which gives that user elevated
privileges.

But if you migrated from an older openSUSE version, this might not have
happened, so you may need to do it manually.


#### Needed Packages

- `sudo`

- `sudo-policy-wheel-auth-self`
  for using your own password, not the _root_ password (see below)


#### Adding my User Account to the Wheel Group

```console
sh@meteor:~> groups
```
If _wheel_ appears here, you are already a member of that group.
If not:

```console
sh@meteor:~> sudo usermod -aG wheel $USER
```

Important: You will need to enter the _root_ password here onle last time
(until you are a member of the _wheel_ group).

Also notice `-aG wheel` to _add_ the _wheel_ group.

Only `-G wheel` would make the _wheel_ group your only (!) group, and you would
lose access to your other groups like _video_, _cdrom_, _audio_ and more.


#### Root Password or my Normal User Account's Password?

`sudo-policy-wheel-auth-self` This is the package with the `sudo` rule that
members of the _wheel_ group need to enter their own password.

If you don't like that, and you would prefer to always enter the _root_
password for `sudo`, don't install / remove this package.


## Further Reading

- [↗ Issue #99: Myrlyn Root Authentication](https://github.com/shundhammer/myrlyn/issues/99)


-------------------------

_Next: [↗ Customization](customization.md)_
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
_Previous: (none)_

