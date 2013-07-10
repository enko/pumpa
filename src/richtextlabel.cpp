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

#include "richtextlabel.h"

//------------------------------------------------------------------------------

RichTextLabel::RichTextLabel(QWidget* parent, bool singleLine) :
  QLabel(parent),
  m_singleLine(singleLine)
{
  // useful for debugging layouts and margins
  // setLineWidth(1);
  // setFrameStyle(QFrame::Box);

  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::MinimumExpanding);
  if (!m_singleLine)
    setWordWrap(true);

  setOpenExternalLinks(true);
  setTextInteractionFlags(Qt::TextSelectableByMouse |
                          Qt::LinksAccessibleByMouse);
  setScaledContents(false);
  setTextFormat(Qt::RichText);

  setLineWidth(2);
  setMargin(0);
  setFocusPolicy(Qt::NoFocus);
}

//------------------------------------------------------------------------------

void RichTextLabel::resizeEvent(QResizeEvent*) {
  if (!m_singleLine && minimumSizeHint().width() > size().width()) {
    // qDebug() << "[DEBUG]: chop off" << minimumSizeHint().width() << size().width();
    setStyleSheet( "border-width: 2px; border-top-style: none; border-right-style: solid; border-bottom-style: none; border-left-style: none; border-color: red; ");
  } else {
    setStyleSheet("");
  }
}

