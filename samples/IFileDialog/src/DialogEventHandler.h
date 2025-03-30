/////////////////////////////
// File Dialog Event Handler

// This sample is based on the CommonFileDialogSDKSample that ships with the
// Windows 7 SDK. The original sample can be downloaded from:
// https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/Win7Samples/winui/shell/appplatform/commonfiledialog

#ifndef _FILE_DIALOG_EVENT_HANDLER_
#define _FILE_DIALOG_EVENT_HANDLER_

class CDialogEventHandler : public IFileDialogEvents,
                            public IFileDialogControlEvents
{
public:
    CDialogEventHandler() {}
    ~CDialogEventHandler() {}

    // IUnknown methods
    IFACEMETHODIMP_(ULONG) AddRef()
    {
        // Automatic deletion is not required.
        return 1;
    }

#if defined (_MSC_VER) && (_MSC_VER <= 1900) // For VS2015 and earlier.
#pragma warning( push )
#pragma warning(disable:4838)
#endif

    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        static const QITAB qit[] = {
            QITABENT(CDialogEventHandler, IFileDialogEvents),
            QITABENT(CDialogEventHandler, IFileDialogControlEvents),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

#if defined (_MSC_VER) && (_MSC_VER < 1900)
#pragma warning( pop )
#endif

    IFACEMETHODIMP_(ULONG) Release()
    {
        // Automatic deletion is not required.
        return 1;
    }

    // IFileDialogEvents methods
    IFACEMETHODIMP OnFileOk(IFileDialog *) { return S_OK; };
    IFACEMETHODIMP OnFolderChange(IFileDialog *) { return S_OK; };
    IFACEMETHODIMP OnFolderChanging(IFileDialog *, IShellItem *) { return S_OK; };
    IFACEMETHODIMP OnHelp(IFileDialog *) { return S_OK; };
    IFACEMETHODIMP OnSelectionChange(IFileDialog *) { return S_OK; };
    IFACEMETHODIMP OnShareViolation(IFileDialog *, IShellItem *, FDE_SHAREVIOLATION_RESPONSE *) { return S_OK; };
    IFACEMETHODIMP OnTypeChange(IFileDialog *pfd);
    IFACEMETHODIMP OnOverwrite(IFileDialog *, IShellItem *, FDE_OVERWRITE_RESPONSE *) { return S_OK; };

    // IFileDialogControlEvents methods
    IFACEMETHODIMP OnItemSelected(IFileDialogCustomize *pfdc, DWORD dwIDCtl, DWORD dwIDItem);
    IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize *, DWORD) { return S_OK; };
    IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize *, DWORD, BOOL) { return S_OK; };
    IFACEMETHODIMP OnControlActivating(IFileDialogCustomize *, DWORD) { return S_OK; };
};


#endif // _FILE_DIALOG_EVENT_HANDLER_
