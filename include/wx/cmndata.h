/////////////////////////////////////////////////////////////////////////////
// Name:        wx/cmndata.h
// Purpose:     Common GDI data classes
// Author:      Julian Smart and others
// Created:     01/02/97
// Copyright:   (c)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CMNDATA_H_BASE_
#define _WX_CMNDATA_H_BASE_

#include "wx/defs.h"

#if wxUSE_PRINTING_ARCHITECTURE

#include "wx/gdicmn.h"

#if wxUSE_STREAMS
#include "wx/stream.h"
#endif

#include <vector>

class WXDLLIMPEXP_FWD_CORE wxPrintNativeDataBase;

/*
 * wxPrintData
 * Encapsulates printer information (not printer dialog information)
 */

enum wxPrintBin
{
    wxPRINTBIN_DEFAULT,

    wxPRINTBIN_ONLYONE,
    wxPRINTBIN_LOWER,
    wxPRINTBIN_MIDDLE,
    wxPRINTBIN_MANUAL,
    wxPRINTBIN_ENVELOPE,
    wxPRINTBIN_ENVMANUAL,
    wxPRINTBIN_AUTO,
    wxPRINTBIN_TRACTOR,
    wxPRINTBIN_SMALLFMT,
    wxPRINTBIN_LARGEFMT,
    wxPRINTBIN_LARGECAPACITY,
    wxPRINTBIN_CASSETTE,
    wxPRINTBIN_FORMSOURCE,

    wxPRINTBIN_USER
};

const int wxPRINTMEDIA_DEFAULT = 0;

class WXDLLIMPEXP_CORE wxPrintData: public wxObject
{
public:
    wxPrintData();
    wxPrintData(const wxPrintData& printData);
    virtual ~wxPrintData();

    int GetNoCopies() const { return m_printNoCopies; }
    bool GetCollate() const { return m_printCollate; }
    wxPrintOrientation GetOrientation() const { return m_printOrientation; }
    bool IsOrientationReversed() const { return m_printOrientationReversed; }

    // Is this data OK for showing the print dialog?
    bool Ok() const { return IsOk(); }
    bool IsOk() const ;

    const wxString& GetPrinterName() const { return m_printerName; }
    bool GetColour() const { return m_colour; }
    wxDuplexMode GetDuplex() const { return m_duplexMode; }
    wxPaperSize GetPaperId() const { return m_paperId; }
    const wxSize& GetPaperSize() const { return m_paperSize; }
    wxPrintQuality GetQuality() const { return m_printQuality; }
    wxPrintBin GetBin() const { return m_bin; }
    wxPrintMode GetPrintMode() const { return m_printMode; }
    int GetMedia() const { return m_media; }

    void SetNoCopies(int v) { m_printNoCopies = v; }
    void SetCollate(bool flag) { m_printCollate = flag; }

    // Please use the overloaded method below
    wxDEPRECATED_INLINE(void SetOrientation(int orient),
                        m_printOrientation = (wxPrintOrientation)orient; )
    void SetOrientation(wxPrintOrientation orient) { m_printOrientation = orient; }
    void SetOrientationReversed(bool reversed) { m_printOrientationReversed = reversed; }

    void SetPrinterName(const wxString& name) { m_printerName = name; }
    void SetColour(bool colour) { m_colour = colour; }
    void SetDuplex(wxDuplexMode duplex) { m_duplexMode = duplex; }
    void SetPaperId(wxPaperSize sizeId) { m_paperId = sizeId; }
    void SetPaperSize(const wxSize& sz) { m_paperSize = sz; }
    void SetQuality(wxPrintQuality quality) { m_printQuality = quality; }
    void SetBin(wxPrintBin bin) { m_bin = bin; }
    void SetMedia(int media) { m_media = media; }
    void SetPrintMode(wxPrintMode printMode) { m_printMode = printMode; }

    wxString GetFilename() const { return m_filename; }
    void SetFilename( const wxString &filename ) { m_filename = filename; }

    wxPrintData& operator=(const wxPrintData& data);

    char* GetPrivData() { return m_privData.empty() ? nullptr : &m_privData[0]; }
    const char* GetPrivData() const { return m_privData.empty() ? nullptr : &m_privData[0]; }
    int GetPrivDataLen() const { return wxSsize(m_privData); }
    void SetPrivData( char *privData, int len );


    // Convert between wxPrintData and native data
    void ConvertToNative();
    void ConvertFromNative();
    // Holds the native print data
    wxPrintNativeDataBase *GetNativeData() const { return m_nativeData.get(); }

private:
    wxPrintBin      m_bin = wxPRINTBIN_DEFAULT;
    int             m_media = wxPRINTMEDIA_DEFAULT;
    wxPrintMode     m_printMode = wxPRINT_MODE_PRINTER;

    int             m_printNoCopies = 1;
    wxPrintOrientation m_printOrientation = wxPORTRAIT;
    bool            m_printOrientationReversed = false;
    bool            m_printCollate = false;

    wxString        m_printerName;
    bool            m_colour = true;
    wxDuplexMode    m_duplexMode = wxDUPLEX_SIMPLEX;
    wxPrintQuality  m_printQuality = wxPRINT_QUALITY_HIGH;

    // we intentionally don't initialize paper id and size at all, like this
    // the default system settings will be used for them
    wxPaperSize     m_paperId = wxPAPER_NONE;
    wxSize          m_paperSize = wxDefaultSize;

    wxString        m_filename;

    std::vector<char> m_privData;

    wxObjectDataPtr<wxPrintNativeDataBase> m_nativeData;

private:
    wxDECLARE_DYNAMIC_CLASS(wxPrintData);
};

/*
 * wxPrintPageRange
 * Defines a range of pages to be printed.
 */

class wxPrintPageRange
{
public:
    wxPrintPageRange() = default;
    wxPrintPageRange(int from, int to) : fromPage(from), toPage(to)
    {
        wxASSERT_MSG( IsValid(), "Invalid page range values" );
    }

    // check if both components are set/initialized correctly
    bool IsValid() const { return fromPage > 0 && fromPage <= toPage; }

    // get the number of pages in the range
    int GetNumberOfPages() const { return toPage - fromPage + 1; }

    int fromPage = 0;
    int toPage = 0;
};

using wxPrintPageRanges = std::vector<wxPrintPageRange>;

/*
 * wxPrintDialogData
 * Encapsulates information displayed and edited in the printer dialog box.
 * Contains a wxPrintData object which is filled in according to the values retrieved
 * from the dialog.
 */

class WXDLLIMPEXP_CORE wxPrintDialogData: public wxObject
{
public:
    wxPrintDialogData();
    wxPrintDialogData(const wxPrintDialogData& dialogData) = default;
    wxPrintDialogData(const wxPrintData& printData);
    virtual ~wxPrintDialogData();

    // These functions can be used only when printing all pages or a single
    // continuous range of pages, use GetPageRanges() to support multiple
    // ranges.
    int GetFromPage() const;
    int GetToPage() const;

    int GetMinPage() const { return m_printMinPage; }
    int GetMaxPage() const { return m_printMaxPage; }
    int GetNoCopies() const { return m_printNoCopies; }
    bool GetAllPages() const { return m_printWhat == Print::AllPages; }
    bool GetSelection() const { return m_printWhat == Print::Selection; }
    bool GetCurrentPage() const { return m_printWhat == Print::CurrentPage; }
    bool GetSpecifiedPages() const { return m_printWhat == Print::SpecifiedPages; }
    bool GetCollate() const { return m_printCollate; }
    bool GetPrintToFile() const { return m_printToFile; }

    // Similarly to the getters above, these functions can be used only to
    // define a single continuous range of pages to print, use SetPageRanges()
    // for anything else. Note that if you do use SetPageRanges() to specify
    // multiple ranges, then these functions cannot be used.
    void SetFromPage(int v);
    void SetToPage(int v);

    void SetMinPage(int v) { m_printMinPage = v; }
    void SetMaxPage(int v) { m_printMaxPage = v; }
    void SetNoCopies(int v) { m_printNoCopies = v; }

    // Avoid calling these functions with flag == false as it's not really
    // obvious what they do in this case.
    void SetAllPages(bool flag = true) { DoSetWhat(Print::AllPages, flag); }
    void SetSelection(bool flag = true) { DoSetWhat(Print::Selection, flag); }
    void SetCurrentPage(bool flag = true) { DoSetWhat(Print::CurrentPage, flag); }

    void SetCollate(bool flag) { m_printCollate = flag; }
    void SetPrintToFile(bool flag) { m_printToFile = flag; }

    void EnablePrintToFile(bool flag) { m_printEnablePrintToFile = flag; }
    void EnableSelection(bool flag) { m_printEnableSelection = flag; }
    void EnableCurrentPage(bool flag) { m_printEnableCurrentPage = flag; }
    void EnablePageNumbers(bool flag) { m_printEnablePageNumbers = flag; }
    void EnableHelp(bool flag) { m_printEnableHelp = flag; }

    bool GetEnablePrintToFile() const { return m_printEnablePrintToFile; }
    bool GetEnableSelection() const { return m_printEnableSelection; }
    bool GetEnableCurrentPage() const { return m_printEnableCurrentPage; }
    bool GetEnablePageNumbers() const { return m_printEnablePageNumbers; }
    bool GetEnableHelp() const { return m_printEnableHelp; }

    // Is this data OK for showing the print dialog?
    bool Ok() const { return IsOk(); }
    bool IsOk() const { return m_printData.IsOk() ; }

    wxPrintData& GetPrintData() { return m_printData; }
    void SetPrintData(const wxPrintData& printData) { m_printData = printData; }

    void SetPageRanges(const wxPrintPageRanges& pageRanges) { m_printPageRanges = pageRanges; }
    const wxPrintPageRanges& GetPageRanges() const { return m_printPageRanges; }

    void SetMaxPageRanges(int maxPageRanges) { m_maxPageRanges = maxPageRanges; }
    int GetMaxPageRanges() const { return m_maxPageRanges; }

    wxPrintDialogData& operator=(const wxPrintDialogData& data) = default;
    void operator=(const wxPrintData& data); // Sets internal m_printData member

private:
    enum class Print
    {
        SpecifiedPages, // Default used if none of the other flags are selected.
        AllPages,
        Selection,
        CurrentPage
    };

    void DoSetWhat(Print what, bool flag);

    Print           m_printWhat = Print::AllPages;

    int             m_printMinPage = 0;
    int             m_printMaxPage = 0;
    int             m_printNoCopies = 1;

    bool            m_printCollate = false;
    bool            m_printToFile = false;
    bool            m_printEnableSelection = false;
    bool            m_printEnableCurrentPage = false;
    bool            m_printEnablePageNumbers = true;
    bool            m_printEnableHelp = false;
    bool            m_printEnablePrintToFile = true;
    wxPrintData     m_printData;

    // Maximum number of page ranges that the user can specify via the print dialog.
    int m_maxPageRanges = 64;

    // The page ranges to print. If this vector contains more then m_maxPageRanges
    // elements, then the maximum number of page ranges that the user can specify
    // via the print dialog is the size of this vector.
    wxPrintPageRanges m_printPageRanges;

private:
    wxDECLARE_DYNAMIC_CLASS(wxPrintDialogData);
};

/*
* This is the data used (and returned) by the wxPageSetupDialog.
*/

// Compatibility with old name
#define wxPageSetupData wxPageSetupDialogData

class WXDLLIMPEXP_CORE wxPageSetupDialogData: public wxObject
{
public:
    wxPageSetupDialogData();
    wxPageSetupDialogData(const wxPageSetupDialogData& dialogData) = default;
    wxPageSetupDialogData(const wxPrintData& printData);

    wxSize GetPaperSize() const { return m_paperSize; }
    wxPaperSize GetPaperId() const { return m_printData.GetPaperId(); }
    wxPoint GetMinMarginTopLeft() const { return m_minMarginTopLeft; }
    wxPoint GetMinMarginBottomRight() const { return m_minMarginBottomRight; }
    wxPoint GetMarginTopLeft() const { return m_marginTopLeft; }
    wxPoint GetMarginBottomRight() const { return m_marginBottomRight; }

    bool GetDefaultMinMargins() const { return m_defaultMinMargins; }
    bool GetEnableMargins() const { return m_enableMargins; }
    bool GetEnableOrientation() const { return m_enableOrientation; }
    bool GetEnablePaper() const { return m_enablePaper; }
    bool GetEnablePrinter() const { return m_enablePrinter; }
    bool GetDefaultInfo() const { return m_getDefaultInfo; }
    bool GetEnableHelp() const { return m_enableHelp; }

    // Is this data OK for showing the page setup dialog?
    bool Ok() const { return IsOk(); }
    bool IsOk() const { return m_printData.IsOk() ; }

    // If a corresponding paper type is found in the paper database, will set the m_printData
    // paper size id member as well.
    void SetPaperSize(const wxSize& sz);

    void SetPaperId(wxPaperSize id) { m_printData.SetPaperId(id); }

    // Sets the wxPrintData id, plus the paper width/height if found in the paper database.
    void SetPaperSize(wxPaperSize id);

    void SetMinMarginTopLeft(const wxPoint& pt) { m_minMarginTopLeft = pt; }
    void SetMinMarginBottomRight(const wxPoint& pt) { m_minMarginBottomRight = pt; }
    void SetMarginTopLeft(const wxPoint& pt) { m_marginTopLeft = pt; }
    void SetMarginBottomRight(const wxPoint& pt) { m_marginBottomRight = pt; }
    void SetDefaultMinMargins(bool flag) { m_defaultMinMargins = flag; }
    void SetDefaultInfo(bool flag) { m_getDefaultInfo = flag; }

    void EnableMargins(bool flag) { m_enableMargins = flag; }
    void EnableOrientation(bool flag) { m_enableOrientation = flag; }
    void EnablePaper(bool flag) { m_enablePaper = flag; }
    void EnablePrinter(bool flag) { m_enablePrinter = flag; }
    void EnableHelp(bool flag) { m_enableHelp = flag; }

    // Use paper size defined in this object to set the wxPrintData
    // paper id
    void CalculateIdFromPaperSize();

    // Use paper id in wxPrintData to set this object's paper size
    void CalculatePaperSizeFromId();

    wxPageSetupDialogData& operator=(const wxPageSetupDialogData& data) = default;
    wxPageSetupDialogData& operator=(const wxPrintData& data);

    wxPrintData& GetPrintData() { return m_printData; }
    const wxPrintData& GetPrintData() const { return m_printData; }
    void SetPrintData(const wxPrintData& printData);

private:
    wxSize          m_paperSize; // The dimensions selected by the user (on return, same as in wxPrintData?)
    wxPoint         m_minMarginTopLeft;
    wxPoint         m_minMarginBottomRight;
    wxPoint         m_marginTopLeft;
    wxPoint         m_marginBottomRight;
    bool            m_defaultMinMargins = false;
    bool            m_enableMargins = true;
    bool            m_enableOrientation = true;
    bool            m_enablePaper = true;
    bool            m_enablePrinter = true;
    bool            m_getDefaultInfo = false; // Equiv. to PSD_RETURNDEFAULT
    bool            m_enableHelp = false;
    wxPrintData     m_printData;

private:
    wxDECLARE_DYNAMIC_CLASS(wxPageSetupDialogData);
};

#endif // wxUSE_PRINTING_ARCHITECTURE

#endif
// _WX_CMNDATA_H_BASE_
