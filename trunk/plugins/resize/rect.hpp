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

#ifndef IMAGINABLE__PLUGINS__RESIZE__RECT__INCLUDED
#define IMAGINABLE__PLUGINS__RESIZE__RECT__INCLUDED


class rect
{
public:
	rect(void)
		: m_x(0)
		, m_y(0)
		, m_w(0)
		, m_h(0)
	{}

	rect(int x,int y,int w,int h)
		: m_x(x)
		, m_y(y)
		, m_w(w)
		, m_h(h)
	{ normalise(); }

	~rect()
	{}


	int x     (void) const { return m_x; }
	int y     (void) const { return m_y; }
	int width (void) const { return m_w; }
	int height(void) const { return m_h; }
	int left  (void) const { return m_x; }
	int top   (void) const { return m_y; }
	int right (void) const { return m_x+m_w; }
	int bottom(void) const { return m_y+m_h; }

	void setX     (int value) { m_x=value; }
	void setY     (int value) { m_y=value; }
	void setWidth (int value) { m_w=value; normalise(); }
	void setHeight(int value) { m_h=value; normalise(); }
	void setLeft  (int value) { m_x=value; }
	void setTop   (int value) { m_y=value; }
	void setRight (int value) { m_w=value-m_x;  normalise(); }
	void setBottom(int value) { m_h=value-m_y;  normalise(); }
	void setSize  (int w,int h) { m_w=w; m_h=h; normalise(); }

	bool isEmpty(void) const { return (!m_w) || (!m_h) ; }
	unsigned area(void) const {return m_w*m_h; }

	void shiftH(int value) { m_x+=value; }
	void shiftV(int value) { m_y+=value; }

	void moveLeft  (int value) { m_x+=value; m_w-=value; normalise(); }
	void moveTop   (int value) { m_y+=value; m_h-=value; normalise(); }
	void moveRight (int value) {             m_w+=value; normalise(); }
	void moveBottom(int value) {             m_h+=value; normalise(); }

	void normalise(void)
	{
		if (m_w<0)
		{
			m_x+=m_w;
			m_w=-m_w;
		}
		if (m_h<0)
		{
			m_y+=m_h;
			m_h=-m_h;
		}
	}

private:
	int m_x;
	int m_y;
	int m_w;
	int m_h;
};

#endif // IMAGINABLE__PLUGINS__RESIZE__RECT__INCLUDED
