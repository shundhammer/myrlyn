# Myrlyn Help: Installation Summary

_Author: Stefan Hundhammer (GitHub: @shundhammer)_

_Up: [↗ Help Center](help-center.md)_

 <img width="1000" src="https://raw.githubusercontent.com/shundhammer/myrlyn/refs/heads/master/screenshots/Myrlyn-14-inst-summary-view.png">

-------------------------


This is about Myrlyn's "Installation Summary" view. If you don't see it, use
the "View" button at the left of the open tabs, then "Installation Summary".

This view is also available as a secondary filter named "Status" in the
"Repositories" view.


## The General Idea

This view is meant to be used as a summary of pending changes after you've made
more than just very minor changes in other views; for example:

- You marked packages for installation or deletion in another view,
  e.g., in the "Search" or "Repositories" view

- You marked whole patterns for installation or deletion in the "Patterns" view

- You used "Package Update" or "Dist-Upgrade" in the "Updates" view.

All of these actions may automatically mark other packages for installation,
update, or deletion because of dependencies.

If this is the case, when you click the "Accept" button in any view other than
"Installation Summary", you will get a pop-up dialog showing the automatic
changes.

Unlike this pop-up dialog, the "Installation Summary" by default shows _all_
pending changes, not only the automatic ones. You use the checkboxes in the
left panel to select which status values to show.

By default, it will show all packages that will be newly installed, updated or
removed, no matter whether they were selected explicitly by you or marked
automatically because of dependencies. In addition to that, it will show you
all packages in status "Protected" (installed and locked against change) or
"Taboo" (not installed and locked against being installed).

If you have many packages in status "Protected" or "Taboo", consider
deselecting them in the status checkboxes in the left panel so they don't
obscure important changes.


## Managing Huge Updates

A large update with more than, say, 50 packages in the list may be overwhelming
in this view; you might want to be more selective what changes to display.

Of course an update has many packages in status "update"; that is expected. But
does it also cause new packages being installed by dependencies? Or even
packages being removed? Let's find out.


### Check for Auto-Installed Packages

Select only status "Auto-Install". You can uncheck all other status checkboxes
in the left panel, or you can simply click "Auto-Install" with the middle mouse
button to check only this one and uncheck all others.

The remaining packages in the list are those that will be installed by
dependencies; you didn't explicitly ask for them. It probably makes sense to
keep them, but it's your decision.

Click "Install" to also see the packages that you did explicitly ask for.

Click "Defaults" to restore the default settings for the checkboxes.


### Check for Auto-Deleted Packages

Similar to above, select only "Auto-Delete" or middle-click it.

The remaining packages were automatically marked for deletion because of
dependencies.

This typically happens when another package that you requested has a
"Conflicts" or "Obsoletes" dependency on one of these packages.

Remember to use "Defaults" before you leave this view to avoid surprises the
next time you use it.


-------------------------

_Next: [↗ Package Classification View](classification.md)_
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
_Previous: [↗ Patterns](patterns.md)_
