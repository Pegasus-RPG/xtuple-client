/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef METASQLHIGHLIGHTER_H
#define METASQLHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class QColor;
class QTextDocument;

class MetaSQLHighlighter : public QSyntaxHighlighter
{
  Q_OBJECT

  public:
    MetaSQLHighlighter(QObject *parent);
    MetaSQLHighlighter(QTextDocument *document);
    MetaSQLHighlighter(QTextEdit *editor);
    ~MetaSQLHighlighter();

  protected:
    enum State { NormalState = -1, InsideString };
    virtual void highlightBlock(const QString &text);

    QColor       _commentColor;
    QColor       _errorColor;
    QColor       _extensionColor;
    QColor       _keywordColor;
    QColor       _literalColor;

  private:
    virtual void init();

    QRegExp _kwtest;
    QRegExp _extest;
    QRegExp _numerictest;
    QRegExp _wordtest;
    QRegExp _quotetest;

};

#endif // METASQLHIGHLIGHTER_H
