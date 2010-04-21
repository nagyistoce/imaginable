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


#include "wait.hpp"
#include "main.hpp"

#include "dbus_core_q_interface.h"

#include <QtCore/QTextStream>
#include <QtCore/QTimer>


Wait::Wait(QObject* parent)
	: QObject(parent)
	, m_rest(0)
	, m_timer(new QTimer(this))
{
	QTimer::singleShot(0,this,SLOT(init()));

	m_timer->setSingleShot(false);
	m_timer->setInterval(333);
	connect(m_timer,SIGNAL(timeout()),this,SLOT(timeout()));
}

void Wait::init(void)
{
	name::kuzmashapran::imaginable::core* core=new name::kuzmashapran::imaginable::core("name.kuzmashapran.imaginable","/",QDBusConnection::sessionBus(),this);

	foreach(QString Id,program_options().unnamed())
	{
		name::kuzmashapran::imaginable::image_busy* image=NULL;
		bool ok;
		qulonglong intId=Id.toULongLong(&ok);
		if( ok
		&&  core->hasImage(intId)
		&&  (image=new name::kuzmashapran::imaginable::image_busy("name.kuzmashapran.imaginable",QString("/%1").arg(Id),QDBusConnection::sessionBus(),this))
		&&  image->isValid() )
		{
			if(image->busy())
			{
				connect(image,SIGNAL(longProcessingFinished()),this,SLOT(finished()));

				m_images[intId]=image;
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
	else
	{
		foreach(qulonglong Id,m_images.keys())
			QTextStream(stdout)<<QString(" %1").arg(Id,6);
		QTextStream(stdout)<<"\n";
		QTextStream(stdout).flush();
		m_timer->start();
	}
}

void Wait::finished(void)
{
	--m_rest;
	timeout();

	if(!m_rest)
	{
		QTextStream(stdout)<<QString("All images are idle.\n");
		QCoreApplication::quit();
	}
}

void Wait::timeout(void)
{
	foreach(qulonglong Id,m_images.keys())
	{
		if(m_images[Id]->busy())
		{
			double v=m_images[Id]->percent();
			QTextStream(stdout)<<QString(" %1\%").arg(v,5,'f',(v>=100.)?1:((v>=10.)?2:3));
		}
		else
			QTextStream(stdout)<<" idle";
	}
	QTextStream(stdout)<<"\r";
	QTextStream(stdout).flush();
}
