$t = Get-Date -Format "yyyyMMdd_HHmm"
$dest = "${PSScriptRoot}/Builds/build_$t"

New-Item -ItemType Directory -Force -Path $dest
New-Item -ItemType Directory -Force -Path "$dest/Assets"

if (Test-Path "${PSScriptRoot}/Assets") {
    Copy-Item -Path "${PSScriptRoot}/Assets/*" -Destination "$dest/Assets" -Recurse -Force
}

$libs_string = C:\msys64\ucrt64\bin\pkg-config.exe --static --libs sdl2 SDL2_image SDL2_ttf

$libs_array = $libs_string.Split(' ', [System.StringSplitOptions]::RemoveEmptyEntries)

g++ *.cpp Engine/*.cpp -o "$dest/main.exe" `
    -mconsole `
    -std=gnu++20 `
    -I"C:/msys64/ucrt64/include/SDL2" `
    -L"C:/msys64/ucrt64/lib" `
    -static `
    -lSDL_FontCache `
    $libs_array `
    -lstdc++ -lrpcrt4 `
    "-Wl,--dynamicbase,--nxcompat,--high-entropy-va"

Write-Host "Build finished: $dest"
