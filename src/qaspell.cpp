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

#include "qaspell.h"

#ifdef USE_ASPELL

//------------------------------------------------------------------------------

QString QASpell::s_locale = "en_US";

//------------------------------------------------------------------------------

QASpell::QASpell(QObject* parent) : QObject(parent) {
  spell_config = new_aspell_config();
  aspell_config_replace(spell_config, "lang",
                        s_locale.toLocal8Bit().constData());
  aspell_config_replace(spell_config, "encoding", "ucs-2");

  AspellCanHaveError* possible_err = new_aspell_speller(spell_config);
  spell_checker = NULL;
  if (aspell_error_number(possible_err) != 0) {
    qDebug() << aspell_error_message(possible_err);
    return;
  }
  spell_checker = to_aspell_speller(possible_err);
}

//------------------------------------------------------------------------------

QASpell::~QASpell() {
  if (spell_checker != NULL)
    delete_aspell_speller(spell_checker);
  delete_aspell_config(spell_config);
}

//------------------------------------------------------------------------------

void QASpell::setLocale(QString locale) {
  if (!locale.isEmpty())
    s_locale = locale;
}

//------------------------------------------------------------------------------

bool QASpell::checkWord(const QString& word) const {
  if (spell_checker == NULL) {
    qDebug() << "aspell was not initialised properly!";
    return true;
  }
  int correct = 
    aspell_speller_check(spell_checker, 
                         reinterpret_cast<const char *>(word.utf16()), -1);
  return correct;
}

#endif // USE_ASPELL
