# Myrlyn GUI Scaling

_Up: [↗ Customization](customization.md)_


-------------------------

If you are using a hiDPI (4k) display, or even just a laptop with a full-HD
(1920x1080, 1920x1200) display with a size of 14", 13" or smaller, only
selecting larger fonts may come to its limits: You might want to scale up the
entire GUI (graphical user interface), including icons and window subelements
like borders and margins.

Myrlyn uses the Qt6 libraries that have good support for this, automatically as
well as with environment variables that you can set it in your shell startup
files like `~/.bashrc`.

By default, Qt automatically scales up the whole application when a certain DPI
(dots per inch) value is reached, but that may not always be desired: If you
are using a laptop that is just at the edge of Qt's automatic scaling, Myrlyn's
window might begin to lose some fine details like thin vertical or horizontal
lines, and the result may look like pixel mush. In that case, you may want to
set

```
export QT_ENABLE_HIGHDPI_SCALING=0
```

On the flip side, you may actually want some higher scale factor to make
everything larger. In that case, set
```
export QT_SCALE_FACTOR=2
```
(or 3 or more).

Multiples of 0.25 may also work: 1.25, 1.5, 1.75, 2.0, 2.25, ...

Other fractional numbers may result in graphical glitches due to rounding
errors. You will have to experiment what looks best for your needs.

Start simple: Open a terminal window and start Myrlyn as a normal user with the
scale factor:

```console
sh@meteor:~> QT_SCALE_FACTOR=1.25 myrlyn
```

Experiment until you found the right scale factor. Once you found it, make it
permanent:

If you use `myrlyn-sudo` (which is recommended), add the scale factor to your
normal user's `~/.config/openSUSE/myrlyn-sudo.conf`
(see [↗ Customization](customization.md)).

If you use something else (`kdesu` / `gnomesu`), add it to `/root/.bashrc`.


## Debugging

Relevant environment variables and some other settings are written to the first
few lines of Myrlyn's log file:

```console
sh@meteor:~> sudo head -n 22 /tmp/myrlyn-root/myrlyn.log

2026-11-03 13:58:56.258 [378349] <Info>    Logger.cc:124 openLogFile():  -- Log Start --

...  main.cc:172 logVersion():  myrlyn-1.1.0 built with Qt 6.11.2
     ...
     ...
...  MyrlynApp.cc:134 logQtEnv():  QT_ENABLE_HIGHDPI_SCALING=0
...  MyrlynApp.cc:134 logQtEnv():  QT_QPA_PLATFORMTHEME=qt6ct
...  MyrlynApp.cc:134 logQtEnv():  QT_SCALE_FACTOR=1
     ...
     ...
...  MainWindow.cc:41 MainWindow():  Device pixel ratio: 1
...  MainWindow.cc:42 MainWindow():  Logical  dpi: 128
...  MainWindow.cc:43 MainWindow():  Physical dpi: 161.806
...  MainWindow.cc:44 MainWindow():  Default font: Droid Sans,10,-1,5,400,0,0,0,0,0,0,0,0,0,0,1,Regular,0,0
...  MainWindow.cc:45 MainWindow():  Heading font: Droid Sans,11,-1,5,700,0,0,0,0,0,0,0,0,0,0,1,Regular,0,0
```



## Further Reading

- [↗ Qt6 Reference Documentation: HighDPI](https://doc.qt.io/qt-6/highdpi.html)
- [↗ Myrlyn issue #79: Qt 6 HiDPI Automatic Scales Everything to Pixel Mush](https://github.com/shundhammer/myrlyn/issues/79)



-------------------------

_Next: [↗ Dark theme](dark-theme.md)_
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
_Previous: [↗ Font size](font-size.md)_
