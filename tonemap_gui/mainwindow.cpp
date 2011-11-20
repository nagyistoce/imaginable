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
#include <QtCore/QTimer>

#include <boost/bind.hpp>

#include <fstream>
#include <iostream>

#include <imaginable/io_pnm_loader.hpp>
#include <imaginable/io_pam_saver.hpp>
#include <imaginable/io_qt.hpp>
#include <imaginable/colourspace.hpp>
#include <imaginable/tonemap.hpp>
#include <imaginable/crop.hpp>
#include <imaginable/scale.hpp>
#include <imaginable/tools.hpp>
#include <imaginable/gamma.hpp>

#include "mainwindow.hpp"


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, current_zoom(new QLabel(this))
	, progress_bar(new QProgressBar(this))
	, less_side(0)
	, m_lock_update(0)
	, m_update_flags(0)
	, m_prev_update_flags(0)
{
	setupUi(this);
	status_bar->addPermanentWidget(progress_bar,1);
	progress_bar->setValue(0);
	progress_bar->hide();

	mainToolBar->addWidget(current_zoom);

	action_Open->setShortcut(QKeySequence::Open);
	if (action_Open->shortcut().isEmpty())
		action_Open->setShortcut(QKeySequence(tr("Ctrl+O")));

	action_Save->setShortcut(QKeySequence::Save);
	if (action_Save->shortcut().isEmpty())
		action_Save->setShortcut(QKeySequence(tr("Ctrl+S")));

	action_Save_as->setShortcut(QKeySequence::SaveAs);
	if (action_Save_as->shortcut().isEmpty())
		action_Save_as->setShortcut(QKeySequence(tr("Ctrl+Alt+S")));

	action_Quit->setShortcut(QKeySequence::Quit);
	if (action_Quit->shortcut().isEmpty())
		action_Quit->setShortcut(QKeySequence(tr("Ctrl+Q")));

	action_Zoom_in->setShortcut(QKeySequence::ZoomIn);
	if (action_Zoom_in->shortcut().isEmpty())
		action_Zoom_in->setShortcut(QKeySequence(tr("Ctrl++")));

	action_Zoom_out->setShortcut(QKeySequence::ZoomOut);
	if (action_Zoom_out->shortcut().isEmpty())
		action_Zoom_out->setShortcut(QKeySequence(tr("Ctrl+-")));


	update_timer.setInterval(250);
	update_timer.setSingleShot(false);
	connect(&update_timer,SIGNAL(timeout()),this,SLOT(updateTimeout()));


	last_user_dir = QDir::homePath();

	open_filters = tr("Portable Arbitrary Map images")+" [*.pam](*.pam);;"+tr("Portable Any Map images")+" [*.pnm](*.pnm)";

	save_filters = tr("Portable Arbitrary Map images")+" [*.pam](*.pam);;";
	QStringList qt_can_write;
	foreach (QByteArray ba,QImageWriter::supportedImageFormats())
	{
		QString mask=QString("*.")+QString(ba);
		qt_can_write << mask;
		save_filters += QString("[")+mask+"]("+mask+");;";
	}
	save_filters += tr("All images supported by QT")+"("+qt_can_write.join(" ")+")";
}

void MainWindow::imageLoaded(void)
{
	safe_as_file_name.clear();
	action_Save       ->setEnabled(true);
	action_Save_as    ->setEnabled(true);
	action_Zoom_in    ->setEnabled(true);
	action_Zoom_out   ->setEnabled(true);
	action_Zoom_to_fit->setChecked(true);
	current_zoom_type = ZOOM_TO_FIT;

	less_side = std::min(original_image->width(),original_image->height());

	m_update_flags |= UPDATE_SCALE;
	update_timer.start();

	setBlur(slider_blur->value());
}

bool MainWindow::loadFile(QString file_name)
{
	if (!file_name.isEmpty())
	{
		last_user_dir = QFileInfo(file_name).path();

		imaginable::SharedImage new_image(new imaginable::Image);

		if (file_name.endsWith(".pnm",Qt::CaseInsensitive)
		||  file_name.endsWith(".pam",Qt::CaseInsensitive) )
		{
			std::ifstream stream(qPrintable(file_name));
			if (stream.good())
				stream >> imaginable::PNM_loader(*new_image);
		}
		else
		{
			QImage qimage(file_name);
			if (!qimage.isNull())
				qimage >> imaginable::QImage_loader(*new_image);
		}

		if (!new_image->empty())
		{
			original_image = new_image;

			imageLoaded();

			return true;
		}
	}
	return false;
}

void MainWindow::previewImageDropped(QImage image)
{
	imaginable::SharedImage new_image(new imaginable::Image);

	if (!image.isNull())
		image >> imaginable::QImage_loader(*new_image);

	if (!new_image->empty())
	{
		original_image = new_image;

		imageLoaded();
	}
}

void MainWindow::previewUrlDropped(QString url)
{
	loadFile(url);
}

void MainWindow::fileOpen(void)
{
	loadFile(QFileDialog::getOpenFileName(this,tr("Open HDRI file"),last_user_dir,open_filters+";;"+tr("All files(*)")));
}

void MainWindow::fileSave(void)
{
	if (safe_as_file_name.isEmpty())
		fileSaveAs();
	else
	{
		imaginable::SharedImage tonemapped_image = original_image->copy();
		imaginable::rgb_to_hcy(*tonemapped_image,false);

		imaginable::TimedProgress timed_progress(boost::bind(&MainWindow::tonemap_notification,this,_1));
		imaginable::tonemap_local(*tonemapped_image,imaginable::gamma(static_cast<double>(slider_saturation->value())/100.,-3.)*10.,blur_in_pixels,static_cast<double>(slider_mix->value())/100./2.,timed_progress.notifier());

		imaginable::hcy_to_rgb(*tonemapped_image,false);


		if (safe_as_file_name.endsWith(".pam",Qt::CaseInsensitive))
		{
			std::ofstream stream(qPrintable(safe_as_file_name));
			if (stream.good())
				stream << imaginable::PAM_saver(*tonemapped_image);
		}
		else
		{
			QImage qimage;
			qimage << imaginable::QImage_saver(*tonemapped_image);
			qimage.save(safe_as_file_name);
		}
	}
}

void MainWindow::fileSaveAs(void)
{
	QString file_name = QFileDialog::getSaveFileName(this,tr("Save LDRI file"),last_user_dir,save_filters+";;"+tr("All files (*)"));
	if (!file_name.isEmpty())
	{
		last_user_dir = QFileInfo(file_name).path();
		safe_as_file_name = file_name;

		fileSave();
	}
}

void MainWindow::zoomIn(void)
{
	action_Zoom_to_fit->setChecked(false);
	zoom *= 1.125;
	current_zoom_type = ZOOM_CUSTOM;

	m_update_flags |= UPDATE_SCALE;
}

void MainWindow::zoomOut(void)
{
	action_Zoom_to_fit->setChecked(false);
	zoom /= 1.125;
	current_zoom_type = ZOOM_CUSTOM;

	m_update_flags |= UPDATE_SCALE;
}

void MainWindow::zoomOne(void)
{
	action_Zoom_to_fit->setChecked(false);
	zoom = 1.;
	current_zoom_type = ZOOM_ONE;

	m_update_flags |= UPDATE_SCALE;
}

void MainWindow::zoomToFit(void)
{
	current_zoom_type = ZOOM_TO_FIT;

	m_update_flags |= UPDATE_SCALE;
}

void MainWindow::setSaturation(int value)
{
	value_saturation->setText(QString("+%1").arg(imaginable::gamma(static_cast<double>(value)/100.,-3.)*10.,1,'f',2,'0'));

	if (!m_lock_update)
		m_update_flags |= UPDATE_TONEMAP;
}

void MainWindow::setBlur(int value)
{
	double corrected_blur = imaginable::gamma(static_cast<double>(value)/100.,-2.);
	value_blur_percent->setText(QString("%1%").arg(static_cast<size_t>(corrected_blur*100.)));
	blur_in_pixels        = std::max(static_cast<size_t>(1),static_cast<size_t>(static_cast<double>(less_side)        * corrected_blur));
	scaled_blur_in_pixels = std::max(static_cast<size_t>(1),static_cast<size_t>(static_cast<double>(less_scaled_side) * corrected_blur));
	value_blur_pixels->setText(QString("%1 px").arg(blur_in_pixels));

	if (!m_lock_update)
		m_update_flags |= UPDATE_TONEMAP;
}

void MainWindow::setMix(int value)
{
	value_mix->setText(QString("%1%").arg(value));

	if (!m_lock_update)
		m_update_flags |= UPDATE_TONEMAP;
}

void MainWindow::resetSliders(void)
{
	++m_lock_update;
		slider_saturation->setValue(imaginable::gamma(0.1,3.)*100.);
		slider_blur      ->setValue(imaginable::gamma(0.2,2.)*100.);
		slider_mix       ->setValue(50);
	--m_lock_update;

	m_update_flags |= UPDATE_TONEMAP;
}

//QPixmap MainWindow::image_to_pixmap(const imaginable::Image& src)
//{
//	QImage qimage;
//	qimage << imaginable::QImage_saver(src);
//	return QPixmap::fromImage(qimage);
//}

QImage MainWindow::image_to_qimage(const imaginable::Image& src)
{
	QImage qimage;
	qimage << imaginable::QImage_saver(src);
	return qimage;
}

void MainWindow::showOriginal(bool value)
{
	original_view->setVisible(value);

	m_update_flags |= action_Zoom_to_fit->isChecked() ? UPDATE_SCALE : UPDATE_VIEWS;
}

void MainWindow::previewResized(int,int)
{
	m_update_flags |= action_Zoom_to_fit->isChecked() ? UPDATE_SCALE : UPDATE_VIEWS;
}

void MainWindow::previewShifted(int dx,int dy)
{
	++m_lock_update;
		horizontalScrollBar->setValue(horizontalScrollBar->value()-dx);
		verticalScrollBar  ->setValue(  verticalScrollBar->value()-dy);
	--m_lock_update;

	m_update_flags |= UPDATE_VIEWS;
}

void MainWindow::horizontallySlided(int)
{
	if (!m_lock_update)
		m_update_flags |= UPDATE_VIEWS;
}

void MainWindow::verticallySlided(int)
{
	if (!m_lock_update)
		m_update_flags |= UPDATE_VIEWS;
}

void MainWindow::updateTimeout(void)
{
	if (m_prev_update_flags && !m_update_flags)
	{
		progress_bar->setMaximum(100);
		if      (m_prev_update_flags & UPDATE_SCALE)
			update_scale();
		else if (m_prev_update_flags & UPDATE_TONEMAP)
			update_tonemap();
		else if (m_prev_update_flags & UPDATE_VIEWS)
			update_views();
		m_prev_update_flags = 0;
	}
	else if (m_update_flags)
	{
		if (progress_bar->maximum())
		{
			progress_bar->setMaximum(0);
			progress_bar->show();
		}
		m_prev_update_flags |= m_update_flags;
		m_update_flags = 0;
	}
}

void MainWindow::update_scale(void)
{
	if (!original_image)
		return;

	switch (current_zoom_type)
	{
	case ZOOM_TO_FIT:
		zoom = std::min(
			static_cast<double>(tonemapped_view->width ())/static_cast<double>(original_image->width ()),
			static_cast<double>(tonemapped_view->height())/static_cast<double>(original_image->height()) );
	// FALL THROUGH
	case ZOOM_CUSTOM:
		zoom = std::min(std::max(zoom,2./static_cast<double>(std::min(original_image->width(),original_image->height()))),1.);
		scaled_image = scale_nearest(*original_image,
				static_cast<size_t>(static_cast<double>(original_image->width ())*zoom),
				static_cast<size_t>(static_cast<double>(original_image->height())*zoom) );
	break;
	case ZOOM_ONE:
		scaled_image = original_image->copy();
	}

	switch (current_zoom_type)
	{
	case ZOOM_CUSTOM:
	case ZOOM_TO_FIT:
		current_zoom->setText(QString("%1%").arg(zoom*100.,3,'f',1,'0'));
		break;
	case ZOOM_ONE:
		current_zoom->setText(QString("100.0%"));
		break;
	}

	int cx = -1;
	if (horizontalScrollBar->maximum())
		cx = horizontalScrollBar->value() + (scaled_image->width () - horizontalScrollBar->maximum())/2;

	int cy = -1;
	if (verticalScrollBar->maximum())
		cy =   verticalScrollBar->value() + (scaled_image->height() -   verticalScrollBar->maximum())/2;

	if ((tonemapped_view->width () < static_cast<int>(scaled_image->width ()))
	||  (tonemapped_view->height() < static_cast<int>(scaled_image->height())))
	{
		horizontalScrollBar->setValue   (0);
		horizontalScrollBar->setMaximum (std::max(tonemapped_view->width (),static_cast<int>(scaled_image->width ())) - tonemapped_view->width ());
		if (cx < 0)
			cx =  horizontalScrollBar->maximum()/2;
		else
			cx -= (scaled_image->width () - horizontalScrollBar->maximum())/2;
		horizontalScrollBar->setValue   (cx);
		horizontalScrollBar->setPageStep(horizontalScrollBar->maximum()/10);

		verticalScrollBar  ->setValue   (0);
		verticalScrollBar  ->setMaximum (std::max(tonemapped_view->height(),static_cast<int>(scaled_image->height())) - tonemapped_view->height());
		if (cy < 0)
			cy =  verticalScrollBar->maximum()/2;
		else
			cy -= (scaled_image->height() - verticalScrollBar->maximum())/2;
		verticalScrollBar  ->setValue   (cy);
		verticalScrollBar  ->setPageStep(verticalScrollBar->maximum()/10);
	}
	else
	{
		horizontalScrollBar->setMaximum(0);
		verticalScrollBar  ->setMaximum(0);
	}


	less_scaled_side = std::min(scaled_image->width(),scaled_image->height());
	double corrected_blur = imaginable::gamma(static_cast<double>(slider_blur->value())/100.,-2.);
	scaled_blur_in_pixels = std::max(static_cast<size_t>(1),static_cast<size_t>(static_cast<double>(less_scaled_side) * corrected_blur));

	update_tonemap();
}

void MainWindow::update_tonemap(void)
{
	if (!scaled_image)
		return;


	tonemapped_scaled_image = scaled_image->copy();

	progress_bar->show();

	imaginable::rgb_to_hcy(*tonemapped_scaled_image,false);

	imaginable::TimedProgress timed_progress(boost::bind(&MainWindow::tonemap_notification,this,_1));
	imaginable::tonemap_local(*tonemapped_scaled_image,imaginable::gamma(static_cast<double>(slider_saturation->value())/100.,-3.)*10.,scaled_blur_in_pixels,static_cast<double>(slider_mix->value())/100./2.,timed_progress.notifier());

	imaginable::hcy_to_rgb(*tonemapped_scaled_image,false);

	progress_bar->hide();


	update_views();
}

void MainWindow::update_views(void)
{
	if (!scaled_image)
		return;


	if (horizontalScrollBar->maximum()
	||  verticalScrollBar  ->maximum())
	{
		int crop_x = horizontalScrollBar->value();
		int crop_y = verticalScrollBar  ->value();
		int crop_w = std::min(tonemapped_view->width (),static_cast<int>(scaled_image->width ()));
		int crop_h = std::min(tonemapped_view->height(),static_cast<int>(scaled_image->height()));

		if (original_view->isVisible())
			original_view->setImage(image_to_qimage(*imaginable::crop(*scaled_image,crop_x,crop_y,crop_w,crop_h)));
		tonemapped_view->setImage(image_to_qimage(*imaginable::crop(*tonemapped_scaled_image,crop_x,crop_y,crop_w,crop_h)));
	}
	else
	{
		if (original_view->isVisible())
			original_view->setImage(image_to_qimage(*scaled_image));
		tonemapped_view->setImage(image_to_qimage(*tonemapped_scaled_image));
	}
}

void MainWindow::tonemap_notification(float value)
{
	progress_bar->setValue(value * 100);
}
