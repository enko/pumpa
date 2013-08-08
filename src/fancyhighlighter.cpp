/*
  Copyright 2013 Mats Sjöberg
  
  This file is part of the Pumpa programme.

  Pumpa is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Pumpa is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU General Public License
  along with Pumpa.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "fancyhighlighter.h"

#include <QRegExp>

//------------------------------------------------------------------------------

FancyHighlighter::FancyHighlighter(QTextDocument* doc) : QSyntaxHighlighter(doc) 
{
#ifdef USE_ASPELL
  checker = new QASpell(this);
#endif
}

//------------------------------------------------------------------------------

void FancyHighlighter::highlightBlock(const QString& text) {
  int index;
  QTextCharFormat urlHighlightFormat;
  urlHighlightFormat.setForeground(QBrush(Qt::blue));

#ifdef USE_ASPELL
  QTextCharFormat spellErrorFormat;
  spellErrorFormat.setFontUnderline(true);
  spellErrorFormat.setUnderlineColor(Qt::red);
  spellErrorFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

  QRegExp rxa("(^|\\s)([\\w']+)");

  index = text.indexOf(rxa);
  while (index >= 0) {
    int length = rxa.matchedLength();
    int offset = rxa.cap(1).count();
    int s = index+offset;
    int l = length-offset;

    if (!checker->checkWord(rxa.cap(2)))
      setFormat(s, l, spellErrorFormat);
    index = text.indexOf(rxa, index + length);
  }
#endif // USE_ASPELL
}
