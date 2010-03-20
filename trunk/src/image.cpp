/***** *
 *
*Project:   Imaginable
*Created:   2009-09-03
*Author:    Kuzma Shapran <Kuzma.Shapran@gmail.com>
*License:   GPLv3
 *
******/
// $Id$


#include "image.hpp"

#include <cstring>


Image::Image(void)
{
	clear();
}

Image::Image(size_t width,size_t height,Pixel maximum)
{
	clear();
	setSize(width,height);
	setMaximum(maximum);
}

Image::~Image()
{
}

Image* Image::clone(void) const
{
	Image* ret=new Image;
	copyTo(*ret);
	return ret;
}

void Image::copyTo(Image& dst) const
{
	dst.setSize(m_width,m_height);
	dst.setMaximum(m_maximum);
	size_t squareBytes=square()*sizeof(Pixel);
	for(ColourPlanes::const_iterator I=m_plane.begin();I!=m_plane.end();++I)
	{
		dst.addPlane(I->first);
		memcpy(dst.plane(I->first),I->second.get(),squareBytes);
	}
	dst.m_planeName=m_planeName;
	dst.m_text=m_text;
}

Image::ColourSpace Image::colourSpace(void) const
{
	if( m_plane.empty() )
		return SPACE__EMPTY;

	if( (m_plane.size()==1) && (m_plane.begin()->first==PLANE_ALPHA) )
		return SPACE_ALPHA;

	ColourPlaneSet planeSet=planes();
	planeSet.erase(PLANE_ALPHA);

	switch(planeSet.size())
	{
		case 1:
			switch(*planeSet.begin())
			{
				case PLANE_LIGHTNESS:
					return SPACE_LIGHTNESS;
				default:;//to shut up compiler
			}
		break;
		case 3:
			if( planeSet.find(PLANE_RED)  !=planeSet.end()
			&&  planeSet.find(PLANE_GREEN)!=planeSet.end()
			&&  planeSet.find(PLANE_BLUE) !=planeSet.end() )
				return SPACE_RGB;
		break;
		case 4:
			if( planeSet.find(PLANE_HUE_SECTOR)    !=planeSet.end()
			&&  planeSet.find(PLANE_HUE)           !=planeSet.end()
			&&  planeSet.find(PLANE_HSV_SATURATION)!=planeSet.end()
			&&  planeSet.find(PLANE_HSV_VALUE)     !=planeSet.end() )
				return SPACE_HSV;

			if( planeSet.find(PLANE_HUE_SECTOR)    !=planeSet.end()
			&&  planeSet.find(PLANE_HUE)           !=planeSet.end()
			&&  planeSet.find(PLANE_HSL_SATURATION)!=planeSet.end()
			&&  planeSet.find(PLANE_LIGHTNESS)     !=planeSet.end() )
				return SPACE_HSL;
		break;
	}
	return SPACE__CUSTOM;
}

Image::ColourPlaneSet Image::planes(void) const
{
	ColourPlaneSet ret;
	for(ColourPlanes::const_iterator I=m_plane.begin();I!=m_plane.end();++I)
		ret.insert(I->first);
	return ret;
}

bool Image::addPlane(ColourPlane planeName)
{
	if(empty())
		return false;

	ColourPlanes::const_iterator I=m_plane.find(planeName);
	if(I!=m_plane.end())
		return false;

	const size_t& square_=square();
	Plane new_plane(new Pixel[square_]);
	memset(new_plane.get(),0,square_*sizeof(Pixel));
	m_plane[planeName]=new_plane;

	return true;
}

bool Image::removePlane(ColourPlane planeName)
{
	ColourPlanes::iterator I=m_plane.find(planeName);
	if(I==m_plane.end())
		return false;

	m_plane.erase(I);
	return true;
}

bool Image::movePlane(ColourPlane from,ColourPlane to)
{
	if(empty())
		return false;

	ColourPlanes::iterator F=m_plane.find(from);
	if(F==m_plane.end())
		return false;

	ColourPlanes::iterator T=m_plane.find(to);
	if(T!=m_plane.end())
		return false;

	m_plane[to]=F->second;
	m_plane.erase(F);
	return true;
}

bool Image::planeHasName(ColourPlane planeName) const
{
	return m_planeName.find(planeName)!=m_planeName.end();
}

std::string Image::planeName(ColourPlane planeName) const
{
	ColourPlaneNames::const_iterator I=m_planeName.find(planeName);
	return (I==m_planeName.end()) ? std::string() : I->second;
}

bool Image::setPlaneName(ColourPlane planeName,const std::string& value)
{
	if(value.empty())
		return erasePlaneName(planeName);

	ColourPlaneNames::iterator I=m_planeName.find(planeName);
	if(I!=m_planeName.end())
	{
		I->second=value;
		return false;
	}

	m_planeName[planeName]=value;
	return true;
}

bool Image::erasePlaneName(ColourPlane planeName)
{
	ColourPlaneNames::iterator I=m_planeName.find(planeName);

	if(I==m_planeName.end())
		return false;

	m_planeName.erase(I);
	return true;
}

const Image::Pixel* Image::plane(ColourPlane planeName) const
{
	ColourPlanes::const_iterator I=m_plane.find(planeName);

	return (I==m_plane.end()) ? NULL : I->second.get();
}

Image::Pixel* Image::plane(ColourPlane planeName)
{
	ColourPlanes::iterator I=m_plane.find(planeName);

	return (I==m_plane.end()) ? NULL : I->second.get();
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
	m_width=m_height=0;
	m_maximum=static_cast<Pixel>(-1);
	m_plane.clear();
	m_text.clear();
}

std::string Image::text(const std::string& key) const
{
	Text::const_iterator I=m_text.find(key);

	return (I==m_text.end()) ? std::string() : I->second;
}

Image::TextKeys Image::textKeys(void) const
{
	TextKeys ret;

	for(Text::const_iterator I=m_text.begin();I!=m_text.end();++I)
		ret.insert(I->first);

	return ret;
}

bool Image::setText(const std::string& key,const std::string& value)
{
	if(value.empty())
		return eraseText(key);

	Text::iterator I=m_text.find(key);
	if(I!=m_text.end())
	{
		I->second=value;
		return false;
	}

	m_text[key]=value;
	return true;
}

bool Image::eraseText(const std::string& key)
{
	Text::iterator I=m_text.find(key);

	if(I==m_text.end())
		return false;

	m_text.erase(I);
	return true;
}
