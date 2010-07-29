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
**  GNU General Public License if  more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Imaginable.  If not, see <http://www.gnu.org/licenses/>.
**
*************/


#include <QtCore/QFileInfo>
#include <QtGui/QImage>

#include "qt_io.hpp"

#include "dbus_plugin_qt_io_adaptor.h"


Q_EXPORT_PLUGIN2(qt_io,PluginQT_IO)


PluginQT_IO::PluginQT_IO(void)
	: QObject(NULL)
	, PluginInterface()
{
	new Qt_ioAdaptor(this);
}

uint PluginQT_IO::load(QString fileName,qulonglong Id)
{
	if (!QFileInfo(fileName).isFile())
	{
		complain(LOG_WARNING,"load",QString("Source image [\"%1\"] is not a file").arg(fileName));
		return CODE_NO_SRC_FILE;
	}

	bool busy;
	Image *dst=getOrComplain("load","destination image",Id,busy);
	if (!dst)
		return busy ? (Core::CODE_DST_IMAGE_BUSY) : (Core::CODE_NO_DST_IMAGE);


	message(LOG_INFO,"load",QString("Loading from file [%1]").arg(fileName),Id);

	doLongProcessing(dst,QtConcurrent::run(boost::bind(&PluginQT_IO::do_load,this,fileName,Id,dst)));

	return Core::CODE_OK;
}

void PluginQT_IO::do_load(QString fileName,qulonglong Id,Image *dst)
{
	QImage src;
	if (!src.load(fileName))
	{
		complain(LOG_WARNING,"load",QString("Cannot load source image [\"%1\"]").arg(fileName));
		m_lastErrorCodes[Id]=CODE_INVALID_SRC_FILE;
		return;
	}

	src=src.convertToFormat(src.hasAlphaChannel() ? (QImage::Format_ARGB32) : (QImage::Format_RGB32));


	dst->clear();

	dst->setOffset(src.offset());
	dst->setSize(QPoint(src.width(),src.height()));
	foreach (QString key,src.textKeys())
		dst->setText(key,src.text(key));

	dst->addPlane(Image::PLANE_RED);
	dst->addPlane(Image::PLANE_GREEN);
	dst->addPlane(Image::PLANE_BLUE);
	bool alpha=src.hasAlphaChannel();
	if (alpha)
		dst->addPlane(Image::PLANE_ALPHA);

	Image::Pixel *dst_red  =dst->plane(Image::PLANE_RED);
	Image::Pixel *dst_green=dst->plane(Image::PLANE_GREEN);
	Image::Pixel *dst_blue =dst->plane(Image::PLANE_BLUE);
	const uchar *src_bits=src.bits();

	const QPoint &sz=dst->size();
	for (int y=0; y<sz.y(); ++y)
	{
		int yo=y*sz.x();
		for (int x=0; x<sz.x(); ++x)
		{
			int sxo=(yo+x)*4;
			dst_blue [yo+x]=Image::scaleUp(src_bits[sxo  ]);
			dst_green[yo+x]=Image::scaleUp(src_bits[sxo+1]);
			dst_red  [yo+x]=Image::scaleUp(src_bits[sxo+2]);
		}
	}
	if (alpha)
	{
		Image::Pixel *dst_alpha=dst->plane(Image::PLANE_ALPHA);

		for (int y=0; y<sz.y(); ++y)
		{
			int yo=y*sz.x();
			for (int x=0; x<sz.y(); ++x)
				dst_alpha[yo+x]=Image::scaleUp(src_bits[(yo+x)*4+3]);
		}
	}


	message(LOG_INFO,"load",QString("Loaded from file [%1]").arg(fileName),Id);

	m_lastErrorCodes.remove(Id);
}

qulonglong PluginQT_IO::loadNew(QString fileName)
{
	fileName=QFileInfo(fileName).absoluteFilePath();

	qulonglong Id=m_core->createImage();
	if (!Id)
	{
		complain(LOG_CRIT,"load","Cannot create destination image",Id);
		return 0ULL;
	}

	if (load(fileName,Id))
	{
		m_core->deleteImage(Id);
		return 0ULL;
	}

	return Id;
}

uint PluginQT_IO::saveWithQuality(qulonglong Id,QString fileName,int quality)
{
	fileName=QFileInfo(fileName).absoluteFilePath();

	bool busy;
	Image *src=getOrComplain("save","source image",Id,busy);
	if (!src)
		return busy ? (Core::CODE_SRC_IMAGE_BUSY) : (Core::CODE_NO_SRC_IMAGE);

	if (src->colourSpace() != Image::SPACE_RGB)
	{
		complain(LOG_WARNING,"save","Not supported colour space",Id);
		return CODE_NOT_SUPPORTED_COLOURSPACE;
	}

	if (QFileInfo(fileName).exists())
	{
		complain(LOG_WARNING,"save",QString("Destination image file [\"%1\"] already exists").arg(fileName));
		return CODE_DST_FILE_EXIST;
	}

	if ( (quality < -1)
	||   (quality > 100) )
	{
		complain(LOG_WARNING,"save",QString("Invalid quality=%1").arg(quality));
		quality=-1;
	}


	message(LOG_INFO,"save",QString("Saving to file [%1]").arg(fileName),Id);

	doLongProcessing(src,QtConcurrent::run(boost::bind(&PluginQT_IO::do_save,this,Id,src,fileName,quality)));

	return Core::CODE_OK;
}

void PluginQT_IO::do_save(qulonglong Id,Image *src,QString fileName,int quality)
{
	bool alpha=src->hasAlpha();
	QImage dst(src->width(),src->height(),alpha ? (QImage::Format_ARGB32) : (QImage::Format_RGB32));

	dst.setOffset(src->offset());
	foreach (QString key,src->textKeys())
		dst.setText(key,src->text(key));

	Image::Pixel *src_red  =src->plane(Image::PLANE_RED);
	Image::Pixel *src_green=src->plane(Image::PLANE_GREEN);
	Image::Pixel *src_blue =src->plane(Image::PLANE_BLUE);
	uchar *dst_bits=dst.bits();

	const QPoint &sz=src->size();
	for (int y=0; y<sz.y(); ++y)
	{
		int yo=y*sz.x();
		for (int x=0; x<sz.x(); ++x)
		{
			int dxo=(yo+x)*4;
			dst_bits[dxo]  =Image::scaleDown(src_blue [yo+x]);
			dst_bits[dxo+1]=Image::scaleDown(src_green[yo+x]);
			dst_bits[dxo+2]=Image::scaleDown(src_red  [yo+x]);
		}
	}
	if (alpha)
	{
		Image::Pixel *src_alpha=src->plane(Image::PLANE_ALPHA);

		for (int y=0; y<sz.y(); ++y)
		{
			int yo=y*sz.x();
			for (int x=0; x<sz.x(); ++x)
				dst_bits[(yo+x)*4+3]=Image::scaleDown(src_alpha[yo+x]);
		}
	}


	if (!dst.save(fileName,NULL,quality))
	{
		complain(LOG_WARNING,"save",QString("Cannot save destination image [\"%1\"]").arg(fileName));
		m_lastErrorCodes[Id]=CODE_INVALID_DST_FILE;
		return;
	}

	if (quality<0)
		message(LOG_INFO,"save",QString("Saved to file [%1]")                .arg(fileName)             ,Id);
	else
		message(LOG_INFO,"save",QString("Saved to file [%1] with quality %2").arg(fileName).arg(quality),Id);

	m_lastErrorCodes.remove(Id);
}

uint PluginQT_IO::save(qulonglong Id,QString fileName)
{
	return saveWithQuality(Id,fileName,-1);
}

uint PluginQT_IO::lastErrorCode(qulonglong image)
{
	lastErrorCodes_t::ConstIterator I=m_lastErrorCodes.constFind(image);
	if (I == m_lastErrorCodes.constEnd())
		return Core::CODE_OK;
	return I.value();
}

QString PluginQT_IO::errorCodeToString(uint errorCode) const
{
	switch (errorCode)
	{
		#define CASE(ERR,STR) case CODE_##ERR: return STR ;
		CASE(NO_SRC_FILE     ,"No source file")
		CASE(INVALID_SRC_FILE,"Invalid source file")

		CASE(NOT_SUPPORTED_COLOURSPACE,"Not supported colour space")
		CASE(DST_FILE_EXIST           ,"Destination file exists")
		CASE(INVALID_DST_FILE         ,"Invalid destination file")
		#undef CASE
	}
	return m_core->errorCodeToString(errorCode);
}
