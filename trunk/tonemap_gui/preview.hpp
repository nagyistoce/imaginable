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


#ifndef IMAGINABLE__TONEMAP_GUI__PREVIEW__INCLUDED
#define IMAGINABLE__TONEMAP_GUI__PREVIEW__INCLUDED


#include <QtGui/QWidget>
#include <QtGui/QImage>
#include <QtCore/QPoint>


class Preview : public QWidget
{
	Q_OBJECT
public:
	explicit Preview(QWidget *parent = NULL);

	void setImage(const QImage&);

protected:
	void mousePressEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void resizeEvent(QResizeEvent*);
	void dragEnterEvent(QDragEnterEvent*);
	void dropEvent(QDropEvent*);
	void paintEvent(QPaintEvent*);

signals:
	void resized(int,int);
	void shifted(int,int);
	void imageDropped(QImage);
	void urlDropped(QString);

private:
	bool m_is_shifting;
	int m_old_x;
	int m_old_y;

	QImage m_image;
};

#endif // IMAGINABLE__TONEMAP_GUI__PREVIEW__INCLUDED
