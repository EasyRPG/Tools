
# kde-xyz-thumbnailer

Generates thumbnails from RPG Maker 2000/2003 XYZ graphics format for view in
the KDE file manager Dolphin.

## Screenshot

![screenshot](assets/screenshot.png)

## Prequisites

 * `zlib` from zlib.org
 * `KDE Framework 5 (KF5)` from KDE5

## Installation

    $ cmake .
    $ make
    $ sudo make install

## Usage

Open the Dolphin settings, navigate to General -> Previews and enable "RPG
Maker 2k/2k3 XYZ Images". The file manager should create thumbnails now.
