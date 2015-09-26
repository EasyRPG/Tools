
# xyz-thumbnailer

Generates thumbnails from RPG Maker 2000/2003 XYZ graphics format for view in
the Windows Explorer.

## Screenshot

![screenshot](assets/screenshot.png)

## Installation

Enter folder `bin\Release\x86` (32bit) or `bin\Release\amd64` (64bit)
and open a elevated command prompt (admin).

    $ regsvr32 EasyRpgXyzShellExtThumbnailHandler.dll
	
Important: Don't delete the file, this will break the shell handler!
	
## Uninstallation

Same steps as above.

    $ regsvr32 /u EasyRpgXyzShellExtThumbnailHandler.dll

## Usage

Enter any directory containing xyz files and choose an explorer view with
huge symbols.
