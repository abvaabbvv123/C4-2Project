@echo off
set UE="E:\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"
set PROJ="%~dp0CX.uproject"
set LOBBY=/Game/C4Project/Maps/Lobby
set MENU=/Game/C4Project/Maps/MainMenuMap

rem 서버 2대 (방 2개)
start "Server7777" %UE% %PROJ% %LOBBY% -server -log -port=7777
start "Server7778" %UE% %PROJ% %LOBBY% -server -log -port=7778
start "Server7779" %UE% %PROJ% %LOBBY% -server -log -port=7779

rem 서버가 맵을 다 로드할 때까지 대기
timeout /t 20

rem 클라이언트 2개
start "ClientA" %UE% %PROJ% %MENU% -game -windowed -ResX=1280 -ResY=720 -WinX=0 -WinY=40 -log
start "ClientB" %UE% %PROJ% %MENU% -game -windowed -ResX=1280 -ResY=720 -WinX=1290 -WinY=40 -log