/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */


#ifndef datecluster_h
#define datecluster_h

#include <QDateTime>

#include "xlineedit.h"

class QLabel;
class DLineEdit;
class ParameterList;
class XSqlQuery;
class QFocusEvent;

#define _dateFormat "%02d/%02d/%02d"

QString OPENMFGWIDGETS_EXPORT formatDate(const QDate &);


class OPENMFGWIDGETS_EXPORT DLineEdit : public XLineEdit
{
  Q_OBJECT

  public:
    DLineEdit(QWidget *parent, const char *name = 0);

    QDate   date();
    QString dateString();

    bool isNull();
    bool isValid();

    inline void setAllowNullDate(bool pAllowNull)         { _allowNull = pAllowNull;                       }
    inline void setNullString(const QString &pNullString) { _nullString = pNullString;                     }
    inline void setNullDate(const QDate &pNullDate)       { _nullDate = pNullDate;                         }
    inline int year()                                     { return ((_valid) ? _currentDate.year() : -1);  }
    inline int month()                                    { return ((_valid) ? _currentDate.month() : -1); }
    inline int day()                                      { return ((_valid) ? _currentDate.day() : -1);   }

  public slots:
    void setNull();
    void setDate(const QDate &, bool);
    void setDate(const QDate &);

  signals:
    void newDate(const QDate &);

  protected:
    void focusOutEvent(QFocusEvent *);

  private:
    void parseDate();
    void validateDate();
    bool fixMonthEnd(int *, int, int);

    QString _dateString;
    QString _nullString;
    QDate   _currentDate;
    QDate   _nullDate;
    bool    _allowNull;
    bool    _valid;
    bool    _null;
};


class OPENMFGWIDGETS_EXPORT DateCluster : public QWidget
{
  Q_OBJECT

  public:
    DateCluster(QWidget *, const char * = 0);

    void setStartNull(const QString &, const QDate &, bool);
    void setEndNull(const QString &, const QDate &, bool);

    void appendValue(ParameterList &);
    void bindValue(XSqlQuery &);

    inline QDate startDate() { return _startDate->date(); };
    inline QDate endDate()   { return _endDate->date();   };

    inline void  setStartDate(const QDate &pDate) { _startDate->setDate(pDate); };
    inline void  setEndDate(const QDate &pDate)   { _endDate->setDate(pDate);   };

    inline bool  allValid() { return ((_startDate->isValid()) && (_endDate->isValid())); };

    void setStartCaption(const QString &);
    void setEndCaption(const QString &);

  signals:
    void updated();

  protected:
    DLineEdit *_startDate;
    DLineEdit *_endDate;

  private:
    QLabel    *_startDateLit;
    QLabel    *_endDateLit;
};

#endif

