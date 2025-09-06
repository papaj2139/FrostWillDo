# FrostWillDo
Just a simple to-do app in C++ and QT6

# Building
first run:
```bash
qmake6
```
it will create a Makefile

then run:
```bash
make
```
to build the binary 'frostwilldo'

# Dependencies
requires the Qt 6 development libraries. The specific package name and installation method varies by distribution and package manager

on debian and debian-based:
```bash
sudo apt install qt6-base-dev
```

on arch and arch-based:
```
sudo pacman -S qt6-base
```

on fedora and fedora-based:
```
sudo dnf install qt6-qtbase-devel
```

on void:
```
sudo xbps-install -S qt6-base-devel
```

# License
Licensed under GPLv3
