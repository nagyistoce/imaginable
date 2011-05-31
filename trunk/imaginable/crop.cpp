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

#include <cstring>

#include "crop.hpp"


namespace imaginable {

Image crop(const Image& img,size_t left,size_t top,size_t width,size_t height,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	if ( (left >= img.width())
	||   (top  >= img.height()) )
		throw exception(exception::EMPTY_IMAGE);

	if ( left + width > img.width())
		width = img.width() - left;
	if ( top + height > img.height())
		height = img.height() - top;

	Image ret(width,height);

	const Image::t_planeNames& planeNames=img.planeNames();
	for (Image::t_planeNames::const_iterator I=planeNames.begin(); I!=planeNames.end(); ++I)
		ret.addPlane(*I);
	ret.setMaximum(img.maximum());
	ret.copyTextFrom(img);

	size_t m=planeNames.size();
	size_t i=0;

	if ( (left==0) && (width==img.width()) )
	{
		for (Image::t_planeNames::const_iterator I=planeNames.begin(); I!=planeNames.end(); ++I)
		{
			notifier(static_cast<float>(i++)/static_cast<float>(m));

			const Image::pixel* from=img.plane(*I);
			Image::pixel* to=ret.plane(*I);
			memcpy(to,from+top*img.width(),height*width*sizeof(Image::pixel));
		}
	}
	else
	{
		for (Image::t_planeNames::const_iterator I=planeNames.begin(); I!=planeNames.end(); ++I)
		{
			notifier(static_cast<float>(i++)/static_cast<float>(m));

			const Image::pixel* from=img.plane(*I);
			Image::pixel* to=ret.plane(*I);
			for (size_t row=0; row<height; ++row)
				memcpy(to+row*width,from+(row+top)*img.width()+left,width*sizeof(Image::pixel));
		}
	}

	return ret;
}

}
