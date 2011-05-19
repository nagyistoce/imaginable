/*************
**
** Project:      Imaginable
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


#ifndef IMAGINABLE__POINT__INCLUDED
#define IMAGINABLE__POINT__INCLUDED



namespace imaginable {

class Point
{
public:
	union {
		double x;
		double r;
	};
	union {
		double y;
		double f;
	};

	Point(void);
	Point(double x_,double y_);
	template<typename T> Point(T x_,T y_)
		: x(static_cast<double>(x_))
		, y(static_cast<double>(y_))
	{
	}
	~Point();

	Point polar(void) const;
	Point rect (void) const;

	#define OPERATOR(OP) \
	inline Point& operator OP (double value) \
	{ \
		x OP value; \
		y OP value; \
		return *this; \
	}
	OPERATOR(*=)
	OPERATOR(/=)
	#undef OPERATOR

	#define OPERATOR(OP) \
	inline Point& operator OP (const Point& rs) \
	{ \
		x OP rs.x; \
		y OP rs.y; \
		return *this; \
	}
	OPERATOR(+=)
	OPERATOR(-=)
	OPERATOR(*=)
	OPERATOR(/=)
	#undef OPERATOR
};

}

#endif // IMAGINABLE__POINT__INCLUDED
