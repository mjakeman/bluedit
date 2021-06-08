# Bluedit
Bluedit is a word processor written in C and Gtk. I created it
for NCEA Level 3 Digital Technologies AS 91637 and it has been
iterated on and cleaned-up for public release. Enjoy :)

![Screenshot](screenshot.png)

## Project Structure
The source code for the program is in the `/src` subdirectory, while the code
for libsplit is in the `/subprojects/libsplit` directory. The program otherwise
follows the default GNOME application directory layout.

## Features
The main features of the program are:

 - Interactive markdown editor (with six levels of headings and bold/italic styles)
 - Splitting areas into any desirable combination
 - Joining areas by clicking and dragging from the corners
 - Opening and saving files
 - Saving as new files
 - Changing and configuring the font
 
## Running the Program
This program is designed for Linux, and this is the intended environment
for the program. In order to run, please make sure you are running a linux
distribution, or Windows Subsystem for Linux with the required dependencies
(see below). Note there is no guarantee the program will function as intended
on WSL, as this is not the intended use case scenario.

It is possible to build this project for Windows under a MSYS2/MinGW
environment. It works largely as expected, although it hasn't been fully
tested.

## Building
### Flatpak (Recommended)
The easiest way to build the program is to open it with GNOME Builder
and allow it to download the required SDKs for you. Alternatively, you
could build it directly with the flatpak command line tool:

```
# Build the flatpak
$ flatpak-builder flatpak-build com.mattjakeman.bluedit.json --force-clean --disable-cache

# Test the flatpak
$ flatpak-builder --run flatpak-build com.mattjakeman.bluedit.json bluedit

# Package in repo
$ flatpak-builder --repo=repo --force-clean flatpak-build com.mattjakeman.bluedit.json

# Add repo
$ flatpak --user remote-add --no-gpg-verify bluedit-repo repo

# Option 1: Install
$ flatpak --user install bluedit-repo com.mattjakeman.bluedit

# Option 2: Update
$ flatpak update com.mattjakeman.bluedit

# Run
$ flatpak run com.mattjakeman.bluedit

```

### Meson
Building with meson can be done as follows. This requires that the
following dependencies are installed:
 - gtk3
 - cmark (latest release)
 - cmake

```
$ meson build
$ ninja -C build
```
