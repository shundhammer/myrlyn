# Myrlyn Help: Third Party Repos and Updates

_Author: Stefan Hundhammer (GitHub: @shundhammer)_

_Up: [↗ Updates](updates.md)_

 <img width="1000" src="https://raw.githubusercontent.com/shundhammer/myrlyn/refs/heads/master/screenshots/Myrlyn-12-repos-view.png">

-------------------------


## Third Party Repos

Some third-party repos, such as Packman or VideoLAN, provide packages that are
also present in the openSUSE OSS or update repos, but with more capabilities or
supporting hardware acceleration for audio, video and similar algorithms.

The main reason for using these third-party repositories is that they provide
codecs or patent-encumbered software that SUSE's corporate lawyers don't want
the company to distribute because of the risk of lawsuits.


## Packages from Third-Party Repos Causing Dependency Problems

Packages in those repos tend to have slightly different versions, and sometimes
they also require library packages in different versions than the openSUSE
repos, and those third-party repos usually have their own update cycles that
don't quite match the openSUSE update cycle.

As long as both the third-party repos and the openSUSE repos are in sync,
everything is fine. But if either one lags behind with its versions, there
will be dependency problems.

These problems are often easy enough to resolve, but still annoying: If you get
a dozen dependency problems that all have a very similar solution, such as
"Keep obsolete xy, keep obsolete yz, ...", simply select that one for
each of the problems.

But there is a more convenient approach: Set all packages from that repo to
the "protected" status to exclude them from "dup" / "up".


## Excluding Third-Party Repos From Updates

If a number of packages from a third-party repo have an updates available
(displayed in the package list as blue version numbers), and there are also
updates from the official openSUSE repos, exclude the third-party repo from the
update, update only the openSUSE packages, and then update the third-party
packages separately:

- Switch to the "Repositories" view.

- Select the third-party repo to fill the package list with its packages.

- Click the "Protect All" button:
  It will set all installed packages that are in that repo to "protected",
  i.e., they can't be modified (no version updates).

  Note that this doesn't check whether the installed version actually comes
  from that repo. It only checks whether a version of the package is
  available in that repo..

- If there are more third-party repos that also contaian packages available in
  the openSUSE repos, and they also have updates, do the same with them.

- Go to the "Updates" view. You will notice that there are now fewer updates in
  the list.

- Click one of the update buttons.

- (Optional) Go to the "Installation Summary" to check what will be done.
  You may want to uncheck the "protected" status in the left panel to exclude
  protected packages from the package list.

- Click the "Accept" button to start the transaction.

- After the transaction, go back, select the "Repositories" view again, and
  decide whether you now want to update the third-party repos separately.

- If yes, select the third-party repo and click "Un-Protect All".

- Go to the "Updates" view and do the update for those third-party packages.

- Lots of sad faces? Nevermind; try again in a few days.
  The situation will resolve itself eventually.

- Dependency problems? Either select the conservative solution ("keep obsolete
  xy", "don't install xy") or click "Cancel" in the dependency problem dialog,
  then "Cancel" again in the main window and abandon your changes.

  Note that this will also undo the "Un-Protect All", so the third-party
  packages remain protected.


-------------------------

_Up: [↗ Updates](updates.md)_
