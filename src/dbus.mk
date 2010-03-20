# # # # # #
#
# Title: Imaginable
# Author: Kuzma Shapran
# Created: 2010-02-16
# Copyright: Kuzma Shapran <Kuzma.Shapran@gmail.com>
# License: GPLv3
#
# # # # # #

XMLC = /opt/qtsdk-2010.02/qt/bin/qdbusxml2cpp
RM = rm -f
TOUCH = touch

SOURCES = \
	dbus_a_root.cpp \
	dbus_a_root.h \
	dbus_a_image_q.cpp \
	dbus_a_image_q.h

XMLS = \
	dbus_a_root.xml \
	dbus_a_image_q.xml

all: $(SOURCES)

dbus_a_root.cpp dbus_a_root.h: dbus_a_root.xml
	$(XMLC) -a dbus_a_root -i root.hpp -c DBus_Adaptor_Root -l Root "$<"

dbus_a_image_q.cpp dbus_a_image_q.h: dbus_a_image_q.xml
	$(XMLC) -a dbus_a_image_q -i image_q.hpp -c DBus_Adaptor_Image_Q -l Image_Q "$<"


clean:
	-$(RM) $(SOURCES)
	-$(TOUCH) $(SOURCES)
	sleep 1
	-$(TOUCH) $(XMLS)
