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


#include "image_q_status.hpp"
#include "dbus_image_q_status_adaptor.h"

#include <sys/syslog.h> // for log levels

#include <QtCore/QVariant>


Image_Q_Status::Image_Q_Status(QObject* parent)
	: QObject(parent)
{
}

bool Image_Q_Status::init(QString nodeName)
{
	new Image_statusAdaptor(this);
	if(!QDBusConnection::sessionBus().registerObject(nodeName,this))
	{
		message(LOG_ALERT,"Cannot register D-Bus object interface `status`");
		return false;
	}
	return true;
}

bool Image_Q_Status::busy(void) const
{
	return qvariant_cast<bool>(parent()->property("busy"));
}

double Image_Q_Status::percent(void) const
{
	return qvariant_cast<double>(parent()->property("percent"));
}
