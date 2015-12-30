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

#include "dtwimage.h"
#include "dtwimage_p.h"

using namespace dtw;

DtwImage::DtwImage(const QImage& img) : d_ptr(new DtwImagePrivate(this))
{
    Q_D(DtwImage);
    d->m_original = img;
}

DtwImage::~DtwImage()
{
    delete d_ptr;
}

QImage DtwImage::resize(const QSize& size) const
{
    Q_UNUSED(size);
    Q_D(const DtwImage);
    return QImage();
}

QImage DtwImage::makeColoringPage(void) const
{
    Q_D(const DtwImage);
    return QImage();
}

#ifdef QT_DEBUG
QImage DtwImage::dumpEnergy() const {
    Q_D(const DtwImage);
    QImage energy = QImage(d->m_original.size(), QImage::Format_Grayscale8);
    //TODO
    return QImage();
}
#endif
