// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32resourcestream.h"

#if WINDOWS

#include "../../cresourcedescription.h"
#include "../../cstring.h"
#include "../platform_win32.h"
#include "win32support.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
ResourceStream::ResourceStream () : resData (nullptr), streamPos (0), resSize (0), _refcount (1)
{
}

//-----------------------------------------------------------------------------
bool ResourceStream::open (const CResourceDescription& resourceDesc, const char* type)
{
	HRSRC rsrc = nullptr;
	if (resourceDesc.type == CResourceDescription::kIntegerType)
		rsrc = FindResourceA (GetInstance (), MAKEINTRESOURCEA (resourceDesc.u.id), type);
	else
		rsrc = FindResourceA (GetInstance (), resourceDesc.u.name, type);
	if (rsrc)
	{
		resSize = SizeofResource (GetInstance (), rsrc);
		HGLOBAL resDataLoad = LoadResource (GetInstance (), rsrc);
		if (resDataLoad)
		{
			resData = LockResource (resDataLoad);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::Read (void* pv, ULONG cb, ULONG* pcbRead)
{
	int readSize = std::min<int> (static_cast<int> (resSize - streamPos), static_cast<int> (cb));
	if (readSize > 0)
	{
		memcpy (pv, ((uint8_t*)resData + streamPos), static_cast<size_t> (readSize));
		streamPos += static_cast<uint32_t> (readSize);
		if (pcbRead)
			*pcbRead = static_cast<ULONG> (readSize);
		return S_OK;
	}
	if (pcbRead)
		*pcbRead = 0;
	return S_FALSE;
}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::Write (const void* pv, ULONG cb, ULONG* pcbWritten)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::Seek (LARGE_INTEGER dlibMove, DWORD dwOrigin,
                                                ULARGE_INTEGER* plibNewPosition)
{
	switch (dwOrigin)
	{
		case STREAM_SEEK_SET:
		{
			if (dlibMove.QuadPart < resSize)
			{
				streamPos = (uint32_t)dlibMove.QuadPart;
				if (plibNewPosition)
					plibNewPosition->QuadPart = streamPos;
				return S_OK;
			}
			break;
		}
		case STREAM_SEEK_CUR:
		{
			if (streamPos + dlibMove.QuadPart < resSize && streamPos + dlibMove.QuadPart >= 0)
			{
				streamPos += static_cast<uint32_t> (dlibMove.QuadPart);
				if (plibNewPosition)
					plibNewPosition->QuadPart = streamPos;
				return S_OK;
			}
			break;
		}
		case STREAM_SEEK_END: { break;
		}
		default: return STG_E_INVALIDFUNCTION; break;
	}
	return S_FALSE;
}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::SetSize (ULARGE_INTEGER libNewSize)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::CopyTo (IStream* pstm, ULARGE_INTEGER cb,
                                                  ULARGE_INTEGER* pcbRead,
                                                  ULARGE_INTEGER* pcbWritten)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::Commit (DWORD grfCommitFlags)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::Revert ()
{
	streamPos = 0;
	return S_OK;
}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::LockRegion (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
                                                      DWORD dwLockType)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::UnlockRegion (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
                                                        DWORD dwLockType)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::Stat (STATSTG* pstatstg, DWORD grfStatFlag)
{
	memset (pstatstg, 0, sizeof (STATSTG));
	pstatstg->cbSize.QuadPart = resSize;
	return S_OK;
}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::Clone (IStream** ppstm)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::QueryInterface (REFIID iid, void** ppvObject)
{
	if (iid == __uuidof (IUnknown) || iid == __uuidof (IStream) ||
	    iid == __uuidof (ISequentialStream))
	{
		*ppvObject = static_cast<IStream*> (this);
		AddRef ();
		return S_OK;
	}
	else
		return E_NOINTERFACE;
}

//-----------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE ResourceStream::AddRef ()
{
	return (ULONG)InterlockedIncrement (&_refcount);
}

//-----------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE ResourceStream::Release ()
{
	ULONG res = (ULONG)InterlockedDecrement (&_refcount);
	if (res == 0)
		delete this;
	return res;
}

//-----------------------------------------------------------------------------
auto WinResourceInputStream::create (const CResourceDescription& desc) -> PlatformResourceInputStreamPtr
{
	auto stream = ResourceStreamPtr (new ResourceStream ());
	if (stream->open (desc, "DATA"))
	{
		return PlatformResourceInputStreamPtr (new WinResourceInputStream (std::move (stream)));
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
WinResourceInputStream::WinResourceInputStream (ResourceStreamPtr&& stream)
: stream (std::move (stream))
{
}

//-----------------------------------------------------------------------------
uint32_t WinResourceInputStream::readRaw (void* buffer, uint32_t size)
{
	ULONG read = 0;
	if (stream->Read (buffer, size, &read) == S_OK)
		return read;
	return kStreamIOError;
}

//-----------------------------------------------------------------------------
int64_t WinResourceInputStream::seek (int64_t pos, SeekMode mode)
{
	DWORD dwOrigin = 0;
	switch (mode)
	{
		case SeekMode::Set: dwOrigin = STREAM_SEEK_SET; break;
		case SeekMode::Current: dwOrigin = STREAM_SEEK_CUR; break;
		case SeekMode::End: dwOrigin = STREAM_SEEK_END; break;
	}
	LARGE_INTEGER li;
	li.QuadPart = pos;
	if (stream->Seek (li, dwOrigin, nullptr) == S_OK)
		return tell ();
	return kStreamSeekError;
}

//-----------------------------------------------------------------------------
int64_t WinResourceInputStream::tell ()
{
	ULARGE_INTEGER pos;
	LARGE_INTEGER dummy = {};
	if (stream->Seek (dummy, STREAM_SEEK_CUR, &pos) == S_OK)
		return static_cast<int64_t> (pos.QuadPart);
	return kStreamSeekError;
}

//------------------------------------------------------------------------
} // VSTGUI

#endif
