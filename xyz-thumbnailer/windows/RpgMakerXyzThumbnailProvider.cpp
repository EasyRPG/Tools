/******************************** Module Header ********************************\
This source is subject to the Microsoft Public License.
See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\*******************************************************************************/

#include "RpgMakerXyzThumbnailProvider.h"
#include <Shlwapi.h>

#include <sstream>
#include <vector>
#include <zlib.h>

typedef UCHAR uint8_t;

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "msxml6.lib")


extern HINSTANCE g_hInst;
extern long g_cDllRef;


RpgMakerXyzThumbnailProvider::RpgMakerXyzThumbnailProvider() : m_cRef(1), m_pStream(NULL)
{
    InterlockedIncrement(&g_cDllRef);
}


RpgMakerXyzThumbnailProvider::~RpgMakerXyzThumbnailProvider()
{
    InterlockedDecrement(&g_cDllRef);
}


#pragma region IUnknown

// Query to the interface the component supported.
IFACEMETHODIMP RpgMakerXyzThumbnailProvider::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(RpgMakerXyzThumbnailProvider, IThumbnailProvider),
        QITABENT(RpgMakerXyzThumbnailProvider, IInitializeWithStream), 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

// Increase the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) RpgMakerXyzThumbnailProvider::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

// Decrease the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) RpgMakerXyzThumbnailProvider::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
    }

    return cRef;
}

#pragma endregion


#pragma region IInitializeWithStream

// Initializes the thumbnail handler with a stream.
IFACEMETHODIMP RpgMakerXyzThumbnailProvider::Initialize(IStream *pStream, DWORD grfMode)
{
    // A handler instance should be initialized only once in its lifetime. 
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    if (m_pStream == NULL)
    {
        // Take a reference to the stream if it has not been initialized yet.
        hr = pStream->QueryInterface(&m_pStream);
    }
    return hr;
}

#pragma endregion


#pragma region IThumbnailProvider

// Gets a thumbnail image and alpha type. The GetThumbnail is called with the 
// largest desired size of the image, in pixels. Although the parameter is 
// called cx, this is used as the maximum size of both the x and y dimensions. 
// If the retrieved thumbnail is not square, then the longer axis is limited 
// by cx and the aspect ratio of the original image respected. On exit, 
// GetThumbnail provides a handle to the retrieved image. It also provides a 
// value that indicates the color format of the image and whether it has 
// valid alpha information.
IFACEMETHODIMP RpgMakerXyzThumbnailProvider::GetThumbnail(UINT cx, HBITMAP *phbmp, 
    WTS_ALPHATYPE *pdwAlpha)
{
    // Load the Xyz document.
    HRESULT hr = GetXyzImage(cx, phbmp, pdwAlpha);

    return hr;
}

#pragma endregion

HRESULT RpgMakerXyzThumbnailProvider::GetXyzImage(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha)
{
	ULONG bytesRead;
	STATSTG statstg;
	HRESULT hr;

	*pdwAlpha = WTSAT_ARGB;

	hr = m_pStream->Stat(&statstg, STATFLAG_NONAME);

	if (SUCCEEDED(hr)) {
		if (statstg.cbSize.QuadPart > 1024*1024*1024) {
			// Skip too large files (> 1 MB) to save parsing time
			return E_NOT_SUFFICIENT_BUFFER;
		}

		size_t size = (size_t)statstg.cbSize.QuadPart;

		char* data = (char*)malloc(size);
		if (!data) {
			return E_OUTOFMEMORY;
		}

		hr = m_pStream->Read(data, (ULONG)size, &bytesRead);

		if (SUCCEEDED(hr)) {
			if (size < 8 || strncmp((char *) data, "XYZ1", 4) != 0) {
				return E_INVALIDARG;
			}
	
			unsigned short w;
			memcpy(&w, &data[4], 2);
			unsigned short h;
			memcpy(&h, &data[6], 2);

			uLongf src_size = (uLongf)(size - 8);
			Bytef* src_buffer = (Bytef*)&data[8];
			uLongf dst_size = 768 + (w * h);
			std::vector<Bytef> dst_buffer(dst_size);

			int status = uncompress(&dst_buffer.front(), &dst_size, src_buffer, src_size);

			free(data);

			if (status != Z_OK) {
				return E_INVALIDARG;
			}
			const uint8_t (*palette)[3] = (const uint8_t(*)[3]) &dst_buffer.front();

			int width = w;
			int height = h;
			void* pixels = malloc(w * h * 4);

			if (!pixels) {
				return E_OUTOFMEMORY;
			}

			uint8_t* dst = (uint8_t*) pixels;
			const uint8_t* src = (const uint8_t*) &dst_buffer[768];
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++) {
					uint8_t pix = *src++;
					const uint8_t* color = palette[pix];
					*dst++ = color[2];
					*dst++ = color[1];
					*dst++ = color[0];
					*dst++ = 255;
				}
			}

			*phbmp = CreateBitmap(w, h, 1, 32, pixels);

			free(pixels);

			if (!(*phbmp)) {
				return E_UNEXPECTED;
			}
		}
	}

	return hr;
}