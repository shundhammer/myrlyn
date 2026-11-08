# Myrlyn Help: Best Practices

_Author: Stefan Hundhammer (GitHub: @shundhammer)_

_Up: [↗ Help Center](help-center.md)_

-------------------------


## Do Many Small Changes, not One Huge Change

Myrlyn is quick to start, and you can go back from a the final "Summary" page
after a finished package transaction as often as you want.

This was different in YaST times: Back then, it took a while for the package
selection to initialize, and it wasn't always possible to go back for more
after a finished package transaction. That trained users to do one long,
careful package selection (which could take an hour or more) and then send it
off.

Try to get rid of that habit with Myrlyn. Don't try to do the "golden" package
selection, the one to rule them all: Something might go wrong, and then you'd
lose all that work.

Rather, select some packages, check on the "Installation Summary" page if
that's really what you want, hit "Accept", watch the transaction (which can be
a deeply satisfying experience), and when you are on the final "Summary" page,
hit "Back" for more.

Of course this can be different when you are doing something critical such as
an update of the NVidia drivers; check carefully if the versions match (use the
"Versions" tab) and accept the changes when you are very, very sure that you
won't be greeted by a black screen after the next reboot.

But in general, work in short sprints, not in a long marathon.


### Updates

This is also true for updates: If you fear that your codecs from a third-party
repo like Packman or VideoLAN might clash with those from the openSUSE OSS
repo, do the "dup" in several steps: Use the "Protect All" button for the
Packman or VideoLAN repo in the "Repositories" view, go to the "Updates" view,
hit "Dist-Upgrade" (or "Package Update") there to do the "dup" / "up" only for
the _other_ (non-Packman, non-VideoLAN) packages, hit "Accept".

Then come back after the transaction, un-protect the protected Packman or
VideoLAN repo again, go to the "Updates" view and try to "dup" / "up" those
remaining packages from Packman / VideoLAN.

If it works (checkmarks everywhere) - great. If you see a lot of sad faces,
nothing is lost. If it causes dozens of dependency problems, just cancel the
dependency problems window and then cancel Myrlyn entirely, abandoning your
changes. Simply restart Myrlyn - or just wait for another day or two.



## Be Prepared to Start Over

Like descried above, if you get caught in a maze of dependency problems, don't
dwell on it - just cancel and abandon your changes and restart Myrlyn. This is
often much easier than trying to solve all those dependency problems manually.

Myrlyn is quick to start. Use it to your advantage.

This is one more reason not to spend an hour or more on a carefully
hand-crafted package selection: It would hurt to lose all those changes. So try
to work in a way where that simply doesn't matter: Don't accumulate so many
changes in the first place. Work in short sprints, not in a long marathon.



## Handling Dependency Problems

Dependency problems can get out of hand quickly.

If it's only one package that conflicts with one other package, it's easy:
`foo-branding-upstream` conflicting with `foo-branding-openSUSE` is an easy
decision; select one or the other. It doesn't matter much.

But an attempt to remove a library package `libfoo` which is required by a
dozen Qt libraries that are in turn required by 108 KDE Plasma packages is
something different: You should really select the "keep package libfoo"
solution.

Similar for updates: If `vlc-47.11` just became available, but that would
require `libQtCore-58.22` which isn't available yet for your distro version, go
the conservative way: Select the "keep obsolete vlc-58.22" solution. "Obsolete"
is a relative thing; better last week's version than a broken version because
it can't load the shared libs that it needs.

During a "dup" there might be a dozen such problems if third-party repos like
Packman or VideoLAN are involved. It can be simple to go through the list of
dependency problems and always select the "Keep obsolete ..." solution for each
one. But if it's too many, consider hitting "Cancel" to get out of the
dependency problems window (the problems don't just go away!), then "Cancel"
again in the main window, abandon your changes and restart; this time maybe
without that problematic repo: Use "Protect all" for that one.

Avoid to get tangled up in dependency hell. You might have tried something that
turned out too ambitious; maybe try it in smaller steps.

And again, be prepared to start over.


-------------------------

_Next: [↗ Misc](misc.md)_
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
_Previous: [↗ Repositories View](repos-view.md)_
