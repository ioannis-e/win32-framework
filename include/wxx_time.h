// Win32++   Version 9.5.2
// Release Date: TBA
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//           https://github.com/DavidNash2024/Win32xx
//
//
// Copyright (c) 2005-2024  David Nash
//
// Permission is hereby granted, free of charge, to
// any person obtaining a copy of this software and
// associated documentation files (the "Software"),
// to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify,
// merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice
// shall be included in all copies or substantial portions
// of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.
//
////////////////////////////////////////////////////////


////////////////////////////////////////////////////////
// Acknowledgement:
//
// The original author of CTime and CTimeSpan is:
//
//      Robert C. Tausworthe
//      email: robert.c.tausworthe@ieee.org
//
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
//
//  Declaration of the CTime class.
//
////////////////////////////////////////////////////////

#ifndef _WIN32XX_CTIME_H_
#define _WIN32XX_CTIME_H_

#include "wxx_wincore.h"
#include <errno.h>
#include <time.h>


//  The CTime class is based internally on the time_t data type, which is
//  measured in seconds past the January 1, 00:00:00 1970 UTC epoch. Thus,
//  any given time_t instant is strictly a UTC epoch, which may then be
//  rendered, using standard C library functions, as either a UTC or local
//  time.  Local instants may further appear as standard or daylight times.
//  Thus, the base epoch on a system in the PST zone will be displayed as
//  December 31, 1969 16:00:00. Any attempt to construct or evaluate a date
//  before this epoch will assert or be in error. Even though the time_t
//  type is implemented (in MinGW, anyway) as a signed integer type,
//  negative values are not allowed.

//  On systems where time_t is defined as a 32-bit integer, there is an
//  upper date limit of January 18, 19:14:07, 2038. On 64-bit systems,
//  there is no such upper date limit. This issue can affect code compiled
//  with Visual Studio 6 and Visual Studio 2003.

//  Windows also has other time types that also interface with the CTime
//  type. These are FILETIME, SYSTEMTIME, and MS-DOS date and time, each
//  of which are described in the MSDN API documentation. Basically:

//  FILETIME is a structure containing a 64-bit value representing the
//  number of 100-nanosecond intervals since January 1, 1601 UTC.

//  SYSTEMTIME is a structure representing a date and  time using individual
//  values for the month, day, year, weekday, hour, minute, second, and
//  millisecond. This is not useful in itself, but must be translated into
//  a FILETIME or time_t.

//  MS-DOS date and time are two 16-bit words packed with day, month, year
//  and second, minute, hour fields.  The format may be found in the MSDN
//  Library article on DosDateTimeToFileTime().


namespace Win32xx
{
    // Define the time_tm type.
    typedef struct tm time_tm;

    // Define the timespane_t type.
    // This can be int or __int64 depending on the compiler.
    typedef time_t timespan_t;

    // Forward declaration.
    class CTimeSpan;


    ////////////////////////////////////////////////////////
    // The CTime class represents an absolute time and date.
    class CTime
    {
    public:
        // Constructors.
        CTime();
        CTime(const CTime& t);
        CTime(time_t t);
        CTime(time_tm& t);
        CTime(int yr, int mo, int wkday, int nthwk, int hr, int min, int sec, int isDST);
        CTime(int year, int month, int day, int hour, int min, int sec, int isDST = -1);
        CTime(int yr, int doy, int hr, int min, int sec);
        CTime(WORD dosDate, WORD dosTime, int isDST = -1);
        CTime(const SYSTEMTIME& st, int isDST = -1);
        CTime(const FILETIME& ft,  int isDST = -1);

        // CString conversion.
        CString     Format(LPCTSTR format) const;
        CString     Format(UINT formatID) const;
        CString     FormatGmt(LPCTSTR format) const;
        CString     FormatGmt(UINT formatID) const;

        // Method members.
        bool      GetAsFileTime(FILETIME& ft) const;
        bool      GetAsSystemTime(SYSTEMTIME& st) const;
        int       GetDay(bool local = true) const;
        int       GetDayOfWeek(bool local = true) const;
        int       GetDayOfYear(bool local = true) const;
        time_tm*  GetGmtTm(time_tm* ptm) const;
        int       GetHour(bool local = true) const;
        time_tm*  GetLocalTm(time_tm* ptm) const;
        int       GetMinute(bool local = true) const;
        int       GetMonth(bool local = true) const;
        int       GetSecond(bool local = true) const;
        int       GetYear(bool local = true) const;
        time_t    GetTime() const;

        // Assignment operators.
        CTime&  operator=(const CTime& timeSrc);
        CTime&  operator=(const time_t& t);

        // Computational operators.
        CTime&  operator+=(const CTimeSpan& ts);
        CTime&  operator-=(const CTimeSpan& ts);
        const CTimeSpan operator-(const CTime& t) const;
        const CTime operator-(const CTimeSpan& ts) const;
        const CTime operator+(const CTimeSpan& ts) const;
        bool        operator==(const CTime& t) const;
        bool        operator!=(const CTime& t) const;
        bool        operator<(const CTime& time) const;
        bool        operator>(const CTime& time) const;
        bool        operator<=(const CTime& time) const;
        bool        operator>=(const CTime& time) const;

        operator time_t() const { return m_time; }

        // Static methods
        static  CTime   GetCurrentTime();

    private:
        // Private data members.
        time_t      m_time;
    };


    ////////////////////////////////////////////////////////////
    // The CTimeSpan class defines the data type for differences
    // between two CTime values, measured in seconds of time.
    class CTimeSpan
    {
        // Global friends.
        friend class CTime;

    public:
        // Constructors.
        CTimeSpan();
        CTimeSpan(timespan_t t);
        CTimeSpan(long days, int hours, int mins, int secs);
        CTimeSpan(const CTimeSpan& ts);

        // CString conversion.
        CString     Format(LPCTSTR format) const;
        CString     Format(UINT formatID) const;

        // Methods to extract items.
        LONGLONG    GetDays() const;
        LONGLONG    GetTotalHours() const;
        int         GetHours() const;
        LONGLONG    GetTotalMinutes() const;
        int         GetMinutes() const;
        LONGLONG    GetTotalSeconds() const;
        int         GetSeconds() const;

        // Assignment operators.
        CTimeSpan& operator=(const CTimeSpan& ts);
        CTimeSpan& operator=(const timespan_t& t);

        // Computational operators.
        const CTimeSpan operator-() const;
        const CTimeSpan operator-(CTimeSpan& ts) const;
        const CTimeSpan operator+(CTimeSpan& ts) const;
        CTimeSpan& operator+=(CTimeSpan& ts);
        CTimeSpan& operator-=(CTimeSpan& ts);
        bool       operator==(const CTimeSpan& ts) const;
        bool       operator!=(const CTimeSpan& ts) const;
        bool       operator<(const CTimeSpan& ts) const;
        bool       operator>(const CTimeSpan& ts) const;
        bool       operator<=(const CTimeSpan& ts) const;
        bool       operator>=(const CTimeSpan& ts) const;

        operator timespan_t() const { return m_timespan; }

    private:
        // Private data members.
        timespan_t m_timespan;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    /////////////////////////////////////////////////
    // Global functions within the Win32xx namespace.
    //

    // Calls either ::gmtime or ::gmtime_s, depending on the compiler.
    // The value of tm is updated and its pointer is returned if successful.
    // Returns NULL on failure.
    inline time_tm* GMTime(time_tm& tm, const time_t& timet)
    {
        time_tm* ptm = &tm;

// If Visual Studio >= 2005 or GNU > 11 or clang compiler.
#if ((defined (_MSC_VER) &&  ( _MSC_VER >= 1400 )) || \
    (defined(__GNUC__) && (__GNUC__ >= 11)) || \
    defined(__clang_major__))

        if (::gmtime_s(&tm, &timet) != 0)
            ptm = NULL;
#else
        time_tm* ptmTemp = ::gmtime(&timet);
        if (ptmTemp != NULL)
            *ptm = *ptmTemp;  // Update the value of tm.
        else
            ptm = NULL;
#endif

        // Note: ptm points to tm (which isn't a local variable) or NULL. 
        return ptm;
    }

    // Calls either ::localtime or ::localtime_s depending on the compiler.
    // The value of tm is updated and its pointer is returned if successful.
    // Returns NULL on failure.
    inline time_tm* LocalTime(time_tm& tm, const time_t& timet)
    {
        time_tm* ptm = &tm;

// If Visual Studio >= 2005 or GNU > 11 or clang compiler.
#if ((defined (_MSC_VER) &&  ( _MSC_VER >= 1400 )) || \
    (defined(__GNUC__) && (__GNUC__ >= 11)) || \
    defined(__clang_major__))

        if (::localtime_s(&tm, &timet) != 0)
            ptm = NULL;
#else
        time_tm* ptmTemp = ::localtime(&timet);
        if (ptmTemp != NULL)
            *ptm = *ptmTemp;  // Update the value of tm.
        else
            ptm = NULL;
#endif

        // Note: ptm points to tm (which isn't a local variable) or NULL.
        return ptm;
    }

    // Returns the time_t t corresponding to the date given in atm as a UTC time.
    inline time_t UTCtime(time_tm* atm)
    {
        // Compute the local time from atm.
        assert(atm != NULL);
        time_t t0 = ::mktime(atm);
        assert(t0 != -1);
        time_tm tm0;
        time_tm* ptm0 = GMTime(tm0, t0);

        time_t t1 = ::mktime(ptm0);
        assert(t1 != -1);
        timespan_t zt = static_cast<timespan_t>(t0 - t1);
        t0 += zt;

        assert(GMTime(tm0, t0));
        return t0;
    }

    // Writes the time t into the archive file.
    // Throw an exception on failure.
    inline CArchive& operator<<(CArchive& ar, CTime& t)
    {
        ULONGLONG tx64 = 0;
        UINT size = sizeof(tx64);

        // Store CTime as x64.
        time_t tt = t;
        tx64 = static_cast<ULONGLONG>(tt);
        ar.Write(&tx64, size);
        return ar;
    }

    // Reads a CTime from the archive and stores it in t.
    // Throws an exception on failure.
    inline CArchive& operator>>(CArchive& ar, CTime& t)
    {
        UINT size;
        ar.Read(&size, sizeof(size));
        if (size != sizeof(ULONGLONG))
        {
            CString str = ar.GetFile().GetFilePath();
            throw CFileException(str, GetApp()->MsgArReadFail());
        }

        // Load CTime as x64.
        ULONGLONG tx64 = 0;
        ar.Read(&tx64, size);
        time_t tt = static_cast<time_t>(tx64);
        t = CTime(tt);
        return ar;
    }

    // Writes the time span object ts into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& operator<<(CArchive& ar, CTimeSpan& ts)
    {
        ULONGLONG tsx64 = 0;
        UINT size = sizeof(tsx64);
        ar.Write(&size, sizeof(size));

        // Store CTimeSpan as x64.
        tsx64 = static_cast<ULONGLONG>(ts);
        ar.Write(&tsx64, size);
        return ar;
    }

    // Reads a CTimeSpan object from the archive and store it in ts.
    // Throws an exception if an error occurs.
    inline CArchive& operator>>(CArchive& ar, CTimeSpan& ts)
    {
        UINT size;
        ar.Read(&size, sizeof(size));
        if (size != sizeof(ULONGLONG))
        {
            CString str = ar.GetFile().GetFilePath();
            throw CFileException(str, GetApp()->MsgArReadFail());
        }

        // Load CTimeSpan as x64.
        ULONGLONG tsx64 = 0;
        ar.Read(&tsx64, size);
        timespan_t tst = static_cast<timespan_t>(tsx64);
        ts = CTimeSpan(tst);
        return ar;
    }


    ///////////////////////////////////
    // Definitions for the CTime class.
    //

    // Constructs an CTime object initialized to the Jan 1, 1970 00:00:00 epoch.
    inline CTime::CTime()
    {
        m_time = 0;
    }

    // Constructs a CTime object from another (valid) CTime object t.
    inline CTime::CTime(const CTime& t)
    {
        m_time = t.m_time;
    }

    // Constructs a CTime object from the time_t value t, or assert if t is invalid.
    inline CTime::CTime(time_t t)
    {
        time_tm tm;
        VERIFY(GMTime(tm, t));
        m_time = t;
    }

    // Constructs a CTime object from the time_tm atm, or assert if atm is invalid.
    inline CTime::CTime(time_tm& atm)
    {
        // Compute the object time_t.
        m_time = ::mktime(&atm);

        // Check for acceptable range.
        assert(m_time != -1);
    }

    // Constructs a CTime of the nthwk occurrence of the given wkday (0..6)
    // in the mo month of yr year, at hr:min:sec of that day, local time.
    inline CTime::CTime(int yr, int mo, int wkday, int nthwk, int hr,
                        int min, int sec, int isDST)
    {
        // Validate parameters w.r.t. ranges.
        assert(yr >= 1969); // Last few hours of 1969 might be a valid local time.
        assert(wkday <= 6);
        assert(1 <= mo && mo <= 12);

        // This computation is tricky because adding whole days to a time_t
        // may result in date within the DST zone, which, when rendered into
        // calendar date form, appears off by the daylight bias. Rather, we
        // need to work in UTC calendar days and  add integer calendar days to
        // the first-of-month epoch in the given year to yield the desired
        // date.  To start, compute the first of the month in the given year
        // at the given hour, minute, and  second.
        time_tm atm = {sec, min, hr, 1, mo - 1, yr - 1900, 0, 0, isDST};

        // Get the (valid) local time of the UTC time corresponding to this.
        time_t t1st = UTCtime(&atm);

        // Recover the day of the week.
        time_tm tm1;
        time_tm* ptm1 = GMTime(tm1, t1st);

        // Compute number of days until the nthwk occurrence of wkday.
        time_t nthwkday = (7 + time_t(wkday) - ptm1->tm_wday) % 7 + time_t(nthwk - 1) * 7;

        // Add this to the first of the month.
        time_t sec_per_day = 86400;
        time_t tnthwkdy = t1st + nthwkday * sec_per_day;

        VERIFY(GMTime(tm1, tnthwkdy));

        // Compute the object time_t.
        m_time = ::mktime(ptm1);
        assert(m_time != -1);
    }

    // Constructs a CTime object from local time elements. Each element is
    // constrained to lie within the following UTC ranges:
    //   year       1970-2038 (on 32-bit systems)
    //   month      1-12
    //   day        1-31
    //   hour, min, sec no constraint
    inline CTime::CTime(int year, int month, int day, int hour, int min,
                        int sec, int isDST /* = -1*/)
    {
        // Validate parameters w.r.t. ranges.
        assert(1 <= day && day   <= 31);
        assert(1 <= month && month <= 12);
        assert(year >= 1969);  // Last few hours of 1969 might be a valid local time.

        // Fill out a time_tm with the calendar date.
        time_tm atm = {sec, min, hour, day, month - 1, year - 1900, 0, 0, isDST};

        // Compute the object time_t.
        m_time = ::mktime(&atm);
        assert(m_time != -1);
    }

    // Constructs a CTime using the day-of-year doy,
    // where doy = 1 is January 1 in the specified year.
    inline CTime::CTime(int yr, int doy, int hr, int min, int sec)
    {
        // Validate parameters w.r.t. ranges.
        assert(yr >= 1969);  // Last few hours of 1969 might be a valid local time.

        // Fill out a time_tm with the calendar date for Jan 1, yr, hr:min:sec.
        int isDST = -1;
        time_tm atm1st = {sec, min, hr, 1, 0, yr - 1900, 0, 0, isDST};

        // Get the local time of the UTC time corresponding to this.
        time_t Jan1 = UTCtime(&atm1st);
        time_t sec_per_day = 86400;
        time_t tDoy = Jan1 + doy * sec_per_day - sec_per_day;
        time_tm* ptm = GMTime(atm1st, tDoy);
        assert(ptm);

        // Compute the object time_t.
        m_time = ::mktime(ptm);
        assert(m_time != -1);
    }

    // Constructs a CTime object from the MS-DOS dosDate and dosTime values.
    inline CTime::CTime(WORD dosDate, WORD dosTime, int isDST /* = -1 */)
    {
        FILETIME ft;
        VERIFY( ::DosDateTimeToFileTime(dosDate, dosTime, &ft) );
        CTime t(ft, isDST);
        m_time = t.m_time;
    }

    // Constructs a CTime object from a SYSTEMTIME structure st.
    inline CTime::CTime(const SYSTEMTIME& st, int isDST /* = -1 */)
    {
        CTime t(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute,
                st.wSecond, isDST);   // Asserts if invalid.

        m_time = t.m_time;
    }

    // Constructs a CTime object from a (UTC) FILETIME structure ft.
    inline CTime::CTime(const FILETIME& ft, int isDST /* = -1 */)
    {
        // Convert ft (a UTC time) to local time.
        FILETIME localTime;
        VERIFY( ::FileTimeToLocalFileTime(&ft, &localTime) );

        // Convert localTime to a SYSTEMTIME structure.
        SYSTEMTIME st;
        VERIFY( ::FileTimeToSystemTime(&localTime, &st) );

        // Convert the system time to a CTime.
        CTime t(st, isDST);  // Asserts if invalid.
        m_time = t.m_time;
    }

    // Returns a CString that contains formatted time. The format parameter
    // is a formatting string similar to the printf formatting string.
    // The valid format directives are
    //   %D - number of days
    //   %H - hour (0-23)
    //   %M - minute (0-59)
    //   %S - seconds (0-59)
    inline CString CTime::Format(LPCTSTR format) const
    {
        time_tm tm;
        time_tm* ptm = GetLocalTm(&tm);
        CString formatString;

        assert(ptm != NULL);
        if (ptm != NULL)
        {
            const size_t  bufferSize = 128;
            VERIFY((::_tcsftime(formatString.GetBuffer(bufferSize), bufferSize, format, &tm)) != 0);
            formatString.ReleaseBuffer();
        }

        return formatString;
    }

    // Returns a CString that contains formatted time. The FormatID parameter
    // specifies a resource containing the formatting string which is similar
    // to the printf formatting string.  The valid format directives are
    //   %D - number of days
    //   %H - hour (0-23)
    //   %M - minute (0-59)
    //   %S - seconds (0-59)
    inline CString CTime::Format(UINT formatID) const
    {
        CString strFormat;
        strFormat.LoadString(formatID);
        return Format(strFormat);
    }

    // Returns a CString that contains formatted time as a UTC time. The format
    // parameter is a formatting string similar to the printf formatting string.
    inline CString CTime::FormatGmt(LPCTSTR format) const
    {
        CString fmt0 = format;
        fmt0.Replace(_T("%Z"), _T("Coordinated Universal Time"));
        fmt0.Replace(_T("%z"), _T("UTC"));

        time_tm tm;
        time_tm* ptm = GetGmtTm(&tm);
        CString formatString;

        assert(ptm != NULL);
        if (ptm != NULL)
        {
            const size_t  bufferSize = 128;
            VERIFY(::_tcsftime(formatString.GetBuffer(bufferSize), bufferSize, fmt0.c_str(), ptm) != 0);
            formatString.ReleaseBuffer();
        }

        return formatString;
    }

    // Returns a CString that contains formatted time as a UTC time. The FormatID
    // parameter specifies a resource containing the formatting string which is
    // similar to the printf formatting string.
    inline CString CTime::FormatGmt(UINT formatID) const
    {
        CString strFormat;
        strFormat.LoadString(formatID);
        return FormatGmt(strFormat);
    }

    // Converts *this CTime object into a FILETIME structure and stores it in ft.
    // Returns true if successful.
    inline bool CTime::GetAsFileTime(FILETIME& ft) const
    {
        bool rval = false;
        time_tm tm;
        time_tm* ptm = GetGmtTm(&tm);
        assert(ptm != NULL);

        if (ptm)
        {
            SYSTEMTIME st = {static_cast<WORD>(1900 + ptm->tm_year), static_cast<WORD>(1 + ptm->tm_mon),
                static_cast<WORD>(ptm->tm_wday), static_cast<WORD>(ptm->tm_mday), static_cast<WORD>(ptm->tm_hour),
                static_cast<WORD>(ptm->tm_min), static_cast<WORD>(ptm->tm_sec), 0};
            SystemTimeToFileTime(&st, &ft);
            rval = true;
        }

        return rval;
    }

    // Converts *this CTime object into a SYSTEMTIME structure and stores it
    // in st. Returns true if successful.
    inline bool CTime::GetAsSystemTime(SYSTEMTIME& st) const
    {
        bool rval = false;
        time_tm tm;
        time_tm* ptm = GetLocalTm(&tm);
        assert(ptm != NULL);

        if (ptm)
        {
            st.wYear    = static_cast<WORD>(1900 + ptm->tm_year);
            st.wMonth   = static_cast<WORD>(1 + ptm->tm_mon);
            st.wDayOfWeek   = static_cast<WORD>(ptm->tm_wday);
            st.wDay     = static_cast<WORD>(ptm->tm_mday);
            st.wHour    = static_cast<WORD>(ptm->tm_hour);
            st.wMinute  = static_cast<WORD>(ptm->tm_min);
            st.wSecond  = static_cast<WORD>(ptm->tm_sec);
            st.wMilliseconds = 0;
            rval = true;
        }

        return rval;
    }

    // Returns a pointer to a time_tm that contains a decomposition of *this
    // CTime object expressed in UTC. Returns NULL on failure.
    inline time_tm* CTime::GetGmtTm(time_tm* ptm) const
    {
        // NULL argument not supported.
        assert (ptm != NULL);
        if (ptm)
            return GMTime(*ptm, m_time);

        return NULL;
    }

    // Return a pointer to a time_tm that contains a decomposition of *this
    // CTime object expressed in the local time base. Returns NULL on failure.
    inline time_tm* CTime::GetLocalTm(time_tm* ptm) const
    {
        // NULL argument not supported.
        assert(ptm != NULL);

        if (ptm)
            return LocalTime(*ptm, m_time);

        return  NULL;
    }

    // Return *this time as a time_t value.
    inline time_t  CTime::GetTime() const
    {
        return m_time;
    }

    // Returns the year of *this time object, local (true) or UTC (false).
    inline int  CTime::GetYear(bool local /* = true */) const
    {
        time_tm tm;
        time_tm* ptmbuffer = (local ? GetLocalTm(&tm) : GetGmtTm(&tm));
        return 1900 + ptmbuffer->tm_year;
    }

    // Returns the month of *this time object (1 through 12), local (true) or
    // UTC (false).
    inline int  CTime::GetMonth(bool local /* = true */) const
    {
        time_tm tm;
        time_tm* ptmbuffer = (local ? GetLocalTm(&tm) : GetGmtTm(&tm));
        return ptmbuffer->tm_mon + 1;
    }

    // Returns the day of *this object (1 through 31), local (true) or
    // UTC (false).
    inline int  CTime::GetDay(bool local /* = true */) const
    {
        time_tm tm;
        time_tm* ptmbuffer = (local ? GetLocalTm(&tm) : GetGmtTm(&tm));
        return ptmbuffer->tm_mday ;
    }

    // Returns the hour of *this object (0 through 23), local (true) or
    // UTC (false).
    inline int  CTime::GetHour(bool local /* = true */) const
    {
        time_tm tm;
        time_tm* ptmbuffer = (local ? GetLocalTm(&tm) : GetGmtTm(&tm));
        return ptmbuffer->tm_hour;
    }

    // Returns the minute of *this object (0 through 59), local (true) or
    // UTC (false).
    inline int  CTime::GetMinute(bool local /* = true */) const
    {
        time_tm tm;
        time_tm* ptmbuffer = (local ? GetLocalTm(&tm) : GetGmtTm(&tm));
        return ptmbuffer->tm_min;
    }

    // Returns the second of *this object (0 through 61), local (true) or
    // UTC (false).
    inline int  CTime::GetSecond(bool local /* = true */) const
    {
        time_tm tm;
        time_tm* ptmbuffer = (local ? GetLocalTm(&tm) : GetGmtTm(&tm));
        return ptmbuffer->tm_sec;
    }

    // Returns the day of the week of *this object (0-6, Sunday = 0), local
    // (true) or UTC (false).
    inline int  CTime::GetDayOfWeek(bool local /* = true */) const
    {
        time_tm tm;
        time_tm* ptmbuffer = (local ? GetLocalTm(&tm) : GetGmtTm(&tm));
        return ptmbuffer->tm_wday;
    }

    // Returns the day of the year of *this object (1-366), local (true)  or
    // UTC (false).
    inline int  CTime::GetDayOfYear(bool local /* = true */) const
    {
        time_tm tm;
        time_tm* ptmbuffer = (local ? GetLocalTm(&tm) : GetGmtTm(&tm));
        return ptmbuffer->tm_yday + 1;
    }

    // Assigns the CTime t value to *this.
    inline CTime& CTime::operator=(const CTime& t)
    {
        // Self assignment is safe.
        m_time = t.m_time;
        return *this;
    }

    // Assigns the time_t value to *this time.
    inline CTime& CTime::operator=(const time_t& t)
    {
        assert(t >= 0);
        // Self assignment is safe.
        m_time = t;
        return *this;
    }

    // Returns the time span between *this time and  time t.
    inline const CTimeSpan CTime::operator-(const CTime& t) const
    {
        timespan_t d = static_cast<timespan_t>(m_time - t.m_time);
        CTimeSpan t0(d);
        return  t0;
    }

    // Returns the CTime that is the time span ts before *this time.
    inline const CTime CTime::operator-(const CTimeSpan& ts) const
    {
        time_t d = m_time - ts.m_timespan;
        CTime t(d);  // Asserts if d is invalid.
        return t;
    }

    // Returns the CTime that is the time span ts after *this time.
    inline const CTime CTime::operator+(const CTimeSpan& ts) const
    {
        time_t s = m_time + ts.m_timespan;
        CTime t(s); // Asserts if s is invalid.
        return t;
    }

    // Increments *this time by the time span ts and  return this CTime.
    inline CTime& CTime::operator+=(const CTimeSpan& ts)
    {
        m_time += ts.m_timespan;
        assert(m_time >= 0); // Invalid addition to time object.
        return *this;
    }

    // Decrements *this time by the time span ts and  return this CTime.
    inline CTime& CTime::operator-=(const CTimeSpan& ts)
    {
        m_time -= ts.m_timespan;
        assert(m_time >= 0); // Invalid subtraction from time object.
        return *this;
    }

    // Returns true if *this and t are the same times.
    inline bool CTime::operator==(const CTime& t) const
    {
        return m_time == t.m_time;
    }

    // Returns true if *this and t are not the same times.
    inline bool CTime::operator!=(const CTime& t) const
    {
        return m_time != t.m_time;
    }

    // Returns true if *this time is less than time t.
    inline bool CTime::operator<(const CTime& t) const
    {
        return m_time < t.m_time;
    }

    // Returns true if *this time is greater than time t.
    inline bool CTime::operator>(const CTime& t) const
    {
        return m_time > t.m_time;
    }

    // Returns true if *this time is less than or equal to time t.
    inline bool CTime::operator<=(const CTime& time) const
    {
        return m_time <= time.m_time;
    }

    // Returns true if *this time is greater than or equal to time t.
    inline bool CTime::operator>=(const CTime& time) const
    {
        return m_time >= time.m_time;
    }

    // Static function. Returns a CTime holding the current system time.
    inline CTime CTime::GetCurrentTime()
    {
        return CTime(::time(NULL));
    }


    ///////////////////////////////////////
    // Definitions for the CTimeSpan class.
    //

    inline CTimeSpan::CTimeSpan()
    {
        m_timespan = 0;
    }

    inline CTimeSpan::CTimeSpan(timespan_t t)
    {
        m_timespan = t;
    }

    // Contructs a CTimeSpan. Valid parameter ranges:
    //   lDays  0-24,855 (approximately)
    //   nHours 0-23
    //   nMins  0-59
    //   nSecs  0-59
    inline CTimeSpan::CTimeSpan(long days, int hours, int mins, int secs)
    {
        time_t sec_per_day  = 86400;
        time_t sec_per_hour = 3600;
        time_t sec_per_min  = 60;
        m_timespan = days * sec_per_day + hours * sec_per_hour +
            mins * sec_per_min + secs;
    }

    inline CTimeSpan::CTimeSpan(const CTimeSpan& ts)
    {
        m_timespan = ts.m_timespan;
    }

    // Returns a rendering of *this CTimeSpan object in CString form using the
    // format as the template. The valid format directives are
    //   %D - number of days
    //   %H - hour (0-23)
    //   %M - minute (0-59)
    //   %S - seconds (0-59)
    inline CString CTimeSpan::Format(LPCTSTR format) const
    {
        CString fmt0 = format;
        CString insert;

        while (fmt0.Find(_T("%D")) != -1)  // number of days
        {
            insert.Format(_T("%ld"), GetDays());
            fmt0.Replace(_T("%D"), insert);
        }
        while (fmt0.Find(_T("%H")) != -1)  // hours (00 - 23)
        {
            insert.Format(_T("%02d"), GetHours());
            fmt0.Replace(_T("%H"), insert);
        }
        while (fmt0.Find(_T("%M")) != -1)  // minutes (00 - 59)
        {
            insert.Format(_T("%02d"), GetMinutes());
            fmt0.Replace(_T("%M"), insert);
        }
        while (fmt0.Find(_T("%S")) != -1)  // seconds (00 - 59)
        {
            insert.Format(_T("%02d"), GetSeconds());
            fmt0.Replace(_T("%S"), insert);
        }
        return fmt0;
    }

    // Returns a rendering of *this CTimeSpan object in CString form using the
    // string resource having the formatID identifier as the template. The
    // valid format directives are
    //   %D - number of days
    //   %H - hour (0-23)
    //   %M - minute (0-59)
    //   %S - seconds (0-59)
    inline CString CTimeSpan::Format(UINT formatID) const
    {
        CString strFormat;
        VERIFY(strFormat.LoadString(formatID));
        return Format(strFormat);
    }

    // Returns the number of complete days in this CTimeSpan.
    inline LONGLONG CTimeSpan::GetDays() const
    {
        int sec_per_day = 86400;
        return m_timespan / sec_per_day;
    }

    // Returns the total number of complete hours in this CTimeSpan.
    inline LONGLONG CTimeSpan::GetTotalHours() const
    {
        int sec_per_hour = 3600;
        return m_timespan / sec_per_hour;
    }

    // Returns the total number of complete minutes in this CTimeSpan.
    inline LONGLONG CTimeSpan::GetTotalMinutes() const
    {
        int sec_per_min = 60;
        return m_timespan / sec_per_min;
    }

    // Returns the total number of complete seconds in this CTimeSpan.
    inline LONGLONG CTimeSpan::GetTotalSeconds() const
    {
        return m_timespan;
    }

    // Return the number of hours in the day component of this time
    // span (-23 through 23).
    inline int CTimeSpan::GetHours() const
    {
        int sec_per_hour  = 3600;
        int hours_per_day = 24;
        return static_cast<int>((m_timespan / sec_per_hour) % hours_per_day);
    }

    // Returns the number of minutes in the hour component of this time
    // span (-59 through 59).
    inline int CTimeSpan::GetMinutes() const
    {
        int sec_per_min = 60;
        return static_cast<int>((m_timespan / sec_per_min) % sec_per_min);
    }

    // Returns the number of seconds in the minute component of this time
    // span (-59 through 59).
    inline int CTimeSpan::GetSeconds() const
    {
        int sec_per_min = 60;
        return static_cast<int>(m_timespan % sec_per_min);
    }

    // Assigns the CTimeSpan ts to *this time span.
    inline CTimeSpan& CTimeSpan::operator=(const CTimeSpan& ts)
    {
        // Self assignment is safe.
        m_timespan = ts.m_timespan;
        return *this;
    }

    // Assigns the timespan_t t value to *this time span.
    inline CTimeSpan& CTimeSpan::operator=(const timespan_t& t)
    {
        // Self assignment is safe.
        m_timespan = t;
        return *this;
    }

    // Returns the negated value of *this time span.
    inline const CTimeSpan CTimeSpan::operator-() const
    {
        CTimeSpan t0(-m_timespan);
        return  t0;
    }

    // Returns the result of subtracting the time span ts from *this time span.
    inline const CTimeSpan CTimeSpan::operator-(CTimeSpan& ts) const
    {
        timespan_t d = m_timespan - ts.m_timespan;
        CTimeSpan tsp(d);
        return tsp;
    }

    // Returns the result of adding the time span ts to *this time span.
    inline const CTimeSpan CTimeSpan::operator+(CTimeSpan& ts) const
    {
        timespan_t s = m_timespan + ts.m_timespan;
        CTimeSpan tsp(s);
        return tsp;
    }

    // Increases *this time span by the ts time span and  return the result.
    inline CTimeSpan& CTimeSpan::operator+=(CTimeSpan& ts)
    {
        m_timespan += ts.m_timespan;
        return *this;
    }

    // Decreases *this time span by the ts time span and  return the result.
    inline CTimeSpan& CTimeSpan::operator-=(CTimeSpan& ts)
    {
        m_timespan -= ts.m_timespan;
        return *this;
    }

    // Returns true if the time span ts equals *this one.
    inline bool CTimeSpan::operator==(const CTimeSpan& ts) const
    {
        return m_timespan == ts.m_timespan;
    }

    // Returns true if the time span ts does not equal *this one.
    inline bool CTimeSpan::operator!=(const CTimeSpan& ts) const
    {
        return m_timespan != ts.m_timespan;
    }

    // Returns true if *this time span is less than the ts time span.
    inline bool CTimeSpan::operator<(const CTimeSpan& ts) const
    {
        return m_timespan < ts.m_timespan;
    }

    // Returns true if *this time span is greater than the ts time span.
    inline bool CTimeSpan::operator>(const CTimeSpan& ts) const
    {
        return m_timespan > ts.m_timespan;
    }

    // Returns true if *this time span is less than or equal to the ts
    // time span.
    inline bool CTimeSpan::operator<=(const CTimeSpan& ts) const
    {
        return m_timespan <= ts.m_timespan;
    }

    // Returns true if *this time span is greater than or equal to the ts
    // time span.
    inline bool CTimeSpan::operator>=(const CTimeSpan& ts) const
    {
        return m_timespan >= ts.m_timespan;
    }


} // namespace Win32XX


#endif // _WIN32XX_CTIME_H_
