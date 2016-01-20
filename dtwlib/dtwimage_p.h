/*****************************************************************************
 *                          Dye The World Project                            *
 *   Copyright (C) 2016  Alexey Novikov (novikov_DOT_aleksey@gmail_DOT_com)  *
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
#include <QDebug>

namespace dtw {

enum Neighbour { UP, RIGHT, DOWN, LEFT,
#ifdef DIAGONAL_NEIGHBOURS
                 UP_RIGHT, DOWN_RIGHT, DOWN_LEFT, UP_LEFT,
#endif
                 NEIGHBOUR_LAST };
typedef int index_t;
typedef double energy_t;
typedef QList<index_t> Seam;
typedef std::array<Neighbour, 3> Directions;

class DtwImagePrivate
{

struct Cell {
        std::array<index_t, NEIGHBOUR_LAST> neighbours;
        energy_t energy;
        Cell();
    };

class SeamLayer {
protected:
    QVector<index_t> indexes;
    QVector<index_t> edgeTo;
    QVector<energy_t> distTo;

public:
    SeamLayer(size_t size) {
        indexes.reserve(size);
        edgeTo.reserve(size);
        distTo.reserve(size);
    }

    SeamLayer(const SeamLayer& r)
        : indexes(r.indexes), edgeTo(r.edgeTo), distTo(r.distTo) //We relay on Qt's "copy on write"
    { }

    void squeeze() { //Free excessive memory
        indexes.squeeze();
        edgeTo.squeeze();
        distTo.squeeze();
    }


    void add(index_t i, index_t g, energy_t d) {
        indexes.append(i);
        edgeTo.append(g);
        distTo.append(d);
    }

    void relax(index_t g, energy_t d) {
        if (d < distTo.back()) {
            edgeTo.back() = g;
            distTo.back() = d;
        }
    }

    void relax(index_t i, index_t g, energy_t d, index_t from) {
        if (d < distTo[i]) {
            edgeTo[i] = g;
            distTo[i] = d;
            indexes[i] = from;
        }
    }

    int size() const {
        Q_ASSERT(edgeTo.size() == distTo.size() && edgeTo.size() == indexes.size());
        return edgeTo.size();
    }

    index_t edge(int i) const {
        Q_ASSERT(i >= 0 && i < edgeTo.size());
        return edgeTo[i];
    }

    energy_t dist(int i) const {
        Q_ASSERT(i >= 0 && i < distTo.size());
        return distTo[i];
    }

    energy_t index(int i) const {
        Q_ASSERT(i >= 0 && i < indexes.size());
        return indexes[i];
    }

    index_t getMinPathEdge() const;

};

struct Cache {
    bool isUpToDate;
    QList<energy_t> sortedEnergies;

    Cache() : isUpToDate(false) {}
    void invalidate() { isUpToDate = false; }
};

public:
    DtwImage * q_ptr;
    Q_DECLARE_PUBLIC(DtwImage)

    QSize size;
    int NM;
    QVector<QRgb> colors;
    QVector<Cell> cells;
    index_t startingCell;

    mutable Cache cache;

    DtwImagePrivate(DtwImage *q, QImage img);
    DtwImagePrivate(DtwImage *q, const DtwImagePrivate * r);
    DtwImagePrivate(DtwImage *q, const QSize& size);

    QImage makeImage() const;
    QImage makeHighEnergyImage(int detailRatio) const;

    Seam findVerticalSeam() const;
    Seam findHorizontalSeam() const;
    Seam findSeamHelper(SeamLayer&& firstLayer, const Neighbour dir, int length) const;

    QList<Seam> findAllCountours( int minLength = 0, energy_t minEnergy = 0.0 ) const;
    QPair<Seam, energy_t>  findContour(index_t start, int minLength = 3) const;

#ifdef QT_DEBUG
    void drawSeams();
    void drawTopContour();
#endif

    void removeVerticalSeam(const Seam&);
    void removeHorizontalSeam(const Seam&);
    void removeContour(const Seam&);

    void resize(const QSize& size);

private:

    energy_t energy(int x, int y) const;
    void updateEnergy(index_t idx);
    energy_t getThresholdEnergy(int ratio) const;

    energy_t dualGradientEnergy(int left, int rigth, int up, int down) const;

};//struct DtwImagePrivate

}//namespace dtw
#endif // DTWIMAGE_P_H

