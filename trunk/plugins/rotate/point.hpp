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

#ifndef IMAGINABLE__PLUGINS__ROTATE__POINT__INCLUDED
#define IMAGINABLE__PLUGINS__ROTATE__POINT__INCLUDED


class point
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

	point(void);
	point(double X,double Y);
	~point();

	point polar(void) const;
	point rect (void) const;

	#define OPERATOR(OP) \
	inline point& operator OP (double value) \
	{ \
		x OP value; \
		y OP value; \
		return *this; \
	}
	OPERATOR(*=)
	OPERATOR(/=)
	#undef OPERATOR

	#define OPERATOR(OP) \
	inline point& operator OP (const point& rs) \
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

template<typename T> static inline point make_point(T a,T b)
{
	return point(static_cast<double>(a),static_cast<double>(b));
}

#endif // IMAGINABLE__PLUGINS__ROTATE__POINT__INCLUDED
