/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "package.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#define DEBUG false

// TODO: XDialog should have a default implementation that returns FALSE
bool package::userHasPriv(const int pMode)
{
  bool retval = false;
  bool canmaintain = false;

  if (pMode == cNew || pMode == cEdit)
  {
    XSqlQuery su;
    su.exec("SELECT rolsuper FROM pg_roles WHERE (rolname=CURRENT_USER);");
    if (su.first())
      canmaintain = su.value("rolsuper").toBool();
    else if (su.lastError().type() != QSqlError::NoError)
    {
      systemError(0, su.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  switch (pMode)
  {
    case cView:
      retval = _privileges->check("ViewPackages") || canmaintain;
      break;
    case cNew:
    case cEdit:
      retval = canmaintain;
      break;
    default:
      retval = false;
      break;
  }
  return retval;
}

// TODO: this code really belongs in XDialog
void package::setVisible(bool visible)
{
  if (! visible)
    XDialog::setVisible(false);

  else if (! userHasPriv(_mode))
  {
    systemError(this,
		tr("You do not have sufficient privilege to view this window"),
		__FILE__, __LINE__);
    reject();
  }
  else
    XDialog::setVisible(true);
}

package::package(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _rec->addColumn(tr("Type"),       -1, Qt::AlignLeft, true, "type");
  _rec->addColumn(tr("Name"),       -1, Qt::AlignLeft, true, "pkgitem_name");
  _rec->addColumn(tr("Description"),-1, Qt::AlignLeft, true, "pkgitem_descrip");

  _req->addColumn(tr("Package"),    -1, Qt::AlignLeft, true, "pkghead_name");
  _req->addColumn(tr("Description"),-1, Qt::AlignLeft, true, "pkghead_descrip");
  _req->addColumn(tr("Version"),    -1, Qt::AlignRight,true, "pkghead_version");

  _dep->addColumn(tr("Package"),    -1, Qt::AlignLeft, true, "pkghead_name");
  _dep->addColumn(tr("Description"),-1, Qt::AlignLeft, true, "pkghead_descrip");
  _dep->addColumn(tr("Version"),    -1, Qt::AlignRight,true, "pkghead_version");

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

}

package::~package()
{
  // no need to delete child widgets, Qt does it all for us
}

void package::languageChange()
{
  retranslateUi(this);
}

enum SetResponse package::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("pkghead_id", &valid);
  if (valid)
  {
    _pkgheadid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _name->setEnabled(false);
      _version->setEnabled(false);
      _description->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(false);
      _version->setEnabled(false);
      _description->setEnabled(false);
      _developer->setEnabled(false);
      _notes->setEnabled(false);
      _enabled->setEnabled(false);
      _indev->setEnabled(false);

      _save->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();
    }
  }

  return NoError;
}

void package::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT pkghead_id "
               "FROM pkghead "
               "WHERE (UPPER(pkghead_name)=UPPER(:pkghead_name));" );
    q.bindValue(":pkghead_code", _name->text());
    q.exec();
    if (q.first())
    {
      _pkgheadid = q.value("pkghead_id").toInt();
      _mode = cEdit;
      populate();
    }
  }
}

void package::sSave()
{
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('pkghead_pkghead_id_seq') AS _pkghead_id");
    if (q.first())
      _pkgheadid = q.value("_pkghead_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO pkghead ("
               "  pkghead_id, pkghead_name, pkghead_descrip,"
               "  pkghead_version, pkghead_developer, pkghead_notes,"
               "  pkghead_indev"
               ") VALUES ("
               "  :pkghead_id, :pkghead_name, :pkghead_descrip, "
               "  :pkghead_version, :pkghead_developer, :pkghead_notes,"
               "  :pkghead_indev);");
  }
  else if (_mode == cEdit)
  {
    q.prepare( "SELECT pkghead_id "
               "FROM pkghead "
               "WHERE ( (UPPER(pkghead_name)=UPPER(:pkghead_name))"
               " AND (pkghead_id<>:pkghead_id) );" );
    q.bindValue(":pkghead_id",   _pkgheadid);
    q.bindValue(":pkghead_name", _name->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Package"),
                            tr("<p>You may not rename this Package to %1 as "
                               "this value is used by a different Package.") );
      return;
    }

    q.prepare( "UPDATE pkghead "
               "SET pkghead_name=:pkghead_name,"
               "    pkghead_descrip=:pkghead_descrip,"
               "    pkghead_version=:pkghead_version,"
               "    pkghead_developer=:pkghead_developer,"
               "    pkghead_notes=:pkghead_notes,"
               "    pkghead_indev=:pkghead_indev "
               "WHERE (pkghead_id=:pkghead_id);" );
  }

  q.bindValue(":pkghead_id",       _pkgheadid);
  q.bindValue(":pkghead_name",     _name->text());
  q.bindValue(":pkghead_descrip",  _description->text());
  q.bindValue(":pkghead_version",  _version->text());
  q.bindValue(":pkghead_developer",_developer->text());
  q.bindValue(":pkghead_notes",    _notes->text());
  q.bindValue(":pkghead_indev",    _indev->isChecked());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_pkgheadid);
}

void package::populate()
{
  if (DEBUG)    qDebug("package::populate() entered");

  q.prepare("SELECT *, packageIsEnabled(pkghead_name) AS enabled "
            "FROM pkghead WHERE (pkghead_id=:pkghead_id);");
  q.bindValue(":pkghead_id", _pkgheadid);
  q.exec();
  if (q.first())
  {
    if (DEBUG)    qDebug("package::populate() select pkghead succeeded");
    _name->setText(q.value("pkghead_name").toString());
    _description->setText(q.value("pkghead_descrip").toString());
    if (DEBUG)    qDebug("package::populate() select pkghead half done");
    _version->setText(q.value("pkghead_version").toString());
    _developer->setText(q.value("pkghead_developer").toString());
    _notes->setText(q.value("pkghead_notes").toString());
    _enabled->setChecked(q.value("enabled").toBool());
    _indev->setChecked(q.value("pkghead_indev").toBool());
    if (DEBUG)    qDebug("package::populate() select pkghead complete");
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT *,"
            "       CASE WHEN pkgitem_type='C' THEN :script"
            "            WHEN pkgitem_type='D' THEN :cmd"
            "            WHEN pkgitem_type='F' THEN :function"
            "            WHEN pkgitem_type='G' THEN :trigger"
            "            WHEN pkgitem_type='I' THEN :image"
            "            WHEN pkgitem_type='M' THEN :metasql"
            "            WHEN pkgitem_type='P' THEN :priv"
            "            WHEN pkgitem_type='R' THEN :report"
            "            WHEN pkgitem_type='S' THEN :schema"
            "            WHEN pkgitem_type='T' THEN :table"
            "            WHEN pkgitem_type='U' THEN :uiform"
            "            WHEN pkgitem_type='V' THEN :view"
            "       ELSE pkgitem_type END AS type "
            "FROM pkgitem "
            "WHERE (pkgitem_pkghead_id=:pkghead_id) "
            "ORDER BY type, pkgitem_name;");
  q.bindValue(":pkghead_id", _pkgheadid);
  q.bindValue(":script",     tr("Script"));
  q.bindValue(":cmd",        tr("Custom Command"));
  q.bindValue(":function",   tr("Stored Procedure"));
  q.bindValue(":trigger",    tr("Trigger"));
  q.bindValue(":image",      tr("Image"));
  q.bindValue(":metasql",    tr("MetaSQL"));
  q.bindValue(":priv",       tr("Privilege"));
  q.bindValue(":report",     tr("Report"));
  q.bindValue(":schema",     tr("Schema"));
  q.bindValue(":table",      tr("Table"));
  q.bindValue(":uiform",     tr("Screen"));
  q.bindValue(":view",       tr("View"));

  q.exec();
  if (DEBUG)    qDebug("package::populate() select pkgitem exec'ed");
  _rec->populate(q);
  if (DEBUG)    qDebug("package::populate() populate pkgitem done");
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // TODO: make this recursive?
  q.prepare("SELECT * "
            "FROM pkgdep, pkghead "
            "WHERE ((pkgdep_pkghead_id=pkghead_id)"
            "  AND  (pkgdep_parent_pkghead_id=:pkghead_id));");
  q.bindValue(":pkghead_id", _pkgheadid);
  q.exec();
  if (DEBUG)    qDebug("package::populate() select pkgdep exec'ed");
  _dep->populate(q);
  if (DEBUG)    qDebug("package::populate() populate pkgdep done");
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // TODO: make this recursive?
  q.prepare("SELECT * "
            "FROM pkgdep, pkghead "
            "WHERE ((pkgdep_parent_pkghead_id=pkghead_id)"
            "  AND  (pkgdep_pkghead_id=:pkghead_id));");
  q.bindValue(":pkghead_id", _pkgheadid);
  q.exec();
  if (DEBUG)    qDebug("package::populate() select pkgdep exec'ed");
  _req->populate(q);
  if (DEBUG)    qDebug("package::populate() populate pkgdep done");
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

}
