/* * * * * *
 *
 * Title:     Imaginable
 * Created:   2010-02-16
 * Author:    Kuzma Shapran
 * Copyright: Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */

#ifndef KIP_HPP
#define KIP_HPP


#include <QtCore/QObject>
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

	void quit(void);

private:
	QTimer m_autoCloseTimer;

	//QMap<QString,image> m_images;
};

#endif // KIP_HPP
