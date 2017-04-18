TinyAntivirus
==============

[![Build status](https://ci.appveyor.com/api/projects/status/github/develbranch/TinyAntivirus?branch=master&svg=true)](https://ci.appveyor.com/project/quangnh89/TinyAntivirus/branch/master)
[![License](https://img.shields.io/badge/license-gpl2-blue.svg)](LICENSE)
![Platform](https://img.shields.io/badge/platform-windows-lightgrey.svg)

**TinyAntivirus (TinyAv)** is an open source antivirus engine designed for detecting polymorphic virus and disinfecting it. Now, TinyAv can detect and disinfect Sality polymorphic virus. In the future, I will deveplop some modules for removing other polymorphic viruses, such as Virut or Polip.

## License

This project is released under the [GPL2](COPYING) [license](LICENSE).

## Requirements

* Microsoft Visual Studio 2015
* [zlib 1.2.8](http://www.zlib.net) or newer
* [unicorn-engine 0.9](http://www.unicorn-engine.org/)

## Quick start

* Clone the repository: `git clone https://github.com/develbranch/TinyAntivirus.git`.
* Build: Core engine, Console and scan module.
* You can see `Release` Directory. Change the `Release` directory and run `TinyAvConsole.exe`.

## Usage

```
TinyAvConsole.exe [options]

```
| Option   |      Meaning      |  Default value |
|----------|-------------|:------:|
| -e | plug-in directory | current directory |
| -A | Archive scan depth | -1 : any depth|
| -D | scan depth | -1 : any depth |
| -d | path to scan |  |
| -p | file pattern | \*.\* |
| -s | max file size in bytes| 10 \* 1024 \* 1024 (10 MB) |
| -m | Scan mode: Kill-virus (k) or Scan-only(s) | Kill-virus (k) |
| -h | Show usage ||

You may scan all directories and files by using default values.

**Example:** Scan for all files (include ZIP files) to detect and disinfect virus.
ZIP files which contain virus will be deleted.
```
C:\build>TinyAvConsole.exe -d C:\sample
------------------------------------------------------
TinyAntivirus version 0.1
Copyright (C) 2016, Quang Nguyen. All rights reserved.
Website: http://develbranch.com
------------------------------------------------------
Scanning ...
C:\sample\calc.EXE
        W32.Sality.PE Disinfected
C:\sample\container.zip                                                 OK
C:\sample\container.zip>DiskView.exe                                    OK
C:\sample\container.zip>DMON.SYS                                        OK
C:\sample\container.zip>sub_container.zip                               OK
C:\sample\container.zip>sub_container.zip>NOTEPAD.EXE
        W32.Sality.PE Deleted
C:\sample\dbgview.chm                                                   OK
C:\sample\sub\gmer.EXE
        W32.Sality.PE Disinfected

=============================================
Scanned       : 4 file(s) (10 object(s))
Detected      : 3 file(s)
Removed       : 3 file(s)
Access denied : 0 file(s)

C:\build>
```

## Contribute

If you want to contribute, please pick up something from our [Github issues](https://github.com/develbranch/TinyAntivirus/issues).

I also maintain a list of more problems in a [TODO list](https://github.com/develbranch/TinyAntivirus/wiki/TODO).

I have only one Sality sample to develop Sality killer module. I think there are many variant types of this file infector. Please send me samples which TinyAv can not detect or other kinds of polymorphic viruses. Thank you.

## Author

[Quang Nguyễn](https://github.com/quangnh89)

Blog: [develbranch.com](https://develbranch.com)
