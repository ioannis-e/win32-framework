/////////////////////////////
// Mainfrm.cpp
//

#include "stdafx.h"
#include "resource.h"
#include "Mainfrm.h"
#include "SearchDialog.h"
#include "UserMessages.h"

#if defined (_MSC_VER) && (_MSC_VER == 1900) // == VS2015
#pragma warning (disable : 4458) // disable warning: declaration hides class member.
using std::min;
using std::max;
#endif

#include <gdiplus.h>

#if defined(_MSC_VER) && (_MSC_VER == 1900)
#pragma warning (default : 4458) // return warning to default.
#endif

#if defined (_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning ( push )
#pragma warning ( disable : 26812 )       // enum type is unscoped.
#endif // (_MSC_VER) && (_MSC_VER >= 1400)

#include "MediaInfoDLL.h"

#if defined (_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning ( pop )  // ( disable : 26812 )    enum type is unscoped.
#endif // (_MSC_VER) && (_MSC_VER >= 1400)

// For C++Builder
#if defined (__BORLANDC__) && !(_WIN64)
#pragma comment(lib, "shell32.lib")
#endif

using namespace MediaInfoDLL;
using namespace Gdiplus;

////////////////////////////////
// Global function declarations.
//

// Retrieves the class identifier for the given MIME type of an encoder.
void GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // Number of image encoders.
    UINT  size = 0;         // Size of the image encoder array in bytes.

    ::GetImageEncodersSize(&num, &size);
    if (size != 0)
    {
        // Retrieve the image codec information and store it in codecInfo.
        std::vector<byte> codecInfo(size);
        ImageCodecInfo* pCodecInfo = reinterpret_cast<ImageCodecInfo*>(codecInfo.data());
        if (Gdiplus::Ok == ::GetImageEncoders(num, size, pCodecInfo))
        {
            // Search for the codec information matching the specified format.
            for (UINT i = 0; i < num; i++)
            {
                if (::wcscmp(pCodecInfo[i].MimeType, format) == 0)
                {
                    // Retrieve the CLSID and store it in pClsid.
                    *pClsid = pCodecInfo[i].Clsid;
                    break;
                }
            }
        }
    }
}

///////////////////////////////////
// CMainFrame function definitions.
//

// Constructor.
CMainFrame::CMainFrame() : m_addFilesThread(AddFilesProc, this), m_searchItem(nullptr), m_isAddPuttonPressed(FALSE),
                           m_pDockTree(nullptr), m_pDockDialog(nullptr), m_isDirty(false), m_boxSetsItem(0),
                           m_dialogHeight(0), m_treeWidth(0)
{
}

// This function is the thread procedure for m_addFilesThread.
// This function runs in a separate thread because loading the meta data from
// files can be time consuming. This thread modifies m_moviesData, so all
// write access to m_moviesData should be protected by a CThreadLock.
UINT WINAPI CMainFrame::AddFilesProc(void* pVoid)
{
    CMainFrame* pFrame = reinterpret_cast<CMainFrame*>(pVoid);

    MediaInfo MI;
    if (MI.IsReady())   // Is MediaInfo.dll loaded?
    {
        // Press the Add_Folder button. We use PostMessage for a different thread.
        pFrame->GetToolBar().PostMessage(TB_PRESSBUTTON, (WPARAM)IDM_ADD_FOLDER, TRUE);

        CSplashThread splashThread;
        splashThread.CreateThread();
        CSplash* splash = splashThread.GetSplash();

        // Wait for the splash window to be created.
        ::WaitForSingleObject(splashThread.GetSplashCreated(), INFINITE);

        splash->ShowText(L"Updating Library", pFrame);
        const CProgressBar& progressBar = splash->GetBar();
        progressBar.ShowWindow(SW_SHOW);
        progressBar.SetRange(0, (short)pFrame->m_filesToAdd.size());

        unsigned short barPos = 0;
        for (size_t i = 0; i < pFrame->m_filesToAdd.size(); i++)
        {
            // The stop request is set if the app is trying to close,
            // or when the 'Add Folder' button toggled.
            if (::WaitForSingleObject(pFrame->m_stopRequest, 0) != WAIT_TIMEOUT)
            {
                // Unpress the Add_Folder button. We use PostMessage for a different thread.
                pFrame->GetToolBar().PostMessage(TB_PRESSBUTTON, (WPARAM)IDM_ADD_FOLDER, FALSE);

                // Break out of the loop and end the thread.
                return 0;
            }

            barPos++;

            // Update the splash screen's progress bar.
            progressBar.SetPos(barPos);

            CString fullName = pFrame->m_filesToAdd[i].fileName;
            bool isFileInLibrary = false;
            for (auto it = pFrame->m_moviesData.begin(); it != pFrame->m_moviesData.end();)
            {
                if ((*it).fileName == fullName)
                {
                    CFileFind ff;
                    ff.FindFirstFile(fullName);
                    CTime t1(ff.GetLastWriteTime());
                    CTime t2((*it).lastModifiedTime);
                    if (t1 != t2)
                    {
                        // Lock this code for thread safety.
                        CThreadLock lock(pFrame->m_cs);

                        // Remove the modified file from the library.
                        TRACE(fullName); TRACE(" removed modified file from library\n");
                        pFrame->m_moviesData.erase(it);
                    }
                    else
                        isFileInLibrary = true;

                    // No need to check further.
                    break;
                }

                ++it;
            }

            // Only add files not already in the library.
            if (!isFileInLibrary)
            {
                pFrame->m_isDirty = true;
                MovieInfo mi{};
                pFrame->LoadMovieInfoFromFile(pFrame->m_filesToAdd[i], mi);

                // Lock this code for thread safety.
                CThreadLock lock(pFrame->m_cs);
                pFrame->m_moviesData.push_back(mi);

                TRACE(fullName); TRACE(" added to library\n");
            }
        }
    }
    else
    {
        // Report the error in a message  box.
        ::MessageBox(nullptr, MI.Inform().c_str(), L"Error", MB_OK);
    }

    pFrame->OnFilesLoaded();
    pFrame->GetToolBar().PostMessage(TB_PRESSBUTTON, (WPARAM)IDM_ADD_FOLDER, FALSE);

    return 0;
}

// Clears the contents of the movie info dialog.
void CMainFrame::ClearDisplay()
{
    GetViewDialog().GetTitle().SetWindowText(nullptr);
    GetViewDialog().GetYear().SetWindowText(nullptr);
    GetViewDialog().GetActors().SetWindowText(nullptr);
    GetViewDialog().GetInfo().SetWindowText(nullptr);
    GetViewDialog().SetPicture().SetImageData().clear();
    GetViewDialog().RedrawWindow();
}

// Clears the contents of the ListView.
void CMainFrame::ClearList()
{
    GetViewList().DeleteAllItems();
}

// Create the frame window.
HWND CMainFrame::Create(HWND parent)
{
    // Set m_view as the view window of the frame.
    SetView(m_viewList);

    // Set the registry key name, and load the initial window position.
    // Use a registry key name like "CompanyName\\Application".
    LoadRegistrySettings(L"Win32++\\MovieShow");

    m_genres.push_back(L"Action, Crime, Thriller, War, Western");
    m_genres.push_back(L"Adventure, Horror, Fantasy, Science Fiction");
    m_genres.push_back(L"Animation, Family");
    m_genres.push_back(L"Comedy");
    m_genres.push_back(L"Documentary, History, Music, TV Movie");
    m_genres.push_back(L"Drama, Romance");

    int currentYear = CTime::GetCurrentTime().GetYear();
    m_decades.push_back(L"1900 - 1949");
    for (int year = 1950; year <= currentYear; year += 10)
    {
        CString str;
        str.Format(L"%d - %d", year, year + 9);
        m_decades.push_back(str);
    }

    return CDockFrame::Create(parent);
}

// Converts a text string to a byte stream.
// Loads an image from the byte stream and converts it to a thumbnail.
// Saves the byte stream for the thumbnail in the specified vector.
void CMainFrame::FillImageData(const CString& source, std::vector<BYTE>& dest)
{
    DWORD bufferSize = 0;
    if (::CryptStringToBinary(source.c_str(), source.GetLength(),
        CRYPT_STRING_BASE64, nullptr, &bufferSize, nullptr, nullptr))
    {
        // Use a vector for an array of BYTE.
        std::vector<BYTE> sourceData(bufferSize, 0);
        BYTE* pSource = sourceData.data();
        ::CryptStringToBinary(source.c_str(), (DWORD)source.GetLength(),
            CRYPT_STRING_BASE64, pSource, &bufferSize, nullptr, nullptr);

        // Create the IStream from the array of BYTE.
        IStream* pStream = SHCreateMemStream(pSource, bufferSize);
        if (pStream)
        {
            // Acquire GDI+ Image from the IStream.
            Image image(pStream);

            // Create a smaller thumbnail Image.
            using  BitmapPtr = std::unique_ptr<Bitmap>;
            BitmapPtr img((Bitmap*)image.GetThumbnailImage(280, 420, nullptr, nullptr));

            IStream* stream = SHCreateMemStream(nullptr, 0);
            if (stream)
            {
                CLSID gifClsid = {};
                GetEncoderClsid(L"image/gif", &gifClsid);
                if (Gdiplus::Ok == img->Save(stream, &gifClsid, nullptr))
                {
                    // Get the size of the stream.
                    ULARGE_INTEGER streamSize;
                    VERIFY(S_OK == IStream_Size(stream, &streamSize));
                    ULONG len = streamSize.LowPart;
                    dest.assign(len, 0);

                    // Fill buffer from stream.
                    VERIFY(S_OK == IStream_Reset(stream));
                    VERIFY(S_OK == IStream_Read(stream, dest.data(), len));
                }
                // Cleanup.
                stream->Release();
            }
            // Cleanup.
            pStream->Release();
        }
    }
}

// Fills the list view with all movies.
void CMainFrame::FillList()
{
    GetViewList().SetRedraw(FALSE);
    m_splashThread.GetSplash()->ShowText(L"Updating List", this);
    ClearList();

    // Lock this function for thread safety.
    CThreadLock lock(m_cs);

    for(const auto& i : m_moviesData)
    {
        GetViewList().AddItem(i);
    }

    m_splashThread.GetSplash()->Hide();
    GetViewList().SetRedraw(TRUE);
    GetViewList().SetLastColumnWidth();
    if (GetViewList().GetItemCount() == 0)
        ClearDisplay();
}

// Fills the list view with movies belonging to all boxsets.
void CMainFrame::FillListFromAllBoxSets()
{
    GetViewList().SetRedraw(FALSE);
    m_splashThread.GetSplash()->ShowText(L"Updating List", this);
    ClearList();

    // Lock this function for thread safety.
    CThreadLock lock(m_cs);

    for (const auto& i : m_moviesData)
    {
        if (i.boxset != L"")
            GetViewList().AddItem(i);
    }

    m_splashThread.GetSplash()->Hide();
    GetViewList().SetRedraw(TRUE);
    GetViewList().SetLastColumnWidth();
    if (GetViewList().GetItemCount() == 0)
        ClearDisplay();
}

// Fills the list view with movies belonging to the specified boxset.
void CMainFrame::FillListFromBoxSet(LPCWSTR boxset)
{
    GetViewList().SetRedraw(FALSE);
    m_splashThread.GetSplash()->ShowText(L"Updating List", this);
    ClearList();

    // Lock this function for thread safety.
    CThreadLock lock(m_cs);

    for (const auto& i : m_moviesData)
    {
        if (i.boxset == boxset)
            GetViewList().AddItem(i);
    }

    m_splashThread.GetSplash()->Hide();
    GetViewList().SetRedraw(TRUE);
    GetViewList().SetLastColumnWidth();
    if (GetViewList().GetItemCount() == 0)
        ClearDisplay();
}

// Fills the list view with movies within the specified date range.
void CMainFrame::FillListFromDateRange(LPCWSTR dateRange)
{
    GetViewList().SetRedraw(FALSE);
    m_splashThread.GetSplash()->ShowText(L"Updating List", this);
    ClearList();

    // Lock this function for thread safety.
    CThreadLock lock(m_cs);

    CString str = dateRange;
    CString Year1 = str.Left(4);
    CString Year2 = str.Right(4);

    for (const auto& i : m_moviesData)
    {
        CString year = i.releaseDate.Left(4);
        if (((year >= Year1) && (year <= Year2)) && i.videoType == L"Movies")
            GetViewList().AddItem(i);
    }

    m_splashThread.GetSplash()->Hide();
    GetViewList().SetRedraw(TRUE);
    GetViewList().SetLastColumnWidth();
    if (GetViewList().GetItemCount() == 0)
        ClearDisplay();
}

// Fills the list view with movies matching the specified mask.
void CMainFrame::FillListFromFlags(DWORD mask)
{
    GetViewList().SetRedraw(FALSE);
    m_splashThread.GetSplash()->ShowText(L"Updating List", this);
    ClearList();

    // Lock this function for thread safety.
    CThreadLock lock(m_cs);

    for (const auto& i : m_moviesData)
    {
        if (i.flags & mask)
            GetViewList().AddItem(i);
    }

    m_splashThread.GetSplash()->Hide();
    GetViewList().SetRedraw(TRUE);
    GetViewList().SetLastColumnWidth();
    if (GetViewList().GetItemCount() == 0)
        ClearDisplay();
}

// Fills the list view with movies matching the specified genre.
void CMainFrame::FillListFromGenre(LPCWSTR genre)
{
    // Lock this function for thread safety.
    CThreadLock lock(m_cs);

    for (const auto& i : m_moviesData)
    {
        if ((i.genre.Find(genre, 0) >= 0) && i.videoType == L"Movies")
            GetViewList().AddItem(i);
    }

    GetViewList().SetLastColumnWidth();
}

// Fills the list view with movies from all genres.
void CMainFrame::FillListFromGenres(LPCWSTR genreList)
{
    GetViewList().SetRedraw(FALSE);
    m_splashThread.GetSplash()->ShowText(L"Updating List", this);
    ClearList();

    CString str = genreList;
    CString resToken;
    int curPos = 0;

    resToken = str.Tokenize(L",", curPos);
    while (resToken != L"")
    {
        resToken.TrimLeft();
        FillListFromGenre(resToken);
        resToken = str.Tokenize(L",", curPos);
    }

    m_splashThread.GetSplash()->Hide();
    GetViewList().SetRedraw(TRUE);
    GetViewList().SetLastColumnWidth();
    if (GetViewList().GetItemCount() == 0)
        ClearDisplay();
}

// Fill the listview with movies found while searching.
void CMainFrame::FillListFromSearch()
{
    GetViewList().SetRedraw(FALSE);
    m_splashThread.GetSplash()->ShowText(L"Updating List", this);
    ClearList();

    for (UINT i = 0; i < m_foundMovies.size(); ++i)
    {
        for (const auto& j :m_moviesData)
        {
            if (&j == m_foundMovies[i])
            {
                GetViewList().AddItem(j);
                break;
            }
        }
    }

    m_splashThread.GetSplash()->Hide();
    GetViewList().SetRedraw(TRUE);
    GetViewList().SetLastColumnWidth();
    if (GetViewList().GetItemCount() == 0)
        ClearDisplay();
}

// Fills the list view with movies matching the specified type.
void CMainFrame::FillListFromType(LPCWSTR videoType)
{
    GetViewList().SetRedraw(FALSE);
    m_splashThread.GetSplash()->ShowText(L"Updating List", this);
    ClearList();

    // Lock this function for thread safety.
    CThreadLock lock(m_cs);

    for (const auto& i : m_moviesData)
    {
        if (i.videoType.Find(videoType, 0) >= 0)
            GetViewList().AddItem(i);
    }

    m_splashThread.GetSplash()->Hide();
    GetViewList().SetRedraw(TRUE);
    GetViewList().SetLastColumnWidth();
    if (GetViewList().GetItemCount() == 0)
        ClearDisplay();
}

// Fills the tree view.
void CMainFrame::FillTreeItems()
{
    GetViewTree().DeleteAllItems();

    // Add some tree-view items.
    HTREEITEM libraryItem = GetViewTree().InsertItem(L"Video Library", 0, 0);
    HTREEITEM moviesItem = GetViewTree().InsertItem(L"Movies", 1, 1, libraryItem);

    m_boxSetsItem = GetViewTree().InsertItem(L"Box Sets", 2, 2, moviesItem);
    for (const auto& i : m_boxSets)
    {
        GetViewTree().InsertItem(i, 2, 2, m_boxSetsItem);
    }

    HTREEITEM genreItem = GetViewTree().InsertItem(L"Genre", 5, 5, moviesItem);
    for (const auto& i : m_genres)
    {
        GetViewTree().InsertItem(i, 5, 5, genreItem);
    }

    HTREEITEM decadesItem = GetViewTree().InsertItem(L"Release Dates", 3, 3, moviesItem);
    for (const auto& i : m_decades)
    {
        GetViewTree().InsertItem(i, 3, 3, decadesItem);
    }

    GetViewTree().InsertItem(L"Live Performances", 6, 6, libraryItem);
    GetViewTree().InsertItem(L"Favourites", 4, 4, libraryItem);
    GetViewTree().InsertItem(L"Watch List", 8, 8, libraryItem);
    m_searchItem = GetViewTree().InsertItem(L"Search", 7, 7, libraryItem);

    // Expand some tree-view items.
    GetViewTree().Expand(libraryItem, TVE_EXPAND);
    GetViewTree().Expand(moviesItem, TVE_EXPAND);

    GetViewTree().SelectItem(libraryItem);
}

// Force the window to the foreground.
void CMainFrame::ForceToForeground()
{
    BYTE keyState[256] = { 0 };
    if (::GetKeyboardState(keyState))
    {
        if (!(keyState[VK_MENU] & 0x80))
        {
            // Simulate Alt Key down.
            ::keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);

            ShowWindow(SW_SHOW);
            SetForegroundWindow();

            // Simulate Alt Key up.
            ::keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
        }
    }
}

// Returns a vector of CString holding the box set names.
std::vector<CString> CMainFrame::GetBoxSets()
{
    std::vector<CString> boxSets;
    HTREEITEM item = GetViewTree().GetChild(m_boxSetsItem);
    while (item != 0)
    {
        boxSets.emplace_back(GetViewTree().GetItemText(item));
        item = GetViewTree().GetNextSibling(item);
    };

    return boxSets;
}

// Returns the path used for the data files.
CString CMainFrame::GetDataPath() const
{
    CString filePath = GetAppDataPath();

    assert(!filePath.IsEmpty());
    filePath += L"\\";
    filePath += GetRegistryKeyName();

    return filePath;
}

// Converts a text string to a vector of words.
std::vector<CString> CMainFrame::GetWords(const CString& str) const
{
    int position = 0;
    CString token;
    token = str.Tokenize(L" ", position);
    std::vector<CString> words;
    while (!token.IsEmpty())
    {
        // Get next token.
        words.push_back(token);
        token = str.Tokenize(L" ", position);
    }

    return words;
}

// Returns true if the file's extension is m4v, mp4 or mp3.
bool CMainFrame::IsVideoFile(const CString& filename) const
{
    int dot = filename.ReverseFind(L'.');
    CString ext = filename.Mid(dot + 1, lstrlen(filename));
    ext.MakeLower();

    return (ext == L"m4v" || ext == L"mp4" || ext == L"mov");
}

// Performs a case-insensitive search for a word in sentence.
// Returns true if a matching word is found.
bool CMainFrame::IsWordInString(const CString& sentence, const CString& word) const
{
    int pos = 0;
    CString sentenceLow = sentence;
    sentenceLow.MakeLower();

    CString wordLow = word;
    wordLow.MakeLower();

    // find each substring in sentence that matches word.
    while ((pos = sentenceLow.Find(wordLow, pos)) >= 0)
    {
        // words are bound by non-isalpha characters or begin/end of word.
        int nNextChar = pos + wordLow.GetLength();
        bool isWordStart = (pos == 0) || !(iswalpha(sentenceLow[pos - 1]));
        bool isWordEnd = (nNextChar == sentenceLow.GetLength()) || !(iswalpha(sentenceLow[nNextChar]));

        // return true if the substring found is a word.
        if (isWordStart && isWordEnd)
            return true;

        pos++;
    }

    return false;
}

// Loads the meta data info from the specified movie file.
// Stores the information in the specified MovieInfo struct.
void CMainFrame::LoadMovieInfoFromFile(const FoundFileInfo& ffi, MovieInfo& movie)
{
    movie.fileName = ffi.fileName;
    movie.lastModifiedTime = ffi.lastModifiedTime;

    MediaInfo MI;
    MI.Option(L"Cover_Data", L"base64");    // Required for v18.03.1 and higher.

    MI.Open(ffi.fileName.c_str());
    movie.movieName   = MI.Get(Stream_General, 0, L"Movie", Info_Text, Info_Name).c_str();
    movie.genre       = MI.Get(Stream_General, 0, L"Genre", Info_Text, Info_Name).c_str();
    movie.actors      = MI.Get(Stream_General, 0, L"Actor", Info_Text, Info_Name).c_str();
    movie.description = MI.Get(Stream_General, 0, L"LongDescription",  Info_Text, Info_Name).c_str();
    movie.releaseDate = MI.Get(Stream_General, 0, L"Recorded_Date",    Info_Text, Info_Name).c_str();
    movie.duration    = MI.Get(Stream_General, 0, L"Duration/String3", Info_Text, Info_Name).c_str();

    CString CoverImage = MI.Get(Stream_General, 0, L"Cover_Data", Info_Text, Info_Name).c_str();
    FillImageData(CoverImage, movie.imageData);
    movie.genre.Remove(L" / 57");
    movie.videoType = L"Movies";
    movie.flags = 0x0002;   // Adds video to the watch list.

    MI.Close();
}

// Loads the movie data information from the archive.
void CMainFrame::LoadMovies()
{
    // Information about MediaInfo.
    MediaInfo MI;
    CString dataPath = GetDataPath();
    CString dataFile = GetDataPath() + L"\\" + L"MovieData.bin";
    SHCreateDirectoryEx(nullptr, dataPath.c_str(), nullptr);
    CSplash* pSplash = m_splashThread.GetSplash();

    if (PathFileExists(dataFile))
    {
        try
        {
            // Display the splash screen.
            pSplash->ShowText(L"", this);
            pSplash->ShowText(L"Loading Library", this);

            // Lock this code for thread safety.
            CThreadLock lock(m_cs);

            m_moviesData.clear();
            TRACE("Loading Movies Data\n");

            CArchive ar(dataFile, CArchive::load);
            UINT nBoxSets;
            ar >> nBoxSets;
            for (UINT i = 0; i < nBoxSets; ++i)
            {
                CString str;
                ar >> str;
                m_boxSets.push_back(str);
            }

            UINT size;
            ar >> size;
            for (UINT i = 0; i < size; ++i)
            {
                MovieInfo mi{};

                ar >> mi.fileName;
                ArchiveObject ao(&mi.lastModifiedTime, sizeof(FILETIME));
                ar >> ao;
                ar >> mi.movieName;
                ar >> mi.duration;
                ar >> mi.releaseDate;
                ar >> mi.description;
                ar >> mi.genre;
                ar >> mi.actors;
                ar >> mi.videoType;
                ar >> mi.boxset;
                ar >> mi.flags;

                UINT ImageDataSize = 0;
                ar >> ImageDataSize;
                if (ImageDataSize > 0)
                {
                    mi.imageData.resize(ImageDataSize);
                    ar.Read(mi.imageData.data(), ImageDataSize);
                }

                m_moviesData.push_back(mi);
            }
        }
        catch (const CFileException& e)
        {
            pSplash->Hide();
            Trace(e.GetErrorString()); Trace("\n");
            ::MessageBox(nullptr, L"Failed to load Movie Library", L"Error", MB_OK);
            m_moviesData.clear();
            m_boxSets.clear();
        }
    }

    TRACE("Load Movies data complete \n ");
}

// Loads settings from the registry.
BOOL CMainFrame::LoadRegistrySettings(LPCWSTR szKeyName)
{
    assert(szKeyName != nullptr);

    if (CDockFrame::LoadRegistrySettings(szKeyName))
    {
        // Always show the toolbar and status bar (fixes old values).
        InitValues iv = GetInitValues();
        iv.showStatusBar = TRUE;
        iv.showToolBar = TRUE;
        SetInitValues(iv);

        CString strKey = L"Software\\" + GetRegistryKeyName() + L"\\Frame Settings";
        CRegKey Key;
        if (ERROR_SUCCESS == Key.Open(HKEY_CURRENT_USER, strKey, KEY_READ))
        {
            try
            {
                DWORD dwDialogHeight = 0;
                DWORD dwTreeWidth = 0;

                if (ERROR_SUCCESS != Key.QueryDWORDValue(L"Dialog Width", dwDialogHeight))
                    throw CUserException(L"QueryDWORDValue Failed");
                if (ERROR_SUCCESS != Key.QueryDWORDValue(L"Tree Height", dwTreeWidth))
                    throw CUserException(L"QueryDWORDValue Failed");

                m_dialogHeight = dwDialogHeight;
                m_treeWidth = dwTreeWidth;
                return TRUE;
            }

            catch (const CUserException&)
            {
                m_dialogHeight = 0;
                m_treeWidth = 0;
            }
        }
    }

    return FALSE;
}

// Called in response to the add box set from the context menu on the list view.
BOOL CMainFrame::OnAddBoxSet()
{
    GetViewTree().SetFocus();
    HTREEITEM newItem = GetViewTree().InsertItem(L"New Box Set", 2, 2, m_boxSetsItem);

    GetViewTree().Expand(m_boxSetsItem, TVE_EXPAND);
    GetViewTree().EditLabel(newItem);
    m_isDirty = true;

    return true;
}

// Called when the Add Folder toolbar button is pressed.
BOOL CMainFrame::OnAddFolder()
{
    if (::WaitForSingleObject(m_addFilesThread, 0) != WAIT_TIMEOUT) // if thread is not running.
    {
        CFolderDialog fd;
        fd.SetTitle(L"Choose a folder to add to the video library.");
        if (fd.DoModal(*this) == IDOK)
        {
            // Lock this code for thread safety.
            CThreadLock lock(m_cs);

            // Remove entries from the library if the file has been removed.
            for (auto it = m_moviesData.begin(); it != m_moviesData.end();)
            {
                if (!::PathFileExists((*it).fileName))
                {
                    TRACE((*it).fileName); TRACE("  removed from library\n");
                    it = m_moviesData.erase(it);
                }
                else
                    ++it;
            }

            CString searchString = fd.GetFolderPath() + L"\\*.m??";
            CFileFind fileFound;

            m_filesToAdd.clear();
            if (fileFound.FindFirstFile(searchString))
            {
                do
                    if (!fileFound.IsDots())
                    {
                        FoundFileInfo ffi{};
                        ffi.fileName = fileFound.GetFilePath();

                        if (IsVideoFile(ffi.fileName))
                        {
                            ffi.lastModifiedTime = fileFound.GetLastWriteTime();
                            m_filesToAdd.push_back(ffi);
                        }
                    }
                while (fileFound.FindNextFile());
            }

            // Create the thread and run ThreadProc.
            m_addFilesThread.CreateThread(0, 0, nullptr);
        }
    }
    else
    {
        m_stopRequest.SetEvent();
    }

    GetToolBar().CheckButton(IDM_ADD_FOLDER, FALSE);
    return TRUE;
}

// Called when the splitter bar move has completed.
LRESULT CMainFrame::OnBarEnd(DragPos* pDragPos)
{
    GetViewList().SetLastColumnWidth();
    return CDockFrame::OnBarEnd(pDragPos);
}

// Called when the user clicks on Box Sets (parent or child)
// in the tree view.
BOOL CMainFrame::OnBoxSet(UINT nID)
{
    int index = nID - IDM_BOXSET_1;
    std::vector<CString> boxsets = GetBoxSets();
    int item = -1;
    while ((item = GetViewList().GetNextItem(item, LVIS_SELECTED)) != -1)
    {
        m_isDirty = true;
        MovieInfo* pmi = (MovieInfo*)GetViewList().GetItemData(item);
        pmi->boxset = (index < 0) ? CString("") : boxsets[index];
        GetViewList().UpdateItemImage(item);
    }

    HTREEITEM treeItem = GetViewTree().GetSelection();
    CString itemText = GetViewTree().GetItemText(treeItem);

    HTREEITEM parentItem = GetViewTree().GetParentItem(treeItem);
    CString parentText = GetViewTree().GetItemText(parentItem);

    if (parentText == L"Box Sets")
        FillListFromBoxSet(itemText);

    if (itemText == L"Box Sets")
        FillListFromAllBoxSets();

    CString str;
    int listItemsCount = GetViewList().GetItemCount();
    if (listItemsCount == 1)
        str.Format(L":  %d video", listItemsCount);
    else
        str.Format(L":  %d videos", listItemsCount);

    str = itemText + str;
    SetStatusText(str);
    return TRUE;
}

// Called when the box set name has changed.
LRESULT CMainFrame::OnBoxSetChanged()
{
    m_isDirty = true;
    return 0;
}

// Called when the frame application is closed (before the window is destroyed).
// Stores the movie data in an archive if it has changed.
void CMainFrame::OnClose()
{
    // Ask the add files thread to stop.
    m_stopRequest.SetEvent();
    if (::WaitForSingleObject(m_addFilesThread, 1000) == WAIT_TIMEOUT)
        Trace("The add files thread failed to end cleanly\n");

    ShowWindow(SW_HIDE);

    if (m_isDirty)
    {
        // Lock this code for thread safety.
        CThreadLock lock(m_cs);

        // Display the splash screen.
        // The splash window is destroyed when splash goes out of scope.
        m_splashThread.GetSplash()->ShowText(L"Saving Library", this);

        CString dataPath = GetDataPath();
        CString dataFile = GetDataPath() + L"\\" + L"MovieData.bin";
        ::SHCreateDirectoryEx(nullptr, dataPath.c_str(), nullptr);

        try
        {
            CArchive ar(dataFile, CArchive::store);
            std::vector<CString> boxSets = GetBoxSets();
            ar << UINT(boxSets.size());
            for (size_t i = 0; i < boxSets.size(); ++i)
            {
                ar << boxSets[i];
            }

            ar << UINT(m_moviesData.size());
            for (auto& i : m_moviesData)
            {
                ar << i.fileName;
                ArchiveObject ao(&i.lastModifiedTime, sizeof(FILETIME));
                ar << ao;
                ar << i.movieName;
                ar << i.duration;
                ar << i.releaseDate;
                ar << i.description;
                ar << i.genre;
                ar << i.actors;
                ar << i.videoType;
                ar << i.boxset;
                ar << i.flags;

                UINT ImageDataSize = UINT(i.imageData.size());
                ar << ImageDataSize;
                if (ImageDataSize > 0)
                    ar.Write(i.imageData.data(), ImageDataSize);
            }
            TRACE("\nSave Movies data complete \n ");
        }

        catch (const CFileException& e)
        {
            Trace(e.GetErrorString()); Trace("\n");
            ::MessageBox(nullptr, L"Failed to store Movie Library", L"Error", MB_OK);
        }
    }

    m_splashThread.GetSplash()->Close();
    m_splashThread.PostThreadMessage(WM_QUIT, 0, 0);

    if (::WaitForSingleObject(m_splashThread, 1000) == WAIT_TIMEOUT)
        Trace("Splash thread failed to end cleanly\n");

    CDockFrame::OnClose();
}

// Respond to toolbar or context menu input.
BOOL CMainFrame::OnCommand(WPARAM wparam, LPARAM)
{
    UINT nID = LOWORD(wparam);

    switch (nID)
    {
    case IDM_EDIT_COPY:     GetFocus().SendMessage(WM_COPY);    return TRUE;
    case IDM_EDIT_PASTE:    GetFocus().SendMessage(WM_PASTE);   return TRUE;

    case IDM_ADD_FOLDER:        return OnAddFolder();
    case IDM_PLAY:              return OnPlay();
    case IDM_FAVOURITE:         return OnFavourite();

    case IDM_VIDEOTYPE_LIVEPERFORMANCE: return OnVideoType(L"Live Performances");
    case IDM_VIDEOTYPE_MOVIE:           return OnVideoType(L"Movies");

    case IDM_ADD_BOXSET:        return OnAddBoxSet();
    case IDM_MOVE_UP:           return OnMoveUp();
    case IDM_MOVE_DOWN:         return OnMoveDown();
    case IDM_RENAME_BOXSET:     return OnRenameBoxSet();
    case IDM_REMOVE_BOXSET:     return OnRemoveBoxSet();

    case IDM_WATCHLIST:         return OnWatchList();

    case IDM_REMOVE_FILE:       return OnRemoveFile();
    case IDM_SEARCH:            return OnSearch();
    case IDW_VIEW_STATUSBAR:    return OnViewStatusBar();
    case IDW_VIEW_TOOLBAR:      return OnViewToolBar();
    case IDM_HELP_ABOUT:        return OnHelp();
    }

    if (nID >= IDM_BOXSET_0)
        return OnBoxSet(nID);

    return FALSE;
}

// Called during window creation. Override this functions to perform tasks,
// such as creating child windows.
int CMainFrame::OnCreate(CREATESTRUCT& cs)
{
    // OnCreate controls the way the frame is created.
    // Overriding CFrame::OnCreate is optional.

    // A menu (or MenuBar) is added if the IDW_MAIN menu resource is defined.
    // Frames have all options enabled by default.
    // Use the following functions to disable options.

    // This app has a toolbar, but no menu (or menubar).

    UseIndicatorStatus(FALSE);    // Don't show keyboard indicators in the StatusBar.
    UseMenuStatus(FALSE);         // Don't show menu descriptions in the StatusBar.
    // UseOwnerDrawnMenu(FALSE);     // Don't use owner draw for popup menu items.
    // UseReBar(FALSE);              // Don't use a ReBar.
    // UseStatusBar(FALSE);          // Don't use a StatusBar.
    // UseThemes(FALSE);             // Don't use themes.
    // UseToolBar(FALSE);            // Don't use a ToolBar.

    // Create the splash screen.
    m_splashThread.CreateThread();

    // Call the base class function.
    CDockFrame::OnCreate(cs);

    // Set the color of the frame's caption.
    COLORREF color = GetReBarTheme().clrBkgnd1;
    SetCaptionColor(color);

    return 0;
}

// Called after a DPI change is processed.
LRESULT CMainFrame::OnDpiChanged(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Call base class function first.
    CDockFrame::OnDpiChanged(msg, wparam, lparam);

    // Restore the Add_Folder pressed state after the DPI change.
    GetToolBar().PressButton(IDM_ADD_FOLDER, m_isAddPuttonPressed);
    return 0;
}

// Called in response to favourites on the toolbar or the
// list view popup menu. Sets the movie's favourite flag.
BOOL CMainFrame::OnFavourite()
{
    // Update the item flag.
    int item = -1;
    while ((item = GetViewList().GetNextItem(item, LVIS_SELECTED)) != -1)
    {
        m_isDirty = true;
        MovieInfo* pmi = (MovieInfo*)GetViewList().GetItemData(item);
        BOOL IsFavourite = pmi->flags & 0x0001;
        IsFavourite = !(IsFavourite != FALSE);
        if (IsFavourite)
            pmi->flags |= 0x0001;
        else
            pmi->flags &= ~0x0001;

        // Update the toolbar button and list view image.
        GetToolBar().CheckButton(IDM_FAVOURITE, IsFavourite);
        GetViewList().UpdateItemImage(item);
    }

    // Update the list view.
    HTREEITEM treeItem = GetViewTree().GetSelection();
    CString itemText = GetViewTree().GetItemText(treeItem);

    if (itemText == L"Favourites")
        FillListFromFlags(0x0001);

    return TRUE;
}

// Called after the video library is updated.
void CMainFrame::OnFilesLoaded()
{
    m_filesToAdd.clear();

    // Re-select first tree item.
    HTREEITEM item = GetViewTree().GetSelection();
    GetViewTree().SelectItem(0);
    GetViewTree().SelectItem(item);
}

// Called before a DPI change is processed.
LRESULT CMainFrame::OnGetDpiScaledSize(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Save the Add_Folder pressed state before the DPI change.
    m_isAddPuttonPressed = GetToolBar().IsButtonPressed(IDM_ADD_FOLDER);
    return CDockFrame::OnGetDpiScaledSize(msg, wparam, lparam);
}

// Call when the Help button or F1 is pressed.
BOOL CMainFrame::OnHelp()
{
    // Ensure only one dialog displayed even for multiple hits of the F1 button
    if (!m_aboutDialog.IsWindow())
    {
        m_aboutDialog.DoModal(*this);
    }

    return TRUE;
}

// Called after the frame is created, but before it is displayed.
void CMainFrame::OnInitialUpdate()
{
    SetRedraw(FALSE);

    // Add the tree view.
    DWORD dockStyle = DS_NO_UNDOCK | DS_NO_CAPTION;
    int width = m_treeWidth ? m_treeWidth : (GetViewRect().Width() / 3);
    m_pDockTree = static_cast<CDockTree*>(AddDockedChild(std::make_unique<CDockTree>(),
        dockStyle | DS_DOCKED_LEFT, width));

    // Add the dialog.
    int height = m_dialogHeight ? m_dialogHeight : 2 * GetViewRect().Height() / 3;
    dockStyle = dockStyle | DS_CLIENTEDGE;
    m_pDockDialog = static_cast<CDockDialog*>(m_pDockTree->AddDockedChild(
        std::make_unique<CDockDialog>(), dockStyle | DS_DOCKED_BOTTOM, height));

    // Fill the tree view and list view.
    LoadMovies();
    FillTreeItems();
    SetRedraw(TRUE);
    RedrawWindow();
    ForceToForeground();
}

// Called when a menu is about to be displayed.
// Updates the enable state of menu items.
void CMainFrame::OnMenuUpdate(UINT nID)
{
    switch (nID)
    {

    case IDM_MOVE_DOWN:
    {
        HTREEITEM item = GetViewTree().GetSelection();
        bool isAtBottom = (GetViewTree().GetNextSibling(item) == 0);
        m_boxSetMenu.EnableMenuItem(nID, isAtBottom? MF_GRAYED : MF_ENABLED);
        break;
    }

    case IDM_MOVE_UP:
    {
        HTREEITEM item = GetViewTree().GetSelection();
        bool isAtTop = (GetViewTree().GetPrevSibling(item) == 0);
        m_boxSetMenu.EnableMenuItem(nID, isAtTop ? MF_GRAYED : MF_ENABLED);
        break;
    }

    }
}

// Called in response to the popup menu when the user right clicks
// on a box set in the tree view.
BOOL CMainFrame::OnMoveDown()
{
    HTREEITEM item = GetViewTree().GetSelection();
    HTREEITEM nextItem = GetViewTree().GetNextSibling(item);

    if (item != 0 && nextItem != 0)
    {
        GetViewTree().Swap(item, nextItem);
        GetViewTree().SelectItem(nextItem);
    }

    return TRUE;
}

// Called in response to the popup menu when the user right clicks
// on a box set in the tree view.
BOOL CMainFrame::OnMoveUp()
{
    HTREEITEM item = GetViewTree().GetSelection();
    HTREEITEM prevItem = GetViewTree().GetPrevSibling(item);

    if (item != 0 && prevItem != 0)
    {
        GetViewTree().Swap(item, prevItem);
        GetViewTree().SelectItem(prevItem);
    }
    return TRUE;
}

// Process notification messages (WM_NOTIFY) from child windows.
LRESULT CMainFrame::OnNotify(WPARAM wparam, LPARAM lparam)
{
    LPNMHDR pHeader = reinterpret_cast<LPNMHDR>(lparam);
    switch (pHeader->code)
    {
    // Notification from the listview's header control. The header
    // control is a child window of the listview control.
    // The columns in listview control have been resized.
    case HDN_ENDTRACK:
        GetViewList().SetLastColumnWidth();
        return 0;
    }

    // Call the base class function.
    return CDockFrame::OnNotify(wparam, lparam);
}

// Called in response to toolbar input or a context menu on
// the list view. Plays the chosen movie.
BOOL CMainFrame::OnPlay()
{
    int item = GetViewList().GetNextItem(-1, LVNI_SELECTED);
    MovieInfo* pmi = (MovieInfo*)GetViewList().GetItemData(item);
    if (pmi)
        PlayMovie(pmi->fileName);

    return TRUE;
}

// Called in response to a right mouse click on the list view.
// Displays the popup menus.
LRESULT CMainFrame::OnRClickListItem()
{
    // Load the popup menu
    CMenu topMenu(IDM_VIDEO_MENU);
    m_popupMenu = topMenu.GetSubMenu(0);

    CMenu boxsetMenu = m_popupMenu.GetSubMenu(3);
    std::vector<CString> boxSets = GetBoxSets();

    for (size_t i = 0; i < boxSets.size(); i++)
    {
        boxsetMenu.AppendMenu(MF_STRING, IDM_BOXSET_1 + i, boxSets[i]);
    }

    CPoint screenPoint = GetCursorPos();

    // Start the popup menu.
    m_popupMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, screenPoint.x, screenPoint.y, *this, nullptr);

    return 0;
}

// Displays the popup menus for the tree view. Called in response to a right
// mouse click on the tree view.
LRESULT CMainFrame::OnRClickTreeItem()
{
    CPoint screenPoint = GetCursorPos();
    CPoint clientPoint = screenPoint;
    GetViewTree().ScreenToClient(clientPoint);
    TVHITTESTINFO tv{};
    tv.pt = clientPoint;
    HTREEITEM item = GetViewTree().HitTest(tv);
    if (item != 0)
        GetViewTree().SelectItem(item);

    if (GetViewTree().GetItemText(item) == L"Box Sets")
    {
        GetViewTree().Expand(item, TVE_EXPAND);

        // The menu for a boxset parent.
        CMenu topMenu(IDM_BOXSET_MENU);
        m_boxSetMenu = topMenu.GetSubMenu(0);

        // Start the popup menu.
        m_boxSetMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, screenPoint.x, screenPoint.y, *this, nullptr);
    }

    HTREEITEM parent = GetViewTree().GetParentItem(item);
    if (GetViewTree().GetItemText(parent) == L"Box Sets")
    {
        // The menu for an boxset child.
        CMenu topMenu(IDM_BOXSETCHILD_MENU);
        m_boxSetMenu = topMenu.GetSubMenu(0);

        // Start the popup menu.
        m_boxSetMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, screenPoint.x, screenPoint.y, *this, nullptr);
    }

    return 0;
}

// Called in response to the popup menu. The popup menu is displayed when the
// user right clicks on a box set in the tree view.
BOOL CMainFrame::OnRemoveBoxSet()
{
    HTREEITEM item = GetViewTree().GetSelection();
    CString str = GetViewTree().GetItemText(item);

    // Lock this function for thread safety.
    CThreadLock lock(m_cs);

    for (auto& i : m_moviesData)
    {
        if (i.boxset == str)
            i.boxset = L"";
    }

    GetViewTree().DeleteItem(item);
    m_isDirty = TRUE;

    return TRUE;
}

// Called in response to a popup menu on the list view.
// Removes the chosen file from the movie library.
BOOL CMainFrame::OnRemoveFile()
{
    // Lock this function for thread safety.
    CThreadLock lock(m_cs);

    std::vector<CString> filenames;
    int itemIndex = -1;
    while ((itemIndex = GetViewList().GetNextItem(itemIndex, LVIS_SELECTED)) != -1)
    {
        MovieInfo* pmi = (MovieInfo*)GetViewList().GetItemData(itemIndex);
        filenames.push_back(pmi->fileName);
    }

    for (UINT i = 0; i < filenames.size(); ++i)
    {
        for (auto it = m_moviesData.begin(); it != m_moviesData.end();)
        {
            if ((*it).fileName == filenames[i])
            {
                TRACE(filenames[i]); TRACE(" removed from library\n");
                m_moviesData.erase(it);
                m_isDirty = true;
                break;
            }
            else
                ++it;
        }
    }

    OnSelectTreeItem();

    return TRUE;
}

// Called in response to the popup menu when the user right clicks
// on a box set in the tree view.
BOOL CMainFrame::OnRenameBoxSet()
{
    HTREEITEM item = GetViewTree().GetSelection();
    GetViewTree().EditLabel(item);

    return TRUE;
}

// Called in response to a click on the toolbar search button.
// Opens a dialog to perform a word search on the select items.
BOOL CMainFrame::OnSearch()
{
    CSearchDialog dlg(IDD_SEARCHDLG);
    if (dlg.DoModal(*this) == IDOK)
    {
        ClearList();
        m_foundMovies.clear();

        // Lock this code for thread safety.
        CThreadLock lock(m_cs);

        // Find movies matching each title word.
        if (!dlg.GetTitleString().IsEmpty())
        {
            std::vector<CString> words = GetWords(dlg.GetTitleString());

            for (UINT i = 0; i < words.size(); ++i)
            {
                for (const auto& j : m_moviesData)
                {
                    if (IsWordInString(j.movieName, words[i]))
                        m_foundMovies.push_back(&j);
                }
            }
        }

        // Find movies matching each actor word.
        if (!dlg.GetActorsString().IsEmpty())
        {
            std::vector<CString> words = GetWords(dlg.GetActorsString());
            for (UINT i = 0; i < words.size(); ++i)
            {
                for (auto& j : m_moviesData)
                {
                    if (IsWordInString(j.actors, words[i]))
                        m_foundMovies.push_back(&j);
                }
            }
        }

        // Find movies matching each description word.
        if (!dlg.GetInfoString().IsEmpty())
        {
            std::vector<CString> words = GetWords(dlg.GetInfoString());
            for (UINT i = 0; i < words.size(); ++i)
            {
                for (const auto& j : m_moviesData)
                {
                    if (IsWordInString(j.description, words[i]))
                        m_foundMovies.push_back(&j);
                }
            }
        }

        // Select the treeview search item to fill the listview
        // with the found movies.
        GetViewTree().SelectItem(0);
        GetViewTree().SelectItem(m_searchItem);
        GetViewTree().SetFocus();

        CString str;
        int listItemsCount = GetViewList().GetItemCount();
        if (listItemsCount == 1)
            str.Format(L":  %d video found", listItemsCount);
        else
            str.Format(L":  %d videos found", listItemsCount);

        str = L"Search" + str;
        SetStatusText(str);
    }

    return TRUE;
}

// Updates the dialog with information about the movie.
LRESULT CMainFrame::OnSelectListItem(const MovieInfo* pmi)
{
    assert(pmi);

    // Set the fonts.
    m_pDockDialog->GetViewDialog().SetDialogFonts();
    CViewDialog& dialog = (CViewDialog&)m_pDockDialog->GetView();
    dialog.GetTitle().SetWindowText(pmi->movieName);
    dialog.GetYear().SetWindowText(pmi->releaseDate);

    // Replace " / " with cariage return, line feed to put each actor on a new line.
    CString actors = pmi->actors;
    actors.Replace(L" / ", L"\r\n");
    dialog.GetActors().SetWindowText(actors);
    dialog.GetActors().SendMessage(EM_SCROLL, SB_TOP, 0);
    dialog.GetInfo().SetWindowText(pmi->description);
    dialog.GetInfo().SendMessage(EM_SCROLL, SB_TOP, 0);
    dialog.SetPicture().SetImageData() = pmi->imageData;
    dialog.SetPicture().Invalidate();

    BOOL isFavourite = pmi->flags & 0x0001;
    BOOL isWatch = pmi->flags & 0x0002;

    if (GetViewList().GetNextItem(-1, LVNI_SELECTED) < 0)
    {
        isFavourite = FALSE;
        isWatch = FALSE;
    }

    GetToolBar().CheckButton(IDM_FAVOURITE, isFavourite);
    GetToolBar().CheckButton(IDM_WATCHLIST, isWatch);

    return 0;
}

// Called when a selection is made on the tree view.
LRESULT CMainFrame::OnSelectTreeItem()
{
    GetToolBar().CheckButton(IDM_FAVOURITE, FALSE);
    GetToolBar().CheckButton(IDM_WATCHLIST, FALSE);

    HTREEITEM item = GetViewTree().GetSelection();
    CString itemText = GetViewTree().GetItemText(item);

    HTREEITEM parentItem = GetViewTree().GetParentItem(item);
    CString parentText = GetViewTree().GetItemText(parentItem);

    if (itemText == L"Video Library")
        FillList();
    else if (itemText == L"Movies")
        FillListFromType(itemText);
    else if (itemText == L"Box Sets")
        FillListFromAllBoxSets();
    else if (itemText == L"Release Dates")
        FillListFromType(L"Movies");
    else if (itemText == L"Genre")
        FillListFromType(L"Movies");
    else if (itemText == L"Live Performances")
        FillListFromType(itemText);
    else if (parentText == L"Box Sets")
        FillListFromBoxSet(itemText);
    else if (parentText == L"Release Dates")
        FillListFromDateRange(itemText);
    else if (parentText == L"Genre")
        FillListFromGenres(itemText);
    else if (itemText == L"Favourites")
        FillListFromFlags(0x0001);
    else if (itemText == L"Watch List")
        FillListFromFlags(0x0002);
    else if (itemText == L"Search")
        FillListFromSearch();
    else
        ClearList();

    GetViewList().SortColumn(0, false);

    CString str;
    int listItemsCount = GetViewList().GetItemCount();
    if (listItemsCount == 1)
        str.Format(L":  %d video", listItemsCount);
    else
        str.Format(L":  %d videos", listItemsCount);

    str = itemText + str;
    SetStatusText(str);

    return 0;
}

// Respond to system commands such as maximize and restore.
// Adjust the column sizes to match the list view window width.
LRESULT CMainFrame::OnSysCommand(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Perform default processing first.
    DefWindowProc(msg, wparam, lparam);

    // Now resize the columns.
    GetViewList().SetLastColumnWidth();
    return 0;
}

// Called in response to the popup menu on the list view.
// Sets the video type to a "Movie" or "Live Performance".
BOOL CMainFrame::OnVideoType(LPCWSTR videoType)
{
    int item = -1;
    while ((item = GetViewList().GetNextItem(item, LVIS_SELECTED)) != -1)
    {
        m_isDirty = true;
        MovieInfo* pmi = (MovieInfo*)GetViewList().GetItemData(item);
        pmi->videoType = videoType;
        GetViewList().UpdateItemImage(item);
    }

    return TRUE;
}

// Called in response to the popup menu on the list view or toolbar.
// Toggles items in the Watch list.
BOOL CMainFrame::OnWatchList()
{
    int item = -1;
    while ((item = GetViewList().GetNextItem(item, LVIS_SELECTED)) != -1)
    {
        // Adjust the movie item flag.
        m_isDirty = true;
        MovieInfo* pmi = (MovieInfo*)GetViewList().GetItemData(item);
        BOOL isWatch = pmi->flags & 0x0002;
        isWatch = !(isWatch != FALSE);
        if (isWatch)
            pmi->flags |= 0x0002;
        else
            pmi->flags &= ~0x0002;

        // Update the toolbar button and list view image.
        GetToolBar().CheckButton(IDM_WATCHLIST, isWatch);
        GetViewList().UpdateItemImage(item);
    }

    // Update the list view.
    HTREEITEM treeItem = GetViewTree().GetSelection();
    CString itemText = GetViewTree().GetItemText(treeItem);
    if (itemText == L"Watch List")
        FillListFromFlags(0x0002);

    return TRUE;
}

// Called when the window's position is about to change.
LRESULT CMainFrame::OnWindowPosChanging(UINT msg, WPARAM wparam, LPARAM lparam)
{
    GetViewList().SetLastColumnWidth();
    return FinalWindowProc(msg, wparam, lparam);
}

// Plays the selected movie.
LRESULT CMainFrame::PlayMovie(LPCWSTR path)
{
    if (PathFileExists(path))
        ::ShellExecute(*this, L"open", path, nullptr, nullptr, SW_SHOW);
    else
    {
        CString str = CString(L"Unable to play\n") + path;
        MessageBox(str, L"Unable to play movie", MB_OK);
    }

    return 0;
}

// Sets the CREATESTRUCT struct prior to window creation.
void CMainFrame::PreCreate(CREATESTRUCT& cs)
{
    // This function is called before the frame is created.
    // It provides an opportunity to modify the various CREATESTRUCT
    // parameters used in the frame window's creation.

    // Call base class function to set defaults.
    CDockFrame::PreCreate(cs);

    // Remove the WS_VISIBLE style. The frame will be initially hidden.
    cs.style &= ~WS_VISIBLE;
}

// Saves some settings in the registry.
BOOL CMainFrame::SaveRegistrySettings()
{
    if (!GetRegistryKeyName().IsEmpty())
    {
        try
        {
            CString KeyName = L"Software\\" + GetRegistryKeyName() + L"\\Frame Settings";
            CRegKey Key;

            if (ERROR_SUCCESS != Key.Create(HKEY_CURRENT_USER, KeyName))
                throw CUserException(L"RegCreateKeyEx failed");
            if (ERROR_SUCCESS != Key.Open(HKEY_CURRENT_USER, KeyName))
                throw CUserException(L"RegCreateKeyEx failed");

            DWORD cyDialog = DWORD(m_pDockDialog->GetDockSize());
            DWORD cxTree = DWORD(m_pDockTree->GetDockSize());

            if (ERROR_SUCCESS != Key.SetDWORDValue(L"Dialog Width", cyDialog))
                throw CUserException(L"SetDWORDValue failed");
            if (ERROR_SUCCESS != Key.SetDWORDValue(L"Tree Height", cxTree))
                throw CUserException(L"SetDWORDValue failed");

            return CDockFrame::SaveRegistrySettings();
        }

        catch (const CUserException& e)
        {
            Trace("*** Failed to save registry settings. ***\n");
            Trace(e.GetText()); Trace("\n");

            CString KeyName = L"Software\\" + GetRegistryKeyName();
            CRegKey Key;

            if (ERROR_SUCCESS == Key.Open(HKEY_CURRENT_USER, KeyName))
            {
                // Roll back the registry changes by deleting this subkey.
                Key.DeleteSubKey(L"Frame Settings");
            }

            return FALSE;
        }
    }

    return FALSE;
}

// This function uses the DwmSetWindowAttribute Windows API function to set
// the caption color. The DWMWA_CAPTION_COLOR option requires Windows 11.
void CMainFrame::SetCaptionColor(COLORREF color)
{
    HMODULE dwmapi = ::LoadLibrary(L"Dwmapi.dll");
    if (dwmapi != 0)
    {
        using PDWMSETWINDOWATTRIBUE = UINT (WINAPI*)(HWND, DWORD, LPCVOID, DWORD);
        PDWMSETWINDOWATTRIBUE pDwmSetWindowAttribute = reinterpret_cast<PDWMSETWINDOWATTRIBUE>(
            reinterpret_cast<void*>(::GetProcAddress(dwmapi, "DwmSetWindowAttribute")));

        if (pDwmSetWindowAttribute != nullptr)
        {
            const int DWMWA_CAPTION_COLOR = 35;
            pDwmSetWindowAttribute(*this, DWMWA_CAPTION_COLOR, &color, sizeof(color));
        }
        ::FreeLibrary(dwmapi);
    }
}

// Adds icons for popup menus.
void CMainFrame::SetupMenuIcons()
{
    AddMenuIcon(IDM_ADD_FOLDER,  IDI_ADDFOLDER);
    AddMenuIcon(IDM_PLAY,        IDI_PLAY);
    AddMenuIcon(IDM_FAVOURITE,   IDI_FAVOURITES);
    AddMenuIcon(IDM_WATCHLIST,   IDI_EYE);
    AddMenuIcon(IDM_REMOVE_FILE, IDI_REMOVE);
    AddMenuIcon(IDM_HELP_ABOUT,  IDI_HELPABOUT);

    AddMenuIcon(IDM_VIDEOTYPE_MOVIE, IDI_MOVIES);
    AddMenuIcon(IDM_VIDEOTYPE_LIVEPERFORMANCE, IDI_VIOLIN);

    AddMenuIcon(IDM_ADD_BOXSET,  IDI_ADDBOXSET);
    AddMenuIcon(IDM_MOVE_UP,     IDI_MOVEUP);
    AddMenuIcon(IDM_MOVE_DOWN,   IDI_MOVEDOWN);
    AddMenuIcon(IDM_REMOVE_BOXSET, IDI_REMOVE);
    AddMenuIcon(IDM_RENAME_BOXSET, IDI_EDITBOXSET);

    // Accomodate up to 64 boxsets
    for (int i = IDM_BOXSET_1; i < IDM_BOXSET_1 + 64; i++)
        AddMenuIcon(i, IDI_BOXSET);
}

// Configure the toolbar.
void CMainFrame::SetupToolBar()
{
    // Add toolbar buttons and set their Resource IDs.
    AddToolBarButton(IDM_ADD_FOLDER);
    AddToolBarButton(0);                        // Separator
    AddToolBarButton(IDM_PLAY);
    AddToolBarButton(IDM_FAVOURITE);
    AddToolBarButton(IDM_WATCHLIST);
    AddToolBarButton(IDM_SEARCH);
    AddToolBarButton(0);                        // Separator
    AddToolBarButton(IDM_HELP_ABOUT);

    // Create the normal ImageList for the toolbar.
    int size = DpiScaleInt(32);
    m_toolbarImages.Create(size, size, ILC_COLOR32, 0, 0);
    m_toolbarImages.AddIcon(IDI_ADDFOLDER);
    m_toolbarImages.AddIcon(IDI_PLAY);
    m_toolbarImages.AddIcon(IDI_FAVOURITES);
    m_toolbarImages.AddIcon(IDI_EYE);
    m_toolbarImages.AddIcon(IDI_SEARCH);
    m_toolbarImages.AddIcon(IDI_HELPABOUT);

    // Assign the imagelist to the toolbar.
    GetToolBar().SetImageList(m_toolbarImages);
}

// Process the frame's window messages.
LRESULT CMainFrame::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    try
    {
        switch (msg)
        {
        case WM_WINDOWPOSCHANGING:      return OnWindowPosChanging(msg, wparam, lparam);
        case WM_SYSCOMMAND:             return OnSysCommand(msg, wparam, lparam);

        // User Messages called by CTreeList.
        case UWM_BOXSETCHANGED:         return OnBoxSetChanged();
        case UWM_GETMOVIESDATA:         return (LRESULT)GetMoviesData();
        case UWM_ONSELECTTREEITEM:      return OnSelectTreeItem();
        case UWM_ONRCLICKTREEITEM:      return OnRClickTreeItem();

        // Use Messages called by CViewList.
        case UWM_PLAYMOVIE:             return PlayMovie((LPCWSTR)wparam);
        case UWM_ONSELECTLISTITEM:      return OnSelectListItem((const MovieInfo*)wparam);
        case UWM_ONRCLICKLISTITEM:      return OnRClickListItem();
        }

        // Pass unhandled messages on for default processing.
        return WndProcDefault(msg, wparam, lparam);
    }

    // Catch all unhandled CException types.
    catch (const CException& e)
    {
        // Display the exception and continue.
        CString str1;
        str1 << e.GetText() << L'\n' << e.GetErrorString();
        CString str2;
        str2 << "Error: " << e.what();
        ::MessageBox(nullptr, str1, str2, MB_ICONERROR);
    }

    // Catch all unhandled std::exception types.
    catch (const std::exception& e)
    {
        // Display the exception and continue.
        CString str1 = e.what();
        ::MessageBox(nullptr, str1, L"Error: std::exception", MB_ICONERROR);
    }

    return 0;
}
