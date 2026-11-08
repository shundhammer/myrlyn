# Myrlyn Font Size

_Author: Stefan Hundhammer (GitHub: @shundhammer)_

_Up: [↗ Customization](customization.md)_

-------------------------


Myrlyn as a non-privileged user should use your desktop environment's font
settings out of the box.

But the normal case is to start Myrlyn with root privileges, and that means
that you have to set the fonts for the root user account.

The easy and recommended way is to log in as as root in the graphical login
manager with the same session type (KDE Plasma, GNOME, Xfce, ...) that you use
as a normal (non-privileged) user, set the font family and size, and then log
out again.

If that does not work for any reason, try running `qt6ct` as root and set the
font family and size there.

Don't forget to add

```
export QT_QPA_PLATFORMTHEME=qt6ct
 ```

 to your (normal user's!)  `~/.config/openSUSE/myrlyn-sudo.conf` (if you use
`myrlyn-sudo) or to `/root/.bashrc`.

For more details about both methods, see [↗ Customization](customization.md).


## Debugging

Check the font settings in the log:

```console
sh@meteor:~> sudo head -n 20 /tmp/myrlyn-root/myrlyn.log

2026-11-03 13:58:56.258 [378349] <Info>    Logger.cc:124 openLogFile():  -- Log Start --

...  main.cc:172 logVersion():  myrlyn-1.1.0 built with Qt 6.11.2
     ...
     ...
...  MyrlynApp.cc:134 logQtEnv():  QT_ENABLE_HIGHDPI_SCALING=0
...  MyrlynApp.cc:134 logQtEnv():  QT_QPA_PLATFORMTHEME=qt6ct
...  MyrlynApp.cc:134 logQtEnv():  QT_SCALE_FACTOR=1
     ...
     ...
...  MyrlynApp.cc:169 createMainWin():  Creating the main window
...  MainWindow.cc:41 MainWindow():  Device pixel ratio: 1
...  MainWindow.cc:42 MainWindow():  Logical  dpi: 128
...  MainWindow.cc:43 MainWindow():  Physical dpi: 161.806
...  MainWindow.cc:44 MainWindow():  Default font: Droid Sans,10,-1,5,400,0,0,0,0,0,0,0,0,0,0,1,Regular,0,0
...  MainWindow.cc:45 MainWindow():  Heading font: Droid Sans,11,-1,5,700,0,0,0,0,0,0,0,0,0,0,1,Regular,0,0
```

Notice that highDPI scaling and `QT_SCALE_FACTOR` may also change the effective font sizes.
If in doubt, try `QT_ENABLE_HIGHDPI_SCALING=0` and `QT_SCALE_FACTOR=1`.


-------------------------

_Next: [↗ GUI Scaling](gui-scaling.md)_
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
_Previous: (None)_
