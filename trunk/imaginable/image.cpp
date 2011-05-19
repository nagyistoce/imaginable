/*************
**
** Project:      Imaginable
** File info:    $Id$
** Author:       Copyright (C) 2011 Kuzma Shapran <Kuzma.Shapran@gmail.com>
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

#include "image.hpp"


namespace imaginable {

Image::Image(void)
{
	clear();
}

Image::Image(size_t width,size_t height)
{
	clear();
	setSize(width,height);
}

Image::~Image()
{
	for (t_unloaded_planes::iterator U=m_unloaded_planes.begin(); U!=m_unloaded_planes.end(); ++U)
		close(U->second);
}

Image Image::copy(void) const
{
	Image ret;

	ret.setSize(m_width,m_height);
	ret.setMaximum(m_maximum);
	size_t square=m_width*m_height*sizeof(pixel);
	for(t_planes::const_iterator I=m_plane.begin();I!=m_plane.end();++I)
	{
		ret.addPlane(I->first);
		memcpy(ret.plane(I->first),I->second.get(),square);
	}
	for(t_text::const_iterator I=m_text.begin();I!=m_text.end();++I)
		ret.setText(I->first,I->second);

	return ret;
}

Image::t_colourSpace Image::colourSpace(void) const
{
	if( m_plane.empty() )
		return IMAGE_EMPTY;

	if( (m_plane.size()==1) && (m_plane.begin()->first==PLANE_ALPHA) )
		return IMAGE_MASK;

	t_planeNames names=planeNames();
	names.erase(PLANE_ALPHA);

	switch(names.size())
	{
		case 1:
			switch(*names.begin())
			{
				case PLANE_MONO:
					return IMAGE_MONO;

				case PLANE_GREY:
					return IMAGE_GREY;

				default:;//to shut up compiler
			}
		break;
		case 3:
			if( names.find(PLANE_RED)  !=names.end()
			&&  names.find(PLANE_GREEN)!=names.end()
			&&  names.find(PLANE_BLUE) !=names.end() )
				return IMAGE_RGB;

			if( names.find(PLANE_HUE)           !=names.end()
			&&  names.find(PLANE_HSV_SATURATION)!=names.end()
			&&  names.find(PLANE_HSV_VALUE)     !=names.end() )
				return IMAGE_HSV;

			if( names.find(PLANE_HUE)           !=names.end()
			&&  names.find(PLANE_HSL_SATURATION)!=names.end()
			&&  names.find(PLANE_HSL_LIGHTNESS) !=names.end() )
				return IMAGE_HSL;
		break;
	}
	return IMAGE_UNUSUAL;
}

Image::t_planeNames Image::planeNames(void) const
{
	t_planeNames ret;
	for(t_planes::const_iterator I=m_plane.begin();I!=m_plane.end();++I)
		ret.insert(I->first);
	return ret;
}

bool Image::addPlane(unsigned planeName)
{
	if(empty())
		return false;

	t_planes::const_iterator I=m_plane.find(planeName);
	if(I!=m_plane.end())
		return false;

	pixel* new_plane=new pixel[m_width*m_height];
	memset(new_plane,0,m_width*m_height*sizeof(pixel));
	m_plane[planeName]=t_plane(new_plane);
	return true;
}

bool Image::removePlane(unsigned planeName)
{
	t_planes::iterator I=m_plane.find(planeName);
	if(I==m_plane.end())
		return false;

	if (!I->second.get())
		close(m_unloaded_planes[planeName]);

	m_plane.erase(I);
	return true;
}

bool Image::renamePlane(unsigned from,unsigned to)
{
	if(empty())
		return false;

	t_planes::iterator F=m_plane.find(from);
	if(F==m_plane.end())
		return false;

	t_planes::iterator T=m_plane.find(to);
	if(T!=m_plane.end())
		return false;

	m_plane[to]=F->second;
	m_plane.erase(F);
	return true;
}

const Image::pixel* Image::plane(unsigned planeName) const
{
	t_planes::const_iterator I=m_plane.find(planeName);
	if (I==m_plane.end())
		return NULL;
	if (!I->second.get())
	{
		reloadPlane(planeName);
		I=m_plane.find(planeName);
	}
	return I->second.get();
}

Image::pixel* Image::plane(unsigned planeName)
{
	t_planes::iterator I=m_plane.find(planeName);
	if (I==m_plane.end())
		return NULL;
	if (!I->second.get())
	{
		reloadPlane(planeName);
		I=m_plane.find(planeName);
	}
	return m_plane[planeName].get();
}

bool Image::unloadPlane(unsigned planeName) const
{
	t_planes::iterator I=m_plane.find(planeName);
	if (I==m_plane.end())
		return false;
	if (!I->second.get())
		return false;

	char fn[]="/tmp/imageXXXXXX";
	int fd=mkstemp(fn);
	unlink(fn);
	if (write(fd,I->second.get(),m_width*m_height*sizeof(pixel)) == static_cast<ssize_t>(m_width*m_height*sizeof(pixel)))
	{
		lseek(fd,SEEK_SET,0);
		I->second.reset();
		m_unloaded_planes[planeName] = fd;
		return true;
	}
	else
	{
		close(fd);
		return false;
	}
}

bool Image::reloadPlane(unsigned planeName) const
{
	t_planes::iterator I=m_plane.find(planeName);
	if (I==m_plane.end())
		return false;
	if (I->second.get())
		return false;

	int fd=m_unloaded_planes[planeName];
	lseek(fd,SEEK_SET,0);

	pixel* new_plane=new pixel[m_width*m_height];
	if (read(fd,new_plane,m_width*m_height*sizeof(pixel)) == static_cast<ssize_t>(m_width*m_height*sizeof(pixel)))
	{
		m_plane[planeName]=t_plane(new_plane);
		close(fd);
		m_unloaded_planes.erase(planeName);
		return true;
	}
	else
	{
		delete[] new_plane;
		return false;
	}
}

bool Image::planeUnloaded(unsigned planeName) const
{
	t_planes::iterator I=m_plane.find(planeName);
	if (I==m_plane.end())
		return false;
	if (I->second.get())
		return false;
	return true;
}

bool Image::setWidth(size_t width)
{
	if(!m_plane.empty())
		return false;

	m_width=width;
	return true;
}

bool Image::setHeight(size_t height)
{
	if(!m_plane.empty())
		return false;

	m_height=height;
	return true;
}

bool Image::setSize(size_t width,size_t height)
{
	if(!m_plane.empty())
		return false;

	m_width=width;
	m_height=height;
	return true;
}

void Image::clear(void)
{
	m_width=m_height=m_maximum=0;
	m_plane.clear();
	m_text.clear();
}

Image::t_text_keys Image::text_keys(void) const
{
	t_text_keys ret;
	for(t_text::const_iterator I=m_text.begin();I!=m_text.end();++I)
		ret.insert(I->first);
	return ret;
}

}
