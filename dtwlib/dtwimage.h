/*****************************************************************************
 *                          Dye The World Project                            *
 *   Copyright (C) 2015  Alexey Novikov (novikov_DOT_aleksey@gmail_DOT_com)  *
 *                                                                           *
 *  This program is free software: you can redistribute it and/or modify     *
 *  it under the terms of the GNU Affero General Public License as           *
 *  published by the Free Software Foundation, either version 3 of the       *
 *  License, or (at your option) any later version.                          *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU Affero General Public License for more details.                      *
 *                                                                           *
 *  You should have received a copy of the GNU Affero General Public License *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *****************************************************************************/


#ifndef DTWIMAGE_H
#define DTWIMAGE_H

#include <QObject>
#include <QImage>

namespace dtw {

class DtwImagePrivate;

class DtwImage: public QObject
{
    Q_OBJECT
public:
    static const QImage::Format DTW_FORMAT;

    DtwImage(const QImage&);
    DtwImage(const DtwImage&);

    ~DtwImage();

    DtwImage clone() const;
    QImage resize(const QSize& rect) const;
    QImage makeColoringPage(void) const;
    QImage makeColoringPage(const QSize& size) const;

#ifdef QT_DEBUG
    QImage dumpEnergy() const;
    QImage dumpImage() const;
    QImage dumpSeams() const;
    QImage dumpTopContour() const;
#endif

private:
    DtwImage(const QSize&);
    DtwImagePrivate * d_ptr;
    Q_DECLARE_PRIVATE(DtwImage);
};//class DtwImage

}//namespace dtw
#endif // DTWIMAGE_H
