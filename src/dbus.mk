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
	dbus_a_root.h

XMLS = \
	dbus_a_root.xml

all: $(SOURCES)

dbus_a_root.cpp dbus_a_root.h: dbus_a_root.xml
	$(XMLC) -a dbus_a_root -i root.hpp -c DBus_Adaptor_Root -l Root "$<"


clean:
	-$(RM) $(SOURCES)
	-$(TOUCH) $(SOURCES)
	sleep 1
	-$(TOUCH) $(XMLS)
