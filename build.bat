@echo off
rem run within source folder src\
rem https://docs.microsoft.com/en-us/cpp/build/reference/output-file-f-options?view=msvc-160

set Mode=release
set OutputFolder=release
set Optimization=/O2
set Subsystem=/SUBSYSTEM:console
rem set Subsystem=/SUBSYSTEM:windows
if !%1!==!debug! (
    set Mode=debug
    set OutputFolder=debug
    set Optimization=/Od
)

set WinLibs=User32.lib Gdi32.lib shell32.lib vcruntime.lib
set ExternalLibs=
set IncludePaths=/I..\include
set CompilationFlags=/Zi %Optimization% /EHa- /Zo
set WarningLevel=/W4
set IWPadding=/wd4820
set IWNamelessUnion=/wd4201
set IWInitializedNotReferenced=/wd4189
set IgnoreWarnings=%IWPadding% %IWInitializedNotReferenced% %IWNamelessUnion%

if not exist %OutputFolder% (
  mkdir %OutputFolder%
)

pushd %OutputFolder%

set dlls=..\main.cpp ..\world.cpp ..\memory.cpp

cl  /nologo %WarningLevel% %IgnoreWarnings% %CompilationFlags% %IncludePaths% ^
    %dlls% ^
    /link /incremental:no /opt:ref  ^
    %WinLibs% %ExternalLibs%

popd
