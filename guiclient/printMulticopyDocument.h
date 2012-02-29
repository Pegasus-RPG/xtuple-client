/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTMULTICOPYDOCUMENT_H
#define PRINTMULTICOPYDOCUMENT_H

#include "ui_printMulticopyDocument.h"

#include <parameter.h>

#include "distributeInventory.h"
#include "guiclient.h"
#include "xdialog.h"

class XDocCopySetter;
class printMulticopyDocumentPrivate;

class printMulticopyDocument : public XDialog
{
  Q_OBJECT

  friend class printMulticopyDocumentPrivate;

  public:
    printMulticopyDocument(QWidget    *parent = 0,
                           const char *name   = 0,
                           bool        modal  = false,
                           Qt::WFlags  fl     = 0);
    printMulticopyDocument(QString numCopiesMetric,
                           QString watermarkMetric,
                           QString showPriceMetric,
                           QString postPrivilege = QString(),
                           QWidget    *parent = 0,
                           const char *name   = 0,
                           bool        modal  = false,
                           Qt::WFlags  fl     = 0);
    ~printMulticopyDocument();

    Q_INVOKABLE virtual void            clear();
    Q_INVOKABLE virtual XDocCopySetter *copies();
    Q_INVOKABLE virtual ParameterList   getParamsDocList();
    Q_INVOKABLE virtual ParameterList   getParamsOneCopy(int row, XSqlQuery &qry);
    Q_INVOKABLE virtual int             id();
    Q_INVOKABLE virtual bool            isOkToPrint();
    Q_INVOKABLE virtual bool            isSetup();
    Q_INVOKABLE virtual QWidget        *optionsWidget();
    Q_INVOKABLE virtual void            populate();
    Q_INVOKABLE virtual void            setId(int docid);
    Q_INVOKABLE virtual void            setNumCopiesMetric(QString metric);
    Q_INVOKABLE virtual void            setPostPrivilege(QString priv);
    Q_INVOKABLE virtual void            setSetup(bool setup);
    Q_INVOKABLE virtual void            setShowPriceMetric(QString metric);
    Q_INVOKABLE virtual void            setWatermarkMetric(QString metric);

  public slots:
    virtual enum SetResponse set(const ParameterList & pParams);
    virtual void sPrint();

  signals:
    void docUpdated(int);
    void aboutToStart(XSqlQuery *qry);
    void finishedPrinting(int);
    void finishedWithAll();
    void populated(QSqlRecord *record);
    void posted(int);

  protected slots:
    virtual void languageChange();

  protected:
    printMulticopyDocumentPrivate *_data;

    QString _doctypefull;
    bool    _distributeInventory;
    QString _docinfoQueryString;
    QString _askBeforePostingQry;
    QString _askBeforePostingMsg;
    QString _errCheckBeforePostingQry;
    QString _errCheckBeforePostingMsg;
    QString _markAllPrintedQry;
    QString _markOnePrintedQry;
    QString _postFunction;
    QString _postQuery;
    QString _reportKey;

  private:
};

#endif // PRINTMULTICOPYDOCUMENT_H
