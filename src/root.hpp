/* * * * * *
 *
 * Title:     Imaginable
 * Created:   2010-02-16
 * Author:    Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */
// $Id$

#ifndef IMAGINABLE__ROOT__INCLUDED
#define IMAGINABLE__ROOT__INCLUDED


#include "image_q.hpp"

#include <boost/shared_ptr.hpp>

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QTimer>


class Root : public QObject
{
Q_OBJECT
public:
	explicit Root(QObject* parent=NULL);

	bool init_DBus(void);

signals:

public:
	Q_PROPERTY(unsigned autoCloseTime READ autoCloseTime WRITE setAutoCloseTime)
	unsigned autoCloseTime(void) const { return m_autoCloseTimer.interval()/(60*1000); }
	void setAutoCloseTime(unsigned);

public slots:
	void touch(void);

	QString version(void) const;

	qulonglong newImage(void);
	qulonglong cloneImage(qulonglong);
	void deleteImage(qulonglong);

	void quit(void);

private slots:
	void timeout(void);

private:
	QTimer m_autoCloseTimer;

	typedef QMap<qulonglong,boost::shared_ptr<Image_Q> > ImagesMap;
	ImagesMap m_images;

	qulonglong nextIndex(void);
};

#endif // IMAGINABLE__ROOT__INCLUDED
