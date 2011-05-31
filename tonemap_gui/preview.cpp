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


#include <QtGui/QResizeEvent>

#include "preview.hpp"


Preview::Preview(QWidget *parent)
	: QLabel(parent)
	, m_is_shifting(false)
{
	setMouseTracking(true);
}

void Preview::mousePressEvent(QMouseEvent *event)
{
	if (event->buttons() == (Qt::MouseButtons() |= Qt::LeftButton) )
	{
		m_is_shifting = true;
		m_old_x = event->x();
		m_old_y = event->y();
	}
}

void Preview::mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons() == (Qt::MouseButtons() |= Qt::LeftButton) )
	{
		emit shifted(event->x()-m_old_x, event->y()-m_old_y);
		m_old_x = event->x();
		m_old_y = event->y();
	}
}

void Preview::mouseReleaseEvent(QMouseEvent *event)
{
	if (m_is_shifting)
	{
		emit shifted(event->x()-m_old_x, event->y()-m_old_y);
		m_is_shifting = false;
	}
}

void Preview::resizeEvent(QResizeEvent* event)
{
	emit resized(event->size().width(),event->size().height());
}
