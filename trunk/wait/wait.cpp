/*************
**
** Project:      Imaginable
** File info:    $Id: root_q.cpp 16 2010-04-13 10:59:29Z Kuzma.Shapran@gmail.com $
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


#include "wait.hpp"
#include "main.hpp"
#include "wait1.hpp"

#include "dbus_image_q_busy_interface.h"
#include "dbus_root_q_interface.h"

#include <QtCore/QTextStream>


Wait::Wait(QObject* parent)
	: QObject(parent)
	, m_rest(0)
{
	QTimer::singleShot(0,this,SLOT(init()));
}

void Wait::init(void)
{
	name::kuzmashapran::imaginable::root* root=new name::kuzmashapran::imaginable::root("name.kuzmashapran.imaginable","/",QDBusConnection::sessionBus(),this);

	foreach(QString Id,program_options().unnamed())
	{
		name::kuzmashapran::imaginable::image_busy* image;
		bool ok;
		qulonglong intId=Id.toULongLong(&ok);
		if( ok
		&&  root->hasImage(intId)
		&&  (image=new name::kuzmashapran::imaginable::image_busy("name.kuzmashapran.imaginable",QString("/%1").arg(Id),QDBusConnection::sessionBus(),this))
		&&  image->isValid() )
		{
			if(image->busy())
			{
				Wait1* wait1=new Wait1(intId,this);
				connect(image,SIGNAL(longProcessingFinished()),wait1,SLOT(finished()));
				connect(wait1,SIGNAL(finishedId(qulonglong)),this,SLOT(finished(qulonglong)));

				++m_rest;
				QTextStream(stdout)<<QString("Waiting for image [%1].\n").arg(intId);
			}
			else
				QTextStream(stdout)<<QString("Image [%1] is not busy.\n").arg(intId);
		}
		else
			QTextStream(stdout)<<QString("Image [%1] cannot be connected.\n").arg(Id);

	}
	if(!m_rest)
	{
		QTextStream(stdout)<<QString("Nothing to wait for.\n");
		QCoreApplication::quit();
	}
}

void Wait::finished(qulonglong Id)
{
	--m_rest;
	QTextStream(stdout)<<QString("Image [%1] is idle.\n").arg(Id);

	if(!m_rest)
	{
		QTextStream(stdout)<<QString("Nothing to wait for anymore.\n");
		QCoreApplication::quit();
	}
}
