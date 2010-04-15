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


#include "inb_io.hpp"
#include "dbus_plugin_inb_io_adaptor.h"

#include <boost/scoped_array.hpp>


Q_EXPORT_PLUGIN2(inb_io,PluginINB_IO)


PluginINB_IO::PluginINB_IO(void)
	: QObject(NULL)
	, PluginInterface()
{
}

bool PluginINB_IO::init(Root* root)
{
	m_root=root;

	if(!QDBusConnection::sessionBus().registerObject(name(),new Inb_ioAdaptor(this),QDBusConnection::ExportNonScriptableContents))
	{
		complain(LOG_ALERT,__FUNCTION__,"Cannot register D-Bus object interface");
		return false;
	}
	return true;
}

uint PluginINB_IO::load(QString fileName,qulonglong Id)
{
	if(!QFileInfo(fileName).isFile())
	{
		complain(LOG_WARNING,"load",QString("Source image [\"%1\"] is not a file").arg(fileName));
		return CODE_NO_SRC_FILE;
	}

	QFile src(fileName);
	if(!src.open(QIODevice::ReadOnly))
	{
		complain(LOG_WARNING,"load",QString("Cannot open source image [\"%1\"]").arg(fileName));
		return CODE_ERROR_SRC_FILE;
	}

	bool busy;
	Image* dst=getOrComplain("load","destination image",Id,busy);
	if(!dst)
		return busy?(Root::CODE_DST_IMAGE_BUSY):(Root::CODE_NO_DST_IMAGE);


	QByteArray uncompressed;
	QBuffer buffer;
	QDataStream in;

	QMap<QString,QPair<QString,QStringList> > archives;
	archives["gz" ]=qMakePair<QString,QStringList>("gzip" ,QStringList() << "-c" << "-d");
	archives["bz2"]=qMakePair<QString,QStringList>("bzip2",QStringList() << "-c" << "-d");

	QMap<QString,QPair<QString,QStringList> >::ConstIterator A=archives.constFind(QFileInfo(fileName).suffix());
	if(A!=archives.constEnd())
	{
		QProcess arch;
		arch.start(A.value().first,A.value().second);
		if(!arch.waitForStarted())
		{
			complain(LOG_WARNING,"load",QString("Cannot unpack image [\"%1\"]").arg(fileName));
			return CODE_ARCHIVE_ERROR;
		}

		arch.write(src.readAll());
		arch.closeWriteChannel();

		if(!arch.waitForFinished())
		{
			complain(LOG_WARNING,"load",QString("Cannot unpack image [\"%1\"]").arg(fileName));
			return CODE_ARCHIVE_ERROR;
		}

		uncompressed=arch.readAll();

		buffer.setData(uncompressed);
		buffer.open(QIODevice::ReadOnly);
		in.setDevice(&buffer);
	}
	else
		in.setDevice(&src);


	{
		quint32 magic;
		in >> magic;
		if(magic != 0x494e4200)
		{
			complain(LOG_WARNING,"load",QString("Image file [\"%1\"] has invalid signature [%2]").arg(fileName).arg(magic,8,0x10,QChar('0')));
			return CODE_INVALID_SRC_FILE;
		}
	}
	{
		quint32 version;
		in >> version;
		if(version > 1)
		{
			complain(LOG_WARNING,"load",QString("Image file [\"%1\"] has unsupported version [%2]").arg(fileName).arg(version));
			return CODE_SRC_FILE_UNSUPPORTED_VERSION;
		}
	}

	dst->clear();
	{
		QPoint offset;
		in >> offset;
		dst->setOffset(offset);
	}
	{
		QSize size;
		in >> size;
		dst->setSize(size);
	}
	{
		quint32 textCount;
		in >> textCount;
		for(quint32 i=0;i<textCount;++i)
		{
			QString textKey;
			QString textValue;
			in >> textKey >> textValue;
			if(!dst->setText(textKey,textValue))
				message(LOG_WARNING,"load",QString("Cannot set text [\"%1\" => \"%2\"]").arg(textKey).arg(textValue),Id);
		}
	}
	{
		boost::scoped_array<Image::Pixel> planeData(new Image::Pixel[dst->area()]);
		quint32 planeCount;
		in >> planeCount;
		for(quint32 i=0;i<planeCount;++i)
		{
			qint32 colourPlane;
			bool hasName;
			in >> colourPlane >> hasName;
			if(hasName)
			{
				QString name;
				in >> name;
				dst->setPlaneName(colourPlane,name);
			}
			in.readRawData(reinterpret_cast<char*>(planeData.get()),dst->area()*sizeof(Image::Pixel));
			
			if(!dst->addPlane(colourPlane))
				message(LOG_WARNING,"load",QString("Cannot add colour plane [%1]").arg(colourPlane),Id);
			else
			{
				if(QSysInfo::ByteOrder == QSysInfo::LittleEndian)
					swab  (planeData.get(),dst->plane(colourPlane),dst->area()*sizeof(Image::Pixel));
				else
					memcpy(dst->plane(colourPlane),planeData.get(),dst->area()*sizeof(Image::Pixel));
			}
		}
	}


	message(LOG_INFO,"load",QString("Loaded from file [%1]").arg(fileName),Id);

	return Root::CODE_OK;
}

qulonglong PluginINB_IO::loadNew(QString fileName)
{
	fileName=QFileInfo(fileName).absoluteFilePath();

	qulonglong Id=m_root->createImage();
	if(!Id)
	{
		complain(LOG_CRIT,"load","Cannot create destination image",Id);
		return 0ULL;
	}

	if(load(fileName,Id))
	{
		m_root->deleteImage(Id);
		return 0ULL;
	}

	return Id;
}

uint PluginINB_IO::save(qulonglong Id,QString fileName)
{
	fileName=QFileInfo(fileName).absoluteFilePath();

	bool busy;
	Image* src=getOrComplain("save","source image",Id,busy);
	if(!src)
		return busy?(Root::CODE_SRC_IMAGE_BUSY):(Root::CODE_NO_SRC_IMAGE);

	if(QFileInfo(fileName).exists())
	{
		complain(LOG_WARNING,"save",QString("Destination image file [\"%1\"] already exists").arg(fileName));
		return CODE_DST_FILE_EXIST;
	}

	QByteArray uncompressed;
	QDataStream out(&uncompressed,QIODevice::WriteOnly);

	{
		quint32 magic=0x494e4200;
		out << magic;
	}
	{
		quint32 version=1;
		out << version;
	}

	out << src->offset();
	out << src->size();
	{
		out << static_cast<quint32>(src->textKeysList().size());
		foreach(QString textKey,src->textKeysList())
			out << textKey << src->text(textKey);
	}
	{
		boost::scoped_array<Image::Pixel> planeData(new Image::Pixel[src->area()]);

		out << static_cast<quint32>(src->planesCount());
		foreach(int colourPlane,src->planesList())
		{
			out << colourPlane << src->planeHasName(colourPlane);
			if(src->planeHasName(colourPlane))
				out << src->planeName(colourPlane);

			if(QSysInfo::ByteOrder == QSysInfo::LittleEndian)
				swab  (src->plane(colourPlane),planeData.get(),src->area()*sizeof(Image::Pixel));
			else
				memcpy(planeData.get(),src->plane(colourPlane),src->area()*sizeof(Image::Pixel));

			out.writeRawData(reinterpret_cast<char*>(planeData.get()),src->area()*sizeof(Image::Pixel));
		}
	}


	QMap<QString,QPair<QString,QStringList> > archives;
	archives["gz" ]=qMakePair<QString,QStringList>("gzip" ,QStringList() << "-c");
	archives["bz2"]=qMakePair<QString,QStringList>("bzip2",QStringList() << "-c");

	QByteArray output;

	QMap<QString,QPair<QString,QStringList> >::ConstIterator A=archives.constFind(QFileInfo(fileName).suffix());
	if(A!=archives.constEnd())
	{
		QProcess arch;
		arch.start(A.value().first,A.value().second);
		if(!arch.waitForStarted())
		{
			complain(LOG_WARNING,"save",QString("Cannot pack image [\"%1\"]").arg(fileName));
			return CODE_ARCHIVE_ERROR;
		}

		arch.write(uncompressed);
		arch.closeWriteChannel();

		if(!arch.waitForFinished())
		{
			complain(LOG_WARNING,"save",QString("Cannot pack image [\"%1\"]").arg(fileName));
			return CODE_ARCHIVE_ERROR;
		}

		output=arch.readAll();
	}
	else
		output=uncompressed;


	QFile dst(fileName);
	if(!dst.open(QIODevice::WriteOnly))
	{
		complain(LOG_WARNING,"save",QString("Cannot open destination image [\"%1\"]").arg(fileName));
		return CODE_INVALID_DST_FILE;
	}
	dst.write(output);


	message(LOG_INFO,"save",QString("Saved to file [%1]").arg(fileName),Id);

	return Root::CODE_OK;
}

QString PluginINB_IO::errorCodeToString(uint errorCode) const
{
	switch(errorCode)
	{
		#define CASE(ERR,STR) case CODE_##ERR: return STR ;
		CASE(NO_SRC_FILE     ,"No source file")
		CASE(ERROR_SRC_FILE  ,"Source file error")
		CASE(INVALID_SRC_FILE,"Invalid source file")
		CASE(ARCHIVE_ERROR   ,"Archive error")
		CASE(SRC_FILE_UNSUPPORTED_VERSION,"Source file has unsupported version")

		CASE(DST_FILE_EXIST  ,"Destination file exists")
		CASE(INVALID_DST_FILE,"Invalid destination file")
		#undef CASE
	}
	return m_root->errorCodeToString(errorCode);
}
