/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __CSVDATA_H__
#define __CSVDATA_H__

#include <QObject>
#include <QString>
#include <QStringList>

class CSVDataPrivate;
class QWidget;
class XAbstractMessageHandler;

class CSVData : public QObject
{
  Q_OBJECT

  public:
    CSVData(QObject    *parent = 0,
            const char *name   = 0,
            const QChar delim  = ',');
    virtual ~CSVData();

    unsigned int             columns();
    QChar                    delimiter()       const;
    bool                     firstRowHeaders() const;
    QString                  header(int);
    bool                     load(QString filename, QWidget *parent = 0);
    XAbstractMessageHandler *messageHandler()  const;
    void         setDelimiter(const QChar delim);
    void         setFirstRowHeaders(bool y);
    void         setMessageHandler(XAbstractMessageHandler *handler);
    unsigned int rows();
    QString      value(int row, int column);

  protected:
    CSVDataPrivate          *_data;
    QChar                    _delimiter;
    bool                     _firstRowHeaders;
    XAbstractMessageHandler *_msghandler;
};

#endif

