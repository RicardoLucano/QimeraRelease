@echo off
SETLOCAL

:: The path to the ZIP file you want to extract
:: Receive the path as an argument

:: Check if argument is empty and use default value

SET ZIP_FILE=%1
if "%1"=="" (
    echo No ZIP file path provided. Using default.
    SET ZIP_FILE=".\AssetsBase.zip"
)

:: The directory where you want to extract the files
:: Use temp directory as default
set EXTRACT_TO=%TEMP%\QimeraLibraryAssetsBP

:: Remove the temp directory if it exists
if exist "%EXTRACT_TO%" rmdir /S /Q "%EXTRACT_TO%"

:: Use PowerShell to unzip and move files
powershell -Command "& {Add-Type -AssemblyName System.IO.Compression.FileSystem; [System.IO.Compression.ZipFile]::ExtractToDirectory('%ZIP_FILE%', '%EXTRACT_TO%');}"

:: Copy files to another directory (Replace with the destination if needed)
:: Receive the path as an argument

:: Check if argument is empty and use default value

set DESTINATION_DIR=%2
if "%2"=="" (
    echo No destination directory provided. Using default.
    :: Default is back in savedata folder
    SET DESTINATION_DIR=".\..\..\Saved\SaveGames"
)

:: Check if the destination directory exists and exit if it doesn't
if not exist "%DESTINATION_DIR%" (
    echo Destination directory does not exist. Exiting.
    exit /b
)

:: copy files to the destination directory
xcopy /E /Y "%EXTRACT_TO%" "%DESTINATION_DIR%"

echo Files have been extracted and moved.
ENDLOCAL


:: Example, how to use the script
:: InstallSaveData.bat ".\AssetsBase.zip" "E:\Qimera\Projects\ue\QimeraMainLibraryDev\Plugins\QimeraLibraryAssetsBP"
