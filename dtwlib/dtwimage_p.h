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

#ifndef DTWIMAGE_P_H
#define DTWIMAGE_P_H

#include "dtwimage.h"
#include <QImage>

namespace dtw {

enum Neighbour { UP, RIGHT, DOWN, LEFT, UP_RIGHT, DOWN_RIGHT, DOWN_LEFT,
                       UP_LEFT, NEIGHBOUR_LAST };

class DtwImagePrivate
{
struct Cell {
        std::array<int, NEIGHBOUR_LAST> neighbours;
        double energy;
        Cell();
    };

public:
    DtwImage * q_ptr;
    QSize size;
    QVector<QRgb> colors;
    QVector<Cell> cells;
    QImage m_original;

    Q_DECLARE_PUBLIC(DtwImage);
    DtwImagePrivate(DtwImage *q, QImage img);

    QImage makeImage() const;

    double energy(int x, int y) const;

    double dualGradientEnergy(int left, int rigth, int up, int down);
};//struct DtwImagePrivate

}//namespace dtw
#endif // DTWIMAGE_P_H

