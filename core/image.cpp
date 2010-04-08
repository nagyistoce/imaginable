/* * * * * *
 *
 * Project:   Imaginable
 * Created:   2009-09-03
 * Author:    Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */
// $Id$


#include "image.hpp"


Image::Image(QObject* parent)
	: QObject(parent)
	, m_busy(false)
	, m_percent(0.)
{
	clear();
}

Image::~Image()
{
}

bool Image::copyFrom(const Image& src)
{
	if(src.busy())
		return false;
	m_size     =src.m_size;
	m_maximum  =src.m_maximum;
	m_plane    =src.m_plane;
	m_planeName=src.m_planeName;
	m_text     =src.m_text;
	return true;
}

Image::ColourSpace Image::colourSpace(void) const
{
	if( m_plane.isEmpty() )
		return Image::SPACE__EMPTY;

	if( (m_plane.size()==1) && (m_plane.constFind(Image::PLANE_ALPHA)!=m_plane.constEnd()) )
		return Image::SPACE_ALPHA;

	Image::ColourPlaneSet planeSet=planes();
	planeSet.remove(Image::PLANE_ALPHA);

	switch(planeSet.size())
	{
		case 1:
			if( planeSet.constFind(Image::PLANE_LIGHTNESS) !=planeSet.constEnd() )
				return Image::SPACE_LIGHTNESS;
		break;
		case 3:
			if( planeSet.constFind(Image::PLANE_RED)       !=planeSet.constEnd()
			&&  planeSet.constFind(Image::PLANE_GREEN)     !=planeSet.constEnd()
			&&  planeSet.constFind(Image::PLANE_BLUE)      !=planeSet.constEnd() )
				return Image::SPACE_RGB;
		break;
		case 4:
			if( planeSet.constFind(Image::PLANE_HUE_SECTOR)!=planeSet.constEnd()
			&&  planeSet.constFind(Image::PLANE_HUE)       !=planeSet.constEnd()
			&&  planeSet.constFind(Image::PLANE_SATURATION)!=planeSet.constEnd()
			&&  planeSet.constFind(Image::PLANE_HSV_VALUE) !=planeSet.constEnd() )
				return Image::SPACE_HSV;

			if( planeSet.constFind(Image::PLANE_HUE_SECTOR)!=planeSet.constEnd()
			&&  planeSet.constFind(Image::PLANE_HUE)       !=planeSet.constEnd()
			&&  planeSet.constFind(Image::PLANE_SATURATION)!=planeSet.constEnd()
			&&  planeSet.constFind(Image::PLANE_LIGHTNESS) !=planeSet.constEnd() )
				return Image::SPACE_HSL;
		break;
	}
	return Image::SPACE__CUSTOM;
}

bool Image::addPlane(Image::ColourPlane planeName)
{
	if(isEmpty())
		return false;

	ColourPlanes::ConstIterator I=m_plane.find(planeName);
	if(I!=m_plane.end())
		return false;

	const int& square=area();
	Plane new_plane(new Image::Pixel[square]);
	memset(new_plane.get(),0,square*sizeof(Image::Pixel));
	m_plane[planeName]=new_plane;

	return true;
}

bool Image::removePlane(Image::ColourPlane planeName)
{
	ColourPlanes::Iterator I=m_plane.find(planeName);
	if(I==m_plane.end())
		return false;

	m_plane.erase(I);
	return true;
}

bool Image::movePlane(Image::ColourPlane from,Image::ColourPlane to)
{
	if(isEmpty())
		return false;

	ColourPlanes::Iterator F=m_plane.find(from);
	if(F==m_plane.end())
		return false;

	ColourPlanes::Iterator T=m_plane.find(to);
	if(T!=m_plane.end())
		return false;

	m_plane[to]=F.value();
	m_plane.erase(F);
	return true;
}

bool Image::planeHasName(Image::ColourPlane planeName) const
{
	return m_planeName.find(planeName)!=m_planeName.end();
}

QString Image::planeName(Image::ColourPlane planeName) const
{
	ColourPlaneNames::ConstIterator I=m_planeName.find(planeName);
	return (I==m_planeName.end()) ? QString() : I.value();
}

bool Image::setPlaneName(Image::ColourPlane planeName,QString value)
{
	if(value.isEmpty())
		return erasePlaneName(planeName);

	ColourPlaneNames::Iterator I=m_planeName.find(planeName);
	if(I!=m_planeName.end())
	{
		*I=value;
		return false;
	}

	m_planeName[planeName]=value;
	return true;
}

bool Image::erasePlaneName(Image::ColourPlane planeName)
{
	ColourPlaneNames::Iterator I=m_planeName.find(planeName);

	if(I==m_planeName.end())
		return false;

	m_planeName.erase(I);
	return true;
}

const Image::Pixel* Image::plane(Image::ColourPlane planeName) const
{
	ColourPlanes::ConstIterator I=m_plane.find(planeName);

	return (I==m_plane.end()) ? NULL : I.value().get();
}

Image::Pixel* Image::plane(Image::ColourPlane planeName)
{
	ColourPlanes::Iterator I=m_plane.find(planeName);

	return (I==m_plane.end()) ? NULL : I.value().get();
}

void Image::setWidth(int width)
{
	if(!m_plane.isEmpty())
		return;

	m_size.setWidth(width);
}

void Image::setHeight(int height)
{
	if(!m_plane.isEmpty())
		return;

	m_size.setHeight(height);
}

void Image::clear(void)
{
	m_size=QSize();
	m_maximum=static_cast<Image::Pixel>(-1);
	m_plane.clear();
	m_text.clear();
}

QString Image::text(QString key) const
{
	Text::ConstIterator I=m_text.find(key);

	return (I==m_text.end()) ? QString() : I.value();
}

bool Image::setText(QString key,QString value)
{
	if(value.isEmpty())
		return eraseText(key);

	Text::Iterator I=m_text.find(key);
	if(I!=m_text.end())
	{
		*I=value;
		return false;
	}

	m_text[key]=value;
	return true;
}

bool Image::eraseText(QString key)
{
	Text::Iterator I=m_text.find(key);

	if(I==m_text.end())
		return false;

	m_text.erase(I);
	return true;
}

void Image::setBusy(bool value)
{
	m_busy=value;
	setPercent(0);

	this->onSetBusy();
}

void Image::setPercent(double value)
{
	if(!m_busy)
		return;

	m_percent=std::min(100.,std::max(0.,value));
	this->onSetPercent();
}
