/*************
**
** Project:      Imaginable :: tonemap (GUI)
** File info:    $Id$
** Author:       Copyright (C) 2009 - 2011 Kuzma Shapran <Kuzma.Shapran@gmail.com>
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


#include <QtGui/QProgressBar>
#include <QtGui/QFileDialog>
#include <QtGui/QImageWriter>

#include "mainwindow.hpp"


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, progress_bar(new QProgressBar(this))
{
	setupUi(this);
	status_bar->addPermanentWidget(progress_bar,1);
	progress_bar->setValue(0);
	progress_bar->hide();

	last_user_dir = QDir::homePath();

	QString qt_can_write;
	foreach (QByteArray ba,QImageWriter::supportedImageFormats())
		qt_can_write += QString(" *.")+QString(ba);
	filters
		<< (tr("All images")+" (*.pam *.pnm *.ppm *.exr)"+qt_can_write)
		<< (tr("HDRI images")+" (*.pam *.pnm *.ppm *.exr)")
		<< (tr("LDRI images")+" (*.pam *.pnm *.ppm)"+qt_can_write)
		<< tr("All files (*)");
	open_filter = 1;
	save_filter = 2;
}

void MainWindow::openFile(void)
{
	QString current_filter = filters[open_filter];
	QString file_name = QFileDialog::getOpenFileName(this,tr("Open HDRI file"),last_user_dir,filters.join(";;"),&current_filter);
	if (!file_name.isEmpty())
	{
		last_user_dir = QFileInfo(file_name).path();
		original->setText(file_name);
	}
}

void MainWindow::saveFile(void)
{
	QString current_filter = filters[save_filter];
	QString file_name = QFileDialog::getSaveFileName(this,tr("Save LDRI file"),last_user_dir,filters.join(";;"),&current_filter);
	if (!file_name.isEmpty())
	{
		last_user_dir = QFileInfo(file_name).path();
		preview->setText(file_name);
	}
}

void MainWindow::setSaturation(int value)
{
	value_saturation->setText(QString("+%1").arg(static_cast<double>(value)/10.,3,'f',1,'0'));
}

void MainWindow::setBlur(int value)
{
	value_blur->setText(QString("%1%").arg(value));
}

void MainWindow::setMix(int value)
{
	value_mix->setText(QString("%1%").arg(value));
}

void MainWindow::resetSliders(void)
{
	slider_saturation->setValue(10);
	slider_blur      ->setValue(20);
	slider_mix       ->setValue(50);
}
