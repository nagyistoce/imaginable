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


#include "copy.hpp"
#include "dbus_plugin_copy_adaptor.h"



Q_EXPORT_PLUGIN2(copy,PluginCopy)


PluginCopy::PluginCopy(void)
	: QObject(NULL)
	, PluginInterface()
{
	new CopyAdaptor(this);
}

uint PluginCopy::copyTo(qulonglong from,qulonglong to)
{
	if(from==to)
	{
		complain(LOG_WARNING,__FUNCTION__,"Source and destination images must differ",from);
		return CODE_IMAGES_DONT_DIFFER;
	}

	bool busy;
	Image* src=getOrComplain("copy","source image",from,busy);
	if(!src)
		return busy?(Core::CODE_SRC_IMAGE_BUSY):(Core::CODE_NO_SRC_IMAGE);

	Image* dst=getOrComplain("copy","destination image",to,busy);
	if(!dst)
		return busy?(Core::CODE_DST_IMAGE_BUSY):(Core::CODE_NO_DST_IMAGE);


	message(LOG_INFO,"copy",QString("Copying from image [%1]").arg(from),to);

	doLongProcessing(QList<Image*>()<<src<<dst,QtConcurrent::run(boost::bind(&PluginCopy::do_copy,this,from,src,to,dst)));

	return Core::CODE_OK;
}

qulonglong PluginCopy::copyNew(qulonglong from)
{
	qulonglong to=m_core->createImage();
	if(!to)
	{
		complain(LOG_CRIT,"copy","Cannot create destination image",to);
		return 0ULL;
	}

	if(copyTo(from,to))
	{
		m_core->deleteImage(to);
		return 0ULL;
	}

	return to;
}

void PluginCopy::do_copy(qulonglong srcId,Image* src,qulonglong dstId,Image* dst)
{
	dst->clear();
	dst->setSize(src->size());

	int length=src->area()*sizeof(Image::Pixel);
	foreach(Image::ColourPlane colourPlane,src->planesList())
	{
		dst->addPlane(colourPlane);
		memcpy(dst->plane(colourPlane),src->plane(colourPlane),length);
		if(src->planeHasName(colourPlane))
			dst->setPlaneName(colourPlane,src->planeName(colourPlane));
	}

	foreach(QString key,src->textKeysList())
		dst->setText(key,src->text(key));

	message(LOG_INFO,"copy",QString("Copied from image [%1]").arg(srcId),dstId);
}

QString PluginCopy::errorCodeToString(uint errorCode) const
{
	switch(errorCode)
	{
		#define CASE(ERR,STR) case CODE_##ERR: return STR ;
		CASE(IMAGES_DONT_DIFFER,"Images do not differ")
		#undef CASE
	}
	return m_core->errorCodeToString(errorCode);
}
