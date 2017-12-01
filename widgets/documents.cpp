/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QDebug>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>
#include <QtScript>

#include <openreports.h>
#include <parameter.h>
#include <xsqlquery.h>
#include <metasql.h>
#include "mqlutil.h"

#include "documents.h"
#include "errorReporter.h"
#include "imageview.h"
#include "imageAssignment.h"
#include "docAttach.h"

QMap<QString, struct DocumentMap*> Documents::_strMap;
QMap<int,     struct DocumentMap*> Documents::_intMap;

/** Add another document type to the map by both key and int.

    Actually there are two parallel maps, one by DocumentSources/integer
    and the other by string key. This simplifies lookups and provides
    backward compatibility with existing UI file definitions.

    @param id    A DocumentSources enum value or a unique extension-supplied int
    @param key   A human-readable abbreviation for the document type
    @param trans A human-readable, translatable term for the document type
    @param param The name of the parameter a window checks for the record id
    @param ui    The application or extension window to create a new record
    @param priv  The privileges required to create a new record

    @return true on success, false if this entry would create a duplicate
            on either id or key.
 */
bool Documents::addToMap(int id,        QString key, QString trans,
                         QString param, QString ui,  QString priv)
{
  if (_strMap.contains(key) || _intMap.contains(id)) {
    qDebug() << "Documents::addToMap(" << id << ", " << key << ") duplicate!";
    return false;
  }

  DocumentMap *entry = new DocumentMap(id, key, trans, param, ui, priv);
  _strMap.insert(key, entry);
  _intMap.insert(id,  entry);
  return true;
}

// Inconsistencies between here and the rest of the app: S? Q?
QMap<QString, struct DocumentMap *> &Documents::documentMap() {
  if (! _x_privileges)
    return _strMap;

  if (_strMap.isEmpty()) {
    XSqlQuery q("SELECT * FROM source;");
    addToMap(Uninitialized,     "",      tr("[Pick a Document Type]")                           );
    while (q.next()) {
      addToMap(q.value("source_docass_num").toInt(),
               q.value("source_docass").toString(),
               tr(q.value("source_descrip").toString().toLatin1().data()),
               q.value("source_key_param").toString(),
               q.value("source_uiform_name").toString(),
               q.value("source_create_priv").toString());
    }
    ErrorReporter::error(QtCriticalMsg, 0, tr("Error Getting Document Types"),
                         q, __FILE__, __LINE__);
  }

  return _strMap;
}

GuiClientInterface* Documents::_guiClientInterface = 0;

Documents::Documents(QWidget *pParent) :
  QWidget(pParent), ScriptableWidget(this)
{
  setupUi(this);

  _sourceid = -1;
  _readOnly = false;
  if (_strMap.isEmpty()) {
    (void)documentMap();
  }

  _doc->addColumn(tr("Type"),  _itemColumn,  Qt::AlignLeft, true, "target_type" );
  _doc->addColumn(tr("Number"), _itemColumn, Qt::AlignLeft, true, "target_number" );
  _doc->addColumn(tr("Name"), -1,  Qt::AlignLeft, true, "name" );
  _doc->addColumn(tr("Description"),  -1, Qt::AlignLeft, true, "description");
  _doc->addColumn(tr("Relationship"),  _itemColumn, Qt::AlignLeft,true, "purpose");
  _doc->addColumn(tr("Can View"), _ynColumn, Qt::AlignCenter, false, "canview");
  _doc->addColumn(tr("Can Edit"), _ynColumn, Qt::AlignCenter, false, "canedit");
  
  connect(_attachDoc, SIGNAL(clicked()), this, SLOT(sAttachDoc()));
  connect(_editDoc, SIGNAL(clicked()), this, SLOT(sEditDoc()));
  connect(_viewDoc, SIGNAL(clicked()), this, SLOT(sViewDoc()));
  connect(_detachDoc, SIGNAL(clicked()), this, SLOT(sDetachDoc()));
  connect(_doc, SIGNAL(valid(bool)), this, SLOT(handleSelection()));
  connect(_doc, SIGNAL(itemSelected(int)), this, SLOT(handleItemSelected()));
  handleSelection();

  if (_x_privileges)
  {
    QMenu * newDocMenu = new QMenu;

    QAction* imgAct = new QAction(tr("Image"), this);
    imgAct->setEnabled(_x_privileges->check("MaintainImages"));
    connect(imgAct, SIGNAL(triggered()), this, SLOT(sNewImage()));
    newDocMenu->addAction(imgAct);

    foreach (DocumentMap *value, _strMap) {
      if (! value->newPriv.isEmpty()) {
        QAction* act = new QAction(value->translation, this);
        QStringList data;
        data << value->doctypeStr << value->uiname;
        act->setEnabled(_x_privileges->check(value->newPriv));
        act->setData(data);
        connect(act, SIGNAL(triggered()), this, SLOT(sNewDoc()));
        newDocMenu->addAction(act);
      }
    }

    _newDoc->setMenu(newDocMenu);
  }
}

int Documents::type() const
{
  DocumentMap *elem = _strMap.value(_sourcetype);
  return elem ? elem->doctypeId : Uninitialized;
}

void Documents::setType(int sourceType)
{
  DocumentMap *elem = _intMap.value(sourceType);
  setType(elem ? elem->doctypeStr : "");
}

void Documents::setType(QString sourceType)
{
  _sourcetype = sourceType;
}

void Documents::setId(int pSourceid)
{
  _sourceid = pSourceid;
  loadScriptEngine();
  refresh();
}

void Documents::setReadOnly(bool pReadOnly)
{
  _readOnly = pReadOnly;

  _newDoc->setEnabled(!pReadOnly);
  _attachDoc->setEnabled(!pReadOnly);
  _editDoc->setEnabled(!pReadOnly);
  _detachDoc->setEnabled(!pReadOnly);

  handleSelection();
}

void Documents::showEvent(QShowEvent *e)
{
  loadScriptEngine();
  QWidget::showEvent(e);
}

void Documents::sNewDoc(QString ptype, QString pui)
{
  QString  type = ptype;
  QString  ui   = pui;
  QAction *act = qobject_cast<QAction*>(sender());
  if (act) {
    QStringList elem = act->data().toStringList();
    type = elem.at(0);
    ui   = elem.at(1);
  }

  ParameterList params;
  params.append("mode", "new");
  int target_id = -1;
  QWidget *window = _guiClientInterface->openWindow(ui, params, parentWidget(), Qt::WindowModal, Qt::Dialog);
  QDialog *newdlg = qobject_cast<QDialog*>(window);
  if (newdlg)
  {
    target_id = newdlg->exec();
    if (target_id != QDialog::Rejected) {
      sInsertDocass(type, target_id);
    }
  }
  else if (window)
  {
    window->show();
    // TODO: how do we get the ID and when?
    // sInsertDocass(type, target_id);
  }
  else
  {
    QMessageBox::critical(this, tr("Error Creating Document"),
                          tr("Cannot find the '%1'' window to create a %2").arg(ui, type));
  }
  refresh();
}

void Documents::sNewImage()
{
  ParameterList params;
  params.append("sourceType", _sourcetype);
  params.append("source_id", _sourceid);

  imageAssignment newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    refresh();
}

void Documents::sInsertDocass(QString target_type, int target_id)
{
  XSqlQuery ins;
  ins.prepare("INSERT INTO docass ( docass_source_id, docass_source_type, docass_target_id, docass_target_type )"
            "  VALUES ( :sourceid, :sourcetype::text, :targetid, :targettype); ");
  ins.bindValue(":sourceid", _sourceid);
  ins.bindValue(":sourcetype", _sourcetype);
  ins.bindValue(":targetid", target_id);
  ins.bindValue(":targettype", target_type);
  ins.exec();
  ErrorReporter::error(QtCriticalMsg, this, tr("Attachment Error"),
                       ins, __FILE__, __LINE__);
}

void Documents::sEditDoc()
{
  sOpenDoc("edit");
}

void Documents::sOpenDoc(QString mode)
{
  QString ui;
  QString docType = _doc->currentItem()->rawValue("target_type").toString();
  int targetid = _doc->currentItem()->id("target_number");
  ParameterList params;
  if (docType == "Q" && mode == "view")
    params.append("mode", "viewQuote");
  else if (docType == "Q" && mode == "edit")
    params.append("mode", "editQuote");
  else
    params.append("mode", mode);

  // TODO: image -- change to use docass instead of imageass
  if (docType == "IMG")
  {
    XSqlQuery img;
    img.prepare("SELECT imageass_image_id "
                "FROM imageass "
                "WHERE (imageass_id=:imageass_id); ");
    img.bindValue(":imageass_id", _doc->id());
    img.exec();
    img.first();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Image Info"),
                             img, __FILE__, __LINE__))
      return;

    params.append("image_id", img.value("imageass_image_id").toInt());
    imageview newdlg(this, "", true);
    newdlg.set(params);

    if (newdlg.exec() != QDialog::Rejected)
      refresh();
    return;
  }
  // TODO: url -- change to use docass instead of url
  // TODO: separate URL from FILE handling and replace use of url view
  else if (docType == "URL" || docType == "FILE")
  {
    if (mode == "edit")
    {
      ParameterList params;
      params.append("url_id", targetid);
      params.append("sourceType", _sourcetype);
      params.append("source_id", _sourceid);

      docAttach newdlg(this, "", true);
      newdlg.set(params);
      newdlg.exec();

      refresh();
      return;
    }

    XSqlQuery qfile;
    qfile.prepare("SELECT url_id, url_source_id, url_source, url_title, url_url, url_stream"
                  " FROM url"
                  " WHERE (url_id=:url_id);");

    qfile.bindValue(":url_id", _doc->id());
    qfile.exec();

    // If file is in the database, copy to a temp. directory in the file system and open it.
    if (qfile.first() && (docType == "FILE"))
    {
      QFileInfo fi( qfile.value("url_url").toString() );
      QDir tdir;
      // TODO: QDesktopServices::openUrl(urldb) on windows does not open files
      // containing spaces. why not?
#ifdef Q_OS_WIN
      QString fileName = fi.fileName().remove(" ");
#else
      QString fileName = fi.fileName();
#endif
      QString filePath = tdir.tempPath() + "/xtTempDoc/" +
	                 qfile.value("url_id").toString() + "/";
      QFile tfile(filePath + fileName);

      // Remove any previous watches
      if (_guiClientInterface)
        _guiClientInterface->removeDocumentWatch(tfile.fileName());

      if (! tdir.exists(filePath))
        tdir.mkpath(filePath);

      if (!tfile.open(QIODevice::WriteOnly))
      {
        QMessageBox::warning( this, tr("File Open Error"),
                             tr("Could Not Create File %1.").arg(tfile.fileName()) );
        return;
      }
      tfile.write(qfile.value("url_stream").toByteArray());
      QUrl urldb;
      urldb.setUrl(tfile.fileName());
#ifndef Q_OS_WIN
      urldb.setScheme("file");
#endif
      tfile.close();
      if (! QDesktopServices::openUrl(urldb))
      {
        QMessageBox::warning(this, tr("File Open Error"),
			     tr("Could not open %1.").arg(urldb.toString()));
	return;
      }

      // Add a watch to the file that will save any changes made to the file back to the database.
      if (_guiClientInterface && !_readOnly) // TODO: only if NOT read-only
        _guiClientInterface->addDocumentWatch(tfile.fileName(),qfile.value("url_id").toInt());
      return;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this,
                                  tr("Error Getting Assignment"),
                                  qfile, __FILE__, __LINE__))
      return;
    else
    {
      QUrl url(_doc->currentItem()->rawValue("description").toString());
      if (url.scheme().isEmpty())
        url.setScheme("file");
      QDesktopServices::openUrl(url);
      return;
    }
  }
  else
  {
    struct DocumentMap *elem = _strMap.value(docType);
    if (! elem) {
      QMessageBox::critical(this, tr("Error"),
                            tr("Unknown document type %1").arg(docType));
      return;
    }

    params.append(elem->idParam, targetid);
    ui = elem->uiname;
  }

  QWidget* w = 0;
  if (parentWidget()->window())
  {
    if (parentWidget()->window()->isModal())
      w = _guiClientInterface->openWindow(ui, params, parentWidget()->window(),
                                          Qt::WindowModal, Qt::Dialog);
    else
      w = _guiClientInterface->openWindow(ui, params, parentWidget()->window(),
                                          Qt::NonModal, Qt::Window);
  }

  if (w && w->inherits("QDialog"))
  {
    QDialog* newdlg = qobject_cast<QDialog*>(w);
    newdlg->exec();
  }

  refresh();
}

void Documents::sViewDoc()
{
  sOpenDoc("view");
}

void Documents::sAttachDoc()
{
  ParameterList params;
  params.append("sourceType", _sourcetype);
  params.append("source_id", _sourceid);

  docAttach newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();

  refresh();
}

void Documents::sDetachDoc()
{
  XSqlQuery detach;
  if (_doc->id() < 0)
    return;

  if(QMessageBox::question( this, tr("Confirm Detach"),
       tr("<p>You have requested to detach the selected document."
          " In some cases this may permanently remove the document from the system.</p>"
          "<p>Are you sure you want to continue?</p>"), QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No)
    return; // user doesn't want to continue so get out of here

  if ( _doc->currentItem()->rawValue("target_type") == "IMG" )
  {
    detach.prepare( "DELETE FROM imageass "
                    "WHERE (imageass_id = :docid );" );
  }
  else if ( _doc->currentItem()->rawValue("target_type") == "URL" ||
            _doc->currentItem()->rawValue("target_type") == "FILE")
  {
    detach.prepare( "DELETE FROM url "
                    "WHERE (url_id = :docid );" );
  }
  else
  {
    detach.prepare( "DELETE FROM docass "
                    "WHERE (docass_id = :docid );" );
  }
  detach.bindValue(":docid", _doc->id());
  detach.exec();
  ErrorReporter::error(QtCriticalMsg, this, tr("Error Detaching"),
                       detach, __FILE__, __LINE__);
  refresh();
}

void Documents::refresh()
{
  if(-1 == _sourceid)
  {
    _doc->clear();
    return;
  }

  //Populate doc list
  MetaSQLQuery mql = mqlLoad("documents", "list");
  ParameterList params;

  params.append("inventory", tr("Inventory Description"));
  params.append("product", tr("Product Description"));
  params.append("engineering", tr("Engineering Reference"));
  params.append("misc", tr("Miscellaneous"));

  params.append("parent", tr("Parent"));
  params.append("child", tr("Child"));
  params.append("sibling", tr("Related to"));
  params.append("dupe", tr("Duplicate of"));
  params.append("other", tr("Other"));

  params.append("address",        _strMap.contains("ADDR") ? _strMap.value("ADDR")->translation : "ADDR");
  params.append("bbomhead",       _strMap.contains("BBH") ? _strMap.value("BBH")->translation : "BBH");
  params.append("bbomitem",       _strMap.contains("BBI") ? _strMap.value("BBI")->translation : "BBI");
  params.append("bomhead",        _strMap.contains("BMH") ? _strMap.value("BMH")->translation : "BMH");
  params.append("bomitem",        _strMap.contains("BMI") ? _strMap.value("BMI")->translation : "BMI");
  params.append("boohead",        _strMap.contains("BOH") ? _strMap.value("BOH")->translation : "BOH");
  params.append("booitem",        _strMap.contains("BOI") ? _strMap.value("BOI")->translation : "BOI");
  params.append("crma",           _strMap.contains("CRMA") ? _strMap.value("CRMA")->translation : "CRMA");
  params.append("contact",        _strMap.contains("T") ? _strMap.value("T")->translation : "T");
  params.append("contract",       _strMap.contains("CNTR") ? _strMap.value("CNTR")->translation : "CNTR");
  params.append("creditmemo",     _strMap.contains("CM") ? _strMap.value("CM")->translation : "CM");
  params.append("creditmemoitem", _strMap.contains("CMI") ? _strMap.value("CMI")->translation : "CMI");
  params.append("cust",           _strMap.contains("C") ? _strMap.value("C")->translation : "C");
  params.append("emp",            _strMap.contains("EMP") ? _strMap.value("EMP")->translation : "EMP");
  params.append("incident",       _strMap.contains("INCDT") ? _strMap.value("INCDT")->translation : "INCDT");
  params.append("invoice",        _strMap.contains("INV") ? _strMap.value("INV")->translation : "INV");
  params.append("invoiceitem",    _strMap.contains("INVI") ? _strMap.value("INVI")->translation : "INVI");
  params.append("item",           _strMap.contains("I") ? _strMap.value("I")->translation : "I");
  params.append("itemsite",       _strMap.contains("IS") ? _strMap.value("IS")->translation : "IS");
  params.append("itemsrc",        _strMap.contains("IR") ? _strMap.value("IR")->translation : "IR");
  params.append("location",       _strMap.contains("L") ? _strMap.value("L")->translation : "L");
  params.append("lotserial",      _strMap.contains("LS") ? _strMap.value("LS")->translation : "LS");
  params.append("opp",            _strMap.contains("OPP") ? _strMap.value("OPP")->translation : "OPP");
  params.append("project",        _strMap.contains("J") ? _strMap.value("J")->translation : "J");
  params.append("po",             _strMap.contains("P") ? _strMap.value("P")->translation : "P");
  params.append("poitem",         _strMap.contains("PI") ? _strMap.value("PI")->translation : "PI");
  params.append("ra",             _strMap.contains("RA") ? _strMap.value("RA")->translation : "RA");
  params.append("raitem",         _strMap.contains("RI") ? _strMap.value("RI")->translation : "RI");
  params.append("quote",          _strMap.contains("Q") ? _strMap.value("Q")->translation : "Q");
  params.append("quoteitem",      _strMap.contains("QI") ? _strMap.value("QI")->translation : "QI");
  params.append("so",             _strMap.contains("S") ? _strMap.value("S")->translation : "S");
  params.append("soitem",         _strMap.contains("SI") ? _strMap.value("SI")->translation : "SI");
  params.append("shipto",         _strMap.contains("SHP") ? _strMap.value("SHP")->translation : "SHP");
  params.append("timeexpense",    _strMap.contains("TE") ? _strMap.value("TE")->translation : "TE");
  params.append("todo",           _strMap.contains("TODO") ? _strMap.value("TODO")->translation : "TODO");
  params.append("to",             _strMap.contains("TO") ? _strMap.value("TO")->translation : "TO");
  params.append("toitem",         _strMap.contains("TI") ? _strMap.value("TI")->translation : "TI");
  params.append("vendor",         _strMap.contains("V") ? _strMap.value("V")->translation : "V");
  params.append("voucher",        _strMap.contains("VCH") ? _strMap.value("VCH")->translation : "VCH");
  params.append("whse",           _strMap.contains("WH") ? _strMap.value("WH")->translation : "WH");
  params.append("wo",             _strMap.contains("W") ? _strMap.value("W")->translation : "W");
  params.append("projecttask",    _strMap.contains("TASK") ? _strMap.value("TASK")->translation : "TASK");

  params.append("image", tr("Image"));
  params.append("url", tr("URL"));
  params.append("file", tr("File"));

  params.append("source",   _sourcetype);
  params.append("sourceid", _sourceid);

  setScriptableParams(params);

  XSqlQuery query = mql.toQuery(params);

  _doc->populate(query, true);
  ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Documents"),
                       query, __FILE__, __LINE__);
}

void Documents::handleSelection(bool /*pReadOnly*/)
{
  if (_doc->selectedItems().count() &&
      (_doc->currentItem()->rawValue("target_type").toString() == "URL" ||
       _doc->currentItem()->rawValue("target_type").toString() == "FILE" ))
  {
    _viewDoc->setText(tr("Open"));
  }
  else
  {
    _viewDoc->setText(tr("View"));
  }

  bool valid = (_doc->selectedItems().count() > 0);
  _editDoc->setEnabled(valid && !_readOnly && _doc->currentItem()->rawValue("canedit").toBool());
  _viewDoc->setEnabled(valid && _doc->currentItem()->rawValue("canview").toBool());
}

void Documents::handleItemSelected()
{
  if(_readOnly ||
     (_doc->currentItem()->rawValue("target_type").toString() == "URL" ||
      _doc->currentItem()->rawValue("target_type").toString() == "FILE" ))
  {
    _viewDoc->animateClick();
  }
  else
  {
    _editDoc->animateClick();
  }
}

// script api //////////////////////////////////////////////////////////////////

void setupDocuments(QScriptEngine *engine)
{
  if (! engine->globalObject().property("Documents").isObject())
  {
    QScriptValue ctor = engine->newObject(); //engine->newFunction(scriptconstructor);
    QScriptValue meta = engine->newQMetaObject(&Documents::staticMetaObject, ctor);

    engine->globalObject().setProperty("Documents", meta,
                                       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  }
}
