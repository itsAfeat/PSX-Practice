Clear-Host

Remove-Item './build' -Recurse

cmake -S . -B ./build -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=c:/psn00bsdk/lib/libpsn00b/cmake/sdk.cmake
cmake --build ./build
.\mkpsxiso\mkpsxiso.exe -o game.iso -y -lba ISO_BUILD.log .\build\cd_image_d00471dcf544c6cf.xml
Move-Item -Path game.iso -Destination .\release\game.iso -force

C:\pcsx-redux\pcsx-redux.exe -run -stdout -iso .\release\game.iso

Write-Host("`nPress ENTER...")
Read-Host

Remove-Item *.mcd
Remove-Item offscreen.*
Remove-Item output.*