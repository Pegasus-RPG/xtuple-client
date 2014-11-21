/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <interactivemessagehandler.h>

#include <QMessageBox>

InteractiveMessageHandler::InteractiveMessageHandler(QObject *parent)
  : XAbstractMessageHandler(parent)
{
}

InteractiveMessageHandler::~InteractiveMessageHandler()
{
}

void InteractiveMessageHandler::handleMessage(QtMsgType type, const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation)
{
  QString title;
  switch (type)
  {
    case QtDebugMsg:
      title = tr("Information");
      break;
    case QtWarningMsg:
      title = tr("Warning");
      break;
    case QtCriticalMsg:
    case QtFatalMsg:
    default:
      title = tr("Error");
      break;
  }

  handleMessage(type, title, description, identifier, sourceLocation);
}

void InteractiveMessageHandler::handleMessage(QtMsgType type, const QString title, const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation)
{
  QMessageBox msgbox(qobject_cast<QWidget*>(parent()));

  switch (type)
  {
    case QtDebugMsg:
      msgbox.setIcon(QMessageBox::Information);
      break;
    case QtWarningMsg:
      msgbox.setIcon(QMessageBox::Warning);
      break;
    case QtCriticalMsg:
    case QtFatalMsg:
    default:
      msgbox.setIcon(QMessageBox::Critical);
      break;
  }

  msgbox.setWindowTitle(title);
  msgbox.setText(description);

  msgbox.setText(description);
  if (! identifier.isEmpty() && ! sourceLocation.isNull())
    msgbox.setDetailedText(tr("%1, line %2, column %3")
                            .arg(identifier.toString(QUrl::RemovePassword))
                            .arg(sourceLocation.line())
                            .arg(sourceLocation.column()));
  else if (! identifier.isEmpty() && sourceLocation.isNull())
    msgbox.setDetailedText(tr("%1")
                            .arg(identifier.toString(QUrl::RemovePassword)));
  else if (identifier.isEmpty() && ! sourceLocation.isNull())
    msgbox.setDetailedText(tr("line %1, column %2")
                            .arg(sourceLocation.line())
                            .arg(sourceLocation.column()));
  (void)unhandledMessages();
  msgbox.exec();
}
