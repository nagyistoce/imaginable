# # # # # #
# Title: Imaginable
# Author: Kuzma Shapran
# Created: 2010-02-16
# Copyright: Kuzma Shapran <Kuzma.Shapran@gmail.com>
# License: GPLv3
# # # # # #
TARGET = imaginable
TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
QT += dbus
QT -= gui
BIN_DIR = bin
CONFIG(debug,debug|release):BUILD_DIR = /debug
else:BUILD_DIR = /release
CONFIG(debug,debug|release):DEFINES += _DEBUG
MAIN_DIR = ../$${BIN_DIR}$${BUILD_DIR}
DESTDIR = $${MAIN_DIR}/$${TEMPLATE}
OBJECTS_DIR = $${MAIN_DIR}/obj/$${TEMPLATE}/$${TARGET}
MOC_DIR = $${OBJECTS_DIR}
RCC_DIR = $${OBJECTS_DIR}
OUT_PWD = $${OBJECTS_DIR}
INCLUDEPATH += /usr/include
SOURCES += main.cpp \
    version.cpp \
    options.cpp \
    root.cpp \
    dbus_a_root.cpp \
    image.cpp \
    image_q.cpp \
    dbus_a_image_q.cpp
HEADERS += main.hpp \
    version.hpp \
    options.hpp \
    root.hpp \
    dbus_a_root.h \
    image.hpp \
    image_q.hpp \
    dbus_a_image_q.h \
    types.hpp
OTHER_FILES += version.xml \
    autoversioning.sh \
    version-svn.auto.inl \
    version-build.auto.inl \
    dbus.mk \
    dbus_a_root.xml \
    dbus_a_image_q.xml \
    dbus_a_image_q.xml
