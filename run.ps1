Clear-Host

PSYMAKE.COM
.\mkpsxiso\mkpsxiso.exe -o build\game.iso -y .\cuesheet.xml
#C:\ePSXe\ePSXe.exe -nogui -loadbin .\build\game.iso #MAIN.EXE
#C:\duckstation\duckstation-nogui -slowboot .\build\game.iso