/* * * * * *
 *
 * Title:     Imaginable
 * Created:   2010-03-06
 * Author:    Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */
// $Id$


#include "options.hpp"


Options::Options(QObject* parent_)
	: QObject(parent_)
	, m_parsed(false)
{
}

void Options::setAlias(QString alias,QString option)
{
	m_optionAlias[alias]=option;
	if(property(qPrintable(option)).type()==QVariant::Invalid)
		setProperty(qPrintable(option),QVariant(false));
}

void Options::setInfo(QString option,QString info)
{
	m_optionInfo[option]=info;
	if(property(qPrintable(option)).type()==QVariant::Invalid)
		setProperty(qPrintable(option),QVariant(false));
}

bool Options::parse(QStringList arguments)
{
	makeInfo();

	if(m_parsed)
		return false;
	m_parsed=true;

	QString option;
	for(int i=1;i<arguments.size();++i)
	{
		QString argument=arguments.at(i);
		if(option.isEmpty())
		{
			if(m_optionAlias.contains(argument))
				argument=m_optionAlias[argument];

			int eq=argument.indexOf('=');
			if(eq!=-1)
			{
				arguments.insert(i+1,argument.mid(eq+1));
				argument=argument.left(eq);
			}

			if(dynamicPropertyNames().contains(QByteArray(qPrintable(argument))))
			{
				if(property(qPrintable(argument)).type()==QVariant::String)
					option=argument;
				else
					setProperty(qPrintable(argument),QVariant(true));
			}
			else
				m_optionUnnamed<<argument;
		}
		else
		{
			setProperty(qPrintable(option),QVariant(argument));
			option=QString();
		}
	}
	return option.isEmpty();
}

void Options::makeInfo(void) const
{
	if(!m_info.isEmpty())
		return;

	int widest=0;
	foreach(QString option,m_optionInfo.keys())
	{
		int len=option.length();
		if(widest<len)
			widest=len;
	}
	++widest;

	foreach(QByteArray ba_option,dynamicPropertyNames())
	{
		QString option(ba_option);
		if(m_optionInfo.contains(option))
		{
			m_info+="\t"+option+QString(widest-option.length(),QChar(' '))+"-- ";
			if( (property(qPrintable(option)).type()==QVariant::String)
			&& (!property(qPrintable(option)).toString().isEmpty()) )
				m_info+="[='"+property(qPrintable(option)).toString()+"'] ";
			m_info+=m_optionInfo.value(option)+"\n";
		}
	}
}
