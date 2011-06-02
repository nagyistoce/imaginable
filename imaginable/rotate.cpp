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


#include <cmath>

#include "point.hpp"
#include "rotate.hpp"


namespace imaginable {

template<typename T>
T sign_(const T& value)
{
	if (value>static_cast<T>(0.))
		return static_cast<T>(1.);
	if (value<static_cast<T>(0.))
		return static_cast<T>(-1.);
	return static_cast<T>(0.);
}

#define sign(VALUE) sign_<double>(VALUE)

#define NOTIFY_STEP 10

boost::shared_ptr<Image> rotate(const Image& img,double radian,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	Point center;
	Point corner[4];
	//  0     1
	//   +---+
	//   |   |
	//   +---+
	//  2     3
	center=Point(static_cast<double>(img.width())/2.,static_cast<double>(img.height())/2.);
	corner[0]=Point(           -center.x,            -center.y);
	corner[1]=Point(img.width()-center.x,            -center.y);
	corner[2]=Point(           -center.x,img.height()-center.y);
	corner[3]=Point(img.width()-center.x,img.height()-center.y);
	for (size_t i=0; i<4; ++i)
	{
		Point& p=corner[i];
		p=p.polar();
		p.f+=radian;
		p=p.rect();
	}
	Point topleft(
		std::min(std::min(corner[0].x,corner[1].x),std::min(corner[2].x,corner[3].x)),
		std::min(std::min(corner[0].y,corner[1].y),std::min(corner[2].y,corner[3].y)) );
	Point bottomright(
		std::max(std::max(corner[0].x,corner[1].x),std::max(corner[2].x,corner[3].x)),
		std::max(std::max(corner[0].y,corner[1].y),std::max(corner[2].y,corner[3].y)) );

	size_t new_width =static_cast<size_t>(ceil(bottomright.x+.5*sign(bottomright.x))-ceil(topleft.x+.5*sign(topleft.x)));
	size_t new_height=static_cast<size_t>(ceil(bottomright.y+.5*sign(bottomright.y))-ceil(topleft.y+.5*sign(topleft.y)));

	Point new_center=Point(static_cast<double>(new_width)/2.,static_cast<double>(new_height)/2.);

	boost::shared_ptr<Image> ret(new Image(new_width,new_height));
	Image::t_planeNames planeNames=img.planeNames();
	for (Image::t_planeNames::const_iterator I=planeNames.begin(); I!=planeNames.end(); ++I)
		ret->addPlane(*I);
	ret->addPlane(Image::PLANE__INTERNAL);

	int xo[4]={0,1,0,1};
	int yo[4]={0,0,1,1};
	int xyo[4];
	for (size_t i=0; i<4; ++i)
		xyo[i]=xo[i]+yo[i]*img.width();

	float mult = ((!ret->hasTransparency()) && (img.maximum()==Image::MAXIMUM)) ? 1.0 : 0.5;

	for (size_t y=0; y<new_height; ++y)
	{
		if (!(y%NOTIFY_STEP))
			notifier(mult*static_cast<float>(y)/static_cast<float>(new_height));

		size_t dst_yo=y*new_width;
		for (size_t x=0; x<new_width; ++x)
		{
			Point p(static_cast<double>(x)-new_center.x+.5,static_cast<double>(y)-new_center.y+.5);
			p=p.polar();
			p.f-=radian;
			p=p.rect();
			int s_x=static_cast<int>((p.x+center.x-.5)*16.);
			int s_y=static_cast<int>((p.y+center.y-.5)*16.);
			int src_x=(s_x<0) ? ((s_x-0x0f)/0x10) : (s_x/0x10);
			int src_y=(s_y<0) ? ((s_y-0x0f)/0x10) : (s_y/0x10);

			size_t k[4];
			{
				int fr_x=((s_x%0x10)+0x10)%0x10;
				int fr_y=((s_y%0x10)+0x10)%0x10;

				k[0]=(0x10-fr_x)*(0x10-fr_y);
				k[1]=      fr_x *(0x10-fr_y);
				k[2]=(0x10-fr_x)*      fr_y ;
				k[3]=      fr_x *      fr_y ;
			}

			int src_yxo=src_y*img.width()+src_x;


			size_t k_eff[4]={0};
			Image::pixel* dst_plane=ret->plane(Image::PLANE__INTERNAL);
			if ( (src_x>=0)
			&&   (src_x+1<static_cast<int>(img.width()))
			&&   (src_y>=0)
			&&   (src_y+1<static_cast<int>(img.height())) )
			{
				for (size_t i=0; i<4; ++i)
					k_eff[i]=k[i];
			}
			else
			{
				if ( (src_x+1>=0)
				&&   (src_x<static_cast<int>(img.width()))
				&&   (src_y+1>=0)
				&&   (src_y<static_cast<int>(img.height())) )
					for (size_t i=0; i<4; ++i)
						if( (src_x+xo[i]>=0)
						&&  (src_x+xo[i]<static_cast<int>(img.width()))
						&&  (src_y+yo[i]>=0)
						&&  (src_y+yo[i]<static_cast<int>(img.height())) )
							k_eff[i]=k[i];
			}
			size_t K=0;
			for (size_t i=0; i<4; ++i)
				K+=k_eff[i];
			dst_plane[dst_yo+x]=( K ? ((K*0x100)-1) : 0 );

			for (Image::t_planeNames::const_iterator I=planeNames.begin(); I!=planeNames.end(); ++I)
			{
				dst_plane=ret->plane(*I);

				size_t v=0;
				if (K)
				{
					const Image::pixel* src_plane=img.plane(*I);
					for (size_t i=0; i<4; ++i)
						if (k_eff[i])
							v+=k_eff[i]*static_cast<size_t>(src_plane[src_yxo+xyo[i]]);
				}
				switch (K)
				{
				case 0:
					dst_plane[dst_yo+x]=0;
					break;
				default:
					dst_plane[dst_yo+x]=static_cast<Image::pixel>(v/K);
					break;
				case 0x100:
					dst_plane[dst_yo+x]=static_cast<Image::pixel>(v/0x100);
					break;
				}
			}
		}
	}

	if (ret->hasTransparency())
	{
		const Image::pixel* src_plane=ret->plane(Image::PLANE__INTERNAL);
		const Image::pixel* alpha_plane=img.plane(Image::PLANE_ALPHA);
		Image::pixel* dst_plane=ret->plane(Image::PLANE_ALPHA);
		for (size_t y=0; y<new_height; ++y)
		{
			if (!(y%NOTIFY_STEP))
				notifier(0.5+0.5*static_cast<float>(y)/static_cast<float>(new_height));

			size_t dst_yo=y*new_width;
			for (size_t x=0; x<new_width; ++x)
			{
				size_t dst_yxo=dst_yo+x;
				if (src_plane[dst_yxo]==Image::MAXIMUM)
					dst_plane[dst_yxo]=alpha_plane[dst_yxo];
				else if (!src_plane[dst_yxo])
					dst_plane[dst_yxo]=0;
				else
					dst_plane[dst_yxo]=static_cast<Image::pixel>(
						(static_cast<size_t>(alpha_plane[dst_yxo])
						*static_cast<size_t>(  src_plane[dst_yxo]))/Image::MAXIMUM
						);
			}
		}
		ret->removePlane(Image::PLANE__INTERNAL);
	}
	else
	{
		if (img.maximum()!=Image::MAXIMUM)
		{
			Image::pixel* dst_plane=ret->plane(Image::PLANE__INTERNAL);
			for (size_t y=0; y<new_height; ++y)
			{
				if (!(y%NOTIFY_STEP))
					notifier(0.5+0.5*static_cast<float>(y)/static_cast<float>(new_height));

				size_t dst_yo=y*new_width;
				for (size_t x=0;x<new_width;++x)
				{
					size_t dst_yxo=dst_yo+x;
					if (dst_plane[dst_yxo]==Image::MAXIMUM)
						dst_plane[dst_yxo]=img.maximum();
					else if (dst_plane[dst_yxo])
						dst_plane[dst_yxo]=static_cast<Image::pixel>(
							(img.maximum()
							*static_cast<size_t>(dst_plane[dst_yxo]))/Image::MAXIMUM
							);
				}
			}
		}
		ret->renamePlane(Image::PLANE__INTERNAL,Image::PLANE_ALPHA);
	}

	ret->setMaximum(img.maximum());
	ret->copyTextFrom(img);

	return ret;
}

}
