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

#include "aswidget.h"
#include "activitywidget.h"
#include <QScrollBar>
#include <QDebug>

//------------------------------------------------------------------------------

ASWidget::ASWidget(QWidget* parent, int widgetLimit, int purgeWait) :
  QScrollArea(parent),
  m_firstTime(true),
  m_list(NULL),
  m_asMode(QAS_NULL),
  m_purgeWait(purgeWait),
  m_purgeCounter(purgeWait),
  m_widgetLimit(widgetLimit)
{
  m_reuseWidgets = (m_widgetLimit > 0);

  m_itemLayout = new QVBoxLayout;
  m_itemLayout->setSpacing(10);

  m_listContainer = new QWidget;
  m_listContainer->setLayout(m_itemLayout);
  m_listContainer->setSizePolicy(QSizePolicy::Ignored,
                                 QSizePolicy::Ignored);

  setWidget(m_listContainer);
  setWidgetResizable(true);
}

//------------------------------------------------------------------------------

void ASWidget::clear() {
  QLayoutItem* item;
  while ((item = m_itemLayout->takeAt(0)) != 0) {
    if (dynamic_cast<QWidgetItem*>(item)) {
      QWidget* w = item->widget();
      delete w;
    }
    delete item;
  }

  m_firstTime = true;
  m_itemLayout->addStretch();
}

//------------------------------------------------------------------------------

void ASWidget::setEndpoint(QString endpoint, int asMode) {
  clear();
  m_list = initList(endpoint, parent()->parent()->parent());
  
  if (asMode != -1)
    m_asMode |= asMode;

  connect(m_list, SIGNAL(changed()),
          this, SLOT(update()), Qt::UniqueConnection);
  // connect(m_list, SIGNAL(request(QString, int)),
  //         this, SIGNAL(request(QString, int)), Qt::UniqueConnection);
}

//------------------------------------------------------------------------------

void ASWidget::fetchNewer() {
  emit request(m_list->prevLink(), m_asMode | QAS_NEWER);
}

//------------------------------------------------------------------------------

void ASWidget::fetchOlder() {
  m_purgeCounter = m_purgeWait;
  QString nextLink = m_list->nextLink();
  if (!nextLink.isEmpty())
    emit request(nextLink, m_asMode | QAS_OLDER);
}

//------------------------------------------------------------------------------

void ASWidget::refreshTimeLabels() {
  for (int i=0; i<m_itemLayout->count(); i++) {
    ObjectWidgetWithSignals* ow = widgetAt(i);
    if (ow)
      ow->refreshTimeLabels();
  }
  if (m_purgeCounter > 0) {
    m_purgeCounter--;
#ifdef DEBUG_WIDGETS
    qDebug() << "purgeCounter" << m_purgeCounter << m_list->url();
#endif
  }
}

//------------------------------------------------------------------------------

void ASWidget::keyPressEvent(QKeyEvent* event) {
  int key = event->key();

  if (key == Qt::Key_Home || key == Qt::Key_End) {
    bool home = key==Qt::Key_Home;
    QScrollBar* sb = verticalScrollBar();
    sb->setValue(home ? sb->minimum() : sb->maximum());
  } else {
    QScrollArea::keyPressEvent(event);
  }
}

//------------------------------------------------------------------------------

ObjectWidgetWithSignals* ASWidget::widgetAt(int idx) {
  QLayoutItem* item = m_itemLayout->itemAt(idx);

  if (dynamic_cast<QWidgetItem*>(item))
    return qobject_cast<ObjectWidgetWithSignals*>(item->widget());

  return NULL;
}

//------------------------------------------------------------------------------

QASAbstractObject* ASWidget::objectAt(int idx) {
  ObjectWidgetWithSignals* ows = widgetAt(idx);
  if (!ows)
    return NULL;
  
  ActivityWidget* aw = qobject_cast<ActivityWidget*>(ows);
  if (aw)
    return aw->activity();

  ObjectWidget* ow = qobject_cast<ObjectWidget*>(ows);
  if (ow)
    return ow->object();

  return NULL;
}

//------------------------------------------------------------------------------

void ASWidget::update() {
  /* 
     We assume m_list contains all objects, but new ones might have
     been added either (or both) to the top or end. Go through from
     top (newest) to bottom. If the object doesn't exist add it, if it
     does increment the counter (go further down both in the
     collection and widget list).
  */

  int li = 0; 
  int newCount = 0;
  bool older = false;

  for (size_t i=0; i<m_list->size(); i++) {
    QASAbstractObject* cObj = m_list->at(i);

    if (cObj->isDeleted())
      continue;

    QASAbstractObject* wObj = objectAt(li);
    if (wObj == cObj) {
      li++;
      older = true;
      continue;
    }

    if (m_object_set.contains(cObj)) {
      // qDebug() << "[WARNING]" << cObj->apiLink() << "in wrong order in list"
      //          << m_list->url();
      continue;
    }
    m_object_set.insert(cObj);

    bool countAsNew = false;

    bool doReuse = !older && m_reuseWidgets && (count() > m_widgetLimit) &&
      m_purgeCounter == 0;

    if (doReuse) {
      ObjectWidgetWithSignals* ow = NULL;
      int idx = m_itemLayout->count();
      while (!ow && --idx > 0)
        ow = widgetAt(idx);

#ifdef DEBUG_WIDGETS
      qDebug() << "Reused widget" << idx << li << cObj->apiLink()
               << m_list->url();
#endif

      QASAbstractObject* obj = ow->asObject();
      m_itemLayout->removeWidget(ow);

      m_object_set.remove(obj);
      m_list->removeObject(obj);

      ow->changeObject(cObj);
      m_itemLayout->insertWidget(li++, ow);
    } else {
      ObjectWidgetWithSignals* ow = createWidget(cObj, countAsNew);
      ObjectWidgetWithSignals::connectSignals(ow, this);
      m_itemLayout->insertWidget(li++, ow);
      
#ifdef DEBUG_WIDGETS
      qDebug() << "Created widget" << cObj->apiLink() << m_list->url();
#endif
    }

    if (countAsNew && !older)
      newCount++;
  }

  if (newCount && !isVisible() && !m_firstTime)
    emit highlightMe();
  m_firstTime = false;
}

//------------------------------------------------------------------------------

ObjectWidgetWithSignals* ASWidget::createWidget(QASAbstractObject*, bool&) {
  return NULL;
}

//------------------------------------------------------------------------------

QASAbstractObjectList* ASWidget::initList(QString, QObject*) {
  return NULL;
}

//------------------------------------------------------------------------------

void ASWidget::refreshObject(QASAbstractObject* obj) {
  if (!obj)
    return;
  
  QDateTime now = QDateTime::currentDateTime();
  QDateTime lr = obj->lastRefreshed();

  if (lr.isNull() || lr.secsTo(now) > 10) {
    emit request(obj->apiLink(), obj->asType());
    obj->lastRefreshed(now);
  }
}
