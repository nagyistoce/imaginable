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


#include "cubic_hermite_spline.hpp"


bool CubicHermiteSpline::h_cached[0x10000];

double CubicHermiteSpline::h[2][2][0x10000];

double CubicHermiteSpline::k[2][2][4]={{
	{ 1., 0.,-3., 2.} ,
	{ 0., 0., 3.,-2.} },{
	{ 0., 1.,-2., 1.} ,
	{ 0., 0.,-1., 1.} }};


CubicHermiteSpline::CubicHermiteSpline(Image::Pixel p0,Image::Pixel p1,QAdjustPoint points)
	: CurveFunction()
{
	memset(h_cached,0,sizeof(bool)*0x10000);

	foreach(ushort x,points.keys())
		m_point[clamp<ushort>(x,0,0xffff)]=point_param(clamp<ushort>(points[x],0,0xffff));
	m_point[0]=point_param(clamp<ushort>(p0,0,0xffff));
	m_point.remove(0xffff);

	point_param alpha,omega;

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

		cur.sx=static_cast<double>(I.key())/65535.;
		cur.sy=static_cast<double>(cur.syi)/65535.;
		cur.dx=static_cast<double>(cur.dxi)/65535.;
		cur.dy=static_cast<double>(cur.dyi)/65535.;
	}

	{
		point_params::Iterator I=m_point.begin();
		point_params::Iterator N=I;
		++N;

		alpha.dxi=2*I.value().dxi-N.value().dxi;
		if(alpha.dxi<=0)
			alpha.dxi=I.value().dxi;
		alpha.dyi=2*I.value().dyi-N.value().dyi;

		alpha.dx=static_cast<double>(alpha.dxi)/65535.;
		alpha.dy=static_cast<double>(alpha.dyi)/65535.;
	}
	{
		point_params::Iterator I=m_point.end();
		--I;
		point_params::Iterator P=I;
		--P;

		omega.dxi=2*I.value().dxi-P.value().dxi;
		if(omega.dxi<=0)
			omega.dxi=I.value().dxi;
		omega.dyi=2*I.value().dyi-P.value().dyi;

		omega.sy=static_cast<double>(omega.syi)/65535.;
		omega.dx=static_cast<double>(omega.dxi)/65535.;
		omega.dy=static_cast<double>(omega.dyi)/65535.;
	}
	{
		point_params::Iterator I=m_point.begin();
		alpha.et=(I.value().dy/I.value().dx+alpha.dy/alpha.dx)/2.;
	}

	for(point_params::Iterator I=m_point.begin();I!=m_point.end();++I)
	{
		point_params::Iterator P=I;
		--P;
		point_params::Iterator N=I;
		++N;
		point_param& prev=(I==m_point.begin())?alpha:(P.value());
		point_param& next=(N==m_point.end()  )?omega:(N.value());
		point_param& cur=I.value();

		double finite=(next.dy/next.dx+cur.dy/cur.dx)/2.; ;
		double c_m=cur.dx*(next.dy-prev.dy)/2.;

		cur.et=(finite+c_m)/2.;

		cur.st=prev.et;
	}

	m_point[0xffff]=omega;

	for(point_params::Iterator I=m_point.begin();I!=m_point.end();++I)
	{
		point_param& cur=I.value();

		cur.p[0]=cur.sy;
		cur.p[1]=cur.sy+cur.dy;
		cur.m[0]=cur.st*cur.dx;
		cur.m[1]=cur.et*cur.dx;
	}
}

CubicHermiteSpline::~CubicHermiteSpline()
{}

Image::Pixel CubicHermiteSpline::calc(Image::Pixel x)
{
	point_params::Iterator U=m_point.lowerBound(x);

	if(U.key()==x)//node;
		return U.value().syi;

	point_params::Iterator L=U;
	--L;

	point_param& cur=L.value();

	Image::Pixel ti=static_cast<Image::Pixel>( ( static_cast<double>(x-L.key()) ) / cur.dx );

	calc_h(ti);

	double y=0.;
	for(int i=0;i<2;++i)
		y+=h[0][i][ti]*cur.p[i]+h[1][i][ti]*cur.m[i];

	Image::Pixel ret=static_cast<Image::Pixel>(clamp(y,0.,1.)*65535.);
	return ret;
}

void CubicHermiteSpline::calc_h(Image::Pixel x)
{
	if(h_cached[x])
		return;

	h_cached[x]=true;

	double t=static_cast<double>(x)/65535.;
	double t2=t*t;
	double t3=t2*t;

	for(int i2=0;i2<2;++i2)
		for(int i3=0;i3<2;++i3)
			h[i3][i2][x]=k[i3][i2][3]*t3+k[i3][i2][2]*t2+k[i3][i2][1]*t+k[i3][i2][0];
}
