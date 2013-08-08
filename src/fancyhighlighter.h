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

#ifndef FANCY_HIGHLIGHTER_H
#define FANCY_HIGHLIGHTER_H

#include <QtGui>
#include "qaspell.h"
#include "util.h"

class FancyHighlighter : public QSyntaxHighlighter {
public:
  FancyHighlighter(QTextDocument* doc);

protected:
  void highlightBlock(const QString& text);

#ifdef USE_ASPELL
  QASpell* checker;
#endif

};

#endif /* FANCY_HIGHLIGHTER_H */
