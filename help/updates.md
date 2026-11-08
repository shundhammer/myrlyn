# Myrlyn Help: Updates

_Author: Stefan Hundhammer (GitHub: @shundhammer)_

_Up: [↗ Help Center](help-center.md)_

 <img width="1000" src="https://raw.githubusercontent.com/shundhammer/myrlyn/refs/heads/master/screenshots/Myrlyn-11-updates-view.png">

-------------------------


This is about Myrlyn's "Updates" view. If you don't see it, use the "View"
button at the left of the open tabs, then "Updates".

Note that the tab for this view displays a number; in this example "Updates
(8)". This is the number of updates in the list.


## What's in the Package List?

In this view, the package list contains only installed packages for which a
newer version is available, i.e., an _update candidate_. You can also tell this
from the blue version number.

Note that this does not guarantee that updating to that version will be
possible without dependency problems; this is not known yet at this time, only
that there is a newer version available.


## The Difference Between Up and Dup

The updates view offers two update operations:

- "Package Update". This is the same as the `zypper up` command.

- "Dist-Upgrade". This is the same as the `zypper dup` command.

Both operations are handled by the libzypp _dependency resolver_, in Myrlyn as
in `zypper`. Both Myrlyn and `zypper` run the exact same libzypp code.


### Up

For the "up" command, the dependency resolver goes through the list of packages
that have an update candidate, tries to set the package status to "update"
(just as you would interactively), and resolves the dependencies. If there are
no dependency problems, it keeps the "update" status. If there are any
dependency problems, it restores the status to "keep installed".


### Dup

The "dup" command does the same, but in addition to that, the dependency
resolver also takes package splits, merges, renames and changes to software
_patterns_ into account:

For example, for the first release of Myrlyn the distribution's project
managers decided that it should be installed by default, so they added a
dependency `Recommends: myrlyn` to the `base-x11_enhanced` pattern. This
pattern is in turn required by graphical desktops.

As a result, after a "dup", Myrlyn was automatically installed on all systems
with a graphical desktop. With only an "up", this would not have happened.


### Point Releases vs. Rolling Releases

#### Point Releases

Traditional Linux releases like openSUSE Leap are _point releases_: Each
release has a specific release day, and after a certain number of months, there
is a new release with a new release number:
Leap 15.6 -> Leap 16.0 -> Leap 16.1.

They receive package updates for bug fixes and security fixes, but typically no
new package versions with new features. These updates may be bundled into
_patches_ that address specific bugs or security problems. A patch may contain
one or more packages related to the particular issue.

A point release will certainly not receive a completely new package like Myrlyn
in the example above.

On the flip side, no package will be dropped from such a point release; you can
rely on packages remaining available throughout its lifetime.


#### Rolling Releases

A _rolling release_ like openSUSE Tumbleweed or Slowroll, on the other hand, is
all about a steady flow of new package versions as they become available from
_upstream_ (i.e., from the original authors of that package somewhere on the
Internet). This includes not only bug and security fixes, but new features and
entirely new packages.

From time to time, a package may also be dropped, i.e., removed from the
distribution. This may happen if its upstream authors abandon it for too long,
if it no longer receives security fixes or fixes for other serious bugs, or if
it can no longer be built because it depends on libraries or other software
that have become obsolete and are no longer available.

This doesn't happen too often, but it does happen.

With a rolling release, you are always at the _bleeding edge_ of Linux
technology, but you also need to keep your system up to date, and from time to
time, you may need to fix problems that the constant changes bring with them.

Fortunately, openSUSE Tumbleweed is a very stable rolling release. And
Slowroll, being some weeks delayed compared with Tumbleweed, is even more
stable.


### Update Rules

- For a point release like openSUSE Leap, use "Package Update" ("up").

- For a rolling release like openSUSE Tumbleweed or Slowroll, use
  "Dist-Upgrade" ("dup").

  If this is not possible because of too many dependency problems, it's okay to
  use "up" occasionally. But don't make a habit out of it: You will miss out on
  the benefits of a rolling release.


### Patches vs. Updates

As mentioned above, point releases like openSUSE Leap receive _patches_. You
can use the "Patches" view (available only for distributions that actually have
patches) to install them.

But patches are also just RPM pseudo packages (with a nondescript number
instead of a meaningful name) consisting only of dependencies like `Requires:
libfoo >= 47.11`; a patch is _satisfied_ if all its dependencies are met.

So if you simply always install all new package versions from the "Updates"
view with "Package Update" ("up"), the patches are also satisfied; the result
is the same.

Technically, it would be wrong to use "Dist-Upgrade" ("dup") for a point
release. But since there will simply never be a brand-new package, no package
dropped, split, renamed, or merged, and no pattern changes, the result will be
the same.


## Checkmarks and Sad Faces

Clicking on either the "Package Update" or "Dist-Upgrade" button does several
things:

- The dependency resolver changes its internal state to one of "up" or "dup"
  (see below).

- It iterates over the packages that have an _update candidate_ version.
  For each one of them:
  - It tries to set its status to "update".
  - It checks if that causes dependency problems:
    - If there are no problems, it keeps the "update" status.
    - If there are dependency problems, it reverts that package's status to "keep installed".

- Myrlyn iterates over the list again.
  For each one of them:
  - If the package status is now "update", it adds a checkmark
    in front of the "version" column in the package list.
  - If the package status is still "keep installed", it adds a sad face emoji
    in front of the "version" column.


<img width="70" src="https://raw.githubusercontent.com/shundhammer/myrlyn/master/src/icons/emoji-sick.svg">

**Don't be afraid of the sad faces; it's not an error condition**, they only
tell you "better luck next time".

A package marked with a sad face does have a newer version, but you still can't
update it; something else in the distribution that it needs might still be
missing. It might need a newer version (that is not yet available) of a library
package.

If you are curious what prevents the update, try setting it manually to status
"update". You will get a dependency problem report that describes what is
wrong. Of course you need to _undo_ that attempt to update: Select the "do not
install foo-47.11" or "keep obsolete foo-47.11" solution from the list.


## Dependency Resolver State

Both buttons, "Package Update" and "Dist-Upgrade", put the dependency resolver
into a special state: "up" or "dup". Both modes try to prevent dependency
problems as far as possible; that's exactly what these operations are designed
to do.

But there is a catch: The dependency resolver remains in that state until the
package transaction is executed, i.e., until the updated package versions are
actually installed. When you return from the final "Summary" page, the
dependency resolver returns to its normal state.

This is normally not a problem unless you decide that updating alone isn't
enough and also want to install or remove some packages. If doing so would
cause a dependency problem, you might find yourself "fighting" the dependency
resolver: You try to change a package's status ("install" or "remove"), but it
snaps right back to its previous state.

The solution is simple: After clicking one of the update buttons, execute the
transaction first. You can then go back and make additional changes if
necessary.

The dependency resolver status is displayed at the bottom of Myrlyn's main
window if it's not the normal mode; if you see "Package Update Mode" or
"Dist-Upgrade Mode" there, don't be surprised that it has some real resistance
to you creating problems.

In a sense, it's just like your Mom when you were little. :smiley:


## Third Party Repos and Updates

_Go to topic: [↗ Third Party Repos and Updates](third-party-repos.md)_

- Packages from third-party repos causing dependency problems
- Excluding third-party repos from dup / up


-------------------------

_Next: [↗ Package Search](search.md)_
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
_Previous: [↗ Customization](customization.md)_
