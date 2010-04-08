/* * * * * *
 *
 * Title:     Imaginable
 * Created:   2010-03-21
 * Author:    Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */
// $Id$

#ifndef IMAGINABLE__PLUGIN_CLONEIMAGE__INCLUDED
#define IMAGINABLE__PLUGIN_CLONEIMAGE__INCLUDED


#include "version.hpp"

#include <plugin_iface.hpp>


class PublicThreadSleep: public QThread
{
public:
	static void  sleep(unsigned long v) { QThread:: sleep(v); }
	static void msleep(unsigned long v) { QThread::msleep(v); }
	static void usleep(unsigned long v) { QThread::usleep(v); }
};

class PluginClone : public QObject, PluginInterface
{
Q_OBJECT
Q_INTERFACES(PluginInterface)
public:
	PluginClone(void);
	~PluginClone() {}

	bool init(Root*);
	QString name   (void) const { return "/cloneImage"; }
	QString version(void) const { return QString::fromAscii(version::full_string()); }

signals:
	void lock_percent(double);

public slots:
	bool clone(qulonglong from,qulonglong to);

	void lock(qulonglong,int);

private:
	void long_lock(Image*,int);
};

#endif // IMAGINABLE__PLUGIN_CLONEIMAGE__INCLUDED
