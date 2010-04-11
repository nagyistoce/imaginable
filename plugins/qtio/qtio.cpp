/*************
**
** Project:      Imaginable
** File info:    $Id$
** Author:       Copyright (C) 2009,2010 Kuzma Shapran <Kuzma.Shapran@gmail.com>
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


#include "qtio.hpp"
#include "dbus_plugin_qtio_adaptor.h"

#include <QtGui/QImage>


Q_EXPORT_PLUGIN2(qtio,PluginQTIO)


PluginQTIO::PluginQTIO(void)
	: QObject(NULL)
	, PluginInterface()
{
}

bool PluginQTIO::init(Root* root)
{
	m_root=root;

	if(!QDBusConnection::sessionBus().registerObject(name(),new QtioAdaptor(this),QDBusConnection::ExportNonScriptableContents))
	{
		COMPLAIN2(LOG_ALERT,"Cannot register D-Bus object interface");
		return false;
	}
	return true;
}

uint PluginQTIO::loadTo(QString fileName,qulonglong Id)
{
	if(!QFileInfo(fileName).exists())
	{
		COMPLAIN2(LOG_ERR,"Non-existent source image file");
		return Root::NO_FILE;
	}
	if(!QFileInfo(fileName).isFile())
	{
		COMPLAIN2(LOG_WARNING,"Source image is not a file");
		return Root::NO_FILE;
	}

	QImage src;
	if(!src.load(fileName))
	{
		COMPLAIN2(LOG_WARNING,"Cannot load source image");
		return Root::INVALID_FILE;
	}

	uint ret;
	Image* dst=GET_OR_COMPLAIN("destination image",Id,ret);
	if(!dst)
		return ret;


	src=src.convertToFormat(src.hasAlphaChannel()?(QImage::Format_ARGB32):(QImage::Format_RGB32));


	dst->clear();

	dst->setOffset(src.offset());
	dst->setSize(src.size());
	foreach(QString key,src.textKeys())
		dst->setText(key,src.text(key));

	dst->addPlane(Image::PLANE_RED);
	dst->addPlane(Image::PLANE_GREEN);
	dst->addPlane(Image::PLANE_BLUE);
	bool alpha=src.hasAlphaChannel();
	if(alpha)
		dst->addPlane(Image::PLANE_ALPHA);

	Image::Pixel* dst_red  =dst->plane(Image::PLANE_RED);
	Image::Pixel* dst_green=dst->plane(Image::PLANE_GREEN);
	Image::Pixel* dst_blue =dst->plane(Image::PLANE_BLUE);
	const uchar* src_bits=src.bits();

	const QSize& sz=dst->size();
	for(int y=0;y<sz.height();++y)
	{
		int yo=y*sz.width();
		for(int x=0;x<sz.width();++x)
		{
			int sxo=(yo+x)*4;
			dst_blue [yo+x]=Image::scaleUp(src_bits[sxo  ]);
			dst_green[yo+x]=Image::scaleUp(src_bits[sxo+1]);
			dst_red  [yo+x]=Image::scaleUp(src_bits[sxo+2]);
		}
	}
	if(alpha)
	{
		Image::Pixel* dst_alpha=dst->plane(Image::PLANE_ALPHA);

		for(int y=0;y<sz.height();++y)
		{
			int yo=y*sz.width();
			for(int x=0;x<sz.width();++x)
				dst_alpha[yo+x]=Image::scaleUp(src_bits[(yo+x)*4+3]);
		}
	}


	MESSAGE(LOG_INFO,QString("Loaded from file[%1]").arg(fileName),Id);

	return Root::OK;
}

qulonglong PluginQTIO::load(QString fileName)
{
	fileName=QFileInfo(fileName).absoluteFilePath();

	qulonglong Id=m_root->createImage();
	if(!Id)
	{
		COMPLAIN(LOG_CRIT,"Cannot create destination image",Id);
		return 0ULL;
	}

	if(loadTo(fileName,Id))
	{
		m_root->deleteImage(Id);
		return 0ULL;
	}

	return Id;
}

uint PluginQTIO::saveWithQuality(qulonglong Id,QString fileName,int quality)
{
	fileName=QFileInfo(fileName).absoluteFilePath();

	uint ret;
	Image* src=GET_OR_COMPLAIN("source image",Id,ret);
	if(!src)
		return ret;

	if(src->colourSpace()!=Image::SPACE_RGB)
	{
		COMPLAIN(LOG_WARNING,"Not supported colour space",Id);
		return Root::INVALID_COLOURSPACE;
	}

	if(QFileInfo(fileName).exists())
	{
		COMPLAIN2(LOG_WARNING,"Destination image file already exists");
		return Root::FILE_EXIST;
	}

	if( (quality<-1)
	||  (quality>100) )
	{
		COMPLAIN2(LOG_WARNING,QString("Invalid quality=%1").arg(quality));
		quality=-1;
	}


	bool alpha=src->hasAlpha();
	QImage dst(src->width(),src->height(),alpha?(QImage::Format_ARGB32):(QImage::Format_RGB32));

	dst.setOffset(src->offset());
	foreach(QString key,src->textKeys())
		dst.setText(key,src->text(key));

	Image::Pixel* src_red  =src->plane(Image::PLANE_RED);
	Image::Pixel* src_green=src->plane(Image::PLANE_GREEN);
	Image::Pixel* src_blue =src->plane(Image::PLANE_BLUE);
	uchar* dst_bits=dst.bits();

	const QSize& sz=src->size();
	for(int y=0;y<sz.height();++y)
	{
		int yo=y*sz.width();
		for(int x=0;x<sz.width();++x)
		{
			int dxo=(yo+x)*4;
			dst_bits[dxo]  =Image::scaleDown(src_blue [yo+x]);
			dst_bits[dxo+1]=Image::scaleDown(src_green[yo+x]);
			dst_bits[dxo+2]=Image::scaleDown(src_red  [yo+x]);
		}
	}
	if(alpha)
	{
		Image::Pixel* src_alpha=src->plane(Image::PLANE_ALPHA);

		for(int y=0;y<sz.height();++y)
		{
			int yo=y*sz.width();
			for(int x=0;x<sz.width();++x)
				dst_bits[(yo+x)*4+3]=Image::scaleDown(src_alpha[yo+x]);
		}
	}


	if(!dst.save(fileName,NULL,quality))
	{
		COMPLAIN2(LOG_WARNING,"Cannot save destination image");
		return Root::INVALID_FILE;
	}

	if(quality<0)
		MESSAGE(LOG_INFO,QString("Saved to file[%1] with default quality").arg(fileName),Id);
	else
		MESSAGE(LOG_INFO,QString("Saved to file[%1] with quality %2").arg(fileName).arg(quality),Id);

	return Root::OK;
}

uint PluginQTIO::save(qulonglong Id,QString fileName)
{
	return saveWithQuality(Id,fileName,-1);
}
