IFileDialog sample
==================
This sample illustrates the use of the different Common File Dialog APIs
(IFileOpenDialog, IFileSaveDialog, and associated interfaces) to create a
custom file open/save dialog.

This sample is based on the CommonFileDialogSDKSample that ships with the
Windows 7 SDK. The original sample can be downloaded from:
https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/Win7Samples/winui/shell/appplatform/commonfiledialog

This sample extends the CommonFileDialogSDKSample code by adding the ability
to choose a folder. Using IFileOpenDialog to choose a folder provides a modern
alternative to SHBrowseForFolder. IFileOpenDialog supports dark mode, whereas
SHBrowseForFolderdoes not.

This program displays a taskdialog that provides the following options:
* Basic Choose Folder
* Basic Choose File
* Add Items to Common Places
* Add Custom Controls
* Change Property Order
* Write Properties Using Handlers
* Write Properties without Using Handlers

These options demonstrate the various ways in which to customize the appearance
and behavior of IFileOpenDialog and IFileSaveDialog. The IFileDialogControlEvents
interface is used to exposes methods that allow the application to be notified
of events that are related to controls that the application has added to the 
file diaogs.

The first two options demonstrate a comparatively simple use of IFileOpenDialog
and IFileSaveDialog for choosing folders or files. The methods demonstrated
there would be sufficient for most purposes.

To use the "Write Properties Using Handlers" option, place a jpg image called
Flower.jpg in the "%PUBLIC%\Pictures\Sample Pictures" folder. This folder is
usually "C:\Users\Public\Pictures\Sample Pictures".

This code uses the ComPtr smart pointer from the Windows SDK version 8.1 to 
manage the COM interface pointers. The Windows SDK version 8.1 ships as part
of VS2019, VS2022 and the later versions of MinGW-64. 
