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


#include "io_qt.hpp"


namespace imaginable {

QImage_loader::QImage_loader(Image& image)
	: m_image(image)
{
}

QImage_loader::~QImage_loader()
{
}

QImage_saver::QImage_saver(const Image& image)
	: m_image(image)
{
}

QImage_saver::~QImage_saver()
{
}

void QImage_loader::load(const QImage& qimage)
{
	m_image.clear();

	QImage copy;
	if( (qimage.format()==QImage::Format_Mono)
	||  (qimage.format()==QImage::Format_MonoLSB) )
	{
		copy=qimage.convertToFormat(QImage::Format_Mono);
		//TODO: mono direct support
	}
	else
	{
		bool alpha=qimage.hasAlphaChannel();
		if(alpha)
		{
			copy=qimage.convertToFormat(QImage::Format_ARGB32);
		}
		else
		{
			copy=qimage.convertToFormat(QImage::Format_RGB32);
		}

		size_t mx=qimage.width();
		size_t my=qimage.height();
		m_image.setSize(mx,my);
		m_image.setMaximum(0xff);

		Image::pixel* planes[4]={NULL};
		if( (m_image.addPlane(Image::PLANE_RED))
		&&  (m_image.addPlane(Image::PLANE_GREEN))
		&&  (m_image.addPlane(Image::PLANE_BLUE)) )
		{
			planes[0]=m_image.plane(Image::PLANE_RED);
			planes[1]=m_image.plane(Image::PLANE_GREEN);
			planes[2]=m_image.plane(Image::PLANE_BLUE);
		}
		if(alpha)
			if(m_image.addPlane(Image::PLANE_ALPHA))
				planes[3]=m_image.plane(Image::PLANE_ALPHA);

		for(size_t y=0;y<my;++y)
		{
			size_t yo=y*mx;
			const QRgb* src=reinterpret_cast<const QRgb*>(qimage.scanLine(y));
			for(size_t x=0;x<mx;++x)
			{
				size_t xdo=yo+x;
				planes[0][xdo]=qRed  (src[x]);
				planes[1][xdo]=qGreen(src[x]);
				planes[2][xdo]=qBlue (src[x]);
				if(alpha)
					planes[3][xdo]=qAlpha(src[x]);
			}
		}

	}
}

void QImage_saver::save(QImage& qimage) const
{
	if(!m_image.hasData())
	{
		qimage=QImage();
		return;
	}

	switch(m_image.colourSpace())
	{
		case Image::IMAGE_MONO:
		//TODO: mono direct support
		break;
		case Image::IMAGE_RGB:
		{
			size_t mx=m_image.width();
			size_t my=m_image.height();
			if(m_image.hasTransparency())
			{
				qimage=QImage(mx,my,QImage::Format_ARGB32);

				size_t maximum=static_cast<size_t>(m_image.maximum());
				bool scale=(maximum!=0xff);

				const Image::pixel* planes[4];

				planes[0]=m_image.plane(Image::PLANE_RED);
				planes[1]=m_image.plane(Image::PLANE_GREEN);
				planes[2]=m_image.plane(Image::PLANE_BLUE);
				planes[3]=m_image.plane(Image::PLANE_ALPHA);

				for(size_t y=0;y<my;++y)
				{
					size_t yo=y*mx;
					QRgb* src=reinterpret_cast<QRgb*>(qimage.scanLine(y));
					for(size_t x=0;x<mx;++x)
					{
						size_t yxo=yo+x;
						if(scale)
							src[x]=qRgba(
								static_cast<size_t>(planes[0][yxo])*0xff/maximum,
								static_cast<size_t>(planes[1][yxo])*0xff/maximum,
								static_cast<size_t>(planes[2][yxo])*0xff/maximum,
								static_cast<size_t>(planes[3][yxo])*0xff/maximum );
						else
							src[x]=qRgba(planes[0][yxo],planes[1][yxo],planes[2][yxo],planes[3][yxo]);
					}
				}
			}
			else
			{
				qimage=QImage(mx,my,QImage::Format_RGB32);

				size_t maximum=static_cast<size_t>(m_image.maximum());
				bool scale=(maximum!=0xff);

				const Image::pixel* planes[3];

				planes[0]=m_image.plane(Image::PLANE_RED);
				planes[1]=m_image.plane(Image::PLANE_GREEN);
				planes[2]=m_image.plane(Image::PLANE_BLUE);

				for(size_t y=0;y<my;++y)
				{
					size_t yo=y*mx;
					QRgb* src=reinterpret_cast<QRgb*>(qimage.scanLine(y));
					for(size_t x=0;x<mx;++x)
					{
						size_t yxo=yo+x;
						if(scale)
							src[x]=qRgb(
								static_cast<size_t>(planes[0][yxo])*0xff/maximum,
								static_cast<size_t>(planes[1][yxo])*0xff/maximum,
								static_cast<size_t>(planes[2][yxo])*0xff/maximum );
						else
							src[x]=qRgb(planes[0][yxo],planes[1][yxo],planes[2][yxo]);
					}
				}
			}

		}
		break;
		default:
		//NOT SUPPORTED
		;
	}
}

}
