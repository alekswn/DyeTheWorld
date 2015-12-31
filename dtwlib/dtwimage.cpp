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
    QImage energyImage = QImage(d->m_original.size(), QImage::Format_Grayscale8);
    const int width =  d->m_original.width() - 1;
    const int height = d->m_original.height() - 1;
    double maxEnergy = 0.0;
    QVector<QVector<double>> energyArr(width, QVector<double>(height));
    for (int i = 1; i < width; i++) {
        for (int j = 1; j < height; j++) {
            double e = d->energy(i, j);
            energyArr[i][j] = e;
            if (maxEnergy < e) maxEnergy = e;
        }
    }
    const double scaleFactor = 255.0/maxEnergy;
    for (int j = 1; j < height; j++) {
        uchar * line = energyImage.scanLine(j);
        for (int i = 1; i < width; i++) {
            line[i] = energyArr[i][j]*scaleFactor;
        }
    }
    return energyImage;
}
#endif

double DtwImagePrivate::dualGradientEnergy(int x, int y, const QImage& picture) {
    Q_ASSERT(x > 0 && x < picture.width() - 1 && y > 0 && y < picture.height() - 1);

    const QRgb pX = picture.pixel(x - 1, y);
    const QRgb sX = picture.pixel(x + 1, y);
    const QRgb pY = picture.pixel(x, y - 1);
    const QRgb sY = picture.pixel(x, y + 1);

    const int RpX  = qRed(pX);
    const int GpX  = qGreen(pX);
    const int BpX  = qBlue(pX);
    const int RsX  = qRed(sX);
    const int GsX  = qGreen(sX);
    const int BsX  = qBlue(sX);
    const int RpY  = qRed(pY);
    const int GpY  = qGreen(pY);
    const int BpY  = qBlue(pY);
    const int RsY  = qRed(sY);
    const int GsY  = qGreen(sY);
    const int BsY  = qBlue(sY);

    const int dRx  = RpX - RsX;
    const int dGx  = GpX - GsX;
    const int dBx  = BpX - BsX;
    const int dRy  = RpY - RsY;
    const int dGy  = GpY - GsY;
    const int dBy  = BpY - BsY;

    const int Dx2  = dRx*dRx + dGx*dGx + dBx*dBx;
    const int Dy2  = dRy*dRy + dGy*dGy + dBy*dBy;

    return std::sqrt(Dx2 + Dy2);
}
