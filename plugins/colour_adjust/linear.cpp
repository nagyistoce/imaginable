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


#include "linear.hpp"


Linear::Linear(Image::Pixel p0,Image::Pixel p1,QAdjustPoint points)
	: CurveFunction()
{
	foreach(ushort x,points.keys())
		m_point[clamp<ushort>(x,0,0xffff)]=point_param(clamp<ushort>(points[x],0,0xffff));
	m_point[0]=point_param(clamp<ushort>(p0,0,0xffff));
	m_point.remove(0xffff);

	point_param omega;

	omega.syi=clamp<ushort>(p1,0,0xffff);

	for(point_params::Iterator I=m_point.begin();I!=m_point.end();++I)
	{
		point_params::Iterator N=I;
		++N;
		Image::Pixel next_x=(N==m_point.end())?0xffff:(N.key());
		point_param& next=(N==m_point.end())?omega:(N.value());
		point_param& cur=I.value();

		cur.dxi=next_x-I.key();
		cur.dyi=next.syi-cur.syi;

//		cur.sx=static_cast<double>(I.key())/65535.;
//		cur.sy=static_cast<double>(cur.syi)/65535.;
		cur.dx=static_cast<double>(cur.dxi)/65535.;
		cur.dy=static_cast<double>(cur.dyi)/65535.;
	}

	m_point[0xffff]=omega;
}

Linear::~Linear()
{}

Image::Pixel Linear::calc(Image::Pixel x)
{
	point_params::Iterator U=m_point.lowerBound(x);

	if(U.key()==x)//node;
		return U.value().syi;

	point_params::Iterator L=U;
	--L;

	point_param& cur=L.value();

	return static_cast<Image::Pixel>( ( ( static_cast<double>(x-L.key()) ) / cur.dx ) * cur.dy ) + L.value().syi;
}
