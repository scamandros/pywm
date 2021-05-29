# pywm - backend for [newm](https://github.com/jbuchermn/newm)

TODO

## Status

See [TESTS.org] for known issues and software with which pywm and newm have been tested.

## Installing

### Prerequisites

Prerequisites for PyWM, apart from Python, are given by [wlroots](https://github.com/swaywm/wlroots):

* python and pip
* gcc, meson and ninja
* pkg-config
* wayland
* wayland-protocols
* xorg-xwayland
* EGL
* GLESv2
* libdrm
* GBM
* libinput
* xkbcommon
* udev
* pixman

### Install

Compilation is handled by meson and started automatically via pip:

```
pip3 install git+https://github.com/jbuchermn/pywm@v0.1
```

In case of issues, clone the repo and execute `meson build && ninja -C build` in order to debug.


### Getting touchpads up and running

Ensure that your user is in the correct group

```
ls -al /dev/input/event*
```

As a sidenote, this is not necessary for a Wayland compositor in general as the devices can be accessed through `systemd-logind` or `seatd` or similar.
However the python `evdev` module does not allow instantiation given a file descriptor (only a path which it then opens itself),
so usage of that module would no longer be possible in this case (plus at first side there is no easy way of getting that file descriptor to the 
Python side). Also `wlroots` (`libinput` in the backend) does not expose touchpads as what they are (`touch-down`, `touch-up`, `touch-motion` for any
number of parallel slots), but only as pointers (`motion` / `axis`), so gesture detection around `libinput`-events is not possible as well.

Therefore, we're stuck with the less secure (and a lot easier) way of using the (probably named `input`) group.

## Notes

- Depending on the settings (see main.py) XWayland apps are responsible for handling HiDPI themselves and will per default appear very small
    - GDK on XWayland: GDK_DPI_SCALE=2
    - Electron apps: --force-device-scale-factor=2

- Screen record: wf-recorder
- Screen shot: grim
- Firefox: MOZ_ENABLE_WAYLAND=1
- Chromium: --enable-features=UseOzonePlatform --ozone-platform=wayland
- Matplotlib / Qt5 on Wayland requires DISPLAY=":0" to be set
- Apple Trackpad
    https://medium.com/macoclock/how-to-pair-apple-magic-keyboard-a1314-on-ubuntu-18-04-and-act-as-numpad-42fe4402454c
    https://wiki.archlinux.org/index.php/Bluetooth
