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


#include "scale.hpp"


namespace imaginable {


#define NOTIFY_STEP 10

SharedImage scale_nearest(const Image& img,size_t width,size_t height,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	if ( !width
	||   !height )
		throw exception(exception::EMPTY_IMAGE);

	SharedImage ret(new Image(width,height));
	Image::PlaneNames planeNames=img.planeNames();
	for (Image::PlaneNames::const_iterator I=planeNames.begin(); I!=planeNames.end(); ++I)
		ret->addPlane(*I);

	float ntf_offset = 0.0;
	float ntf_scale = 1./static_cast<float>(planeNames.size());

	for (Image::PlaneNames::const_iterator I=planeNames.begin(); I!=planeNames.end(); ++I)
	{
		const Image::Pixel* src_plane=img. plane(*I);
		/* */ Image::Pixel* dst_plane=ret->plane(*I);

		for (size_t y=0; y<height; ++y)
		{
			if (!(y%NOTIFY_STEP))
				notifier(ntf_offset+ntf_scale*static_cast<float>(y)/static_cast<float>(height));

			size_t src_y=( y * img.height() ) / height;

			size_t src_yo=src_y*img.width();
			size_t dst_yo=y*width;
			for (size_t x=0; x<width; ++x)
			{
				size_t src_x=( x * img.width() ) / width;
				dst_plane[dst_yo+x] = src_plane[src_yo + src_x];
			}
		}
		ntf_offset += ntf_scale;
		notifier(ntf_offset);
	}
	notifier(1.);

	ret->setMaximum(img.maximum());
	ret->copyTextFrom(img);

	return ret;
}

}
