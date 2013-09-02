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

/** QASpell
  Tries to be a wrapper around libaspell, following this:
  http://aspell.net/man-html/Through-the-C-API.html#Through-the-C-API
**/

#ifdef USE_ASPELL

#ifndef _QASPELL_H_
#define _QASPELL_H_

#include <QtCore>
#include <aspell.h>

class QASpell : public QObject {
public:
  QASpell(QObject* parent=0);

  ~QASpell();

  bool checkWord(const QString& word) const;
  static void setLocale(QString locale);

protected:
  AspellConfig* spell_config;
  AspellSpeller* spell_checker;

  bool ok;
  static QString s_locale;
};

#endif /* _QASPELL_H_ */
#endif
