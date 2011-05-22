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

#include "maximum.hpp"


namespace imaginable {

Image::pixel findMaximum(const Image& img,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	Image::pixel ret=0;
	Image::t_planeNames planeNames=img.planeNames();

	size_t m=planeNames.size();
	size_t i=0;

	for (Image::t_planeNames::const_iterator I=planeNames.begin(); I!=planeNames.end(); ++I)
	{
		notifier(static_cast<float>(i++)/static_cast<float>(m));

		ret=std::max(ret,findMaximum(img,*I));
	}
	return ret;
}

Image::pixel findMaximum(const Image& img,unsigned planeName,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	const Image::pixel* data=img.plane(planeName);
	if (!data)
		throw exception(exception::INVALID_PLANE);

	Image::pixel ret=0;
	size_t m=img.width()*img.height();
	for (size_t i=0; i<m; ++i)
	{
		notifier(static_cast<float>(i)/static_cast<float>(m));

		ret=std::max(ret,data[i]);
	}
	return ret;
}

}
