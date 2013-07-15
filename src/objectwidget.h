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

#ifndef _OBJECTWIDGET_H_
#define _OBJECTWIDGET_H_

#include <QFrame>
#include <QWidget>

#include "qactivitystreams.h"
#include "shortobjectwidget.h"
#include "fullobjectwidget.h"
#include "texttoolbutton.h"
#include "richtextlabel.h"
#include "objectwidgetwithsignals.h"

//------------------------------------------------------------------------------

class ObjectWidget : public ObjectWidgetWithSignals {
  Q_OBJECT

public:
  ObjectWidget(QASObject* obj, QWidget* parent = 0,
                      bool shortWidget=false);

  QASObject* object() const { return m_object; }

  virtual void refreshTimeLabels();

signals:
  void moreClicked();
  void showContext(QASObject*);
                          
private slots:
  void showMore();
  void onChanged();
  void updateContextLabel();
  void onShowContext();

private:
  FullObjectWidget* m_objectWidget;
  ShortObjectWidget* m_shortObjectWidget;
  RichTextLabel* m_contextLabel;
  TextToolButton* m_contextButton;

  QVBoxLayout* m_layout;
  QHBoxLayout* m_topLayout;
  QASObject* m_object;
  QASObject* m_irtObject;

  bool m_short;
};
  

#endif /* _OBJECTWIDGET_H_ */
