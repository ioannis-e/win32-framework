///////////////////////////////////////
// StaticImage.cpp
//

#include "stdafx.h"
#include "StaticImage.h"

void CStaticImage::DpiScaleImage()
{
    m_patternImage.LoadBitmap(IDB_BITMAP1);
    m_patternImage = DpiScaleUpBitmap(m_patternImage);
    LPARAM lparam = reinterpret_cast<LPARAM>(m_patternImage.GetHandle());
    SendMessage(STM_SETIMAGE, IMAGE_BITMAP, lparam);
}

LRESULT CStaticImage::OnPaint(UINT msg, WPARAM wparam, LPARAM lparam)
{
    DpiScaleImage();
    return FinalWindowProc(msg, wparam, lparam);
}
