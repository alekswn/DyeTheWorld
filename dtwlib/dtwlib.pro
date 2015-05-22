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

#greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport

TARGET = dtw
TEMPLATE = lib
#CONFIG += staticlib

SOURCES += dtwimage.cpp

HEADERS += dtwimage.h \
    dtwimage_p.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

include(../DyeTheWorld.pri)
