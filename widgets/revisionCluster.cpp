/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "revisioncluster.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QtScript>

#include "errorReporter.h"

QString RevisionLineEdit::typeText()
{
  return _typeText;
}

RevisionCluster::RevisionCluster(QWidget *pParent, const char *pName) :
  VirtualCluster(pParent, pName)
{
  setLabel("Revision: ");
  addNumberWidget(new RevisionLineEdit(this, pName));

  _description->show();

  connect(_number, SIGNAL(modeChanged()), this, SLOT(sModeChanged()));
  connect(_number, SIGNAL(canActivate(bool)), this, SLOT(sCanActivate(bool)));
  connect(_number, SIGNAL(canDeactivate(bool)), this, SLOT(sCanDeactivate(bool)));
}

void RevisionCluster::activate()
{
  return (static_cast<RevisionLineEdit*>(_number)->activate());
}

void RevisionCluster::deactivate()
{
  return (static_cast<RevisionLineEdit*>(_number)->deactivate());
}

void RevisionCluster::sCanActivate(bool p)
{
  emit canActivate(p);
}

void RevisionCluster::sCanDeactivate(bool p)
{
  emit canDeactivate(p);
}

void RevisionCluster::setActive()
{
  return (static_cast<RevisionLineEdit*>(_number))->setActive();
}

void RevisionCluster::setMode(QString pmode)
{
  return (static_cast<RevisionLineEdit*>(_number))->setMode(pmode);
}

void RevisionCluster::setMode(RevisionLineEdit::Modes pmode)
{
  return (static_cast<RevisionLineEdit*>(_number))->setMode(pmode);
}

void RevisionCluster::setTargetId(int ptargetid)
{
  return (static_cast<RevisionLineEdit*>(_number))->setTargetId(ptargetid);
}

void RevisionCluster::setType(QString ptype)
{
  return (static_cast<RevisionLineEdit*>(_number))->setType(ptype);
}

void RevisionCluster::setType(RevisionLineEdit::RevisionTypes ptype)
{
  return (static_cast<RevisionLineEdit*>(_number))->setType(ptype);
}

void RevisionCluster::sModeChanged()
{
  bool canSearch = false;

  if  (_x_privileges)
  {
    if (_x_metrics->boolean("RevControl"))
    {
      canSearch =  ((RevisionLineEdit::View==(static_cast<RevisionLineEdit*>(_number))->mode()) && (_x_privileges->check("ViewInactiveRevisions") ||
                                                                                                    _x_privileges->check("MaintainRevisions"))) ||
                   ((RevisionLineEdit::Use==(static_cast<RevisionLineEdit*>(_number))->mode()) && _x_privileges->check("UseInactiveRevisions")) ||
                   ((RevisionLineEdit::Maintain==(static_cast<RevisionLineEdit*>(_number))->mode()) && (_x_privileges->check("MaintainRevisions") ||
                                                                                                        _x_privileges->check("ViewInactiveRevisions")));

      (static_cast<RevisionLineEdit*>(_number))->setDisabled(((RevisionLineEdit::Maintain==(static_cast<RevisionLineEdit*>(_number))->mode()) &&
                                                              !_x_privileges->check("MaintainRevisions") &&
                                                              !_x_privileges->check("ViewInactiveRevisions")) ||
                                                             ((RevisionLineEdit::Use==(static_cast<RevisionLineEdit*>(_number))->mode()) ||
                                                              (RevisionLineEdit::View==(static_cast<RevisionLineEdit*>(_number))->mode())));
    }
    else
      (static_cast<RevisionLineEdit*>(_number))->setEnabled(true);

    static_cast<RevisionLineEdit*>(_number)->_listAct->setEnabled(canSearch);
    static_cast<RevisionLineEdit*>(_number)->_searchAct->setEnabled(canSearch);
    static_cast<RevisionLineEdit*>(_number)->sUpdateMenu();
  }
}

RevisionLineEdit::RevisionLineEdit(QWidget *pParent, const char *pName) :
  VirtualClusterLineEdit(pParent, "rev", "rev_id", "rev_number", "rev_status",
                         QString("case rev_status when 'A' then '%1' when 'S' then '%2' when 'P' then '%3' else '%4' end ").arg(tr("Active")).arg(tr("Substitute")).arg(tr("Pending")).arg(tr("Inactive")).toLatin1(),
                         0, pName),
  _allowNew(false),
  _mode(View),
  _type(All),
  _targetId(-1),
  _cachenum(""),
  _typeText(""),
  _status(Inactive),
  _activateSep(0),
  _activateAct(0),
  _deactivateSep(0),
  _deactivateAct(0)

{
  setTitles(tr("Revision"), tr("Revisions"));

  _isRevControl = (_x_metrics && _x_metrics->boolean("RevControl"));
  if (!_isRevControl)
  {
    _menuLabel->hide();
    disconnect(this, SIGNAL(textEdited(QString)), this, SLOT(sHandleCompleter()));
    _listAct->disconnect();
    _searchAct->disconnect();
  }
  else
  {
    _activateSep = new QAction(this);
    _activateSep->setSeparator(true);
    _activateSep->setVisible(false);

    _activateAct = new QAction(tr("Activate"), this);
    _activateAct->setToolTip(tr("Activate revision"));
    _activateAct->setVisible(false);
    connect(_activateAct, SIGNAL(triggered()), this, SLOT(activate()));
    addAction(_activateAct);

    menu()->addAction(_activateSep);
    menu()->addAction(_activateAct);

    _deactivateSep = new QAction(this);
    _deactivateSep->setSeparator(true);
    _deactivateSep->setVisible(false);

    _deactivateAct = new QAction(tr("Deactivate"), this);
    _deactivateAct->setToolTip(tr("Deactivate revision"));
    _deactivateAct->setVisible(false);
    connect(_deactivateAct, SIGNAL(triggered()), this, SLOT(deactivate()));
    addAction(_deactivateAct);

    menu()->addAction(_deactivateSep);
    menu()->addAction(_deactivateAct);
  }
}

RevisionLineEdit::RevisionTypes RevisionCluster::type()
{
  return (static_cast<RevisionLineEdit*>(_number))->type();
}

RevisionLineEdit::RevisionTypes RevisionLineEdit::type()
{
  return _type;
}

RevisionLineEdit::Modes RevisionCluster::mode()
{
  return (static_cast<RevisionLineEdit*>(_number))->mode();
}

RevisionLineEdit::Modes RevisionLineEdit::mode()
{
  return _mode;
}

RevisionLineEdit::Statuses RevisionLineEdit::status()
{
  return _status;
}

void RevisionLineEdit::activate()
{
  QString message = "";
  if (_x_metrics && _x_metrics->boolean("BOOSubstitute"))
    message = tr("This action will make this revision the system default for the "
                 "item it belongs to and substitute the currently active revision. "
                 "Are you sure you want proceed?");
  else
    message = tr("This action will make this revision the system default for the "
                 "item it belongs to and deactivate the currently active revision. "
                 "Are you sure you want proceed?");

  if (QMessageBox::question(this, tr("Activate Revision"), message,
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::No)
    return;

  XSqlQuery activate;
  activate.prepare("SELECT activateRev(:rev_id) AS result;");
  activate.bindValue(":rev_id", id());
  activate.exec();
  if (activate.first())
    setId(_id); // refresh
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Activationg Revision"),
                                activate, __FILE__, __LINE__))
  {
    return;
  }
}

void RevisionLineEdit::deactivate()
{
  if (QMessageBox::question(this, tr("Deactivate Revision"),
                            tr("This action will make this revision inactive. "
                               "Are you sure you want proceed?"),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::No)
    return;

  XSqlQuery deactivate;
  deactivate.prepare("SELECT deactivateRev(:rev_id) AS result;");
  deactivate.bindValue(":rev_id", id());
  deactivate.exec();
  if (deactivate.first())
    setId(_id); // refresh
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deactivating Revision"),
                                deactivate, __FILE__, __LINE__))
  {
    return;
  }
}

void RevisionLineEdit::setMode(Modes pMode)
{
  if (_mode!=pMode)
  {
    _mode = pMode;
    if  (_x_privileges)
      _allowNew=((pMode=Maintain) && (_x_privileges->check("MaintainRevisions")));
    emit modeChanged();
  }
}

void RevisionLineEdit::setMode(QString pmode)
{
  if (pmode == "View")
    setMode(View);
  else if (pmode == "Use")
    setMode(Use);
  else if (pmode == "Maintain")
    setMode(Maintain);
  else
  {
    QMessageBox::critical(this, tr("Invalid Mode"),
                          tr("RevisionLineEdit::setMode received "
                             "an invalid Mode %1").arg(pmode));
    setMode(View);
  }
}

void RevisionLineEdit::setActive()
{
  if (_isRevControl)
  {
    XSqlQuery revision;
    revision.prepare( "SELECT getActiveRevId(:target_type,:target_id) AS rev_id;" );
    revision.bindValue(":target_type", typeText());
    revision.bindValue(":target_id", _targetId);
    revision.exec();
    if (revision.first())
    {
      setId(revision.value("rev_id").toInt());
    }
    else
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Active Revision"),
                           revision, __FILE__, __LINE__);
      setId(-1);
    }
  }
}

void RevisionLineEdit::setId(const int pId)
{
  VirtualClusterLineEdit::setId(pId);
  _cachenum = text().trimmed();
  if (_name == "A")
    _status = Active;
  else if (_name == "S")
    _status = Substitute;
  else if (_name == "P")
    _status = Pending;
  else
    _status = Inactive;

  if  (_x_privileges)
  {
    emit canActivate( (_mode==Maintain) &&
                      (_x_privileges->check("MaintainRevisions")) &&
                      ((_status==Pending) || (_status==Substitute)) );
    emit canDeactivate( (_mode==Maintain) &&
                        (_x_privileges->check("MaintainRevisions")) &&
                        (_status==Substitute) );
  }
}

void RevisionLineEdit::setTargetId(int pItem)
{
  if (_isRevControl)
  {
    _targetId = pItem;
    setExtraClause(QString(" ((rev_target_type='%1') AND (rev_target_id=%2)) ").arg(typeText()).arg(pItem));
    setActive();
  }
}

void RevisionLineEdit::setType(QString ptype)
{
  if (ptype == "BOM")
    setType(BOM);
  else if (ptype == "BOO")
    setType(BOO);
  else
  {
    QMessageBox::critical(this, tr("Invalid Revision Type"),
                          tr("RevisionLineEdit::setType received "
                             "an invalid RevisionType %1").arg(ptype));
    setType(All);
  }
}

void RevisionLineEdit::sUpdateMenu()
{
  VirtualClusterLineEdit::sUpdateMenu();
  if (_x_privileges && _activateSep && _activateAct)
  {
    if( (_mode==Maintain) &&
        ((_status==Pending) || (_status==Substitute)) )
    {
      _activateSep->setVisible(true);
      _activateAct->setVisible(true);
      _activateAct->setEnabled(_x_privileges->check("MaintainRevisions"));
    }
    else
    {
      _activateSep->setVisible(false);
      _activateAct->setVisible(false);
    }
  }
  if (_x_privileges && _deactivateSep && _deactivateAct)
  {
    if( (_mode==Maintain) &&
        (_status==Substitute) )
    {
      _deactivateSep->setVisible(true);
      _deactivateAct->setVisible(true);
      _deactivateAct->setEnabled(_x_privileges->check("MaintainRevisions"));
    }
    else
    {
      _deactivateSep->setVisible(false);
      _deactivateAct->setVisible(false);
    }
  }
}

void RevisionLineEdit::setType(RevisionTypes pType)
{
  _type = pType;
  switch (pType)
  {
    case BOM:
          _typeText="BOM";
          break;

    case BOO:
          _typeText="BOO";
          break;

        default:
          _typeText="All";
        break;
  }
}

void RevisionLineEdit::sParse()
{
  if (_completerId)
  {
    int id = _completerId;
    _completerId = 0;
    setId(id);
  }
  else if (_isRevControl && ! _parsed)
  {
    _parsed = true;
    QString stripped = text().trimmed().toUpper();
    if (stripped.isEmpty())
    {
      setId(-1);
    }
    else
    {
      XSqlQuery numQ;
      numQ.prepare("SELECT rev_id "
                       "FROM rev "
                       "WHERE ((rev_number=UPPER(:number))"
                       " AND (rev_target_id=:target_id)"
                       " AND (rev_target_type=:target_type));");
      numQ.bindValue(":number", stripped);
      numQ.bindValue(":target_id",_targetId);
      numQ.bindValue(":target_type",typeText());
      numQ.exec();
      if (numQ.first())
      {
        _valid = true;
        setId(numQ.value("rev_id").toInt());
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Revision"),
                                    numQ, __FILE__, __LINE__))
      {
        setText(_cachenum);
        return;
      }
      else if (_allowNew)
      {
        if (QMessageBox::question(this, tr("Create New Revision?"),
                tr("Revision does not exist.  Would you like to create a new one from the current active revision?"),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::Yes)
        {
          XSqlQuery newrev;
          if (_type==BOM)
            newrev.prepare("SELECT createBomRev(:target_id,:revision) AS result;");
          else if (_type==BOO)
            newrev.prepare("SELECT createBooRev(:target_id,:revision) AS result;");
          newrev.bindValue(":target_id", _targetId);
          newrev.bindValue(":revision", text());
          newrev.exec();
          if (newrev.first())
            setId(newrev.value("result").toInt());
          else
          {
            ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating Revision"),
                                 newrev, __FILE__, __LINE__);
            setText(_cachenum);
          }
        }
        else
          setText(_cachenum);
      }
    }
  }
  emit valid(_valid);
  emit parsed();
}

// script api //////////////////////////////////////////////////////////////////

void setupRevisionLineEdit(QScriptEngine *engine)
{
  if (! engine->globalObject().property("RevisionLineEdit").isObject())
  {
    QScriptValue ctor = engine->newObject(); //engine->newFunction(scriptconstructor);
    QScriptValue meta = engine->newQMetaObject(&RevisionLineEdit::staticMetaObject, ctor);

    engine->globalObject().setProperty("RevisionLineEdit", meta,
                                       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  }
}
