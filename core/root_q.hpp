/* * * * * *
 *
 * Title:     Imaginable
 * Created:   2010-02-16
 * Author:    Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */
// $Id: root.hpp 9 2010-03-20 09:53:22Z Kuzma.Shapran $

#ifndef IMAGINABLE__ROOT_Q__INCLUDED
#define IMAGINABLE__ROOT_Q__INCLUDED


#include "root.hpp"
#include "image_q.hpp"

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QTimer>
#include <QtCore/QPluginLoader>


class Image;

class Root_Q : public QObject, public Root
{
Q_OBJECT
public:
	Root_Q(QObject* parent=NULL);

	bool init(void);

signals:
	void imageCreated(qulonglong);
	void imageDeleted(qulonglong);

public:
	Q_PROPERTY(unsigned autoCloseTime READ autoCloseTime WRITE setAutoCloseTime)
	unsigned    autoCloseTime(void) const { return m_autoCloseTimer.interval()/(60*1000); }
	void     setAutoCloseTime(unsigned);
	void restartAutoCloseTimer(void) { setAutoCloseTime(autoCloseTime()); }

	Image* image(qulonglong);
	bool hasImage(qulonglong) const;
	void pluginMessage(int level,QString plugin,qulonglong Id,QString message) const;

public slots:
	QString version(void) const;

	qulonglong createImage(void);
	bool       deleteImage(qulonglong);

	bool           loadPlugin       (QString fileName);
	QStringList loadAllPlugins      (QString dirName);
	bool             isPluginLoaded (QString fileName) const;
	QString            pluginName   (QString fileName) const;
	QString            pluginVersion(QString fileName) const;
	QStringList        pluginsList  (void) const;
	bool         unloadPlugin       (QString fileName);

	void quit(void);

protected:
	typedef QMap<qulonglong,Image_Q*> Images;
	Images m_images;

	qulonglong nextIndex(void);

private slots:
	void autoCloseTimeout(void);

private:
	QTimer m_autoCloseTimer;

	void restartAutoCloser(void);

	typedef QMap<QString,QPluginLoader*> Plugins;
	Plugins m_plugins;
};

#endif // IMAGINABLE__ROOT_Q__INCLUDED
