@set PATCHVER=0,91,0,0
@rc /dPATCHVER=%PATCHVER% resource.rc
@cl %* /c /DPATCHVER=%PATCHVER% /LD *.c
@link /DLL /DEF:d3d8.def /OUT:d3d8.dll *.obj resource.res user32.lib version.lib