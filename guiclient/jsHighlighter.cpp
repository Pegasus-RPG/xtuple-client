/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "jsHighlighter.h"

#include <QColor>

#include "format.h"

JSHighlighter::JSHighlighter(QTextDocument *document)
  : QSyntaxHighlighter(document)
{
  _commentColor   = namedColor("altemphasis");
  _errorColor     = namedColor("error");
  _extensionColor = namedColor("warning");
  _keywordColor   = namedColor("emphasis");
  _literalColor   = namedColor("future");

  _keyword  << "break"     << "case"    << "catch"          << "continue"
            << "default"   << "delete"  << "do"             << "else"
            << "false"     << "finally" << "for"            << "function"
            << "if"        << "in"      << "instanceof"     << "new"
            << "null"      << "return"  << "switch"         << "this"
            << "throw"     << "true"    << "try"            << "typeof"
            << "var"       << "void"    << "while"          << "with"
            ;
  _extension  << "abstract" << "boolean"   << "byte"         << "char"
              << "class"    << "const"     << "debugger"     << "double"
              << "enum"     << "export"    << "extends"      << "final"
              << "float"    << "goto"      << "implements"   << "import"
              << "int"      << "interface" << "long"         << "native"
              << "package"  << "private"   << "protected"    << "public"
              << "short"    << "static"    << "super"        << "synchronized"
              << "throws"   << "transient" << "volatile"
              ;
}

JSHighlighter::~JSHighlighter()
{
}

void JSHighlighter::highlightBlock(const QString &text)
{
  int state = previousBlockState();
  int start = 0;

  for (int i = 0; i < text.length(); i++)
  {
    QRegExp kwtest("^(" + _keyword.join("|") + ")((?=\\W)|$)");
    QRegExp extest("^(" + _extension.join("|") + ")((?=\\W)|$)");
    QRegExp numerictest("^(0[xX][0-9a-fA-F]+)|(-?[0-9]+(\\.[0-9]+)?)");
    QRegExp wordtest("^\\w+");
    QRegExp quotetest("^\"[^\"]*\"|'[^']*'");
    QRegExp regexptest("/[^/]*/[igm]?");

    if (state == InsideCStyleComment)
    {
      if (text.mid(i, 2) == "*/")
      {
        state = NormalState;
        setFormat(start, i - start + 2, _commentColor);
      }
    }
    else if (state == InsideString)
    {
      // TODO: if i == 0 then error color until next "
      if (text.mid(i, 1) == "\"")
      {
        state = NormalState;
        setFormat(start, i - start + 1, _literalColor);
        start = i;
      }
    }
    else if (text.mid(i, 2) == "//")
    {
      setFormat(i, text.length() - i, _commentColor);
      break;
    }
    else if (text.mid(i, 2) == "/*")
    {
      start = i;
      state = InsideCStyleComment;
    }
    else if (quotetest.indexIn(text.mid(i)) == 0)
    {
      setFormat(i, quotetest.matchedLength(), _literalColor);
      i += quotetest.matchedLength();
    }
    else if (text.mid(i, 1) == "\"")
    {
      start = i;
      state = InsideString;
    }
    else if (kwtest.indexIn(text.mid(i)) == 0)
    {
      setFormat(i, kwtest.matchedLength(), _keywordColor);
      i += kwtest.matchedLength();
    }
    else if (extest.indexIn(text.mid(i)) == 0)
    {
      setFormat(i, extest.matchedLength(), _extensionColor);
      i += extest.matchedLength();
    }
    else if (numerictest.indexIn(text.mid(i)) == 0)
    {
      setFormat(i, numerictest.matchedLength(), _literalColor);
      i += numerictest.matchedLength();
    }
    else if (regexptest.indexIn(text.mid(i)) == 0)
    {
      setFormat(i, regexptest.matchedLength(), _literalColor);
      i += regexptest.matchedLength();
    }
    else if (wordtest.indexIn(text.mid(i)) == 0)        // skip non-keywords
      i += wordtest.matchedLength();
  }

  if (state == InsideCStyleComment)
    setFormat(start, text.length() - start, _commentColor);
  else if (state == InsideString)
    setFormat(start, text.length() - start, _errorColor);

  setCurrentBlockState(state);
}
