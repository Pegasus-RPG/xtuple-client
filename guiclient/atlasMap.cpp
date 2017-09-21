/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "atlasMap.h"

#include <QAbstractMessageHandler>
#include <QDomDocument>
#include <QFile>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QXmlQuery>

#include "storedProcErrorLookup.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

#define DEBUG false

// TODO: replace with XMessageBoxMessageHandler
class AtlasMapMessageHandler : public QAbstractMessageHandler
{
  public:
    AtlasMapMessageHandler(QObject *p = 0) : QAbstractMessageHandler(p)
    {
    }

  protected:
    virtual void handleMessage(QtMsgType type, const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation)
    {
      QString msg = atlasMap::tr("<p>There was a problem reading the Atlas file, "
                       "line %1 column %2: %3<br>(%4)")
                        .arg(sourceLocation.line())
                        .arg(sourceLocation.column())
                        .arg(description)
                        .arg(identifier.toString());
      switch (type)
      {
        case QtDebugMsg:
          QMessageBox::information(0, atlasMap::tr("XML Error"), msg);
          break;
        case QtWarningMsg:
          QMessageBox::warning(0, atlasMap::tr("XML Error"), msg);
          break;
        case QtCriticalMsg:
        case QtFatalMsg:
        default:
          QMessageBox::critical(0, atlasMap::tr("XML Error"), msg);
          break;
      }
    }
};

bool atlasMap::userHasPriv()
{
  return _privileges->check("ConfigureImportExport");
}

int atlasMap::exec()
{
  if (userHasPriv())
    return XDialog::exec();
  else
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                         tr("%1: Insufficient privileges to view this window")
                         .arg(windowTitle()),__FILE__,__LINE__);
    return XDialog::Rejected;
  }
}

atlasMap::atlasMap(QWidget* parent, const char * name, Qt::WindowFlags fl)
    : XDialog(parent, name, fl)
{
  setupUi(this);

  connect(_atlasFile, SIGNAL(editingFinished()), this, SLOT(sHandleAtlas()));
  connect(_atlasDb, SIGNAL(newID(int)), this, SLOT(sHandleAtlas()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_fileSelect, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_dbSelect, SIGNAL(clicked()), this, SLOT(sHandleButtons()));

  QString filter = tr("Atlas Files (*.xml)");
  _atlasFile->setFilter(filter);

  _filtertype->append(0, tr("File name"),          "filename");
  _filtertype->append(1, tr("First line of file"), "firstline");

  _atlasDb->populate("SELECT atlas_id, atlas_name FROM atlas");

  _mode         = cNew;
  _atlasmapId   = -1;
  _msghandler   = new AtlasMapMessageHandler(this);
#if defined Q_OS_MAC
  _defaultDir   = _metrics->value("CSVAtlasDefaultDirMac");
#elif defined Q_OS_WIN
  _defaultDir   = _metrics->value("CSVAtlasDefaultDirWindows");
#elif defined Q_OS_LINUX
  _defaultDir   = _metrics->value("CSVAtlasDefaultDirLinux");
#endif
}

atlasMap::~atlasMap()
{
  // no need to delete child widgets, Qt does it all for us
}

void atlasMap::languageChange()
{
  retranslateUi(this);
}

enum SetResponse atlasMap::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("defaultDir", &valid);
  if (valid)
    _defaultDir = param.toString();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(false);
      _filter->setEnabled(false);
      _filtertype->setEnabled(false);
      _atlasStack->setEnabled(false);
      _map->setEnabled(false);
      _firstLine->setEnabled(false);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
  }

  param = pParams.value("atlasmap_id", &valid);
  if (valid)
  {
    _atlasmapId = param.toInt();
    sPopulate();
  }

  return NoError;
}

void atlasMap::sSave()
{

  QList<GuiErrorCheck>errors;
  errors<<GuiErrorCheck(_name->text().trimmed().isEmpty(), _name,
                        tr("<p>Please enter a name for this Atlas Map before saving it."))
        <<GuiErrorCheck(_filter->text().trimmed().isEmpty(), _filter,
                         tr("<p>Please enter a filter before saving this Atlas Map."))
        <<GuiErrorCheck(_filtertype->currentText().trimmed().isEmpty(), _filtertype,
                        tr("<p>Please select a filter type before saving this Atlas Map."))
        <<GuiErrorCheck(_fileSelect->isChecked() && _atlasFile->text().trimmed().isEmpty(), _atlasFile,
                        tr("<p>Please enter an Atlas File Name before saving this Atlas Map."))
        <<GuiErrorCheck(_dbSelect->isChecked() && !_atlasDb->isValid(), _atlasDb,
                        tr("<p>Please select an Atlas from the database before saving this Atlas Map."))
        <<GuiErrorCheck(_map->id() <= -1, _map,
                       tr("Please select a Map Name before saving this Atlas Map."));

  if(GuiErrorCheck::reportErrors(this,tr("Cannot Save Atlas Map"),errors))
      return;

  XSqlQuery dupq;
  dupq.prepare( "SELECT atlasmap_name "
             "FROM atlasmap "
             "WHERE ((atlasmap_name=:name)"
             " AND   (atlasmap_id<>:atlasmap_id) );" );

  dupq.bindValue(":atlasmap_id",   _atlasmapId);
  dupq.bindValue(":name",          _name->text());
  dupq.exec();
  if (dupq.first())
  {
    QMessageBox::critical(this, tr("Cannot Save Atlas Map"),
                          tr("<p>This Name is already in use by another "
                             "Atlas Map."));
    _name->setFocus();
    return;
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Atlas Map Information"),
                                dupq, __FILE__, __LINE__))
  {
    return;
  }

  XSqlQuery upsq;
  if (cNew == _mode)
    upsq.prepare("INSERT INTO atlasmap ("
                 "    atlasmap_name,  atlasmap_filter, atlasmap_filtertype,"
                 "    atlasmap_atlas, atlasmap_map,    atlasmap_headerline,"
                 "    atlasmap_atlastype"
                 ") VALUES ("
                 "    :name,  :filter, :filtertype,"
                 "    :atlas, :map,    :headerline,"
                 "    :atlastype"
                 ") RETURNING atlasmap_id;");
  else if (cEdit == _mode)
  {
    upsq.prepare("UPDATE atlasmap SET"
                 "    atlasmap_name=:name,"
                 "    atlasmap_filter=:filter,"
                 "    atlasmap_filtertype=:filtertype,"
                 "    atlasmap_atlas=:atlas,"
                 "    atlasmap_map=:map,"
                 "    atlasmap_headerline=:headerline,"
                 "    atlasmap_atlastype=:atlastype"
                 " WHERE (atlasmap_id=:id)"
                 " RETURNING atlasmap_id;");
    upsq.bindValue(":id", _atlasmapId);
  }

  upsq.bindValue(":name",       _name->text());
  upsq.bindValue(":filter",     _filter->text());
  upsq.bindValue(":filtertype", _filtertype->code());
  if (_fileSelect->isChecked())
  {
    upsq.bindValue(":atlas",      _atlasFile->text());
    upsq.bindValue(":atlastype",  QString("F"));
  }
  else
  {
    upsq.bindValue(":atlas",      _atlasDb->currentText());
    upsq.bindValue(":atlastype",  QString("D"));
  }
  upsq.bindValue(":map",        _map->currentText());
  upsq.bindValue(":headerline", _firstLine->isChecked());
  upsq.exec();
  if (upsq.first())
    _atlasmapId = upsq.value("atlasmap_id").toInt();
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Atlas Map Information"),
                                upsq, __FILE__, __LINE__))
  {
    return;
  }

  accept();
}

void atlasMap::sPopulate()
{
  XSqlQuery atlasPopulate;
  if (_atlasmapId <= 0)
  {
    _name->clear();
    _filter->clear();
    _filtertype->setId(-1);
    _atlasFile->clear();
    _atlasDb->setId(-1);
    _map->setId(-1);
    _firstLine->setChecked(false);
  }
  else
  {
    atlasPopulate.prepare("SELECT * FROM atlasmap WHERE (atlasmap_id=:id);");
    atlasPopulate.bindValue(":id", _atlasmapId);
    atlasPopulate.exec();
    if (atlasPopulate.first())
    {
      _name->setText(atlasPopulate.value("atlasmap_name").toString());
      _filter->setText(atlasPopulate.value("atlasmap_filter").toString());
      _filtertype->setCode(atlasPopulate.value("atlasmap_filtertype").toString());
      if (atlasPopulate.value("atlasmap_atlastype").toString() == "F")
      {
        _fileSelect->setChecked(true);
        _atlasFile->setText(atlasPopulate.value("atlasmap_atlas").toString());
      }
      else
      {
        _dbSelect->setChecked(true);
        _atlasDb->setText(atlasPopulate.value("atlasmap_atlas").toString());
      }
      sHandleAtlas();
      if (! atlasPopulate.value("atlasmap_map").toString().isEmpty())
        _map->setCode(atlasPopulate.value("atlasmap_map").toString());
      _firstLine->setChecked(atlasPopulate.value("atlasmap_headerline").toBool());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Atlas Map Information"),
                                  atlasPopulate, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void atlasMap::sHandleAtlas()
{
  QFile file;
  QSqlQuery atlq;
  QDomDocument doc = QDomDocument();
  QString errMsg;
  int errLine, errCol;

  _map->clear();

  if ((_fileSelect->isChecked() && _atlasFile->text().isEmpty()) ||
      (_dbSelect->isChecked() && !_atlasDb->isValid()))
    return;

  if (_fileSelect->isChecked()) // Load Atlas from File
  {
    if (DEBUG)
      qDebug("atlasMap::sHandleAtlas() entered with %s and %s",
             qPrintable(_atlasFile->text()), qPrintable(_defaultDir));

    if (! _defaultDir.isEmpty() && _atlasFile->text().startsWith(_defaultDir))
      _atlasFile->setText(_atlasFile->text().remove(0, _defaultDir.length() + 1));

    if (QFile::exists(_atlasFile->text()))
      file.setFileName(_atlasFile->text());
    else if (QFile::exists(_defaultDir + QDir::separator() + _atlasFile->text()))
      file.setFileName(_defaultDir + QDir::separator() + _atlasFile->text());
    else
    {
      QMessageBox::warning(this, tr("Could not find Atlas"),
                           tr("<p>Could not find the Atlas file to open to look "
                              "for CSV import Maps."));
      return;
    }

    if(!doc.setContent(&file, &errMsg, &errLine, &errCol))
    {
      QMessageBox::warning(this, tr("Error Reading File"),
                           tr("<p>An error was encountered while trying to read "
                              "the Atlas file: %1.").arg(errMsg));
      return;
    }
  }
  else //Load Atlas from Database
  {
    if (DEBUG)
      qDebug("atlasMap::sHandleAtlas() entered with %s",
             qPrintable(_atlasDb->currentText()));

    atlq.prepare("SELECT atlas_atlasmap FROM atlas WHERE atlas_name=:atlasname;");
    atlq.bindValue(":atlasname", _atlasDb->currentText());
    atlq.exec();
    if (atlq.first())
    {
      if(!doc.setContent(atlq.value("atlas_atlasmap").toString(), &errMsg, &errLine, &errCol))
      {
        QMessageBox::warning(this, tr("Error Reading Database"),
                           tr("<p>An error was encountered while trying to read "
                              "the Atlas %1 from the Database: %2").arg(_atlasDb->currentText(), errMsg));
        return;
      }
    }
  }

  QDomNodeList nodes = doc.elementsByTagName("CSVMap");
  QStringList maplist;
  for(int i = 0; i < nodes.count(); i++)
  {
    QDomElement elem = nodes.at(i).firstChildElement("Name");
    maplist.append(elem.text());
  }
  if (maplist.size() == 0)
  {
    QMessageBox::warning(this, tr("No Maps"),
                         tr("<p>Could not find any Maps in the Atlas"));
    return;
  }
  else
    for (int i = 0; i < maplist.size(); i++)
      _map->append(i, maplist.at(i));
}

void atlasMap::sHandleButtons()
{
  if (_fileSelect->isChecked())
    _atlasStack->setCurrentIndex(0);
  else
    _atlasStack->setCurrentIndex(1);
}
