@echo OFF

set PATCH_VERSION_MAJOR=0
set PATCH_VERSION_MINOR=94
set PATCH_VERSION_DEFINES=/DPATCH_VERSION_MAJOR=%PATCH_VERSION_MAJOR% /DPATCH_VERSION_MINOR=%PATCH_VERSION_MINOR%

if "%1"=="debug" (
	set COMPILER_FLAGS=/LDd /DDBGSTR
	set LINKER_FLAGS=/DEBUG
) else (
	set COMPILER_FLAGS=/Ox /LD
	set LINKER_FLAGS=
)

if not exist out\ ( mkdir out )

rc /DPATCHVER=%PATCH_VERSION_MAJOR%,%PATCH_VERSION_MINOR%,0,0 /Foout/resource.res resource.rc
cl /c /Foout/ /EHsc %PATCH_VERSION_DEFINES% %COMPILER_FLAGS% *.cpp
link /DLL /DEF:d3d8.def /OUT:out/d3d8.dll /MAP %LINKER_FLAGS% out/*.obj out/resource.res user32.lib version.lib ole32.lib strmiids.lib