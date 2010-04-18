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


#include "rotate.hpp"
#include "dbus_plugin_rotate_adaptor.h"

#include "point.hpp"

#include <boost/bind.hpp>


Q_EXPORT_PLUGIN2(rotate,PluginRotate)


PluginRotate::PluginRotate(void)
	: QObject(NULL)
	, PluginInterface()
{
}

bool PluginRotate::init(Root* root)
{
	m_root=root;

	if(!QDBusConnection::sessionBus().registerObject(name(),new RotateAdaptor(this),QDBusConnection::ExportNonScriptableContents))
	{
		complain(LOG_ALERT,__FUNCTION__,"Cannot register D-Bus object interface");
		return false;
	}

	return true;
}

qulonglong PluginRotate::rotate(qulonglong Id,double radian)
{
	bool busy;
	Image* src=getOrComplain(__FUNCTION__,"source image",Id,busy);
	if(!src)
	{
		m_lastErrorCodes[Id]=busy?(Root::CODE_SRC_IMAGE_BUSY):(Root::CODE_NO_SRC_IMAGE);
		return 0ULL;
	}
	
	qulonglong ret=m_root->createImage();
	if(!ret)
	{
		complain(LOG_CRIT,"load","Cannot create destination image",ret);
		m_lastErrorCodes[Id]=Root::CODE_NO_DST_IMAGE;
		return 0ULL;
	}

	Image* dst=getOrComplain(__FUNCTION__,"destination image",ret,busy);
	if(!dst)
	{
		m_lastErrorCodes[Id]=busy?(Root::CODE_DST_IMAGE_BUSY):(Root::CODE_NO_DST_IMAGE);
		return 0ULL;
	}
	


	message(LOG_INFO,__FUNCTION__,QString("Rotating to [%1] on angle [%2]").arg(ret).arg(radian),Id);

	doLongProcessing(QList<Image*>()<<src<<dst,QtConcurrent::run(boost::bind(&PluginRotate::do_rotate,this,Id,img,ret,dst,radian)));

	m_lastErrorCodes.remove(Id);
	return ret;
}

void PluginRotate::do_rotate(qulonglong srcId,Image* src,qulonglong dstId,Image* dst,double radian)
{
	connect(this,SIGNAL(setPercent(double)),src,SLOT(setPercent(double)));
	connect(this,SIGNAL(setPercent(double)),dst,SLOT(setPercent(double)));



	point center;
	point corner[4];
	//  0     1
	//   +---+
	//   |   |
	//   +---+
	//  2     3
	center=point(static_cast<double>(img.width())/2.,static_cast<double>(img.height())/2.);
	corner[0]=point(           -center.x,            -center.y);
	corner[1]=point(img.width()-center.x,            -center.y);
	corner[2]=point(           -center.x,img.height()-center.y);
	corner[3]=point(img.width()-center.x,img.height()-center.y);
	for(size_t i=0;i<4;++i)
	{
		point& p=corner[i];
		p=p.polar();
		p.f+=radian;
		p=p.rect();
	}
	point topleft(
		std::min(std::min(corner[0].x,corner[1].x),std::min(corner[2].x,corner[3].x)),
		std::min(std::min(corner[0].y,corner[1].y),std::min(corner[2].y,corner[3].y)) );
	point bottomright(
		std::max(std::max(corner[0].x,corner[1].x),std::max(corner[2].x,corner[3].x)),
		std::max(std::max(corner[0].y,corner[1].y),std::max(corner[2].y,corner[3].y)) );

	int new_width =static_cast<int>(ceil(bottomright.x+.5*sign(bottomright.x))-ceil(topleft.x+.5*sign(topleft.x)));
	int new_height=static_cast<int>(ceil(bottomright.y+.5*sign(bottomright.y))-ceil(topleft.y+.5*sign(topleft.y)));

	point new_center=point(static_cast<double>(new_width)/2.,static_cast<double>(new_height)/2.);



	disconnect(src,SLOT(setPercent(double)));
	disconnect(dst,SLOT(setPercent(double)));

	message(LOG_INFO,"rotate",QString("Rotated to [%1] on angle [%2]").arg(dstId),arg(radian),srcId);
}

uint PluginRotate::lastErrorCode(qulonglong image)
{
	lastErrorCodes_t::ConstIterator I=m_lastErrorCodes.constFind(image);
	if(I==m_lastErrorCodes.constEnd())
		return Root::CODE_OK;
	return I.value();
}

QString PluginRotate::errorCodeToString(uint errorCode) const
{
	return m_root->errorCodeToString(errorCode);
}
