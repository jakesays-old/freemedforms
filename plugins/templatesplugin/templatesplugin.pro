TEMPLATE = lib
TARGET = Templates
PACKAGE_VERSION = 0.0.2
DEFINES += TEMPLATES_LIBRARY
include(../fmf_plugins.pri)
include( templatesplugin_dependencies.pri )
QT += sql
HEADERS = templatesplugin.h \
    templates_exporter.h \
    itemplates.h \
    templatesmodel.h \
    constants.h \
    templatesview.h
SOURCES = templatesplugin.cpp \
    templatesmodel.cpp \
    templatesview.cpp
OTHER_FILES = Templates.pluginspec
FORMS += templatesview.ui
