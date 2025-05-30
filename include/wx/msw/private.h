/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/private.h
// Purpose:     Private declarations: as this header is only included by
//              wxWidgets itself, it may contain identifiers which don't start
//              with "wx".
// Author:      Julian Smart
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PRIVATE_H_
#define _WX_PRIVATE_H_

#include "wx/msw/wrapwin.h"

#include "wx/log.h"

#if wxUSE_GUI
    #include "wx/window.h"
#endif // wxUSE_GUI

class WXDLLIMPEXP_FWD_CORE wxFont;
class WXDLLIMPEXP_FWD_CORE wxWindow;
class WXDLLIMPEXP_FWD_CORE wxWindowBase;

// ---------------------------------------------------------------------------
// private constants
// ---------------------------------------------------------------------------

// 260 was taken from windef.h
#ifndef MAX_PATH
    #define MAX_PATH  260
#endif

// Many MSW functions have parameters which are "reserved". Passing them this
// constant is more clear than just using "0" or "nullptr".
#define wxRESERVED_PARAM    0

// ---------------------------------------------------------------------------
// standard icons from the resources
// ---------------------------------------------------------------------------

#if wxUSE_GUI

extern WXDLLIMPEXP_DATA_CORE(HICON) wxSTD_FRAME_ICON;
extern WXDLLIMPEXP_DATA_CORE(HICON) wxSTD_MDIPARENTFRAME_ICON;
extern WXDLLIMPEXP_DATA_CORE(HICON) wxSTD_MDICHILDFRAME_ICON;
extern WXDLLIMPEXP_DATA_CORE(HICON) wxDEFAULT_FRAME_ICON;
extern WXDLLIMPEXP_DATA_CORE(HICON) wxDEFAULT_MDIPARENTFRAME_ICON;
extern WXDLLIMPEXP_DATA_CORE(HICON) wxDEFAULT_MDICHILDFRAME_ICON;
extern WXDLLIMPEXP_DATA_CORE(HFONT) wxSTATUS_LINE_FONT;

#endif // wxUSE_GUI

// ---------------------------------------------------------------------------
// global data
// ---------------------------------------------------------------------------

extern "C"
{
    WXDLLIMPEXP_BASE HINSTANCE wxGetInstance();
}

WXDLLIMPEXP_BASE void wxSetInstance(HINSTANCE hInst);

// ---------------------------------------------------------------------------
// define things missing from some compilers' headers
// ---------------------------------------------------------------------------

// this defines a CASTWNDPROC macro which casts a pointer to the type of a
// window proc
#if defined(STRICT) || defined(__GNUC__)
    typedef WNDPROC WndProcCast;
#else
    typedef FARPROC WndProcCast;
#endif

#define CASTWNDPROC (WndProcCast)


// ---------------------------------------------------------------------------
// misc macros
// ---------------------------------------------------------------------------

#if wxUSE_GUI

#define MEANING_CHARACTER '0'
#define DEFAULT_ITEM_WIDTH  100
#define DEFAULT_ITEM_HEIGHT 80

// Return the height of a native text control corresponding to the given
// character height (as returned by GetCharHeight() or wxGetCharSize()).
//
// The wxWindow parameter is currently not used but should still be valid.
inline int wxGetEditHeightFromCharHeight(int cy, const wxWindow* WXUNUSED(w))
{
    // The value 8 here is empiric, i.e. it's not necessarily correct, but
    // seems to work relatively well.
    // Don't use FromDIP(8), this seems not needed.
    return cy + 8;
}

// Compatibility macro used in the existing code. It assumes that it's called
// from a method of wxWindow-derived object.
#define EDIT_HEIGHT_FROM_CHAR_HEIGHT(cy) \
    wxGetEditHeightFromCharHeight((cy), this)

// Generic subclass proc, for panel item moving/sizing and intercept
// EDIT control VK_RETURN messages
extern LONG APIENTRY
  wxSubclassedGenericControlProc(WXHWND hWnd, WXUINT message, WXWPARAM wParam, WXLPARAM lParam);

#endif // wxUSE_GUI

// ---------------------------------------------------------------------------
// useful macros and functions
// ---------------------------------------------------------------------------

// a wrapper for ZeroMemory()
template <typename T>
inline void wxZeroMemory(T& obj)
{
    // Cast is needed just to avoid clang "nontrivial-memcall" warning.
    ::ZeroMemory(static_cast<void*>(&obj), sizeof(obj));
}

// This one is a macro so that it can be tested with #ifdef, it will be
// undefined if it cannot be implemented for a given compiler.
// Vc++, dmc, ow, mingw akk have _get_osfhandle() and Cygwin has
// get_osfhandle. Others are currently unknown, e.g. Salford, Intel, Visual
// Age.
#if defined(__CYGWIN__)
    #define wxGetOSFHandle(fd) ((HANDLE)get_osfhandle(fd))
#elif defined(__VISUALC__) \
   || defined(__MINGW32__)
    #define wxGetOSFHandle(fd) ((HANDLE)_get_osfhandle(fd))
    #define wxOpenOSFHandle(h, flags) (_open_osfhandle(wxPtrToUInt(h), flags))

    wxDECL_FOR_STRICT_MINGW32(FILE*, _fdopen, (int, const char*))
    #define wx_fdopen _fdopen
#endif

// close the handle in the class dtor
template <wxUIntPtr INVALID_VALUE>
class AutoHANDLE
{
public:
    explicit AutoHANDLE(HANDLE handle = InvalidHandle()) : m_handle(handle) { }

    bool IsOk() const { return m_handle != InvalidHandle(); }
    operator HANDLE() const { return m_handle; }

    ~AutoHANDLE() { if ( IsOk() ) DoClose(); }

    void Close()
    {
        wxCHECK_RET(IsOk(), wxT("Handle must be valid"));

        DoClose();

        m_handle = InvalidHandle();
    }

protected:
    // We need this helper function because integer INVALID_VALUE is not
    // implicitly convertible to HANDLE, which is a pointer.
    static HANDLE InvalidHandle()
    {
        wxUIntPtr h = INVALID_VALUE;
        return reinterpret_cast<HANDLE>(h);
    }

    void DoClose()
    {
        if ( !::CloseHandle(m_handle) )
            wxLogLastError(wxT("CloseHandle"));
    }

    WXHANDLE m_handle;
};

// a template to make initializing Windows structs less painful: it zeros all
// the struct fields and also sets cbSize member to the correct value (and so
// can be only used with structures which have this member...)
template <class T>
struct WinStruct : public T
{
    WinStruct()
    {
        wxZeroMemory(*this);

        // explicit qualification is required here for this to be valid C++
        this->cbSize = sizeof(T);
    }
};


// Macros for converting wxString to the type expected by API functions.
//
// Normally it is enough to just use wxString::t_str() which is implicitly
// convertible to LPCTSTR, but in some cases an explicit conversion is required.
//
// In such cases wxMSW_CONV_LPCTSTR() should be used. But if an API function
// takes a non-const pointer, wxMSW_CONV_LPTSTR() which casts away the
// constness (but doesn't make it possible to really modify the returned
// pointer, of course) should be used. And if a string is passed as LPARAM, use
// wxMSW_CONV_LPARAM() which does the required ugly reinterpret_cast<> too.
#define wxMSW_CONV_LPCTSTR(s) static_cast<const wxChar *>((s).t_str())
#define wxMSW_CONV_LPTSTR(s) const_cast<wxChar *>(wxMSW_CONV_LPCTSTR(s))
#define wxMSW_CONV_LPARAM(s) reinterpret_cast<LPARAM>(wxMSW_CONV_LPCTSTR(s))


#if wxUSE_GUI

#include "wx/gdicmn.h"
#include "wx/colour.h"

#ifdef COM_DECLSPEC_NOTHROW
    #define wxSTDMETHODIMP COM_DECLSPEC_NOTHROW STDMETHODIMP
#else
    #define wxSTDMETHODIMP STDMETHODIMP
#endif

// make conversion from wxColour and COLORREF a bit less painful
inline COLORREF wxColourToRGB(const wxColour& c)
{
    return RGB(c.Red(), c.Green(), c.Blue());
}

inline COLORREF wxColourToPalRGB(const wxColour& c)
{
    return PALETTERGB(c.Red(), c.Green(), c.Blue());
}

inline wxColour wxRGBToColour(COLORREF rgb)
{
    return wxColour(GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
}

inline void wxRGBToColour(wxColour& c, COLORREF rgb)
{
    c.Set(GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
}

// get the standard colour map for some standard colours - see comment in this
// function to understand why is it needed and when should it be used
//
// it returns a wxCOLORMAP (can't use COLORMAP itself here as comctl32.dll
// might be not included/available) array of size wxSTD_COLOUR_MAX
//
// NB: if you change these colours, update wxBITMAP_STD_COLOURS in the
//     resources as well: it must have the same number of pixels!
enum wxSTD_COLOUR
{
    wxSTD_COL_BTNTEXT,
    wxSTD_COL_BTNSHADOW,
    wxSTD_COL_BTNFACE,
    wxSTD_COL_BTNHIGHLIGHT,
    wxSTD_COL_MAX
};

struct WXDLLIMPEXP_CORE wxCOLORMAP
{
    COLORREF from, to;
};

// this function is implemented in src/msw/window.cpp
extern wxCOLORMAP *wxGetStdColourMap();

// create a wxRect from Windows RECT
inline wxRect wxRectFromRECT(const RECT& rc)
{
    return wxRect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
}

// copy Windows RECT to our wxRect
inline void wxCopyRECTToRect(const RECT& rc, wxRect& rect)
{
    rect = wxRectFromRECT(rc);
}

// and vice versa
inline void wxCopyRectToRECT(const wxRect& rect, RECT& rc)
{
    // note that we don't use wxRect::GetRight() as it is one of compared to
    // wxRectFromRECT() above
    rc.top = rect.y;
    rc.left = rect.x;
    rc.right = rect.x + rect.width;
    rc.bottom = rect.y + rect.height;
}

// translations between HIMETRIC units (which OLE likes) and pixels (which are
// liked by all the others) - implemented in msw/utilsexc.cpp
extern void HIMETRICToPixel(LONG *x, LONG *y);
extern void HIMETRICToPixel(LONG *x, LONG *y, HDC hdcRef);
extern void PixelToHIMETRIC(LONG *x, LONG *y);
extern void PixelToHIMETRIC(LONG *x, LONG *y, HDC hdcRef);

// Windows convention of the mask is opposed to the wxWidgets one, so we need
// to invert the mask each time we pass one/get one to/from Windows
extern HBITMAP wxInvertMask(HBITMAP hbmpMask, int w = 0, int h = 0);

// Creates an icon or cursor depending from a bitmap
//
// The bitmap must be valid and it should have a mask. If it doesn't, a default
// mask is created using light grey as the transparent colour.
extern HICON wxBitmapToHICON(const wxBitmap& bmp);

// Same requirements as above apply and the bitmap must also have the correct
// size.
extern
HCURSOR wxBitmapToHCURSOR(const wxBitmap& bmp, int hotSpotX, int hotSpotY);

extern int wxGetSystemMetrics(int nIndex, const wxWindow* win);

extern bool wxSystemParametersInfo(UINT uiAction, UINT uiParam,
                                   PVOID pvParam, UINT fWinIni,
                                   const wxWindow* win);

#if wxUSE_OWNER_DRAWN

// Draw the bitmap in specified state (this is used by owner drawn controls)
enum wxDSBStates
{
    wxDSB_NORMAL = 0,
    wxDSB_SELECTED,
    wxDSB_DISABLED
};

extern
BOOL wxDrawStateBitmap(HDC hDC, HBITMAP hBitmap, int x, int y, UINT uState);

#endif // wxUSE_OWNER_DRAWN

// get the current state of SHIFT/CTRL/ALT keys
inline bool wxIsModifierDown(int vk)
{
    // GetKeyState() returns different negative values on WinME and WinNT,
    // so simply test for negative value.
    return ::GetKeyState(vk) < 0;
}

inline bool wxIsShiftDown()
{
    return wxIsModifierDown(VK_SHIFT);
}

inline bool wxIsCtrlDown()
{
    return wxIsModifierDown(VK_CONTROL);
}

inline bool wxIsAltDown()
{
    return wxIsModifierDown(VK_MENU);
}

inline bool wxIsAnyModifierDown()
{
    return wxIsShiftDown() || wxIsCtrlDown() || wxIsAltDown();
}

// wrapper around GetWindowRect() and GetClientRect() APIs doing error checking
// for Win32
inline RECT wxGetWindowRect(HWND hwnd)
{
    RECT rect;

    if ( !::GetWindowRect(hwnd, &rect) )
    {
        wxLogLastError(wxT("GetWindowRect"));
    }

    return rect;
}

inline RECT wxGetClientRect(HWND hwnd)
{
    RECT rect;

    if ( !::GetClientRect(hwnd, &rect) )
    {
        wxLogLastError(wxT("GetClientRect"));
    }

    return rect;
}

// Call MapWindowPoints() on a RECT: because a RECT is (intentionally) laid out
// as 2 consecutive POINTs, the cast below is valid but we still prefer to hide
// it in this function instead of writing it out in the rest of the code.
inline void wxMapWindowPoints(HWND hwndFrom, HWND hwndTo, RECT* rc)
{
    ::MapWindowPoints(hwndFrom, hwndTo, reinterpret_cast<POINT *>(rc), 2);
}

// For consistency also provide an overload taking a POINT, even if this one is
// even more trivial.
inline void wxMapWindowPoints(HWND hwndFrom, HWND hwndTo, POINT* pt)
{
    ::MapWindowPoints(hwndFrom, hwndTo, pt, 1);
}

// ---------------------------------------------------------------------------
// small helper classes
// ---------------------------------------------------------------------------

// This class can only be used with wxMSW wxWindow, as it doesn't have
// {Set,Get}HWND() methods in the other ports, but this file is currently
// included for wxQt/MSW too. It's not clear whether it should be, really, but
// for now allow it to compile in this port too.
#ifdef __WXMSW__

// Temporarily assign the given HWND to the window in ctor and unset it back to
// the original value (usually 0) in dtor.
class TempHWNDSetter
{
public:
    TempHWNDSetter(wxWindow* win, WXHWND hWnd)
        : m_win(win), m_hWndOrig(m_win->GetHWND())
    {
        m_win->SetHWND(hWnd);
    }

    ~TempHWNDSetter()
    {
        m_win->SetHWND(m_hWndOrig);
    }

private:
    wxWindow* const m_win;
    WXHWND const m_hWndOrig;

    wxDECLARE_NO_COPY_CLASS(TempHWNDSetter);
};

#endif // __WXMSW__

// RAII helper for releasing an HDC in its dtor.
class AutoHDC
{
public:
    ~AutoHDC() { if ( m_hdc ) { ::ReleaseDC(m_hwnd, m_hdc); } }

    operator HDC() const { return m_hdc; }

protected:
    AutoHDC(HWND hwnd, HDC hdc) : m_hwnd(hwnd), m_hdc(hdc) { }

private:
    HWND m_hwnd;
    HDC m_hdc;

    wxDECLARE_NO_COPY_CLASS(AutoHDC);
};

// create an instance of this class and use it as the HDC for screen, will
// automatically release the DC going out of scope
class ScreenHDC : public AutoHDC
{
public:
    ScreenHDC() : AutoHDC(nullptr, ::GetDC(nullptr)) { }
};

// the same as ScreenHDC but for client part of the window (if HWND is null,
// then it's exactly the same as ScreenHDC)
class ClientHDC : public AutoHDC
{
public:
    ClientHDC() : AutoHDC(nullptr, nullptr) { }
    explicit ClientHDC(HWND hwnd) : AutoHDC(hwnd, ::GetDC(hwnd)) { }
};

// same as ClientHDC but includes the non-client part of the window
class WindowHDC : public AutoHDC
{
public:
    explicit WindowHDC(HWND hwnd) : AutoHDC(hwnd, ::GetWindowDC(hwnd)) { }
};

// the same as ScreenHDC but for memory DCs: creates the HDC compatible with
// the given one (screen by default) in ctor and destroys it in dtor
class MemoryHDC
{
public:
    MemoryHDC(HDC hdc = nullptr) { m_hdc = ::CreateCompatibleDC(hdc); }
   ~MemoryHDC() { ::DeleteDC(m_hdc); }

    operator HDC() const { return m_hdc; }

private:
    HDC m_hdc;

    wxDECLARE_NO_COPY_CLASS(MemoryHDC);
};

// Helper function returning the resolution of the given HDC.
inline wxSize wxGetDPIofHDC(HDC hdc)
{
    return wxSize(::GetDeviceCaps(hdc, LOGPIXELSX),
                  ::GetDeviceCaps(hdc, LOGPIXELSY));
}

// a class which selects a GDI object into a DC in its ctor and deselects in
// dtor
class SelectInHDC
{
private:
    void DoInit(HGDIOBJ hgdiobj) { m_hgdiobj = ::SelectObject(m_hdc, hgdiobj); }

public:
    SelectInHDC() : m_hdc(nullptr), m_hgdiobj(nullptr) { }
    SelectInHDC(HDC hdc, HGDIOBJ hgdiobj) : m_hdc(hdc) { DoInit(hgdiobj); }

    void Init(HDC hdc, HGDIOBJ hgdiobj)
    {
        wxASSERT_MSG( !m_hdc, wxT("initializing twice?") );

        m_hdc = hdc;

        DoInit(hgdiobj);
    }

    ~SelectInHDC() { if ( m_hdc ) ::SelectObject(m_hdc, m_hgdiobj); }

    // return true if the object was successfully selected
    operator bool() const { return m_hgdiobj != nullptr; }

private:
    HDC m_hdc;
    HGDIOBJ m_hgdiobj;

    wxDECLARE_NO_COPY_CLASS(SelectInHDC);
};

// a class which cleans up any GDI object
class AutoGDIObject
{
protected:
    AutoGDIObject() { m_gdiobj = nullptr; }
    AutoGDIObject(HGDIOBJ gdiobj) : m_gdiobj(gdiobj) { }
    ~AutoGDIObject() { if ( m_gdiobj ) ::DeleteObject(m_gdiobj); }

    void InitGdiobj(HGDIOBJ gdiobj)
    {
        wxASSERT_MSG( !m_gdiobj, wxT("initializing twice?") );

        m_gdiobj = gdiobj;
    }

    HGDIOBJ GetObject() const { return m_gdiobj; }

private:
    HGDIOBJ m_gdiobj;
};

// TODO: all this asks for using a AutoHandler<T, CreateFunc> template...

// a class for temporary brushes
class AutoHBRUSH : private AutoGDIObject
{
public:
    AutoHBRUSH(COLORREF col)
        : AutoGDIObject(::CreateSolidBrush(col)) { }

    operator HBRUSH() const { return (HBRUSH)GetObject(); }
};

// a class for temporary fonts
class AutoHFONT : private AutoGDIObject
{
private:
public:
    AutoHFONT()
        : AutoGDIObject() { }

    AutoHFONT(const LOGFONT& lf)
        : AutoGDIObject(::CreateFontIndirect(&lf)) { }

    void Init(const LOGFONT& lf) { InitGdiobj(::CreateFontIndirect(&lf)); }

    operator HFONT() const { return (HFONT)GetObject(); }
};

// a class for temporary pens
class AutoHPEN : private AutoGDIObject
{
public:
    AutoHPEN(COLORREF col)
        : AutoGDIObject(::CreatePen(PS_SOLID, 0, col)) { }

    operator HPEN() const { return (HPEN)GetObject(); }
};

// classes for temporary bitmaps
class AutoHBITMAP : private AutoGDIObject
{
public:
    AutoHBITMAP()
        : AutoGDIObject() { }

    AutoHBITMAP(HBITMAP hbmp) : AutoGDIObject(hbmp) { }

    void Init(HBITMAP hbmp) { InitGdiobj(hbmp); }

    operator HBITMAP() const { return (HBITMAP)GetObject(); }
};

class CompatibleBitmap : public AutoHBITMAP
{
public:
    CompatibleBitmap(HDC hdc, int w, int h)
        : AutoHBITMAP(::CreateCompatibleBitmap(hdc, w, h))
    {
    }
};

class MonoBitmap : public AutoHBITMAP
{
public:
    MonoBitmap(int w, int h)
        : AutoHBITMAP(::CreateBitmap(w, h, 1, 1, nullptr))
    {
    }
};

// class automatically destroys the region object
class AutoHRGN : private AutoGDIObject
{
public:
    AutoHRGN(HRGN hrgn) : AutoGDIObject(hrgn) { }

    operator HRGN() const { return (HRGN)GetObject(); }
};

// Class automatically freeing ICONINFO struct fields after retrieving it using
// GetIconInfo().
class AutoIconInfo : public ICONINFO
{
public:
    AutoIconInfo() { wxZeroMemory(*this); }

    bool GetFrom(HICON hIcon)
    {
        if ( !::GetIconInfo(hIcon, this) )
        {
            wxLogLastError(wxT("GetIconInfo"));
            return false;
        }

        return true;
    }

    ~AutoIconInfo()
    {
        if ( hbmColor )
            ::DeleteObject(hbmColor);
        if ( hbmMask )
            ::DeleteObject(hbmMask);
    }
};

// class sets the specified clipping region during its life time
class HDCClipper
{
public:
    HDCClipper(HDC hdc, HRGN hrgn)
        : m_hdc(hdc)
    {
        if ( !::SelectClipRgn(hdc, hrgn) )
        {
            wxLogLastError(wxT("SelectClipRgn"));
        }
    }

    ~HDCClipper()
    {
        ::SelectClipRgn(m_hdc, nullptr);
    }

private:
    HDC m_hdc;

    wxDECLARE_NO_COPY_CLASS(HDCClipper);
};

// set the given map mode for the life time of this object
    class HDCMapModeChanger
    {
    public:
        HDCMapModeChanger(HDC hdc, int mm)
            : m_hdc(hdc)
        {
            m_modeOld = ::SetMapMode(hdc, mm);
            if ( !m_modeOld )
            {
                wxLogLastError(wxT("SelectClipRgn"));
            }
        }

        ~HDCMapModeChanger()
        {
            if ( m_modeOld )
                ::SetMapMode(m_hdc, m_modeOld);
        }

    private:
        HDC m_hdc;
        int m_modeOld;

        wxDECLARE_NO_COPY_CLASS(HDCMapModeChanger);
    };

    #define wxCHANGE_HDC_MAP_MODE(hdc, mm) \
        HDCMapModeChanger wxMAKE_UNIQUE_NAME(wxHDCMapModeChanger)(hdc, mm)

// smart pointer using GlobalAlloc/GlobalFree()
class GlobalPtr
{
public:
    // default ctor, call Init() later
    GlobalPtr()
    {
        m_hGlobal = nullptr;
    }

    // allocates a block of given size
    void Init(size_t size, unsigned flags = GMEM_MOVEABLE)
    {
        m_hGlobal = ::GlobalAlloc(flags, size);
        if ( !m_hGlobal )
        {
            wxLogLastError(wxT("GlobalAlloc"));
        }
    }

    GlobalPtr(size_t size, unsigned flags = GMEM_MOVEABLE)
    {
        Init(size, flags);
    }

    ~GlobalPtr()
    {
        if ( m_hGlobal && ::GlobalFree(m_hGlobal) )
        {
            wxLogLastError(wxT("GlobalFree"));
        }
    }

    // Give ownership of our handle to the caller.
    HGLOBAL Release()
    {
        HGLOBAL h = m_hGlobal;
        m_hGlobal = nullptr;
        return h;
    }

    // implicit conversion
    operator HGLOBAL() const { return m_hGlobal; }

private:
    HGLOBAL m_hGlobal;

    wxDECLARE_NO_COPY_CLASS(GlobalPtr);
};

// when working with global pointers (which is unfortunately still necessary
// sometimes, e.g. for clipboard) it is important to unlock them exactly as
// many times as we lock them which just asks for using a "smart lock" class
class GlobalPtrLock
{
public:
    // default ctor, use Init() later -- should only be used if the HGLOBAL can
    // be null (in which case Init() shouldn't be called)
    GlobalPtrLock()
    {
        m_hGlobal = nullptr;
        m_ptr = nullptr;
    }

    // initialize the object, may be only called if we were created using the
    // default ctor; HGLOBAL must not be null
    void Init(HGLOBAL hGlobal)
    {
        m_hGlobal = hGlobal;

        // NB: GlobalLock() is a macro, not a function, hence don't use the
        //     global scope operator with it (and neither with GlobalUnlock())
        m_ptr = GlobalLock(hGlobal);
        if ( !m_ptr )
        {
            wxLogLastError(wxT("GlobalLock"));
        }
    }

    // initialize the object, HGLOBAL must not be null
    GlobalPtrLock(HGLOBAL hGlobal)
    {
        Init(hGlobal);
    }

    ~GlobalPtrLock()
    {
        if ( m_hGlobal && !GlobalUnlock(m_hGlobal) )
        {
            // this might happen simply because the block became unlocked
            DWORD dwLastError = ::GetLastError();
            if ( dwLastError != NO_ERROR )
            {
                wxLogApiError(wxT("GlobalUnlock"), dwLastError);
            }
        }
    }

    void *Get() const { return m_ptr; }
    operator void *() const { return m_ptr; }

    size_t GetSize() const
    {
        const size_t size = ::GlobalSize(m_hGlobal);
        if ( !size )
            wxLogLastError(wxT("GlobalSize"));

        return size;
    }

private:
    HGLOBAL m_hGlobal;
    void *m_ptr;

    wxDECLARE_NO_COPY_CLASS(GlobalPtrLock);
};

// register the class when it is first needed and unregister it in dtor
class ClassRegistrar
{
public:
    // ctor doesn't register the class, call Initialize() for this
    ClassRegistrar() { m_registered = -1; }

    // return true if the class is already registered
    bool IsInitialized() const { return m_registered != -1; }

    // return true if the class had been already registered
    bool IsRegistered() const { return m_registered == 1; }

    // try to register the class if not done yet, return true on success
    bool Register(const WNDCLASS& wc)
    {
        // we should only be called if we hadn't been initialized yet
        wxASSERT_MSG( m_registered == -1,
                        wxT("calling ClassRegistrar::Register() twice?") );

        m_registered = ::RegisterClass(&wc) ? 1 : 0;
        if ( !IsRegistered() )
        {
            wxLogLastError(wxT("RegisterClassEx()"));
        }
        else
        {
            m_clsname = wc.lpszClassName;
        }

        return m_registered == 1;
    }

    // get the name of the registered class (returns empty string if not
    // registered)
    const wxString& GetName() const { return m_clsname; }

    // unregister the class if it had been registered
    ~ClassRegistrar()
    {
        if ( IsRegistered() )
        {
            if ( !::UnregisterClass(m_clsname.t_str(), wxGetInstance()) )
            {
                wxLogLastError(wxT("UnregisterClass"));
            }
        }
    }

private:
    // initial value is -1 which means that we hadn't tried registering the
    // class yet, it becomes true or false (1 or 0) when Initialize() is called
    int m_registered;

    // the name of the class, only non empty if it had been registered
    wxString m_clsname;
};

// ---------------------------------------------------------------------------
// macros to make casting between WXFOO and FOO a bit easier: the GetFoo()
// returns Foo cast to the Windows type for ourselves, while GetFooOf() takes
// an argument which should be a pointer or reference to the object of the
// corresponding class (this depends on the macro)
// ---------------------------------------------------------------------------

#define GetHwnd()               ((HWND)GetHWND())
#define GetHwndOf(win)          ((HWND)((win)->GetHWND()))
// old name
#define GetWinHwnd              GetHwndOf

#define GetHdc()                ((HDC)GetHDC())
#define GetHdcOf(dc)            ((HDC)(dc).GetHDC())

#define GetHbitmap()            ((HBITMAP)GetHBITMAP())
#define GetHbitmapOf(bmp)       ((HBITMAP)(bmp).GetHBITMAP())

#define GetHicon()              ((HICON)GetHICON())
#define GetHiconOf(icon)        ((HICON)(icon).GetHICON())

#define GetHaccel()             ((HACCEL)GetHACCEL())
#define GetHaccelOf(table)      ((HACCEL)((table).GetHACCEL()))

#define GetHbrush()             ((HBRUSH)GetResourceHandle())
#define GetHbrushOf(brush)      ((HBRUSH)(brush).GetResourceHandle())

#define GetHmenu()              ((HMENU)GetHMenu())
#define GetHmenuOf(menu)        ((HMENU)(menu)->GetHMenu())

#define GetHcursor()            ((HCURSOR)GetHCURSOR())
#define GetHcursorOf(cursor)    ((HCURSOR)(cursor).GetHCURSOR())

#define GetHfont()              ((HFONT)GetHFONT())
#define GetHfontOf(font)        ((HFONT)(font).GetHFONT())

#define GetHimagelist()         ((HIMAGELIST)GetHIMAGELIST())
#define GetHimagelistOf(imgl)   ((HIMAGELIST)(imgl)->GetHIMAGELIST())

#define GetHpalette()           ((HPALETTE)GetHPALETTE())
#define GetHpaletteOf(pal)      ((HPALETTE)(pal).GetHPALETTE())

#define GetHpen()               ((HPEN)GetResourceHandle())
#define GetHpenOf(pen)          ((HPEN)(pen).GetResourceHandle())

#define GetHrgn()               ((HRGN)GetHRGN())
#define GetHrgnOf(rgn)          ((HRGN)(rgn).GetHRGN())

#endif // wxUSE_GUI

// ---------------------------------------------------------------------------
// global functions
// ---------------------------------------------------------------------------

// return the full path of the given module
inline wxString wxGetFullModuleName(HMODULE hmod)
{
    wxString fullname;
    if ( !::GetModuleFileName
            (
                hmod,
                wxStringBuffer(fullname, MAX_PATH),
                MAX_PATH
            ) )
    {
        wxLogLastError(wxT("GetModuleFileName"));
    }

    return fullname;
}

// return the full path of the program file
inline wxString wxGetFullModuleName()
{
    return wxGetFullModuleName((HMODULE)wxGetInstance());
}

// return the run-time version of the OS in a format similar to
// WINVER/_WIN32_WINNT compile-time macros:
//
//      0x0501      Windows XP, 2003
//      0x0502      Windows XP SP2, 2003 SP1
//      0x0600      Windows Vista, 2008
//      0x0601      Windows 7
//      0x0602      Windows 8 (currently also returned for 8.1 if program does not have a manifest indicating 8.1 support)
//      0x0603      Windows 8.1 (currently only returned for 8.1 if program has a manifest indicating 8.1 support)
//      0x1000      Windows 10 (currently only returned for 10 if program has a manifest indicating 10 support)
//
// for the other Windows versions wxWinVersion_Unknown is currently returned.
enum wxWinVersion
{
    wxWinVersion_3 = 0x0300,
    wxWinVersion_NT3 = wxWinVersion_3,

    wxWinVersion_4 = 0x0400,
    wxWinVersion_95 = wxWinVersion_4,
    wxWinVersion_NT4 = wxWinVersion_4,
    wxWinVersion_98 = 0x0410,

    wxWinVersion_5 = 0x0500,
    wxWinVersion_ME = wxWinVersion_5,
    wxWinVersion_NT5 = wxWinVersion_5,
    wxWinVersion_2000 = wxWinVersion_5,
    wxWinVersion_XP = 0x0501,
    wxWinVersion_2003 = 0x0501,
    wxWinVersion_XP_SP2 = 0x0502,
    wxWinVersion_2003_SP1 = 0x0502,

    wxWinVersion_6 = 0x0600,
    wxWinVersion_Vista = wxWinVersion_6,
    wxWinVersion_NT6 = wxWinVersion_6,

    wxWinVersion_7 = 0x601,

    wxWinVersion_8 = 0x602,
    wxWinVersion_8_1 = 0x603,

    wxWinVersion_10 = 0x1000,
    wxWinVersion_11 = 0x1001,

    // Any version we can't recognize will be later than the last currently
    // known one, so give it a value greater than any in the known range.
    wxWinVersion_Unknown = 0x7fff
};

WXDLLIMPEXP_BASE wxWinVersion wxGetWinVersion();

// This is similar to wxSysErrorMsgStr(), but takes an extra HMODULE parameter
// specific to wxMSW.
WXDLLIMPEXP_BASE wxString wxMSWFormatMessage(DWORD nErrCode, HMODULE hModule = 0);

#if wxUSE_GUI && defined(__WXMSW__)

// cursor stuff
extern HCURSOR wxGetCurrentBusyCursor();    // from msw/utils.cpp
extern const wxCursor *wxGetGlobalCursor(); // from msw/cursor.cpp

// GetCursorPos can fail without populating the POINT. This falls back to GetMessagePos.
WXDLLIMPEXP_CORE void wxGetCursorPosMSW(POINT* pt);

#if WXWIN_COMPATIBILITY_3_0
wxDEPRECATED_MSG("Use wxNativeFontInfo::lf directly instead of this private function")
WXDLLIMPEXP_CORE void wxFillLogFont(LOGFONT *logFont, const wxFont *font);
wxDEPRECATED_MSG("Use wxNativeFontInfo(LOGFONT) ctor instead of this private function")
WXDLLIMPEXP_CORE wxFont wxCreateFontFromLogFont(const LOGFONT *logFont);
#endif // WXWIN_COMPATIBILITY_3_0

WXDLLIMPEXP_CORE void wxGetCharSize(WXHWND wnd, int *x, int *y, const wxFont& the_font);
WXDLLIMPEXP_CORE wxFontEncoding wxGetFontEncFromCharSet(int charset);

inline void wxSetWindowFont(HWND hwnd, const wxFont& font)
{
    ::SendMessage(hwnd, WM_SETFONT,
                  (WPARAM)GetHfontOf(font), MAKELPARAM(TRUE, 0));
}

WXDLLIMPEXP_CORE void wxSliderEvent(WXHWND control, WXWORD wParam, WXWORD pos);
WXDLLIMPEXP_CORE void wxScrollBarEvent(WXHWND hbar, WXWORD wParam, WXWORD pos);

// Find maximum size of window/rectangle
extern WXDLLIMPEXP_CORE void wxFindMaxSize(WXHWND hwnd, RECT *rect);

// Safely get the window text (i.e. without using fixed size buffer)
extern WXDLLIMPEXP_CORE wxString wxGetWindowText(WXHWND hWnd);

// get the window class name
extern WXDLLIMPEXP_CORE wxString wxGetWindowClass(WXHWND hWnd);

// get the window id (should be unsigned, hence this is not wxWindowID which
// is, for mainly historical reasons, signed)
extern WXDLLIMPEXP_CORE int wxGetWindowId(WXHWND hWnd);

// check if hWnd's WNDPROC is wndProc. Return true if yes, false if they are
// different
//
// wndProc parameter is unused and only kept for compatibility
extern WXDLLIMPEXP_CORE
bool wxCheckWindowWndProc(WXHWND hWnd, WXWNDPROC wndProc = nullptr);

// Does this window style specify any border?
inline bool wxStyleHasBorder(long style)
{
    return (style & (wxSIMPLE_BORDER | wxRAISED_BORDER |
                     wxSUNKEN_BORDER | wxDOUBLE_BORDER)) != 0;
}

inline bool wxHasWindowExStyle(const wxWindowMSW *win, long style)
{
    return (::GetWindowLong(GetHwndOf(win), GWL_EXSTYLE) & style) != 0;
}

// Common helper of wxUpdate{,Edit}LayoutDirection() below: sets or clears the
// given flag(s) depending on wxLayoutDirection and returns true if the flags
// really changed.
inline bool
wxUpdateExStyleForLayoutDirection(WXHWND hWnd,
                                  wxLayoutDirection dir,
                                  LONG_PTR flagsForRTL)
{
    wxCHECK_MSG( hWnd, false,
                 wxS("Can't set layout direction for invalid window") );

    const LONG_PTR styleOld = ::GetWindowLongPtr(hWnd, GWL_EXSTYLE);

    LONG_PTR styleNew = styleOld;
    switch ( dir )
    {
        case wxLayout_LeftToRight:
            styleNew &= ~flagsForRTL;
            break;

        case wxLayout_RightToLeft:
            styleNew |= flagsForRTL;
            break;

        case wxLayout_Default:
            wxFAIL_MSG(wxS("Invalid layout direction"));
    }

    if ( styleNew == styleOld )
        return false;

    ::SetWindowLongPtr(hWnd, GWL_EXSTYLE, styleNew);

    return true;
}

// Update layout direction flag for a generic window.
//
// See below for the special version that must be used with EDIT controls.
//
// Returns true if the layout direction did change.
inline bool wxUpdateLayoutDirection(WXHWND hWnd, wxLayoutDirection dir)
{
    return wxUpdateExStyleForLayoutDirection(hWnd, dir, WS_EX_LAYOUTRTL);
}

// Update layout direction flag for an EDIT control.
//
// Returns true if anything changed or false if the direction flag was already
// set to the desired direction (which can't be wxLayout_Default).
inline bool wxUpdateEditLayoutDirection(WXHWND hWnd, wxLayoutDirection dir)
{
    return wxUpdateExStyleForLayoutDirection(hWnd, dir,
                                             WS_EX_RIGHT |
                                             WS_EX_RTLREADING |
                                             WS_EX_LEFTSCROLLBAR);
}

// Companion of the above function checking if an EDIT control uses RTL.
inline wxLayoutDirection wxGetEditLayoutDirection(WXHWND hWnd)
{
    wxCHECK_MSG( hWnd, wxLayout_Default, wxS("invalid window") );

    // While we set 3 style bits above, we're only really interested in one of
    // them here. In particularly, don't check for WS_EX_RIGHT as it can be set
    // for a right-aligned control even if it doesn't use RTL. And while we
    // could test WS_EX_LEFTSCROLLBAR, this doesn't really seem useful.
    const LONG_PTR style = ::GetWindowLongPtr(hWnd, GWL_EXSTYLE);

    return style & WS_EX_RTLREADING ? wxLayout_RightToLeft
                                    : wxLayout_LeftToRight;
}

// ----------------------------------------------------------------------------
// functions mapping HWND to wxWindow
// ----------------------------------------------------------------------------

// this function simply checks whether the given hwnd corresponds to a wxWindow
// and returns either that window if it does or nullptr otherwise
extern WXDLLIMPEXP_CORE wxWindow* wxFindWinFromHandle(HWND hwnd);

// find the window for HWND which is part of some wxWindow, i.e. unlike
// wxFindWinFromHandle() above it will also work for "sub controls" of a
// wxWindow.
//
// returns the wxWindow corresponding to the given HWND or nullptr.
extern WXDLLIMPEXP_CORE wxWindow *wxGetWindowFromHWND(WXHWND hwnd);

// Get the size of an icon
extern WXDLLIMPEXP_CORE wxSize wxGetHiconSize(HICON hicon);

WXDLLIMPEXP_CORE void wxDrawLine(HDC hdc, int x1, int y1, int x2, int y2);

WXDLLIMPEXP_CORE void wxDrawHVLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color, int width);

// fill the client rect of the given window on the provided dc using this brush
inline void wxFillRect(HWND hwnd, HDC hdc, HBRUSH hbr)
{
    RECT rc;
    ::GetClientRect(hwnd, &rc);
    ::FillRect(hdc, &rc, hbr);
}

// ----------------------------------------------------------------------------
// 32/64 bit helpers
// ----------------------------------------------------------------------------

// note that the casts to LONG_PTR here are required even on 32-bit machines
// for the 64-bit warning mode of later versions of MSVC (C4311/4312)
inline WNDPROC wxGetWindowProc(HWND hwnd)
{
    return (WNDPROC)(LONG_PTR)::GetWindowLongPtr(hwnd, GWLP_WNDPROC);
}

inline void *wxGetWindowUserData(HWND hwnd)
{
    return (void *)(LONG_PTR)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
}

inline WNDPROC wxSetWindowProc(HWND hwnd, WNDPROC func)
{
    return (WNDPROC)(LONG_PTR)::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)func);
}

inline void *wxSetWindowUserData(HWND hwnd, void *data)
{
    return (void *)(LONG_PTR)::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);
}

#endif // wxUSE_GUI && __WXMSW__

#endif // _WX_PRIVATE_H_
