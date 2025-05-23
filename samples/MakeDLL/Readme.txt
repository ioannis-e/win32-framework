DLL Example
===========
This project demonstrates how to run Win32++ from inside a Dynamically Linked
Library (DLL).
The MakeDLL project creates the DLL which exports one function called ShowDialog.
The TestDLL project creates a program which uses the DLL to display the dialog.

Each time the DLL's ShowDialog function is called, the DLL creates a new thread.
A new modeless dialog is created when the new thread starts. Each new thread 
runs its own message loop. The thread stops running when the dialog is closed.

Features demonstrated in this example
=====================================
* Running Win32++ from inside a DLL.
* Running multiple threads in a DLL.
* Exporting functions from a DLL.
* Creating pointers to functions exported by a DLL.
* Using a DLL to display a modeless dialog created by Win32++.
* Using TRACE within a DLL to send text to the Output window of the IDE.