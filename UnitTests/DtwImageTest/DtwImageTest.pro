#
#    Dye The World Project
#    Copyright (C) 2015  Alexey Novikov (novikov_DOT_aleksey@gmail_DOT_com)
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License as
#    published by the Free Software Foundation, either version 3 of the
#    License, or (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.


QT       += testlib

#QT       -= gui

TARGET = tst_dtwimagetest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_dtwimagetest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

include(../../DyeTheWorld.pri)

message(INCLUDEPATH: $$INCLUDEPATH)
