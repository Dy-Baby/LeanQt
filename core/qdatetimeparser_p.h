/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2022 Rochus Keller (me@rochus-keller.ch) for LeanQt
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QDATETIMEPARSER_P_H
#define QDATETIMEPARSER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qplatformdefs.h"
#include "QtCore/qatomic.h"
#include "QtCore/qdatetime.h"
#include "QtCore/qstringlist.h"
#include "QtCore/qlocale.h"
#ifndef QT_BOOTSTRAPPED
# include "QtCore/qvariant.h"
#endif
#include "QtCore/qvector.h"
#ifndef QT_NO_COREAPPLICATION
#include "QtCore/qcoreapplication.h"
#endif

#define QDATETIMEEDIT_TIME_MIN QTime(0, 0, 0, 0)
#define QDATETIMEEDIT_TIME_MAX QTime(23, 59, 59, 999)
#define QDATETIMEEDIT_DATE_MIN QDate(100, 1, 1)
#define QDATETIMEEDIT_COMPAT_DATE_MIN QDate(1752, 9, 14)
#define QDATETIMEEDIT_DATE_MAX QDate(7999, 12, 31)
#define QDATETIMEEDIT_DATETIME_MIN QDateTime(QDATETIMEEDIT_DATE_MIN, QDATETIMEEDIT_TIME_MIN)
#define QDATETIMEEDIT_COMPAT_DATETIME_MIN QDateTime(QDATETIMEEDIT_COMPAT_DATE_MIN, QDATETIMEEDIT_TIME_MIN)
#define QDATETIMEEDIT_DATETIME_MAX QDateTime(QDATETIMEEDIT_DATE_MAX, QDATETIMEEDIT_TIME_MAX)
#define QDATETIMEEDIT_DATE_INITIAL QDate(2000, 1, 1)

QT_BEGIN_NAMESPACE

#ifndef QT_BOOTSTRAPPED

class Q_CORE_EXPORT QDateTimeParser
{
#ifndef QT_NO_COREAPPLICATION
    Q_DECLARE_TR_FUNCTIONS(QDateTimeParser)
#endif
public:
    enum Context {
        FromString,
        DateTimeEdit
    };
    QDateTimeParser(QVariant::Type t, Context ctx)
        : currentSectionIndex(-1), display(0), cachedDay(-1), parserType(t),
        fixday(false), spec(Qt::LocalTime), context(ctx)
    {
        defaultLocale = QLocale::system();
        first.type = FirstSection;
        first.pos = -1;
        first.count = -1;
        first.zeroesAdded = 0;
        last.type = LastSection;
        last.pos = -1;
        last.count = -1;
        last.zeroesAdded = 0;
        none.type = NoSection;
        none.pos = -1;
        none.count = -1;
        none.zeroesAdded = 0;
    }
    virtual ~QDateTimeParser() {}

    enum Section {
        NoSection     = 0x00000,
        AmPmSection   = 0x00001,
        MSecSection   = 0x00002,
        SecondSection = 0x00004,
        MinuteSection = 0x00008,
        Hour12Section   = 0x00010,
        Hour24Section   = 0x00020,
        HourSectionMask = (Hour12Section | Hour24Section),
        TimeSectionMask = (MSecSection | SecondSection | MinuteSection |
                           HourSectionMask | AmPmSection),

        DaySection         = 0x00100,
        MonthSection       = 0x00200,
        YearSection        = 0x00400,
        YearSection2Digits = 0x00800,
        YearSectionMask = YearSection | YearSection2Digits,
        DayOfWeekSectionShort = 0x01000,
        DayOfWeekSectionLong  = 0x02000,
        DayOfWeekSectionMask = DayOfWeekSectionShort | DayOfWeekSectionLong,
        DaySectionMask = DaySection | DayOfWeekSectionMask,
        DateSectionMask = DaySectionMask | MonthSection | YearSectionMask,

        Internal             = 0x10000,
        FirstSection         = 0x20000 | Internal,
        LastSection          = 0x40000 | Internal,
        CalendarPopupSection = 0x80000 | Internal,

        NoSectionIndex = -1,
        FirstSectionIndex = -2,
        LastSectionIndex = -3,
        CalendarPopupIndex = -4
    }; // extending qdatetimeedit.h's equivalent
    Q_DECLARE_FLAGS(Sections, Section)

    struct Q_CORE_EXPORT SectionNode {
        Section type;
        mutable int pos;
        int count;
        int zeroesAdded;

        static QString name(Section s);
        QString name() const { return name(type); }
        QString format() const;
        int maxChange() const;
    };

    enum State { // duplicated from QValidator
        Invalid,
        Intermediate,
        Acceptable
    };

    struct StateNode {
        StateNode() : state(Invalid), conflicts(false) {}
        QString input;
        State state;
        bool conflicts;
        QDateTime value;
    };

    enum AmPm {
        AmText,
        PmText
    };

    enum Case {
        UpperCase,
        LowerCase
    };

#ifndef QT_NO_DATESTRING
    StateNode parse(QString &input, int &cursorPosition, const QDateTime &currentValue, bool fixup) const;
#endif
    bool parseFormat(const QString &format);
#ifndef QT_NO_DATESTRING
    bool fromString(const QString &text, QDate *date, QTime *time) const;
#endif

    enum FieldInfoFlag {
        Numeric = 0x01,
        FixedWidth = 0x02,
        AllowPartial = 0x04,
        Fraction = 0x08
    };
    Q_DECLARE_FLAGS(FieldInfo, FieldInfoFlag)

    FieldInfo fieldInfo(int index) const;

    void setDefaultLocale(const QLocale &loc) { defaultLocale = loc; }
    virtual QString displayText() const { return text; }

private:
    int sectionMaxSize(Section s, int count) const;
    QString sectionText(const QString &text, int sectionIndex, int index) const;
    int parseSection(const QDateTime &currentValue, int sectionIndex, QString &txt, int &cursorPosition,
                     int index, QDateTimeParser::State &state, int *used = 0) const;
#ifndef QT_NO_TEXTDATE
    int findMonth(const QString &str1, int monthstart, int sectionIndex,
                  QString *monthName = 0, int *used = 0) const;
    int findDay(const QString &str1, int intDaystart, int sectionIndex,
                QString *dayName = 0, int *used = 0) const;
#endif

    enum AmPmFinder {
        Neither = -1,
        AM = 0,
        PM = 1,
        PossibleAM = 2,
        PossiblePM = 3,
        PossibleBoth = 4
    };
    AmPmFinder findAmPm(QString &str, int index, int *used = 0) const;
    bool potentialValue(const QString &str, int min, int max, int index,
                        const QDateTime &currentValue, int insert) const;

protected: // for the benefit of QDateTimeEditPrivate
    int sectionSize(int index) const;
    int sectionMaxSize(int index) const;
    int sectionPos(int index) const;
    int sectionPos(const SectionNode &sn) const;

    const SectionNode &sectionNode(int index) const;
    Section sectionType(int index) const;
    QString sectionText(int sectionIndex) const;
    int getDigit(const QDateTime &dt, int index) const;
    bool setDigit(QDateTime &t, int index, int newval) const;

    int absoluteMax(int index, const QDateTime &value = QDateTime()) const;
    int absoluteMin(int index) const;

    bool skipToNextSection(int section, const QDateTime &current, const QString &sectionText) const;
    QString stateName(State s) const;
    virtual QDateTime getMinimum() const;
    virtual QDateTime getMaximum() const;
    virtual int cursorPosition() const { return -1; }
    virtual QString getAmPmText(AmPm ap, Case cs) const;
    virtual QLocale locale() const { return defaultLocale; }

    mutable int currentSectionIndex;
    Sections display;
    /*
        This stores the most recently selected day.
        It is useful when considering the following scenario:

        1. Date is: 31/01/2000
        2. User increments month: 29/02/2000
        3. User increments month: 31/03/2000

        At step 1, cachedDay stores 31. At step 2, the 31 is invalid for February, so the cachedDay is not updated.
        At step 3, the month is changed to March, for which 31 is a valid day. Since 29 < 31, the day is set to cachedDay.
        This is good for when users have selected their desired day and are scrolling up or down in the month or year section
        and do not want smaller months (or non-leap years) to alter the day that they chose.
    */
    mutable int cachedDay;
    mutable QString text;
    QVector<SectionNode> sectionNodes;
    SectionNode first, last, none, popup;
    QStringList separators;
    QString displayFormat;
    QLocale defaultLocale;
    QVariant::Type parserType;
    bool fixday;
    Qt::TimeSpec spec; // spec if used by QDateTimeEdit
    Context context;
};
Q_DECLARE_TYPEINFO(QDateTimeParser::SectionNode, Q_PRIMITIVE_TYPE);

Q_CORE_EXPORT bool operator==(const QDateTimeParser::SectionNode &s1, const QDateTimeParser::SectionNode &s2);

Q_DECLARE_OPERATORS_FOR_FLAGS(QDateTimeParser::Sections)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDateTimeParser::FieldInfo)

#endif // QT_BOOTSTRAPPED

QT_END_NAMESPACE

#endif // QDATETIME_P_H
