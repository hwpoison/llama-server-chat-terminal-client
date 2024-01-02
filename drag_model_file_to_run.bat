@echo off
set "model_filename=%1"
..\server.exe -m %model_filename%
pause