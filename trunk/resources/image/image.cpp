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


#include "image.hpp"


Q_EXPORT_PLUGIN2(image,ResourceImage)


static const char RESOURCE_TYPE[] = "image";
static QSet<FunctionDescription> FUNCTIONS;
static FunctionDescription CREATE__W_H(
	"create",
	ArgumentDescription()
		(QString("width"),QVariant::UInt,false)
		(QString("height"),QVariant::UInt,false),
	ArgumentDescription()
		(QString("resource"),QVariant::String,false)
	);


ResourceImage::ResourceImage (void)
{
	FUNCTIONS.insert(CREATE__W_H);
}

ResourceImage::~ResourceImage ()
{
	FUNCTIONS.clear();
}

int ResourceImage::init (const Settings &)
{
	return 0;
}

QString ResourceImage::type (void) const
{
	return RESOURCE_TYPE;
}

QSet<FunctionDescription> ResourceImage::provides (void) const
{
	return FUNCTIONS;
}

QSharedPointer<CallResult> ResourceImage::call (QString name,const Arguments &input,Arguments &output)
{
	if (name == CREATE__W_H.name() && input.match(CREATE__W_H.input()))
	{
		return create(input["width"].toUInt(),input["height"].toUInt(),output);
	}
	return CallResult::method_not_provided();
}

QSharedPointer<CallResult> ResourceImage::create (uint /*width*/, uint /*height*/,Arguments &output)
{
	output["resource"]=QString();
	return CallResult::OK();
}
