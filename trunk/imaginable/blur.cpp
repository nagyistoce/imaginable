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


#include <boost/scoped_array.hpp>
#include <boost/bind.hpp>

#include "blur.hpp"


namespace imaginable {

#define NOTIFY_STEP 10

template <typename PIXEL, typename ALPHA_PIXEL>
void universal_box_blur(PIXEL *plane,ssize_t width,ssize_t height,size_t radius,ALPHA_PIXEL *alpha,progress_notifier notifier)
{

	ssize_t sRadius=static_cast<ssize_t>(radius);
	size_t blur_size=sRadius*2+1;


	boost::scoped_array<PIXEL> sc_acc(new PIXEL[blur_size]);
	PIXEL* acc=sc_acc.get();

	boost::scoped_array<bool> sc_use(new bool[blur_size]);
	bool* use=sc_use.get();

	for(ssize_t x=0;x<width;++x)
	{
		if (!(x%NOTIFY_STEP))
			notifier(0.5*static_cast<float>(x)/static_cast<float>(width));

		uint64_t blur=0;

		size_t areaSize=0;

		for(size_t i=0;i<blur_size;++i)
		{
			acc[i]=0;
			use[i]=false;
		}

		for(ssize_t y=-sRadius;y<height;++y)
		{
			ssize_t y_del=y-sRadius;
			ssize_t y_add=y+sRadius;

			ssize_t p_add=y_add*width+x;

			ssize_t i_del=(y_del+blur_size)%blur_size;
			ssize_t i_add= y_add           %blur_size;

			if(( use[i_add]=
				(  (y_add<height)
				&& ( (!alpha)
				||   alpha[p_add] ) ) ))
			{
				const PIXEL& value=plane[p_add];
				acc[i_add]=value;
				blur+=static_cast<uint64_t>(value);
				++areaSize;
			}

			if(use[i_del])
			{
				blur-=static_cast<uint64_t>(acc[i_del]);
				--areaSize;
			}

			if( (y>=0)
			&&  areaSize)
				plane[y*width+x]=static_cast<PIXEL>(blur/areaSize);
		}
	}
	notifier(0.5);

	for(ssize_t y=0;y<height;++y)
	{
		if (!(y%NOTIFY_STEP))
			notifier(0.5+0.5*static_cast<float>(y)/static_cast<float>(height));

		uint64_t blur=0;

		size_t areaSize=0;

		for(size_t i=0;i<blur_size;++i)
		{
			acc[i]=0;
			use[i]=false;
		}

		for(ssize_t x=-sRadius;x<width;++x)
		{
			ssize_t x_del=x-sRadius;
			ssize_t x_add=x+sRadius;

			ssize_t p_add=y*width+x_add;

			ssize_t i_del=(x_del+blur_size)%blur_size;
			ssize_t i_add= x_add           %blur_size;

			if(( use[i_add]=
				(  (x_add<width)
				&& ( (!alpha)
				||   alpha[p_add] ) ) ))
			{
				const PIXEL& value=plane[p_add];
				acc[i_add]=value;
				blur+=static_cast<uint64_t>(value);
				++areaSize;
			}

			if(use[i_del])
			{
				blur-=static_cast<uint64_t>(acc[i_del]);
				--areaSize;
			}

			if( (x>=0)
			&&  areaSize)
				plane[y*width+x]=static_cast<PIXEL>(blur/areaSize);
		}
	}
	notifier(1.0);
}

template <typename PIXEL, typename ALPHA_PIXEL>
void universal_gaussian_blur(PIXEL *plane,ssize_t width,ssize_t height,size_t radius,ALPHA_PIXEL *alpha,progress_notifier notifier)
{
	size_t radius_3 = radius/3;
	if (radius_3)
		universal_box_blur<PIXEL,ALPHA_PIXEL>(plane,width,height,radius_3,alpha,boost::bind(&scaled_notifier,notifier,0.   ,1./3.,_1));
	radius_3 = radius - 2*radius_3;
	if (radius_3)
		universal_box_blur<PIXEL,ALPHA_PIXEL>(plane,width,height,radius_3,alpha,boost::bind(&scaled_notifier,notifier,1./3.,1./3.,_1));
	radius_3 = radius/3;
	if (radius_3)
		universal_box_blur<PIXEL,ALPHA_PIXEL>(plane,width,height,radius_3,alpha,boost::bind(&scaled_notifier,notifier,2./3.,1./3.,_1));
}

void box_blur(Image& img,unsigned planeName,size_t radius,bool use_alpha,progress_notifier notifier)
{
	if(!img.hasData())
		throw exception(exception::NO_IMAGE);

	if( (!radius) || (radius>(std::max(img.width(),img.height()))) )
		throw exception(exception::INVALID_RADIUS);

	if(!img.hasPlane(planeName))
		throw exception(exception::INVALID_PLANE);

	universal_box_blur<Image::Pixel,Image::Pixel>(img.plane(planeName),img.width(),img.height(),radius,use_alpha?img.plane(Image::PLANE_ALPHA):NULL,notifier);
}

void gaussian_blur(Image& img,unsigned planeName,size_t radius,bool use_alpha,progress_notifier notifier)
{
	if(!img.hasData())
		throw exception(exception::NO_IMAGE);

	if( (!radius) || (radius>(std::max(img.width(),img.height()))) )
		throw exception(exception::INVALID_RADIUS);

	if(!img.hasPlane(planeName))
		throw exception(exception::INVALID_PLANE);

	universal_gaussian_blur<Image::Pixel,Image::Pixel>(img.plane(planeName),img.width(),img.height(),radius,use_alpha?img.plane(Image::PLANE_ALPHA):NULL,notifier);
}

void force_instantiate_template(void)
{
	universal_gaussian_blur<uint32_t,Image::Pixel>(NULL,0,0,0,NULL,dont_notify);
}

#if 0
Image::Pixel* box_blur(Image& img,unsigned src_plane,unsigned blurred_plane,size_t radius,bool use_alpha)
{
	if(!img.hasData())
		throw exception(exception::NO_IMAGE);

	if( (!radius) || (radius>(std::max(img.width(),img.height()))) )
		throw exception(exception::INVALID_RADIUS);

	if(!img.hasPlane(src_plane))
		throw exception(exception::INVALID_PLANE);

	if(img.hasPlane(blurred_plane))
		throw exception(exception::PLANE_EXISTS);


	ssize_t width =static_cast<ssize_t>(img.width());
	ssize_t height=static_cast<ssize_t>(img.height());

	img.addPlane(blurred_plane);

	const Image::Pixel* src=img.plane(src_plane);
	Image::Pixel* blurred=img.plane(blurred_plane);
	Image::Pixel* alpha=img.plane(Image::PLANE_ALPHA);

	ssize_t sRadius=static_cast<ssize_t>(radius);

	for(ssize_t y=0;y<height;++y)
	{
		ssize_t ymin=y-sRadius;
		ssize_t ymax=y+sRadius;

		if(ymin<0)
			ymin=0;
		if(ymax>height-1)
			ymax=height-1;

		uint64_t blur=0;

		size_t areaSize=0;

		for(ssize_t x=-sRadius;x<width;++x)
		{
			ssize_t xdel=x-sRadius-1;
			ssize_t xadd=x+sRadius;

			bool add=true;
			if(xadd>width-1)
				add=false;

			bool del=true;
			if(xdel<0)
				del=false;

			bool process=true;
			if(x<0)
				process=false;

			if(add)
			{
				if(use_alpha)
				{
					for(ssize_t ry=ymin;ry<=ymax;++ry)
					{
						size_t p=ry*width+xadd;
						if(alpha[p])
						{
							blur+=static_cast<uint64_t>(src[p]);
							++areaSize;
						}
					}
				}
				else
				{
					if(!del)
						areaSize+=ymax-ymin+1;
					for(ssize_t ry=ymin;ry<=ymax;++ry)
						blur+=static_cast<uint64_t>(src[ry*width+xadd]);
				}
			}

			if(del)
			{
				if(use_alpha)
				{
					for(ssize_t ry=ymin;ry<=ymax;++ry)
					{
						size_t p=ry*width+xdel;
						if(alpha[p])
						{
							blur-=static_cast<uint64_t>(src[p]);
							--areaSize;
						}
					}
				}
				else
				{
					if(!add)
						areaSize-=ymax-ymin+1;
					for(ssize_t ry=ymin;ry<=ymax;++ry)
						blur-=static_cast<uint64_t>(src[ry*width+xdel]);
				}
			}

			if(process)
			{
				if(areaSize)
					blurred[y*width+x]=static_cast<Image::Pixel>(blur/areaSize);
				else
					blurred[y*width+x]=src[y*width+x];
			}
		}
	}
	return blurred;
}
#endif
}
