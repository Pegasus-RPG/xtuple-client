/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DOCATTACH_H
#define DOCATTACH_H

#include "scriptablewidget.h"
#include "documents.h"
#include "ui_docAttach.h"

class docAttachPrivate;

class docAttach : public QDialog, public Ui::docAttach, public ScriptableWidget
{
    Q_OBJECT

public:
    enum SaveStatus { OK, Failed };
    Q_ENUM(SaveStatus);

    docAttach(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~docAttach();

    Q_INVOKABLE virtual int id();
    Q_INVOKABLE virtual QString mode();
    Q_INVOKABLE virtual QString purpose();
    Q_INVOKABLE virtual int sourceId();
    Q_INVOKABLE virtual QString sourceType();
    Q_INVOKABLE virtual int targetId();
    Q_INVOKABLE virtual QString targetType();

    QPushButton* _save;

signals:
    virtual void saveBeforeBegin();
    virtual void saveAfterBegin();
    virtual void saveBeforeCommit();
    virtual void saveAfterCommit();
    virtual void saveBeforeRollback(QSqlQuery*);
    virtual void saveAfterRollback(QSqlQuery*);

public slots:
    virtual void set( const ParameterList & pParams );
    virtual void sFileList();
    virtual void sHandleButtons();
    virtual void sSave();
    virtual void sHandleNewId(int);
    inline virtual void setSaveStatus(SaveStatus status) { _saveStatus = status; };

protected slots:
    virtual void languageChange();
    virtual void showEvent(QShowEvent *);

private:
    int _id;
    int _sourceid;
    QString _sourcetype;
    int _targetid;
    QString _targettype;
    QString _purpose;
    QString _mode;
    docAttachPrivate *_p;
    SaveStatus            _saveStatus;
};

#endif // docAttach_H
