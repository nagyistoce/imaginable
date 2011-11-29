/*************
**
** Project:      Imaginable :: tonemap (GUI)
** File info:    $Id$
** Author:       Copyright (C) 2009 - 2011 Kuzma Shapran <Kuzma.Shapran@gmail.com>
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


#ifndef NOTIFICATION_SIGNALLER_HPP
#define NOTIFICATION_SIGNALLER_HPP


#include <QObject>
#include <imaginable/notifier.hpp>

class Notification_signaller : public QObject, public imaginable::Progress_notifier
{
	Q_OBJECT
public:
	explicit Notification_signaller(QObject *parent = NULL);
	virtual ~Notification_signaller();

	virtual void update(double value) const;

signals:
	void updated(double);

private:
	void emit_updated(double);

};

#endif // NOTIFICATION_SIGNALLER_HPP
