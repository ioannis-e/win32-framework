
// This sample is based on the CommonFileDialogSDKSample that ships with the
// Windows 7 SDK. The original sample can be downloaded from:
// https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/Win7Samples/winui/shell/appplatform/commonfiledialog

#include "stdafx.h"

#define STRICT_TYPED_ITEMIDS
#include <shlobj.h>
#include <objbase.h>      // For COM headers
#include <shobjidl.h>     // for IFileDialogEvents and IFileDialogControlEvents
#include <shlwapi.h>
#include <knownfolders.h> // for KnownFolder APIs/datatypes/function headers
#include <propvarutil.h>  // for PROPVAR-related functions
#include <propkey.h>      // for the Property key APIs/datatypes
#include <propidl.h>      // for the Property System APIs
#include <strsafe.h>      // for StringCchPrintfW
#include <shtypes.h>      // for COMDLG_FILTERSPEC

#include "DialogEventHandler.h"
#include "resource.h"

const COMDLG_FILTERSPEC c_rgSaveTypes[] =
{
    {L"Word Document (*.doc)",       L"*.doc"},
    {L"Web Page (*.htm; *.html)",    L"*.htm;*.html"},
    {L"Text Document (*.txt)",       L"*.txt"},
    {L"All Documents (*.*)",         L"*.*"}
};

// Indices of file types
#define INDEX_WORDDOC 1
#define INDEX_WEBPAGE 2
#define INDEX_TEXTDOC 3

// Controls
#define CONTROL_GROUP           2000
#define CONTROL_RADIOBUTTONLIST 2
#define CONTROL_RADIOBUTTON1    1
#define CONTROL_RADIOBUTTON2    2       // It is OK for this to have the same ID as CONTROL_RADIOBUTTONLIST,
                                        // because it is a child control under CONTROL_RADIOBUTTONLIST

//////////////////////
// Utility Functions

// A helper function that converts UNICODE data to ANSI and writes it to the given file.
// We write in ANSI format to make it easier to open the output file in Notepad.
HRESULT _WriteDataToFile(HANDLE hFile, PCWSTR pszDataIn)
{
    // First figure out our required buffer size.
    DWORD cbData = WideCharToMultiByte(CP_ACP, 0, pszDataIn, -1, NULL, 0, NULL, NULL);
    HRESULT hr = (cbData == 0) ? HRESULT_FROM_WIN32(GetLastError()) : S_OK;
    if (SUCCEEDED(hr))
    {
        // Now allocate a buffer of the required size, and call WideCharToMultiByte again to do the actual conversion.
        char* pszData = new (std::nothrow) CHAR[cbData];
        hr = pszData ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            hr = WideCharToMultiByte(CP_ACP, 0, pszDataIn, -1, pszData, cbData, NULL, NULL)
                ? S_OK
                : HRESULT_FROM_WIN32(GetLastError());
            if (SUCCEEDED(hr))
            {
                DWORD dwBytesWritten = 0;
                hr = WriteFile(hFile, pszData, cbData - 1, &dwBytesWritten, NULL)
                    ? S_OK
                    : HRESULT_FROM_WIN32(GetLastError());
            }
            delete[] pszData;
        }
    }
    return hr;
}

// Helper function to write property/value into a custom file format.
//
// We are inventing a dummy format here:
// [APPDATA]
// xxxxxx
// [ENDAPPDATA]
// [PROPERTY]foo=bar[ENDPROPERTY]
// [PROPERTY]foo2=bar2[ENDPROPERTY]
HRESULT _WritePropertyToCustomFile(PCWSTR pszFileName, PCWSTR pszPropertyName, PCWSTR pszValue)
{
    HANDLE hFile = CreateFileW(pszFileName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS, // We will write only if the file exists.
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    HRESULT hr = (hFile == INVALID_HANDLE_VALUE) ? HRESULT_FROM_WIN32(GetLastError()) : S_OK;
    if (SUCCEEDED(hr))
    {
        const WCHAR           wszPropertyStartTag[] = L"[PROPERTY]";
        const WCHAR           wszPropertyEndTag[] = L"[ENDPROPERTY]\r\n";
        const DWORD           cchPropertyStartTag = (DWORD)wcslen(wszPropertyStartTag);
        const static DWORD    cchPropertyEndTag = (DWORD)wcslen(wszPropertyEndTag);
        DWORD const cchPropertyLine = cchPropertyStartTag +
            cchPropertyEndTag +
            (DWORD)wcslen(pszPropertyName) +
            (DWORD)wcslen(pszValue) +
            2; // 1 for '=' + 1 for NULL terminator.
        PWSTR pszPropertyLine = new (std::nothrow) WCHAR[cchPropertyLine];
        hr = pszPropertyLine ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            hr = StringCchPrintfW(pszPropertyLine,
                cchPropertyLine,
                L"%s%s=%s%s",
                wszPropertyStartTag,
                pszPropertyName,
                pszValue,
                wszPropertyEndTag);
            if (SUCCEEDED(hr))
            {
                hr = SetFilePointer(hFile, 0, NULL, FILE_END) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
                if (SUCCEEDED(hr))
                {
                    hr = _WriteDataToFile(hFile, pszPropertyLine);
                }
            }
            delete[] pszPropertyLine;
        }
        CloseHandle(hFile);
    }

    return hr;
}

// Helper function to write dummy content to a custom file format.
//
// We are inventing a dummy format here:
// [APPDATA]
// xxxxxx
// [ENDAPPDATA]
// [PROPERTY]foo=bar[ENDPROPERTY]
// [PROPERTY]foo2=bar2[ENDPROPERTY]
HRESULT _WriteDataToCustomFile(PCWSTR pszFileName)
{
    HANDLE hFile = CreateFileW(pszFileName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,  // Let's always create this file.
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    HRESULT hr = (hFile == INVALID_HANDLE_VALUE) ? HRESULT_FROM_WIN32(GetLastError()) : S_OK;
    if (SUCCEEDED(hr))
    {
        WCHAR wszDummyContent[] = L"[MYAPPDATA]\r\nThis is an example of how to use the IFileSaveDialog interface.\r\n[ENDMYAPPDATA]\r\n";

        hr = _WriteDataToFile(hFile, wszDummyContent);
        CloseHandle(hFile);
    }
    return hr;
}

//////////////////////////////
// Common File Dialog Snippets

HRESULT BasicChooseFolder()
{
    IFileDialog* pfd;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr))
    {
        DWORD dwOptions;
        hr = pfd->GetOptions(&dwOptions);
        if (SUCCEEDED(hr))
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);

        hr = pfd->Show(NULL);
        if (SUCCEEDED(hr))
        {
            IShellItem* psi;
            hr = pfd->GetResult(&psi);
            if (SUCCEEDED(hr))
            {
                PWSTR pszFilePath = 0;
                hr = psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszFilePath);
                if (SUCCEEDED(hr))
                {
                    TaskDialog(NULL,
                        NULL,
                        L"Choosen Folder:",
                        pszFilePath,
                        0,
                        TDCBF_OK_BUTTON,
                        TD_INFORMATION_ICON,
                        NULL);
                    CoTaskMemFree(pszFilePath);
                }
                else
                    MessageBox(NULL, L"GetIDListName() failed", NULL, 0);

                psi->Release();
            }
        }
        pfd->Release();
    }

    return hr;
}

// This code snippet demonstrates how to work with the common file dialog interface
HRESULT BasicChooseFile()
{
    // CoCreate the File Open Dialog object.
    IFileDialog *pfd = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr))
    {
        // Set the options on the dialog. Before setting, always get the options first
        // in order not to override existing options.
        DWORD dwFlags;
        hr = pfd->GetOptions(&dwFlags);
        if (SUCCEEDED(hr))
        {
            // In this case, get shell items only for file system items.
            hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
            if (SUCCEEDED(hr))
            {
                // Set the file types to display only. Notice that, this is a 1-based array.
                hr = pfd->SetFileTypes(ARRAYSIZE(c_rgSaveTypes), c_rgSaveTypes);
                if (SUCCEEDED(hr))
                {
                    // Set the selected file type index to Word Docs for this example.
                    hr = pfd->SetFileTypeIndex(INDEX_WORDDOC);
                    if (SUCCEEDED(hr))
                    {
                        // Set the default extension to be ".doc" file.
                        hr = pfd->SetDefaultExtension(L"doc");
                        if (SUCCEEDED(hr))
                        {
                            // Show the dialog.
                            hr = pfd->Show(NULL);
                            if (SUCCEEDED(hr))
                            {
                                // Obtain the result, once the user clicks the 'Open' button.
                                // The result is an IShellItem object.
                                IShellItem *psiResult;
                                hr = pfd->GetResult(&psiResult);
                                if (SUCCEEDED(hr))
                                {
                                    // We are just going to print out the name of the file for sample sake.
                                    PWSTR pszFilePath = NULL;
                                    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                                    if (SUCCEEDED(hr))
                                    {
                                        TaskDialog(NULL,
                                                   NULL,
                                                   L"Chosen File: ",
                                                   pszFilePath,
                                                   NULL,
                                                   TDCBF_OK_BUTTON,
                                                   TD_INFORMATION_ICON,
                                                   NULL);
                                        CoTaskMemFree(pszFilePath);
                                    }
                                    psiResult->Release();
                                }
                            }
                        }
                    }
                }
            }
        }

        pfd->Release();
    }
    return hr;
}

// The Common Places area in the File Dialog is extensible.
// This code snippet demonstrates how to extend the Common Places area.
// Look at CDialogEventHandler::OnItemSelected to see how messages pertaining to the added
// controls can be processed.
HRESULT AddItemsToCommonPlaces()
{
    // CoCreate the File Open Dialog object.
    IFileDialog *pfd = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr))
    {
        // Always use known folders instead of hard-coding physical file paths.
        // In this case we are using Public Music KnownFolder.
        IKnownFolderManager *pkfm = NULL;
        hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pkfm));
        if (SUCCEEDED(hr))
        {
            // Get the known folder.
            IKnownFolder *pKnownFolder = NULL;
            hr = pkfm->GetFolder(FOLDERID_PublicMusic, &pKnownFolder);
            if (SUCCEEDED(hr))
            {
                // File Dialog APIs need an IShellItem that represents the location.
                IShellItem *psi = NULL;
                hr = pKnownFolder->GetShellItem(0, IID_PPV_ARGS(&psi));
                if (SUCCEEDED(hr))
                {
                    // Add the place to the bottom of default list in Common File Dialog.
                    hr = pfd->AddPlace(psi, FDAP_BOTTOM);
                    if (SUCCEEDED(hr))
                    {
                        // Show the File Dialog.
                        hr = pfd->Show(NULL);
                        if (SUCCEEDED(hr))
                        {
                            //
                            // You can add your own code here to handle the results.
                            //
                        }
                    }
                    psi->Release();
                }
                pKnownFolder->Release();
            }
            pkfm->Release();
        }
        pfd->Release();
    }
    return hr;
}

// This code snippet demonstrates how to add custom controls in the Common File Dialog.
HRESULT AddCustomControls()
{
    // CoCreate the File Open Dialog object.
    IFileDialog *pfd = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr))
    {
        // Create an event handling object, and hook it up to the dialog.
        DWORD               dwCookie    = 0;
        CDialogEventHandler dfe;
        
        // Hook up the event handler.
        hr = pfd->Advise(&dfe, &dwCookie);
        if (SUCCEEDED(hr))
        {
            // Set up a Customization.
            IFileDialogCustomize *pfdc = NULL;
            hr = pfd->QueryInterface(IID_PPV_ARGS(&pfdc));
            if (SUCCEEDED(hr))
            {
                // Create a Visual Group.
                hr = pfdc->StartVisualGroup(CONTROL_GROUP, L"Sample Group");
                if (SUCCEEDED(hr))
                {
                    // Add a radio-button list.
                    hr = pfdc->AddRadioButtonList(CONTROL_RADIOBUTTONLIST);
                    if (SUCCEEDED(hr))
                    {
                        // Set the state of the added radio-button list.
                        hr = pfdc->SetControlState(CONTROL_RADIOBUTTONLIST, CDCS_VISIBLE | CDCS_ENABLED);
                        if (SUCCEEDED(hr))
                        {
                            // Add individual buttons to the radio-button list.
                            hr = pfdc->AddControlItem(CONTROL_RADIOBUTTONLIST,
                                                      CONTROL_RADIOBUTTON1,
                                                      L"Change Title to Longhorn");
                            if (SUCCEEDED(hr))
                            {
                                hr = pfdc->AddControlItem(CONTROL_RADIOBUTTONLIST,
                                                          CONTROL_RADIOBUTTON2,
                                                          L"Change Title to Vista");
                                if (SUCCEEDED(hr))
                                {
                                    // Set the default selection to option 1.
                                    hr = pfdc->SetSelectedControlItem(CONTROL_RADIOBUTTONLIST,
                                                                      CONTROL_RADIOBUTTON1);
                                }
                            }
                        }
                    }
                    // End the visual group.
                    pfdc->EndVisualGroup();
                }
                pfdc->Release();
            }

            if (FAILED(hr))
            {
                // Unadvise here in case we encounter failures before we get a chance to show the dialog.
                pfd->Unadvise(dwCookie);
            }
        }

        if (SUCCEEDED(hr))
        {
            // Now show the dialog.
            hr = pfd->Show(NULL);
            if (SUCCEEDED(hr))
            {
                //
                // You can add your own code here to handle the results.
                //
            }
            // Unhook the event handler.
            pfd->Unadvise(dwCookie);
        }
        pfd->Release();
    }
    return hr;
}

// This code snippet demonstrates how to add default metadata in the Common File Dialog.
// Look at CDialogEventHandler::OnTypeChange to see to change the order/list of properties
// displayed in the Common File Dialog.
HRESULT SetDefaultValuesForProperties()
{
    // CoCreate the File Open Dialog object.
    IFileSaveDialog *pfsd = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfsd));
    {
        // Create an event handling object, and hook it up to the dialog.
        CDialogEventHandler fde;
        DWORD               dwCookie    = 0;
        
        // Hook up the event handler.
        hr = pfsd->Advise(&fde, &dwCookie);
        if (SUCCEEDED(hr))
        {
            // Set the file types to display.
            hr = pfsd->SetFileTypes(ARRAYSIZE(c_rgSaveTypes), c_rgSaveTypes);
            if (SUCCEEDED(hr))
            {
                hr = pfsd->SetFileTypeIndex(INDEX_WORDDOC);
                if (SUCCEEDED(hr))
                {
                    hr = pfsd->SetDefaultExtension(L"doc");
                    if (SUCCEEDED(hr))
                    {
                        // The InMemory Property Store is a Property Store that is
                        // kept in the memory instead of persisted in a file stream.
                        IPropertyStore* pps = NULL;
                        hr = PSCreateMemoryPropertyStore(IID_PPV_ARGS(&pps));
                        if (SUCCEEDED(hr))
                        {
                            PROPVARIANT propvarValue = {};
                            hr = InitPropVariantFromString(L"SampleKeywordsValue", &propvarValue);
                            if (SUCCEEDED(hr))
                            {
                                // Set the value to the property store of the item.
                                hr = pps->SetValue(PKEY_Keywords, propvarValue);
                                if (SUCCEEDED(hr))
                                {
                                    // Commit does the actual writing back to the in memory store.
                                    hr = pps->Commit();
                                    if (SUCCEEDED(hr))
                                    {
                                        // Hand these properties to the File Dialog.
                                        hr = pfsd->SetCollectedProperties(NULL, TRUE);
                                        if (SUCCEEDED(hr))
                                        {
                                            hr = pfsd->SetProperties(pps);
                                        }
                                    }
                                }
                                PropVariantClear(&propvarValue);
                            }
                            pps->Release();
                        }
                    }
                }
            }

            if (FAILED(hr))
            {
                // Unadvise here in case we encounter failures before we get a chance to show the dialog.
                pfsd->Unadvise(dwCookie);
            }
        }

        if (SUCCEEDED(hr))
        {
            // Now show the dialog.
            hr = pfsd->Show(NULL);
            if (SUCCEEDED(hr))
            {
                //
                // You can add your own code here to handle the results.
                //
            }
            // Unhook the event handler.
            pfsd->Unadvise(dwCookie);
        }
        pfsd->Release();
    }
    return hr;
}

// The following code snippet demonstrates two things:
// 1.  How to write properties using property handlers.
// 2.  Replicating properties in the "Save As" scenario where the user choses to save an existing file
//     with a different name.  We need to make sure we replicate not just the data,
//     but also the properties of the original file.
HRESULT WritePropertiesUsingHandlers()
{
    // CoCreate the File Open Dialog object.
    IFileSaveDialog *pfsd;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfsd));
    if (SUCCEEDED(hr))
    {
        WCHAR szFullPathToTestFile[MAX_PATH] = {};

        // For this exercise, let's just support only one file type to make things simpler.
        // Also, let's use the jpg format for sample purpose because the Windows ships with
        // property handlers for jpg files.
        const COMDLG_FILTERSPEC rgSaveTypes[] = {{L"Photo Document (*.jpg)", L"*.jpg"}};

        // Set the file types to display.
        hr = pfsd->SetFileTypes(ARRAYSIZE(rgSaveTypes), rgSaveTypes);
        if (SUCCEEDED(hr))
        {
            hr = pfsd->SetFileTypeIndex(0);
            if (SUCCEEDED(hr))
            {
                // Set default file extension.
                hr = pfsd->SetDefaultExtension(L"jpg");
                if (SUCCEEDED(hr))
                {
                    // Ensure the dialog only returns items that can be represented by file system paths.
                    DWORD dwFlags;
                    hr = pfsd->GetOptions(&dwFlags);
                    if (SUCCEEDED(hr))
                    {
                        // Let's first get the current property set of the file we are replicating
                        // and give it to the file dialog object.
                        //
                        // For simplicity sake, let's just get the property set from a pre-existing jpg file (in the Pictures folder).
                        // In the real-world, you would actually add code to get the property set of the file
                        // that is currently open and is being replicated.
                        hr = pfsd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
                        if (SUCCEEDED(hr))
                        {
                            // Attempt to retrieve the path to %PUBLIC%\Pictures\Sample Pictures.
                            // The folder must exist for this to succeed.
                            PWSTR pszPicturesFolderPath;
                            hr = SHGetKnownFolderPath(FOLDERID_SamplePictures, 0, NULL, &pszPicturesFolderPath);
                            if (SUCCEEDED(hr))
                            {
                                hr = PathCombineW(szFullPathToTestFile, pszPicturesFolderPath, L"Flower.jpg") ? S_OK : E_FAIL;
                                if (SUCCEEDED(hr))
                                {
                                    IPropertyStore *pps;
                                    hr = SHGetPropertyStoreFromParsingName(szFullPathToTestFile, NULL, GPS_DEFAULT, IID_PPV_ARGS(&pps));
                                    if (FAILED(hr))
                                    {
                                        // Flower.jpg is probably not in the Pictures folder.
                                        TaskDialog(NULL, NULL, L"CommonFileDialogApp", L"Create Flower.jpg in the Pictures folder and try again.",
                                                   NULL, TDCBF_OK_BUTTON, TD_ERROR_ICON , NULL);
                                    }
                                    else
                                    {
                                        // Call SetProperties on the file dialog object for getting back later.
                                        pfsd->SetCollectedProperties(NULL, TRUE);
                                        pfsd->SetProperties(pps);
                                        pps->Release();
                                    }
                                }
                                CoTaskMemFree(pszPicturesFolderPath);
                            }
                        }
                    }
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = pfsd->Show(NULL);
            if (SUCCEEDED(hr))
            {
                IShellItem *psiResult;
                hr = pfsd->GetResult(&psiResult);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszNewFileName;
                    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszNewFileName);
                    if (SUCCEEDED(hr))
                    {
                        // This is where you add code to write data to your file.
                        // For simplicity, let's just copy a pre-existing dummy jpg file.
                        //
                        // In the real-world, you would actually add code to replicate the data of
                        // file that is currently open.
                        hr = CopyFileW(szFullPathToTestFile, pszNewFileName, FALSE) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
                        if (SUCCEEDED(hr))
                        {
                            // Now apply the properties.
                            //
                            // Get the property store first by calling GetPropertyStore and pass it on to ApplyProperties.
                            // This will make the registered propety handler for the specified file type (jpg)
                            // do all the work of writing the properties for you.
                            //
                            // Property handlers for the specified file type should be registered for this
                            // to work.
                            IPropertyStore *pps;

                            // When we call GetProperties, we get back all the properties that we originally set
                            // (in our call to SetProperties above) plus the ones user modified in the file dialog.
                            hr = pfsd->GetProperties(&pps);
                            if (SUCCEEDED(hr))
                            {
                                // Now apply the properties making use of the registered property handler for the file type.
                                //
                                // hWnd is used as parent for any error dialogs that might popup when writing properties.
                                // Pass NULL for IFileOperationProgressSink as we don't want to register any callback for progress notifications.
                                hr = pfsd->ApplyProperties(psiResult, pps, NULL, NULL);
                                pps->Release();
                            }
                        }
                        CoTaskMemFree(pszNewFileName);
                    }
                    psiResult->Release();
                }
            }
        }
        pfsd->Release();
    }
    return hr;
}

// This code snippet demonstrates how to write properties without using property handlers.
HRESULT WritePropertiesWithoutUsingHandlers()
{
    // CoCreate the File Open Dialog object.
    IFileSaveDialog *pfsd;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfsd));
    if (SUCCEEDED(hr))
    {
        // For this exercise, let's use a custom file type.
        const COMDLG_FILTERSPEC rgSaveTypes[] = {{L"MyApp Document (*.myApp)", L"*.myApp"}};

        // Set the file types to display.
        hr = pfsd->SetFileTypes(ARRAYSIZE(rgSaveTypes), rgSaveTypes);
        if (SUCCEEDED(hr))
        {
            hr = pfsd->SetFileTypeIndex(0);
            if (SUCCEEDED(hr))
            {
                // Set default file extension.
                hr = pfsd->SetDefaultExtension(L"myApp");
                if (SUCCEEDED(hr))
                {
                    // Ensure the dialog only returns items that can be represented by file system paths.
                    DWORD dwFlags;
                    hr = pfsd->GetOptions(&dwFlags);
                    if (SUCCEEDED(hr))
                    {
                        hr = pfsd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
                        if (SUCCEEDED(hr))
                        {
                            // Set the properties you want the FileSave dialog to collect from the user.
                            IPropertyDescriptionList *pdl;
                            hr = PSGetPropertyDescriptionListFromString(L"prop:System.Keywords", IID_PPV_ARGS(&pdl));
                            if (SUCCEEDED(hr))
                            {
                                // TRUE as second param == show default properties as well, but show Keyword first.
                                hr = pfsd->SetCollectedProperties(pdl, TRUE);
                                pdl->Release();
                            }
                        }
                    }
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            // Now show the dialog.
            hr = pfsd->Show(NULL);
            if (SUCCEEDED(hr))
            {
                IShellItem *psiResult;
                hr = pfsd->GetResult(&psiResult);
                if (SUCCEEDED(hr))
                {
                    // Get the path to the file.
                    PWSTR pszNewFileName;
                    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszNewFileName);
                    if (SUCCEEDED(hr))
                    {
                        // Write data to the file.
                        hr = _WriteDataToCustomFile(pszNewFileName);
                        if (SUCCEEDED(hr))
                        {
                            // Now get the property store and write each individual property to the file.
                            IPropertyStore *pps;
                            hr = pfsd->GetProperties(&pps);
                            if (SUCCEEDED(hr))
                            {
                                DWORD cProps = 0;
                                hr = pps->GetCount(&cProps);

                                // Loop over property set and write each property/value pair to the file.
                                for (DWORD i = 0; i < cProps && SUCCEEDED(hr); i++)
                                {
                                    PROPERTYKEY key;
                                    hr = pps->GetAt(i, &key);
                                    if (SUCCEEDED(hr))
                                    {
                                        PWSTR pszPropertyName;
                                        hr = PSGetNameFromPropertyKey(key, &pszPropertyName);
                                        if (SUCCEEDED(hr))
                                        {
                                            // Get the value of the property.
                                            PROPVARIANT propvarValue;
                                            PropVariantInit(&propvarValue);
                                            hr = pps->GetValue(key, &propvarValue);
                                            if (SUCCEEDED(hr))
                                            {
                                                WCHAR wszValue[MAX_PATH];

                                                // Always use property system APIs to do the conversion for you.
                                                hr = PropVariantToString(propvarValue, wszValue, ARRAYSIZE(wszValue));
                                                if (SUCCEEDED(hr))
                                                {
                                                    // Write the property to the file.
                                                    hr = _WritePropertyToCustomFile(pszNewFileName, pszPropertyName, wszValue);
                                                }
                                            }
                                            PropVariantClear(&propvarValue);
                                            CoTaskMemFree(pszPropertyName);
                                        }
                                    }
                                }
                                pps->Release();
                            }
                        }
                        CoTaskMemFree(pszNewFileName);
                    }
                    psiResult->Release();
                }
            }
        }
        pfsd->Release();
    }
    return hr;
}

