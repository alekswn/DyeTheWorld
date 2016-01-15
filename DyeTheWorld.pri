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

isEmpty(CONFIG_FILE){
    CONFIG_FILE = "config.pri"
}

exists( $$CONFIG_FILE ) {
    include($$CONFIG_FILE)
} else {
    error("Config file \"$$CONFIG_FILE\" is not exist !")
}

isEmpty(BUILD_ROOT){
    BUILD_ROOT = $$shadowed(build-qt-$$QT_VERSION-$$basename(QMAKESPEC))
}

isEmpty(DESTDIR) {
    release: DESTDIR = $$BUILD_ROOT/release
    debug:   DESTDIR = $$BUILD_ROOT/debug
}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui

DIRS_TO_CREATE =  \
        $$DESTDIR \
        $$OBJECTS_DIR \
        $$MOC_DIR \
        $$RCC_DIR \
        $$UI_DIR \


equals(TEMPLATE, "app") {
    LIBS += -L$$DESTDIR/ -ldtw
    INCLUDEPATH += $$absolute_path(dtwlib)
    DEPENDPATH +=  $$absolute_path(dtwlib)
#    INCLUDEPATH += $$shadowed(dtwlib)
#    DEPENDPATH +=  $$shadowed(dtwlib)

    unix:!mac{
        QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/\''
        QMAKE_RPATH=
    }

    DISTFILES +=

    if (!CONFIG(console) || equals( TARGET, "tst_dtwimagetest" )) {
        RESOURCES += \
            ../../Data/common.qrc
    }

    DEFINES += APP_VERSION=\\\"$$VERSION\\\"

}

debug {
#    DEFINES += BENCH
}

CONFIG += c++11
