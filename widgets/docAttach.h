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

    Q_INVOKABLE virtual QString mode() { return _mode; }
    Q_INVOKABLE virtual QString purpose() { return _purpose; }
    Q_INVOKABLE virtual int sourceId() { return _sourceid; }
    Q_INVOKABLE virtual QString sourceType() { return _sourcetype; }
    Q_INVOKABLE virtual int targetId() { return _targetid; }
    Q_INVOKABLE virtual QString targetType() { return _targettype; }
    Q_INVOKABLE virtual int urlId() { return _urlid; }

    QPushButton* _save;

signals:
    virtual void saveBeforeBegin();
    virtual void saveAfterBegin();
    virtual void saveBeforeCommit(int);
    virtual void saveAfterCommit();
    virtual void saveBeforeRollback(QSqlQuery*);
    virtual void saveAfterRollback(QSqlQuery*);

public slots:
    void set( const ParameterList & pParams );
    void sFileList();
    void sHandleButtons();
    void sSave();
    void sHandleNewId(int);
    inline void setSaveStatus(SaveStatus status) { _saveStatus = status; };

protected slots:
    virtual void languageChange();
    virtual void showEvent(QShowEvent *);

private:
    int _sourceid;
    QString _sourcetype;
    int _targetid;
    int _urlid;
    QString _targettype;
    QString _purpose;
    QString _mode;
    docAttachPrivate *_p;
    SaveStatus            _saveStatus;
};

#endif // docAttach_H
