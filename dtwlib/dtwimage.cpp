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

#include <QDebug>


#include <array>
#include <cmath>

using namespace dtw;

const QImage::Format DtwImage::DTW_FORMAT = QImage::Format_ARGB32;

static const int INVALID_INDEX = -1;
static const std::array<Neighbour, 3> UPPERS = { UP_RIGHT, UP, UP_LEFT };
static const std::array<Neighbour, 3> LOWERS[] = { DOWN_RIGHT, DOWN, DOWN_LEFT };
static const std::array<Neighbour, 3> LEFTS[] = { UP_LEFT, LEFT, DOWN_LEFT };
static const std::array<Neighbour, 3> RIGHTS[] = { UP_RIGHT, RIGHT, DOWN_RIGHT };

static double BORDER_ENERGY = std::numeric_limits<double>::infinity();
static double DELETED_ENERGY = std::nan("DELETED");

DtwImage::DtwImage(const QImage& img) : d_ptr(new DtwImagePrivate(this, img))
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
    return d->makeImage();
}

QImage DtwImage::makeColoringPage(void) const
{
    Q_D(const DtwImage);
    return d->makeImage();
}

QImage DtwImage::makeColoringPage(const QSize& size) const
{
    Q_UNUSED(size);
    Q_D(const DtwImage);
    return d->makeImage();
}

DtwImagePrivate::Cell::Cell()
    : neighbours(), energy(BORDER_ENERGY), color()
{}

static inline double dualGradientEnergy(QRgb pX, QRgb sX, QRgb pY, QRgb sY) {
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


DtwImagePrivate::DtwImagePrivate(DtwImage *q, QImage img)
    : q_ptr(q), size(img.size()),
      cells(size.height() * size.width())
{
    if (img.format() != q->DTW_FORMAT)
        img = img.convertToFormat(q->DTW_FORMAT, Qt::AutoColor);
    const int height = size.height();
    const int width  = size.width();
    if (height < 3 || width < 3)  throw std::invalid_argument("Incorrect image dimensions");
    QMutableVectorIterator<Cell> it(cells);
    //Fillin colors
    for (int i = 0; i < height; i++) {
        const QRgb* rawLine = reinterpret_cast<const QRgb*>(img.constScanLine(i));
        for (int j = 0; j < width; j++)
        {
            Cell & cell = it.next();
            cell.color = rawLine[j];
        }
    }

    //Establish connections and calculate energy (TODO)
    {
        const int down = width;
        const int right = 1;
        cells[0].neighbours[DOWN_LEFT] = INVALID_INDEX;
        cells[0].neighbours[LEFT] = INVALID_INDEX;
        cells[0].neighbours[UP_LEFT] = INVALID_INDEX;
        cells[0].neighbours[UP] = INVALID_INDEX;
        cells[0].neighbours[UP_RIGHT] = INVALID_INDEX;
        cells[0].neighbours[RIGHT] = right;
        cells[0].neighbours[DOWN_RIGHT] = down + 1;
        cells[0].neighbours[DOWN] = down;
        cells[0].energy = dualGradientEnergy(cells[0].color, cells[down].color,
                                         cells[0].color, cells[right].color);

    }
    for (int j = 1; j < width - 1; j++)
    {
        const int down = j + width;
        const int right = j + 1;
        const int left = j - 1;
        cells[j].neighbours[UP_LEFT] = INVALID_INDEX;
        cells[j].neighbours[UP] = INVALID_INDEX;
        cells[j].neighbours[UP_RIGHT] = INVALID_INDEX;
        cells[j].neighbours[RIGHT] = right;
        cells[j].neighbours[DOWN_RIGHT] = down + 1;
        cells[j].neighbours[DOWN] = down;
        cells[j].neighbours[DOWN_LEFT] = down - 1;
        cells[j].neighbours[LEFT] = left;
        cells[j].energy = dualGradientEnergy(cells[j].color, cells[down].color,
                                         cells[left].color, cells[right].color);
    }
    {
        int k = width - 1;
        const int down = k + width;;
        const int left = k - 1;
        cells[k].neighbours[UP_LEFT] = INVALID_INDEX;
        cells[k].neighbours[UP] = INVALID_INDEX;
        cells[k].neighbours[UP_RIGHT] = INVALID_INDEX;
        cells[k].neighbours[RIGHT] = INVALID_INDEX;
        cells[k].neighbours[DOWN_RIGHT] = INVALID_INDEX;
        cells[k].neighbours[DOWN] = down;
        cells[k].neighbours[DOWN_LEFT] = down - 1;
        cells[k].neighbours[LEFT] = left;
        cells[k].energy = dualGradientEnergy(cells[k].color, cells[down].color,
                                         cells[left].color, cells[k].color);
    }
    int k = width;
    for (int i = 1; i < height - 1; i++) {
        {
            const int up = k - width;
            const int down = k + width;
            const int right = k + 1;
            cells[k].neighbours[DOWN_LEFT] = INVALID_INDEX;
            cells[k].neighbours[LEFT] = INVALID_INDEX;
            cells[k].neighbours[UP_LEFT] = INVALID_INDEX;
            cells[k].neighbours[UP] = up;
            cells[k].neighbours[UP_RIGHT] = up + 1;
            cells[k].neighbours[RIGHT] = right;
            cells[k].neighbours[DOWN_RIGHT] = down + 1;
            cells[k].neighbours[DOWN] = down;
            cells[k].energy = dualGradientEnergy(cells[up].color, cells[down].color,
                                             cells[k].color, cells[right].color);
            k++;
        }
        for (int j = 1; j < width - 1; j++)
        {
            const int up = k - width;
            const int down = k + width;
            const int left = k - 1;
            const int right = k + 1;
            cells[k].neighbours[UP_LEFT] = up - 1;
            cells[k].neighbours[UP] = up;
            cells[k].neighbours[UP_RIGHT] = up + 1;
            cells[k].neighbours[RIGHT] = right;
            cells[k].neighbours[DOWN_RIGHT] = down + 1;
            cells[k].neighbours[DOWN] = down;
            cells[k].neighbours[DOWN_LEFT] = down - 1;
            cells[k].neighbours[LEFT] = left;
            cells[k].energy = dualGradientEnergy(cells[up].color, cells[down].color,
                                                 cells[left].color, cells[right].color);
            k++;
        }
        {
            const int up = k - width;
            const int down = k + width;
            const int left = k - 1;
            cells[k].neighbours[UP_LEFT] =  up - 1;
            cells[k].neighbours[UP] =  up;
            cells[k].neighbours[UP_RIGHT] =  INVALID_INDEX;
            cells[k].neighbours[RIGHT] = INVALID_INDEX;
            cells[k].neighbours[DOWN_RIGHT] = INVALID_INDEX;
            cells[k].neighbours[DOWN] = down;
            cells[k].neighbours[DOWN_LEFT] = down - 1;
            cells[k].neighbours[LEFT] = left;
            cells[k].energy = dualGradientEnergy(cells[up].color, cells[down].color,
                                                 cells[left].color, cells[k].color);
            k++;
        }
    }
    {
        const int up = k - width;
        const int right = k + 1;
        cells[k].neighbours[DOWN_LEFT] = INVALID_INDEX;
        cells[k].neighbours[LEFT] = INVALID_INDEX;
        cells[k].neighbours[UP_LEFT] = INVALID_INDEX;
        cells[k].neighbours[UP] = up;
        cells[k].neighbours[UP_RIGHT] = up + 1;
        cells[k].neighbours[RIGHT] = right;
        cells[k].neighbours[DOWN_RIGHT] = INVALID_INDEX;
        cells[k].neighbours[DOWN] = INVALID_INDEX;
        cells[k].energy = dualGradientEnergy(cells[up].color, cells[k].color,
                                             cells[k].color, cells[right].color);
        k++;
    }
    for (int j = 1; j < width - 1; j++)
    {
        const int up = k - width;
        const int left = k - 1;
        const int right = k + 1;
        cells[k].neighbours[UP_LEFT] = up - 1;
        cells[k].neighbours[UP] = up;
        cells[k].neighbours[UP_RIGHT] = up + 1;
        cells[k].neighbours[RIGHT] = right;
        cells[k].neighbours[DOWN_RIGHT] = INVALID_INDEX;
        cells[k].neighbours[DOWN] = INVALID_INDEX;
        cells[k].neighbours[DOWN_LEFT] = INVALID_INDEX;
        cells[k].neighbours[LEFT] = left;
        cells[k].energy = dualGradientEnergy(cells[up].color, cells[k].color,
                                             cells[left].color, cells[right].color);
        k++;
    }
    {
        const int up = k - width;
        const int left = k - 1;
        cells[k].neighbours[UP_LEFT] =  up - 1;
        cells[k].neighbours[UP] =  up;
        cells[k].neighbours[UP_RIGHT] =  INVALID_INDEX;
        cells[k].neighbours[RIGHT] = INVALID_INDEX;
        cells[k].neighbours[DOWN_RIGHT] = INVALID_INDEX;
        cells[k].neighbours[DOWN] = INVALID_INDEX;
        cells[k].neighbours[DOWN_LEFT] = INVALID_INDEX;
        cells[k].neighbours[LEFT] = left;
        cells[k].energy = dualGradientEnergy(cells[up].color, cells[k].color,
                                             cells[left].color, cells[k].color);

    }
    Q_ASSERT(++k == cells.size());
}


QImage DtwImagePrivate::makeImage() const
{
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
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            double e = d->energy(i, j);
            energyArr[i][j] = e;
            if (maxEnergy < e) maxEnergy = e;
        }
    }
    const double scaleFactor = 255.0/maxEnergy;
    for (int j = 0; j < height; j++) {
        uchar * line = energyImage.scanLine(j);
        for (int i = 1; i < width - 1; i++) {
            line[i] = energyArr[i][j]*scaleFactor;
        }
    }
    return energyImage;
}

QImage DtwImage::dumpImage() const {
    Q_D(const DtwImage);
    int k = 0;
    do {
        if (k >= d->cells.size()) return QImage();
        if (!std::isnan(d->cells[k].energy)) break;
        k++;
    } while(true);

    Q_ASSERT(d->cells[k].neighbours[UP] < 0
             && d->cells[k].neighbours[LEFT] < 0);

    const int height = d->size.height();
    const int width = d->size.width();

    QImage image(d->size, DTW_FORMAT);
    for (int i = 0; i < height; i++) {
        QRgb * line = reinterpret_cast<QRgb *>(image.scanLine(i));
        const int nextLineStart = d->cells[k].neighbours[DOWN];
        for (int j = 0; j < width; j++) {
            Q_ASSERT(k >= 0 && k < d->cells.size());
            line[j] = d->cells[k].color;
            k = d->cells[k].neighbours[RIGHT];
        }
        Q_ASSERT(k < 0);
        k = nextLineStart;
    }
    Q_ASSERT(k < 0);

    return image;
}
#endif

double DtwImagePrivate::energy(int x, int y) const { //TODO
    const int k = y*size.width() + x;
    Q_ASSERT(k>=0 && k < cells.size());
    return cells[k].energy;
}
