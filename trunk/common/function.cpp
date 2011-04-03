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

#include <function.hpp>


ArgumentDescription& ArgumentDescription::operator () (QString key,QVariant::Type type,bool array)
{
	ValueDescription value;
	value.type = type;
	value.array = array;
	insert(key,value);
	return *this;
}

int ArgumentDescription::compare (const ArgumentDescription& right) const
{
	ArgumentDescription::ConstIterator L=constBegin();
	ArgumentDescription::ConstIterator R=right.constBegin();
	for ( ; (L != constEnd()) || (R != right.constEnd()); ++L,++R )
	{
		if (L == constEnd())
			return -1;
		if (R == right.constEnd())
			return 1;

		int cmp=L.key().compare(R.key());
		if (cmp)
			return cmp;

		cmp=static_cast<int>(L.value().type) - static_cast<int>(R.value().type);
		if (cmp)
			return cmp;

		cmp=static_cast<int>(L.value().array) - static_cast<int>(R.value().array);
		if (cmp)
			return cmp;
	}
	return 0;
}


bool FunctionDescription::name_is_valid (QString name)
{
	if (!name.contains(QRegExp("^[a-zA-Z]")))
		return false;
	if (name.contains(QRegExp("[^a-zA-Z0-9_]")))
		return false;
	return true;
}

bool FunctionDescription::value_is_valid (QVariant::Type type)
{
	switch (type)
	{
	case QVariant::Bool:
	case QVariant::Int:
	case QVariant::UInt:
	case QVariant::LongLong:
	case QVariant::ULongLong:
	case QVariant::Double:
	case QVariant::Char:
	//case QVariant::Map:
	//case QVariant::List:
	case QVariant::String:
	//case QVariant::StringList:
	//case QVariant::ByteArray:
	//case QVariant::BitArray:
	//case QVariant::Date:
	//case QVariant::Time:
	case QVariant::DateTime:
	//case QVariant::Url:
	//case QVariant::Locale:
	//case QVariant::Rect:
	//case QVariant::RectF:
	//case QVariant::Size:
	//case QVariant::SizeF:
	case QVariant::Line:
	case QVariant::LineF:
	case QVariant::Point:
	case QVariant::PointF:
	//case QVariant::RegExp:
	//case QVariant::Hash:
	//case QVariant::EasingCurve:
		return true;
	default:
		return false;
	}
}

bool FunctionDescription::is_valid (void)
{
	if (!name_is_valid(m_name))
		return false;
	for (ArgumentDescription::ConstIterator I=m_input.constBegin(); I!=m_input.constEnd(); ++I)
	{
		if (!name_is_valid(I.key()))
			return false;
		if (!value_is_valid(I.value().type))
			return false;

	}
	for (ArgumentDescription::ConstIterator I=m_output.constBegin(); I!=m_output.constEnd(); ++I)
	{
		if (!name_is_valid(I.key()))
			return false;
		if (!value_is_valid(I.value().type))
			return false;
	}
	return true;
}

int FunctionDescription::compare (const struct FunctionDescription& right) const
{
	int cmp = m_name.compare(right.m_name);
	if (cmp)
		return cmp;

	cmp = m_input.compare(right.m_input);
	if (cmp)
		return cmp;

	return m_output.compare(right.m_output);
}



QString FunctionDescription::interface_toString (void) const
{
	QString ret=m_name + " (";
	for (ArgumentDescription::ConstIterator I=m_input.constBegin(); I!=m_input.constEnd(); ++I)
	{
		if (I != m_input.constBegin())
			ret += ",";
		ret += QString(" ") + QVariant::typeToName(I.value().type) + (I.value().array ? "[] " : " ") + I.key();
	}
	ret += " )";
	return ret;
}

QString FunctionDescription::toString (void) const
{
	QString ret=interface_toString() + " => (";
	for (ArgumentDescription::ConstIterator I=m_output.constBegin(); I!=m_output.constEnd(); ++I)
	{
		if (I != m_output.constBegin())
			ret += ",";
		ret += QString(" ") + QVariant::typeToName(I.value().type) + (I.value().array ? "[] " : " ") + I.key();
	}
	ret += " )";
	return ret;
}

uint qHash(const FunctionDescription& descr)
{ return qHash(descr.interface_toString()); }


bool Arguments::match (const ArgumentDescription& descr) const
{
	ArgumentDescription::ConstIterator D=descr.constBegin();
	Arguments::ConstIterator R=constBegin();
	for ( ; (R != constEnd()) || (D != descr.constEnd()); ++R, ++D )
	{
		if (D == descr.constEnd())
			return false;
		if (R == constEnd())
			return false;

		if (D.key() != R.key())
			return false;

		if (D.value().array)
		{
			if (R.value().type() != QVariant::List)
				return false;
			foreach (QVariant element, R.value().toList())
				if (D.value().type != element.type())
					return false;
		}
		else
		{
			if (D.value().type != R.value().type())
				return false;
		}
	}
	return true;
}
