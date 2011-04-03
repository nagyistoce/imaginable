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

#ifndef IMAGINABLE__FUNCTION__INCLUDED
#define IMAGINABLE__FUNCTION__INCLUDED


#include <QtCore/QVariant>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QRegExp>
#include <QtCore/QSharedPointer>


class ValueDescription
{
public:
	QVariant::Type type;
	bool array;
};


class ArgumentDescription : public QMap<QString,ValueDescription>
{
public:
	ArgumentDescription& operator () (QString key,QVariant::Type type,bool array);

	int compare (const ArgumentDescription& right) const;
	bool operator < (const ArgumentDescription& right) const { return compare(right) < 0; }
};

class FunctionDescription
{
public:
	FunctionDescription(QString name,const ArgumentDescription& input,const ArgumentDescription& output)
		: m_name(name)
		, m_input(input)
		, m_output(output)
		{}
	~FunctionDescription() {}

	QString             name   (void) const { return m_name; }
	ArgumentDescription input  (void) const { return m_input; }
	ArgumentDescription output (void) const { return m_output; }

	bool is_valid (void);
	bool name_is_valid (QString name);
	bool value_is_valid (QVariant::Type type);

	int compare (const struct FunctionDescription& right) const;
	bool operator <  (const struct FunctionDescription& right) const { return compare(right) < 0; }
	bool operator == (const struct FunctionDescription& right) const { return compare(right) == 0; }


	QString interface_toString (void) const;
	QString toString (void) const;

private:
	QString m_name;
	ArgumentDescription m_input;
	ArgumentDescription m_output;
};

uint qHash(const FunctionDescription& descr);


class Arguments : public QVariantMap
{
public:
	bool match (const ArgumentDescription& descr) const;
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

	static QSharedPointer<CallResult> OK (void)
	{ return QSharedPointer<CallResult>(new CallResult(NO_ERROR)); }

	static QSharedPointer<CallResult> method_not_provided (void)
	{ return QSharedPointer<CallResult>(new CallResult(METHOD_NOT_PROVIDED)); }

	static QSharedPointer<CallResult> resource_is_busy    (                QString resource,    QString message=QString())
	{ return QSharedPointer<CallResult>(new CallResult(RESOURCE_IS_BUSY,0,resource,message)); }

	static QSharedPointer<CallResult> invalid_argument    (                                     QString message=QString())
	{ return QSharedPointer<CallResult>(new CallResult(INVALID_ARGUMENT,0,QString(),message)); }

	static QSharedPointer<CallResult> c_error             (int extra_errno,                     QString message=QString())
	{ return QSharedPointer<CallResult>(new CallResult(C_ERROR,extra_errno,QString(),message)); }

	static QSharedPointer<CallResult> method_error        (int extra_errno,QString method_error,QString message=QString())
	{ return QSharedPointer<CallResult>(new CallResult(METHOD_ERROR,extra_errno,method_error,message)); }

	~CallResult() {}

	int     code         (void) const { return m_code; }
	int     extra_errno  (void) const { return m_extra_errno; }
	QString method_error (void) const { return m_method_error; }
	QString message      (void) const { return m_message; }

private:
	CallResult (int code,int extra_errno=0,QString method_error=QString(),QString message=QString())
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

#endif // IMAGINABLE__FUNCTION__INCLUDED
