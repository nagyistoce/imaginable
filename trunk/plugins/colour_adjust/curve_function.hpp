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

#ifndef IMAGINABLE__PLUGINS__COLOUR_ADJUST__CURVE_FUNCTION__INCLUDED
#define IMAGINABLE__PLUGINS__COLOUR_ADJUST__CURVE_FUNCTION__INCLUDED


#include <image.hpp>


template<typename T>
T clamp(T value,T lower,T upper)
{ return (value<lower) ? lower : ( (value>upper) ? upper : value ) ; }


class CurveFunction
{
public:
	CurveFunction(void);
	virtual ~CurveFunction();

	Image::Pixel get(Image::Pixel);
	inline Image::Pixel operator() (Image::Pixel x) { return get(x); }

private:
	bool m_cached[0x10000];
	Image::Pixel m_value[0x10000];

protected:
	virtual Image::Pixel calc(Image::Pixel) =0;
};

#endif // IMAGINABLE__PLUGINS__COLOUR_ADJUST__CURVE_FUNCTION__INCLUDED
