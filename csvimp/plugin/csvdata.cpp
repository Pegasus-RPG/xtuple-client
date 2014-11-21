/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "csvdata.h"

#include <QDebug>
#include <QFile>
#include <QProgressDialog>

#include "interactivemessagehandler.h"

#define INPUTBUFSIZE 1024

class CSVDataPrivate
{
  public:
    CSVDataPrivate(CSVData *parent)
      : _parent(parent)
    {
    }

    // parser variables
    bool    inQuote;
    QByteArray *field;
    bool    haveText;
    int     row;
    int     col;
    int     maxcols;
    QStringList record;

    bool parseInit()
    {
      inQuote  = false;
      field    = new QByteArray(INPUTBUFSIZE, '\0');
      field->clear(); // without this, sometimes the value(0,0) is empty
      haveText = false;
      row = 0;
      col = 0;
      maxcols = 0;
      record = QStringList();

      _model.clear();
      _header.clear();

      return true;
    }

    bool parse(QByteArray &line)
    {
      for (int i = 0; i < line.length(); i++)
      {
        char c = line.at(i);
        if (inQuote) // handle everything differently inside double-quotes
        {
          if('"' == c && '"' == line.at(i + 1))
          {
            field->append(c);
            i++;
          }
          else if ('"' == c)
            inQuote = false;
          else
          {
            field->append(c);
            haveText = true;
          }
        }
        else
        {
          if (_parent->delimiter() == c || '\r' == c || '\n' == c)
          {
            if (! field->isNull() && haveText)
              record.append(field->trimmed());
            else
              record.append(QString::null);

            col++;

            field->clear();
            inQuote  = false;
            haveText = false;

            // TODO: did Qt strip out the \r's when reading the file?
            if (('\r' == c && '\n' == line.at(i + 1)) ||
                ('\n' == c && '\r' == line.at(i + 1)))
              c = line.at(++i);

            if ('\r' == c || '\n' == c)
            {
              _model.append(record);
              record = QStringList();
              row++;
              if (col > maxcols)
                maxcols = col;
              col = 0;
            }
          }
          else if(('"' == c) && (_parent->delimiter() != '\t'))
          {
            if (line.at(i + 1) == '"')
            {
              field->append(c);
              i++;
            }
            else
            {
              inQuote = true;
              field->clear();
            }
          }
          else if (haveText && QChar(c).isSpace())
            field->append(c);
          else
          {
            haveText = true;
            field->append(c);
          }
        }
      }

      return true;
    }

    bool parseCleanup()
    {
      if (haveText)
      {
        record.append(field->trimmed());
        _model.append(record);
      }

      if (_parent->firstRowHeaders() && ! _model.isEmpty())
      {
        _header = _model.at(0);
        _model.takeFirst();
      }

      delete field;

      return true;
    }

    QStringList         _line;
    QString             _filename;
    QStringList         _header;
    QList<QStringList>  _model;
    CSVData            *_parent;
};

CSVData::CSVData(QObject *parent, const char *name, const QChar delim)
  : QObject(parent),
    _data(0),
    _firstRowHeaders(false)
{
  _data = new CSVDataPrivate(this);
  setObjectName(name ? name : "_CSVData");
  _msghandler = new InteractiveMessageHandler(this);
  setDelimiter(delim);
}

CSVData::~CSVData() {
  if (_data)
  {
    delete _data;
    _data = 0;
  }
}

unsigned int CSVData::columns()
{
  unsigned int n = 0;
  if (_data)
    n = _data->maxcols;

  return n;
}

QChar CSVData::delimiter() const
{
  return _delimiter;
}

void CSVData::setDelimiter(const QChar delim)
{
  QChar newdelim = delim.isNull() ? ',' : delim;
  if (newdelim != _delimiter)
  {
    _delimiter = newdelim;
    if (_data && ! _data->_filename.isEmpty())
      load(_data->_filename, qobject_cast<QWidget*>(parent()));
  }
}

bool CSVData::firstRowHeaders() const
{
  return _firstRowHeaders;
}

void CSVData::setFirstRowHeaders(bool y)
{
  if (_firstRowHeaders != y)
  {
    _firstRowHeaders = y;
    if (_data)
    {
      if (y && ! _data->_model.isEmpty())
      {
        _data->_header = _data->_model.at(0);
        _data->_model.takeFirst();
      }
      else if (! y && ! _data->_header.isEmpty())
      {
        _data->_model.prepend(_data->_header);
        _data->_header.clear();
      }
    }
  }
}

QString CSVData::header(int column)
{
  if (_firstRowHeaders && _data && _data->_header.size() >= column)
    return _data->_header.at(column);

  return QString::null;
}

bool CSVData::load(QString filename, QWidget *parent)
{
  _data->_filename = filename;
  QFile file(filename);

  if(!file.open(QIODevice::ReadOnly))
  {
    _msghandler->message(QtWarningMsg, tr("Open Failed"),
                         tr("<p>Could not open %1 for reading: %2")
                         .arg(filename, file.errorString()));
    return false;
  }

  QString          progresstext(tr("Loading %1: line %2"));
  QProgressDialog *progress = 0;
  qint64           bytes    = 0;
  int              expected = file.size();
  bool             result   = true;

  if (parent)
  {
    progress = new QProgressDialog(progresstext.arg(filename).arg(0),
                                   tr("Stop"), bytes, expected, parent);
    progress->setWindowModality(Qt::WindowModal);
    progress->setValue(0);
  }

  _data->parseInit();
  char   buf[INPUTBUFSIZE];

  for (qint64 lines = 0; ! file.atEnd(); lines++)
  {
    qint64 lineLength = file.readLine(buf, sizeof(buf));
    if (lineLength == -1)
    {
      _msghandler->message(QtWarningMsg, tr("Read Error"),
                           tr("<p>Error Reading %1: %2")
                             .arg(filename, file.errorString()));
      if (progress)
        progress->cancel();
      result = false;
      break;
    }

    QByteArray ba(buf);
    if (! _data->parse(ba))
    {
      _msghandler->message(QtWarningMsg, tr("Parsing Error"),
                           tr("<p>Error parsing the data from %1 (delimiter %2).")
                             .arg(filename).arg(_delimiter));
      if (progress)
        progress->cancel();
      result = false;
      break;
    }

    if (progress)
    {
      if (progress->wasCanceled())
      {
        result = false;
        break;
      }
      bytes += lineLength;
      if ((lines % 10000) == 0)
      {
        progress->setValue(bytes);
        progress->setLabelText(progresstext.arg(filename).arg(lines));
      }
    }
  }

  _data->parseCleanup();

  file.close();

  if (progress)
    progress->setValue(expected);

  return result;
}

XAbstractMessageHandler *CSVData::messageHandler() const
{
  return _msghandler;
}

void CSVData::setMessageHandler(XAbstractMessageHandler *handler)
{
  _msghandler = handler;
}

unsigned int CSVData::rows()
{
  int n = 0;
  if (_data)
    n = _data->_model.size();

  return n;
}

QString CSVData::value(int row, int column)
{
  QString result = QString::null;

  if (_data &&
      row < _data->_model.size() && column < _data->_model.at(row).size())
    result = _data->_model.at(row).at(column);

  return result;
}
