
[![Build status](https://ci.appveyor.com/api/projects/status/2ajaq92mwe4urmkl?svg=true)](https://ci.appveyor.com/project/dejbug/docpaths)

# introduction

Very often would I have too many PDFs open and I needed a one-click way to save a log of open documents before I close their viewers. This is going to become a simple GUI (and a C++ library) for just that.

# download

The binaries for tagged builds can be downladed from the `releases` tab. Development builds can be fetched from the [AppVeyor history](https://ci.appveyor.com/project/dejbug/docpaths/history) (from the `Artifacts` tab).

# usage

Type e.g. `docpaths.exe --name foxit --exts pdf` to print the list of PDFs open in FoxitReader. In the above example, `foxit` must be replaced with the (partial) name of your document viewer's executable. In case your viewer supports multiple extensions, you can type e.g. `--exts pdf chm cbz`.

# progress

This is just starting out.

So far the project is usable in form of a exe. A DLL will be added once SumatraPDF support has been figured out as best as possible.

Sumatra is a special case because it doesn't keep file handles open. I've got a solution for single-window instances (i.e. non-tabbed mode) that were opened from the command line, but no general-case method yet.
