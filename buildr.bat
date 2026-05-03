@set PATCHVER=0,92,0,0
@rc /dPATCHVER=%PATCHVER% resource.rc
@cl %* /c /Fa /DPATCHVER=%PATCHVER% /LD *.cpp
@link /DLL /DEF:d3d8.def /OUT:d3d8.dll /MAP *.obj resource.res user32.lib version.lib ole32.lib strmiids.lib