@echo off
set "model_filename=%1"
..\llama-server.exe -m %model_filename%
pause