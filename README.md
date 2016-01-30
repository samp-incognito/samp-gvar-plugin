SA-MP GVar Plugin
================

This plugin is an extension to the per-player variable (PVar) system introduced in SA-MP 0.3a with a few key differences:

- Global variables (GVars) that are not linked to any player IDs
- Presence in memory until explicit deletion with DeleteGVar
- Much faster execution time than properties (see SA-MP forum thread for benchmarks)

Compilation (Windows)
---------------------

Open the solution file (gvar.sln) in Microsoft Visual Studio 2010 or higher. Build the project.

Compilation (Linux)
-------------------

Install the GNU Compiler Collection and GNU Make. Type "make" in the top directory to compile the source code.


Download
--------

The latest binaries for Windows and Linux can be found [here](https://github.com/samp-incognito/samp-gvar-plugin/releases).

Documentation
-------------

More information can be found in the [SA-MP forum thread](http://forum.sa-mp.com/showthread.php?t=151076) as well as the README file in the binary package.
