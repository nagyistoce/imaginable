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

#ifndef IMAGINABLE__CORE__IMAGE_Q__INCLUDED
#define IMAGINABLE__CORE__IMAGE_Q__INCLUDED


#include "image.hpp"

#include <QtCore/QObject>
#include <QtCore/QStringList>


class Root_Q;
class Image_mainAdaptor;

class Image_Q : public Image
{
Q_OBJECT
public:
	Image_Q(QObject* = NULL);
	~Image_Q();

	bool init(QString);

	Q_PROPERTY(ushort maximum READ maximum WRITE setMaximum)

signals:
	void longProcessingStarted(void);
	void percentChanged(double);
	void longProcessingFinished(void);

public slots:
	ushort maximum (void) const      { return Image::maximum(); }
	void setMaximum(ushort maximum)  { return Image::setMaximum(maximum); }

	static QString colourSpaceToString(int);

	static QString colourPlaneToString(int);

private:
	friend class Root_Q;

	QString m_DBusIFaceNodeName;
	QString m_DBusIFaceImageNodeName;
	Image_mainAdaptor* m_DBusIFaceMain;

	bool set_DBus_main(void);
	void hide_DBus_main(void);


	virtual void onSetBusy(void);
	virtual void onSetPercent(void);
};

#endif // IMAGINABLE__CORE__IMAGE_Q__INCLUDED
