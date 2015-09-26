/****************************** Module Header ******************************\
This source is subject to the Microsoft Public License.
See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma once

#include <windows.h>
#include <thumbcache.h>     // For IThumbnailProvider
#include <wincodec.h>       // Windows Imaging Codecs

#pragma comment(lib, "windowscodecs.lib")


class RpgMakerXyzThumbnailProvider : 
    public IInitializeWithStream, 
    public IThumbnailProvider
{
public:
    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // IInitializeWithStream
    IFACEMETHODIMP Initialize(IStream *pStream, DWORD grfMode);

    // IThumbnailProvider
    IFACEMETHODIMP GetThumbnail(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha);

    RpgMakerXyzThumbnailProvider();

protected:
    ~RpgMakerXyzThumbnailProvider();

private:
    // Reference count of component.
    long m_cRef;

    // Provided during initialization.
    IStream *m_pStream;

    HRESULT GetXyzImage( 
        UINT cx, 
        HBITMAP *phbmp, 
        WTS_ALPHATYPE *pdwAlpha);
};