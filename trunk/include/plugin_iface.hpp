/*************
**
** Project:      Imaginable
** File info:    $Id$
** Author:       Copyright (C) 2009,2010 Kuzma Shapran <Kuzma.Shapran@gmail.com>
** License:      GPLv3
**
**  This file is part of Imaginable.
**
**  Imaginable is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Imaginable is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Imaginable.  If not, see <http://www.gnu.org/licenses/>.
**
*************/

#ifndef IMAGINABLE__PLUGIN_INTERFACE__INCLUDED
#define IMAGINABLE__PLUGIN_INTERFACE__INCLUDED


#include <image.hpp>
#include <root.hpp>

#include <QtCore/QtConcurrentRun>
#include <QtCore/QFutureWatcher>
#include <QtCore/QThread>


class PluginInterface
{
public:
	PluginInterface(void) {}
	virtual ~PluginInterface() {}

	virtual bool init(Root*)=0;
	virtual QString name(void) const =0;
	virtual QString version(void) const =0;


protected:
	Root* m_root;

	void complain(int level,qulonglong Id,const char* function,QString message)
	{ m_root->pluginMessage(level,name(),Id,QString("%1() failed: %2").arg(function).arg(message)); }
#define COMPLAIN(LEVEL,ID,MESSAGE) complain(LEVEL,ID,__FUNCTION__,MESSAGE)

	void complainNoImage(const char* function,qulonglong Id)
	{ complain(LOG_CRIT,Id,function,"does not exist"); }
#define COMPLAIN_NO_IMAGE(ID) complainNoImage(__FUNCTION__,ID)

	void complainBusy(const char* function,qulonglong Id)
	{ complain(LOG_ERR,Id,function,"is busy"); }
#define COMPLAIN_BUSY(ID) complainBusy(__FUNCTION__,ID)

	Image* getOrComplain(const char* function,qulonglong Id)
	{
		Image* image=m_root->image(Id);
		if(!image)
		{
			complainNoImage(function,Id);
			return NULL;
		}
		if(image->busy())
		{
			complainBusy(function,Id);
			return NULL;
		}
		return image;
	}
#define GET_OR_COMPLAIN(ID) getOrComplain(__FUNCTION__,ID)



	typedef QFutureWatcher<void> futureWatcher_t;

	futureWatcher_t* doLongProcessing(QList<Image*> images,QFuture<void> future)
	{
		futureWatcher_t* futureWatcher=new futureWatcher_t;
		foreach(Image* image,images)
		{
			QObject::connect(futureWatcher,SIGNAL(started()), image,SLOT(startLongProcessing()));
			QObject::connect(futureWatcher,SIGNAL(finished()),image,SLOT(finishLongProcessing()));
		}
		QObject::connect(futureWatcher,SIGNAL(finished()),futureWatcher,SLOT(deleteLater()));
		futureWatcher->setFuture(future);
		return futureWatcher;
	}

	futureWatcher_t* doLongProcessing(Image* image,QFuture<void> future)
	{
		return doLongProcessing(QList<Image*>()<<image,future);
	}
};

Q_DECLARE_INTERFACE(PluginInterface,"name.kuzmashapran.imaginable.PluginInterface/1.0")

#endif // IMAGINABLE__PLUGIN_INTERFACE__INCLUDED
