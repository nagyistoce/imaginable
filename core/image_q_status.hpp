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

#ifndef IMAGINABLE__CORE__IMAGE_Q_STATUS__INCLUDED
#define IMAGINABLE__CORE__IMAGE_Q_STATUS__INCLUDED


#include <QtCore/QObject>


class Image_Q_Status : public QObject
{
Q_OBJECT
public:
	Image_Q_Status(QObject * =NULL);

	bool init(QString);

signals:
	void longProcessingStarted(void);
	void percentChanged(double);
	void longProcessingFinished(void);

	void message(int,QString);

public slots:
	bool    busy   (void) const;
	double  percent(void) const;
};

#endif // IMAGINABLE__CORE__IMAGE_Q_STATUS__INCLUDED
