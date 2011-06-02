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

#include <cstring>

#include "auto_crop.hpp"
#include "crop.hpp"


namespace imaginable {

boost::shared_ptr<Image> auto_crop(const Image& img,size_t& left,size_t& top,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	const Image::pixel* alpha=img.plane(Image::PLANE_ALPHA);

	if (!alpha)
		throw exception(exception::NO_ALPHA_CHANNEL);

	const size_t& width=img.width();
	const size_t& height=img.height();

	left=0;
	top =0;
	size_t right =width;
	size_t bottom=height;

	{
		size_t oneline_size=width*sizeof(Image::pixel);
		boost::scoped_array<Image::pixel> oneline(new Image::pixel[width]);
		memset(oneline.get(),0,oneline_size);

		for (size_t y=0; y<height; ++y)
		{
			notifier(0.2*static_cast<float>(y)/static_cast<float>(height));

			top=y;
			if (memcmp(alpha+y*width,oneline.get(),oneline_size))
				break;
		}

		for (size_t y=height; y>0; --y)
		{
			notifier(0.2+0.2*static_cast<float>(y)/static_cast<float>(height));

			bottom=y;
			if (memcmp(alpha+(y-1)*width,oneline.get(),oneline_size))
				break;
		}
	}
	{
		bool stop;

		stop=false;
		for (size_t x=0; x<width; ++x)
		{
			notifier(0.4+0.2*static_cast<float>(x)/static_cast<float>(width));

			for (size_t y=top; y<bottom; ++y)
			{
				if (alpha[y*width+x])
				{
					stop=true;
					break;
				}
			}
			left=x;
			if (stop)
				break;
		}

		stop=false;
		for (size_t x=width; x>0; --x)
		{
			notifier(0.6+0.2*static_cast<float>(x)/static_cast<float>(width));

			for (size_t y=top; y<bottom; ++y)
			{
				if (alpha[y*width+x-1])
				{
					stop=true;
					break;
				}
			}
			right=x;
			if (stop)
				break;
		}
	}

	if( (left==right) || (top==bottom) )
		throw exception(exception::FULLY_TRANSPARENT_IMAGE);

	return crop(img,left,top,right-left,bottom-top,boost::bind(&scaled_notifier,notifier,0.8,0.2,_1));
/*
	Image* ret=new Image(right-left,bottom-top);

	const Image::t_planeNames& planeNames=img.planeNames();
	for (Image::t_planeNames::const_iterator I=planeNames.begin(); I!=planeNames.end(); ++I)
		ret.addPlane(*I);
	ret.setMaximum(img.maximum());
	ret.copyTextFrom(img);

	size_t m=planeNames.size();
	size_t i=0;

	if ( (left==0) && (right==width) )
	{
		for (Image::t_planeNames::const_iterator I=planeNames.begin(); I!=planeNames.end(); ++I)
		{
			notifier(0.8+0.2*static_cast<float>(i++)/static_cast<float>(m));

			const Image::pixel* from=img.plane(*I);
			Image::pixel* to=ret.plane(*I);
			memcpy(to,from+top*width,(bottom-top)*width*sizeof(Image::pixel));
		}
	}
	else
	{
		for (Image::t_planeNames::const_iterator I=planeNames.begin(); I!=planeNames.end(); ++I)
		{
			notifier(0.8+0.2*static_cast<float>(i++)/static_cast<float>(m));

			const Image::pixel* from=img.plane(*I);
			Image::pixel* to=ret.plane(*I);
			for (size_t y=0; y<(bottom-top); ++y)
				memcpy(to+y*(right-left),from+(y+top)*width+left,(right-left)*sizeof(Image::pixel));
		}
	}

	return ret;
*/
}

}
