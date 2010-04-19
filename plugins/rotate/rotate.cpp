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

        doLongProcessing(QList<Image*>()<<src<<dst,QtConcurrent::run(boost::bind(&PluginRotate::do_rotate,this,Id,src,ret,dst,radian)));

        m_lastErrorCodes.remove(Id);
        return ret;
}

namespace {

template<typename T>
T sign(const T& value)
{
        if(value>static_cast<T>(0.))
                return static_cast<T>(1.);
        if(value<static_cast<T>(0.))
                return static_cast<T>(-1.);
        return static_cast<T>(0.);
}

}

void PluginRotate::do_rotate(qulonglong srcId,Image* src,qulonglong dstId,Image* dst,double radian)
{
        connect(this,SIGNAL(setPercent(double)),src,SLOT(setPercent(double)));
	connect(this,SIGNAL(setPercent(double)),dst,SLOT(setPercent(double)));



        int old_width =src->width();
        int old_height=src->height();

        point corner[4];
        //  0     1
        //   +---+
        //   |   |
        //   +---+
        //  2     3
        point offset=point(src->offset().x(),src->offset().y());
        point center=point(static_cast<double>(src->width())/2.,static_cast<double>(src->height())/2.);
        corner[0]=point(         -center.x,          -center.y);
        corner[1]=point(old_width-center.x,          -center.y);
        corner[2]=point(         -center.x,old_height-center.y);
        corner[3]=point(old_width-center.x,old_height-center.y);
        for(size_t i=0;i<4;++i)
        {
                point& p=corner[i];
                p.to_polar().f+=radian;
                p.to_rect();
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
        point new_offset=point()-topleft;


        src->removePlane(Image::PLANE__INTERNAL);

        dst->setOffset(QPoint(static_cast<int>(new_offset.x),static_cast<int>(new_offset.y)));
        dst->setSize(QSize(new_width,new_height));
        foreach(Image::ColourPlane colourPlane,src->planesList())
                dst->addPlane(colourPlane);
        dst->addPlane(Image::PLANE__INTERNAL);


        int xo[4]={0,1,0,1};
        int yo[4]={0,0,1,1};
        int xyo[4];
        for(size_t i=0;i<4;++i)
                xyo[i]=xo[i]+yo[i]*old_width;


        for(int y=0;y<new_height;++y)
        {
                emit setPercent(static_cast<double>(y)*100./static_cast<double>(new_height));

                int dst_yo=y*new_width;
                for(int x=0;x<new_width;++x)
                {
                        point p(static_cast<double>(x)-new_center.x+.5,static_cast<double>(y)-new_center.y+.5);
                        p.to_polar().f-=radian;
                        p.to_rect();
                        int s_x=static_cast<int>((p.x+center.x-.5)*16.);
                        int s_y=static_cast<int>((p.y+center.y-.5)*16.);
                        int src_x=(s_x<0) ? ((s_x-0x0f)/0x10) : (s_x/0x10);
                        int src_y=(s_y<0) ? ((s_y-0x0f)/0x10) : (s_y/0x10);

                        unsigned k[4];
                        {
                                int fr_x=((s_x%0x10)+0x10)%0x10;
                                int fr_y=((s_y%0x10)+0x10)%0x10;

                                k[0]=(0x10-fr_x)*(0x10-fr_y);
                                k[1]=      fr_x *(0x10-fr_y);
                                k[2]=(0x10-fr_x)*      fr_y ;
                                k[3]=      fr_x *      fr_y ;
                        }

                        int src_yxo=src_y*old_width+src_x;


                        unsigned k_eff[4]={0};
                        Image::Pixel* dst_plane=dst->plane(Image::PLANE__INTERNAL);
                        if( (src_x>=0)
                        &&  (src_x+1<old_width )
                        &&  (src_y>=0)
                        &&  (src_y+1<old_height) )
                        {
                                for(size_t i=0;i<4;++i)
                                        k_eff[i]=k[i];
                        }
                        else
                        {
                                if( (src_x+1>=0)
                                &&  (src_x<old_width )
                                &&  (src_y+1>=0)
                                &&  (src_y<old_height) )
                                        for(size_t i=0;i<4;++i)
                                                if( (src_x+xo[i]>=0)
                                                &&  (src_x+xo[i]<old_width )
                                                &&  (src_y+yo[i]>=0)
                                                &&  (src_y+yo[i]<old_height) )
                                                        k_eff[i]=k[i];
                        }
                        unsigned K=0;
                        for(size_t i=0;i<4;++i)
                                K+=k_eff[i];
                        dst_plane[dst_yo+x]=( K ? ((K*0x100)-1) : 0 );

                        foreach(Image::ColourPlane colourPlane,src->planesList())
                        {
                                dst_plane=dst->plane(colourPlane);

                                unsigned v=0;
                                if(K)
                                {
                                        const Image::Pixel* src_plane=src->plane(colourPlane);
                                        for(size_t i=0;i<4;++i)
                                                if(k_eff[i])
                                                        v+=k_eff[i]*static_cast<unsigned>(src_plane[src_yxo+xyo[i]]);
                                }
                                switch(K)
                                {
                                        case 0:
                                                dst_plane[dst_yo+x]=0;
                                        break;
                                        default:
                                                dst_plane[dst_yo+x]=static_cast<Image::Pixel>(v/K);
                                        break;
                                        case 0x100:
                                                dst_plane[dst_yo+x]=static_cast<Image::Pixel>(v/0x100);
                                        break;
                                }
                        }
                }
        }


        if(dst->hasAlpha())
        {
                const Image::Pixel* src_plane=dst->plane(Image::PLANE__INTERNAL);
                const Image::Pixel* alpha_plane=src->plane(Image::PLANE_ALPHA);
                Image::Pixel* dst_plane=dst->plane(Image::PLANE_ALPHA);
                for(int y=0;y<new_height;++y)
                {
                        int dst_yo=y*new_width;
                        for(int x=0;x<new_width;++x)
                        {
                                int dst_yxo=dst_yo+x;
                                if(src_plane[dst_yxo]==0xffff)
                                        dst_plane[dst_yxo]=alpha_plane[dst_yxo];
                                else if(!src_plane[dst_yxo])
                                        dst_plane[dst_yxo]=0;
                                else
                                        dst_plane[dst_yxo]=static_cast<Image::Pixel>(
                                                (static_cast<unsigned>(alpha_plane[dst_yxo])
                                                *static_cast<unsigned>(  src_plane[dst_yxo]))/0xffff );
                        }
                }
                dst->removePlane(Image::PLANE__INTERNAL);
        }
        else
                dst->movePlane(Image::PLANE__INTERNAL,Image::PLANE_ALPHA);


        foreach(QString key,src->textKeysList())
                dst->setText(key,src->text(key));


        disconnect(src,SLOT(setPercent(double)));
        disconnect(dst,SLOT(setPercent(double)));

        message(LOG_INFO,"rotate",QString("Rotated to [%1] on angle [%2]").arg(dstId).arg(radian),srcId);
}

QString PluginRotate::errorCodeToString(uint errorCode) const
{
	return m_root->errorCodeToString(errorCode);
}
