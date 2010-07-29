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

#ifndef IMAGINABLE__WAIT__WAIT__INCLUDED
#define IMAGINABLE__WAIT__WAIT__INCLUDED


#include <QtCore/QObject>
#include <QtCore/QMap>

#include "dbus_image_q_status_interface.h"


class QTimer;

class Wait : public QObject
{
Q_OBJECT
public:
	Wait(QObject * =NULL);

public slots:
	void finished(void);
	void timeout(void);

private slots:
	void init(void);

private:
	int m_rest;

	QTimer *m_timer;

	QMap<qulonglong,name::kuzmashapran::imaginable::image_status*> m_images;
};

#endif // IMAGINABLE__WAIT__WAIT__INCLUDED
