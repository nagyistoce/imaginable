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

#ifndef IMAGINABLE__MODULE_INTERFACE__INCLUDED
#define IMAGINABLE__MODULE_INTERFACE__INCLUDED


#include <QtCore/QtPlugin>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <QtCore/QRegExp>
#include <QtCore/QSharedPointer>


typedef struct ValueDescription
{
	QVariant::Type type;
	bool array;
} ValueDescription;

class ArgumentDescription : public QMap<QString,ValueDescription>
{
public:
	int compare (const ArgumentDescription& right) const
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

	bool operator < (const ArgumentDescription& right) const
	{ return compare(right) < 0; }
};

typedef struct FunctionDescription
{
	QString name;
	ArgumentDescription input;
	ArgumentDescription output;

	bool name_is_valid(QString name)
	{
		if (!name.contains(QRegExp("^[a-zA-Z]")))
			return false;
		if (name.contains(QRegExp("[^a-zA-Z0-9_]")))
			return false;
		return true;
	}

	bool value_is_valid(QVariant::Type type)
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
//		case QVariant::Map:
//		case QVariant::List:
		case QVariant::String:
//		case QVariant::StringList:
//		case QVariant::ByteArray:
//		case QVariant::BitArray:
//		case QVariant::Date:
//		case QVariant::Time:
		case QVariant::DateTime:
//		case QVariant::Url:
//		case QVariant::Locale:
//		case QVariant::Rect:
//		case QVariant::RectF:
//		case QVariant::Size:
//		case QVariant::SizeF:
		case QVariant::Line:
		case QVariant::LineF:
		case QVariant::Point:
		case QVariant::PointF:
//		case QVariant::RegExp:
//		case QVariant::Hash:
//		case QVariant::EasingCurve:
			return true;
		default:
			return false;
		}
	}

	bool is_valid(void)
	{
		if (!name_is_valid(name))
			return false;
		for (ArgumentDescription::ConstIterator I=input.constBegin(); I!=input.constEnd(); ++I)
		{
			if (!name_is_valid(I.key()))
				return false;
			if (!value_is_valid(I.value().type))
				return false;

		}
		for (ArgumentDescription::ConstIterator I=output.constBegin(); I!=output.constEnd(); ++I)
		{
			if (!name_is_valid(I.key()))
				return false;
			if (!value_is_valid(I.value().type))
				return false;
		}
		return true;
	}

	int compare (const struct FunctionDescription& right) const
	{
		int cmp = name.compare(right.name);
		if (cmp)
			return cmp;

		cmp = input.compare(right.input);
		if (cmp)
			return cmp;

		return output.compare(right.output);
	}

	bool operator < (const struct FunctionDescription& right) const
	{ return compare(right) < 0; }

	bool operator == (const struct FunctionDescription& right) const
	{ return compare(right) == 0; }


	QString interface_toString(void) const
	{
		QString ret=name + " (";
		for (ArgumentDescription::ConstIterator I=input.constBegin(); I!=input.constEnd(); ++I)
		{
			if (I != input.constBegin())
				ret += ",";
			ret += QString(" ") + QVariant::typeToName(I.value().type) + (I.value().array ? "[] " : " ") + I.key();
		}
		ret += " )";
		return ret;
	}

	QString toString(void) const
	{
		QString ret=interface_toString() + " => (";
		for (ArgumentDescription::ConstIterator I=output.constBegin(); I!=output.constEnd(); ++I)
		{
			if (I != output.constBegin())
				ret += ",";
			ret += QString(" ") + QVariant::typeToName(I.value().type) + (I.value().array ? "[] " : " ") + I.key();
		}
		ret += " )";
		return ret;
	}
} FunctionDescription;

class Arguments : public QVariantMap
{
public:
	bool match (const ArgumentDescription& descr) const
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
};

class CallResult
{
public:
	typedef enum
	{
		NO_ERROR = 0,
		METHOD_NOT_PROVIDED,
		RESOURCE_IS_BUSY,
		INVALID_ARGUMENT,
		C_ERROR,
		METHOD_ERROR
	} Code;

	static CallResult OK(void)
	{ return CallResult(NO_ERROR); }

	static CallResult method_not_provided(void)
	{ return CallResult(METHOD_NOT_PROVIDED); }

	static CallResult resource_is_busy(QString resource,QString message=QString())
	{ return CallResult(RESOURCE_IS_BUSY,0,resource,message); }

	static CallResult invalid_argument(QString message=QString())
	{ return CallResult(INVALID_ARGUMENT); }

	static CallResult c_error(int extra_errno,QString message=QString())
	{ return CallResult(C_ERROR,extra_errno,QString(),message); }

	static CallResult method_error(int extra_errno,QString method_error=QString(),QString message=QString())
	{ return CallResult(METHOD_ERROR,extra_errno,method_error,message); }

	~CallResult()
	{}

	int     code        (void) const { return m_code; }
	int     extra_errno (void) const { return m_extra_errno; }
	QString method_error(void) const { return m_method_error; }
	QString message     (void) const { return m_message; }

private:
	CallResult(int code,int extra_errno=0,QString method_error=QString(),QString message=QString())
		: m_code(code)
		, m_extra_errno(extra_errno)
		, m_method_error(method_error)
		, m_message(message)
	{}

	int m_code;
	int m_extra_errno;
	QString m_method_error;
	QString m_message;
};

class Module
{
public:
	Module(void)
	{}
	virtual ~Module()
	{}

	virtual QString resource_type (void) const =0;

	virtual QSet<FunctionDescription> depends_on (void) const =0;

	virtual QSet<FunctionDescription> provides   (void) const =0;

	virtual QSharedPointer<CallResult> call (QString name,const Arguments &input,Arguments &output) =0;
};

Q_DECLARE_INTERFACE(Module,"imaginable.Module/1.0")

#endif // IMAGINABLE__MODULE_INTERFACE__INCLUDED
