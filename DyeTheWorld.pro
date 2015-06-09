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

include(DyeTheWorld.pri)

TEMPLATE = subdirs
CONFIG += ordered
TEMP_SUBDIRS = $$SUBDIRS
SUBDIRS = dtwlib
SUBDIRS += $$TEMP_SUBDIRS

#Create output dirs
#copied from http://comp-phys.net/2013/11/18/creating-directories-in-the-build-folder-using-qmake/

#Loop over all given directories and append
#the build directory to make absolute paths
for(DIR, DIRS_TO_CREATE) {

     #add these absolute paths to a variable which
     #ends up as 'mkcommands = path1 path2 path3 ...'
     mkcommands += $$OUT_PWD/$$DIR
}

#make a custom createDir variable with
#the command mkdir path1 path2 path3 ...
createDirs.commands = $(MKDIR) $$mkcommands

#Do Svenn-Arne explained magic!
first.depends += createDirs
QMAKE_EXTRA_TARGETS += createDirs


message(SUBDIRS: $$SUBDIRS)
message(CONFIG: $$CONFIG)

