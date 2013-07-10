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

#ifndef _TEXTTOOLBUTTON_H_
#define _TEXTTOOLBUTTON_H_

#include <QToolButton>

class TextToolButton : public QToolButton {
Q_OBJECT
public:
  TextToolButton(QWidget* parent=0);
  TextToolButton(QString text, QWidget* parent=0);

private:
  void setup();
};

#endif /* _TEXTTOOLBUTTON_H_ */
