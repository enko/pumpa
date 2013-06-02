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

#include "objectwidget.h"

//------------------------------------------------------------------------------

ObjectWidget::ObjectWidget(QASObject* obj, QWidget* parent) :
  QLabel(parent), m_object(obj)
{
  // useful for debugging layouts and margins
  // setLineWidth(1);
  // setFrameStyle(QFrame::Box);

  setWordWrap(true);

  setOpenExternalLinks(true);
  setTextInteractionFlags(Qt::TextSelectableByMouse |
                          Qt::LinksAccessibleByMouse);
  setScaledContents(false);

  setLineWidth(2);
  setMargin(0);
  setFocusPolicy(Qt::NoFocus);
}

//------------------------------------------------------------------------------

void ObjectWidget::mousePressEvent(QMouseEvent* e) {
  QLabel::mousePressEvent(e);
  e->ignore();
}
