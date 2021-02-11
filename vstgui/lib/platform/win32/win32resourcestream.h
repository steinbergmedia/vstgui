// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../vstguibase.h"

#if WINDOWS

#include "../iplatformresourceinputstream.h"
#include "../../optional.h"
#include <combaseapi.h>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
class ResourceStream final : public IStream
{
public:
	ResourceStream ();
	virtual ~ResourceStream () noexcept = default;

	bool open (const CResourceDescription& resourceDesc, const char* type);

	HRESULT STDMETHODCALLTYPE Read (void *pv, ULONG cb, ULONG *pcbRead) override;
    HRESULT STDMETHODCALLTYPE Write (const void *pv, ULONG cb, ULONG *pcbWritten) override;
    HRESULT STDMETHODCALLTYPE Seek (LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition) override;    
    HRESULT STDMETHODCALLTYPE SetSize (ULARGE_INTEGER libNewSize) override;
    HRESULT STDMETHODCALLTYPE CopyTo (IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten) override;
    HRESULT STDMETHODCALLTYPE Commit (DWORD grfCommitFlags) override;
    HRESULT STDMETHODCALLTYPE Revert () override;
    HRESULT STDMETHODCALLTYPE LockRegion (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override;
    HRESULT STDMETHODCALLTYPE UnlockRegion (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override;
    HRESULT STDMETHODCALLTYPE Stat (STATSTG *pstatstg, DWORD grfStatFlag) override;
    HRESULT STDMETHODCALLTYPE Clone (IStream **ppstm) override;
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;

protected:
	HGLOBAL resData;
	uint32_t streamPos;
	uint32_t resSize;
	LONG _refcount;
};

//-----------------------------------------------------------------------------
class WinResourceInputStream final : public IPlatformResourceInputStream
{
public:
	using ResourceStreamPtr = std::unique_ptr<ResourceStream>;
	static PlatformResourceInputStreamPtr create (const CResourceDescription& desc);

private:
	WinResourceInputStream (ResourceStreamPtr&& stream);

	uint32_t readRaw (void* buffer, uint32_t size) override;
	int64_t seek (int64_t pos, SeekMode mode) override;
	int64_t tell () override;

	ResourceStreamPtr stream;
};


//------------------------------------------------------------------------
} // VSTGUI

#endif // WINDOWS
