/*************
**
** Project:      Imaginable
** File info:    $Id: root_q.hpp 16 2010-04-13 10:59:29Z Kuzma.Shapran@gmail.com $
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


class Wait : public QObject
{
Q_OBJECT
public:
	Wait(QObject* parent=NULL);

public slots:
	void finished(qulonglong);

private slots:
	void init(void);

private:
	int m_rest;
};

#endif // IMAGINABLE__WAIT__WAIT__INCLUDED
