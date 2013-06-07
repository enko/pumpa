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

#include "tabwidget.h"

//------------------------------------------------------------------------------

TabWidget::TabWidget(QWidget* parent) : QTabWidget(parent) {
  sMap = new QSignalMapper(this);
  connect(sMap, SIGNAL(mapped(int)), this, SLOT(highlightTab(int)));
}

//------------------------------------------------------------------------------

int TabWidget::addTab(QWidget* page, const QString& label, 
                           bool highlight) {
  int index = QTabWidget::addTab(page, label);
  if (highlight)
    addHighlightConnection(page, index);
  return index;
}

//------------------------------------------------------------------------------

int TabWidget::addTab(QWidget* page, const QIcon& icon, 
                           const QString& label, bool highlight) {
  int index = QTabWidget::addTab(page, icon, label);
  if (highlight)
    addHighlightConnection(page, index);
  return index;
}

//------------------------------------------------------------------------------

void TabWidget::highlightTab(int index) {
  if (index == -1)
    index = currentIndex();
  tabBar()->setTabTextColor(index, Qt::red);
}

//------------------------------------------------------------------------------

void TabWidget::deHighlightTab(int index) {
  if (index == -1)
    index = currentIndex();
  QPalette pal;
  tabBar()->setTabTextColor(index, pal.color(foregroundRole()));
}

//------------------------------------------------------------------------------

void TabWidget::addHighlightConnection(QWidget* page, int index) {
  sMap->setMapping(page, index);
  connect(page, SIGNAL(highlightMe()), sMap, SLOT(map()));
}

