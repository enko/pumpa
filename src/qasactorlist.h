/*
  Copyright 2013 Mats Sjöberg
  
  This file is part of the Pumpa programme.

  Pumpa is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Pumpa is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Pumpa.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "qasobjectlist.h"
#include "qasactor.h"

//------------------------------------------------------------------------------

class QASActorList : public QASObjectList {
  Q_OBJECT

protected:
  QASActorList(QString url, QObject* parent);

public:
  static void clearCache();

  static QASActorList* getActorList(QVariantMap json, QObject* parent,
                                    int id=0);

  virtual QASActor* at(size_t i) const;

  void addActor(QASActor* actor) { addObject(actor); }
  void removeActor(QASActor* actor) { removeObject(actor); }

  bool onlyYou() const { return size()==1 && at(0)->isYou(); }

  // virtual void refresh();

  QString actorNames() const;

private:
  static QMap<QString, QASActorList*> s_actorLists;
};

