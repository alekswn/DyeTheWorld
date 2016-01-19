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

#include "dtwimage.h"
#include "dtwimage_p.h"

#include <QQueue>

#include <QDebug>
#include "benchmark.h"

#include <array>
#include <cmath>

using namespace dtw;

const QImage::Format DtwImage::DTW_FORMAT = QImage::Format_ARGB32;

static const index_t INVALID_INDEX = -1;
static const int DEF_CONTOUR_RATIO = 20;

#ifdef DIAGONAL_NEIGHBOURS
static const Directions UPPERS = { UP_LEFT, UP, UP_RIGHT };
static const Directions LOWERS = {  DOWN_LEFT, DOWN, DOWN_RIGHT };
static const Directions LEFTS = { UP_LEFT, LEFT, DOWN_LEFT };
static const Directions RIGHTS = { UP_RIGHT, RIGHT, DOWN_RIGHT };
#endif

//static energy_t BORDER_ENERGY = std::numeric_limits<energy_t>::infinity();
static energy_t BORDER_ENERGY = 1000.0;
static energy_t DELETED_ENERGY = std::nan("DELETED");

DtwImage::DtwImage(const QSize& size) : d_ptr(new DtwImagePrivate(this, size))
{
    Q_D(DtwImage);
}

DtwImage::DtwImage(const DtwImage& dtw)
    : QObject(), d_ptr(new DtwImagePrivate(this, dtw.d_ptr))
{
    Q_D(DtwImage);
}

DtwImage::DtwImage(const QImage& img) : d_ptr(new DtwImagePrivate(this, img))
{
    Q_D(DtwImage);
}

DtwImage::~DtwImage()
{
    delete d_ptr;
}

DtwImage DtwImage::clone() const
{
    return DtwImage(*this);
}

QImage DtwImage::resize(const QSize& size) const
{
    DtwImage tp(*this);
    tp.d_ptr->resize(size);
    return tp.d_ptr->makeImage();
}

QImage DtwImage::makeColoringPage(int detailRatio) const
{
    Q_D(const DtwImage);
    if (detailRatio <= 0) detailRatio = DEF_CONTOUR_RATIO;
    return d->makeHighEnergyImage(detailRatio);
}

QImage DtwImage::makeColoringPage(int detailRatio, const QSize& size) const
{
    return makeColoringPage(detailRatio).scaled(size);
}

DtwImagePrivate::Cell::Cell()
    : neighbours(), energy(BORDER_ENERGY)
{}

energy_t DtwImagePrivate::dualGradientEnergy(int left, int right, int up, int down) {

    const QRgb pX = colors[left];
    const QRgb sX = colors[right];
    const QRgb pY = colors[up];
    const QRgb sY = colors[down];

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

    const energy_t Dx2  = dRx*dRx + dGx*dGx + dBx*dBx;
    const energy_t Dy2  = dRy*dRy + dGy*dGy + dBy*dBy;

    return std::sqrt(Dx2 + Dy2);
/*
    const energy_t xBase = right - left;
    const energy_t yBase = down - up;

    return std::sqrt(Dx2/xBase/xBase + Dy2/yBase/yBase);
*/
}

DtwImagePrivate::DtwImagePrivate(DtwImage *q, const QSize& size)
: q_ptr(q), size(size), NM(size.height() * size.width()),
  colors(NM), cells(NM),
  startingCell(INVALID_INDEX)
{}

DtwImagePrivate::DtwImagePrivate(DtwImage *q, const DtwImagePrivate* r)
: q_ptr(q), size(r->size), NM(r->NM),
  colors(r->colors), cells(r->cells),
  startingCell(r->startingCell)
{}


DtwImagePrivate::DtwImagePrivate(DtwImage *q, QImage img)
    : q_ptr(q), size(img.size()), NM(size.height() * size.width()),
      colors(NM), cells(NM),
      startingCell(0)
{
    BENCHMARK_START();
    if (img.format() != q->DTW_FORMAT)
        img = img.convertToFormat(q->DTW_FORMAT, Qt::AutoColor);
    const int height = size.height();
    const int width  = size.width();
    if (height < 3 || width < 3)  throw std::invalid_argument("Incorrect image dimensions");
    //Fillin colors
    {
        int k = 0;
        for (int i = 0; i < height; i++) {
            const QRgb* rawLine = reinterpret_cast<const QRgb*>(img.constScanLine(i));
            for (int j = 0; j < width; j++)
            {
                colors[k++] = rawLine[j];
            }
        }
    }

    //Establish connections and calculate energy
    {
        const int down = width;
        const int right = 1;
        cells[0].neighbours[LEFT] = INVALID_INDEX;
        cells[0].neighbours[UP] = INVALID_INDEX;
        cells[0].neighbours[RIGHT] = right;
        cells[0].neighbours[DOWN] = down;
#ifdef DIAGONAL_NEIGHBOURS
        cells[0].neighbours[DOWN_LEFT] = INVALID_INDEX;
        cells[0].neighbours[UP_LEFT] = INVALID_INDEX;
        cells[0].neighbours[UP_RIGHT] = INVALID_INDEX;
        cells[0].neighbours[DOWN_RIGHT] = down + 1;
#endif
        cells[0].energy = dualGradientEnergy(0, right, 0, down);
    }
    for (int j = 1; j < width - 1; j++)
    {
        const int down = j + width;
        const int right = j + 1;
        const int left = j - 1;
        cells[j].neighbours[UP] = INVALID_INDEX;
        cells[j].neighbours[RIGHT] = right;
        cells[j].neighbours[DOWN] = down;
        cells[j].neighbours[LEFT] = left;
#ifdef DIAGONAL_NEIGHBOURS
        cells[j].neighbours[UP_LEFT] = INVALID_INDEX;
        cells[j].neighbours[UP_RIGHT] = INVALID_INDEX;
        cells[j].neighbours[DOWN_RIGHT] = down + 1;
        cells[j].neighbours[DOWN_LEFT] = down - 1;
#endif

        cells[j].energy = dualGradientEnergy(left, right, j, down);
    }
    {
        int k = width - 1;
        const int down = k + width;;
        const int left = k - 1;
        cells[k].neighbours[UP] = INVALID_INDEX;
        cells[k].neighbours[RIGHT] = INVALID_INDEX;
        cells[k].neighbours[DOWN] = down;
        cells[k].neighbours[LEFT] = left;
#ifdef DIAGONAL_NEIGHBOURS
        cells[k].neighbours[UP_LEFT] = INVALID_INDEX;
        cells[k].neighbours[UP_RIGHT] = INVALID_INDEX;
        cells[k].neighbours[DOWN_RIGHT] = INVALID_INDEX;
        cells[k].neighbours[DOWN_LEFT] = down - 1;
#endif
        cells[k].energy = dualGradientEnergy( left, k, k, down );
    }
    int k = width;
    for (int i = 1; i < height - 1; i++) {
        {
            const int up = k - width;
            const int down = k + width;
            const int right = k + 1;
            cells[k].neighbours[LEFT] = INVALID_INDEX;
            cells[k].neighbours[UP] = up;
            cells[k].neighbours[RIGHT] = right;
            cells[k].neighbours[DOWN] = down;
#ifdef DIAGONAL_NEIGHBOURS
            cells[k].neighbours[DOWN_LEFT] = INVALID_INDEX;
            cells[k].neighbours[UP_LEFT] = INVALID_INDEX;
            cells[k].neighbours[DOWN_RIGHT] = down + 1;
            cells[k].neighbours[UP_RIGHT] = up + 1;
#endif
            cells[k].energy = dualGradientEnergy( k, right, up, down );
            k++;
        }
        for (int j = 1; j < width - 1; j++)
        {
            const int up = k - width;
            const int down = k + width;
            const int left = k - 1;
            const int right = k + 1;
            cells[k].neighbours[UP] = up;
            cells[k].neighbours[RIGHT] = right;
            cells[k].neighbours[DOWN] = down;
            cells[k].neighbours[LEFT] = left;
#ifdef DIAGONAL_NEIGHBOURS
            cells[k].neighbours[UP_LEFT] = up - 1;
            cells[k].neighbours[UP_RIGHT] = up + 1;
            cells[k].neighbours[DOWN_RIGHT] = down + 1;
            cells[k].neighbours[DOWN_LEFT] = down - 1;
#endif
            cells[k].energy = dualGradientEnergy( left, right, up, down );
            k++;
        }
        {
            const int up = k - width;
            const int down = k + width;
            const int left = k - 1;
            cells[k].neighbours[UP] =  up;
            cells[k].neighbours[RIGHT] = INVALID_INDEX;
            cells[k].neighbours[DOWN] = down;
            cells[k].neighbours[LEFT] = left;
#ifdef DIAGONAL_NEIGHBOURS
            cells[k].neighbours[UP_LEFT] =  up - 1;
            cells[k].neighbours[UP_RIGHT] =  INVALID_INDEX;
            cells[k].neighbours[DOWN_RIGHT] = INVALID_INDEX;
            cells[k].neighbours[DOWN_LEFT] = down - 1;
#endif
            cells[k].energy = dualGradientEnergy( left, k, up, down );
            k++;
        }
    }
    {
        const int up = k - width;
        const int right = k + 1;
        cells[k].neighbours[LEFT] = INVALID_INDEX;
        cells[k].neighbours[UP] = up;
        cells[k].neighbours[RIGHT] = right;
        cells[k].neighbours[DOWN] = INVALID_INDEX;
#ifdef DIAGONAL_NEIGHBOURS
        cells[k].neighbours[DOWN_LEFT] = INVALID_INDEX;
        cells[k].neighbours[UP_LEFT] = INVALID_INDEX;
        cells[k].neighbours[UP_RIGHT] = up + 1;
        cells[k].neighbours[DOWN_RIGHT] = INVALID_INDEX;
#endif
        cells[k].energy = dualGradientEnergy( k, right, up, k );
        k++;
    }
    for (int j = 1; j < width - 1; j++)
    {
        const int up = k - width;
        const int left = k - 1;
        const int right = k + 1;
        cells[k].neighbours[UP] = up;
        cells[k].neighbours[RIGHT] = right;
        cells[k].neighbours[DOWN] = INVALID_INDEX;
        cells[k].neighbours[LEFT] = left;
#ifdef DIAGONAL_NEIGHBOURS
        cells[k].neighbours[UP_LEFT] = up - 1;
        cells[k].neighbours[UP_RIGHT] = up + 1;
        cells[k].neighbours[DOWN_RIGHT] = INVALID_INDEX;
        cells[k].neighbours[DOWN_LEFT] = INVALID_INDEX;
#endif
        cells[k].energy = dualGradientEnergy( left, right, up, k );
        k++;
    }
    {
        const int up = k - width;
        const int left = k - 1;
        cells[k].neighbours[UP] =  up;
        cells[k].neighbours[RIGHT] = INVALID_INDEX;
        cells[k].neighbours[DOWN] = INVALID_INDEX;
        cells[k].neighbours[LEFT] = left;
#ifdef DIAGONAL_NEIGHBOURS
        cells[k].neighbours[UP_LEFT] =  up - 1;
        cells[k].neighbours[UP_RIGHT] =  INVALID_INDEX;
        cells[k].neighbours[DOWN_RIGHT] = INVALID_INDEX;
        cells[k].neighbours[DOWN_LEFT] = INVALID_INDEX;
#endif
        cells[k].energy = dualGradientEnergy( left, k, up, k );
    }
    Q_ASSERT(++k == NM);
    BENCHMARK_STOP();
}

QImage DtwImagePrivate::makeHighEnergyImage(int ratio) const
{
    const energy_t threshold = getThresholdEnergy(ratio);
    QImage energyImage = QImage(size, QImage::Format_Grayscale8);
    const int width =  size.width() - 1;
    const int height = size.height() - 1;
    for (int j = 0; j < height; j++) {
        uchar * line = energyImage.scanLine(j);
        for (int i = 1; i < width - 1; i++) {
            line[i] = (energy(i,j) > threshold) ? 0 : 255;
        }
    }
    return energyImage;
}

QImage DtwImagePrivate::makeImage() const
{
    BENCHMARK_START();
    int k = startingCell;

    if (k == INVALID_INDEX) return QImage();

    Q_ASSERT(cells[k].neighbours[UP] < 0
             && cells[k].neighbours[LEFT] < 0);

    const int height = size.height();
    const int width = size.width();

    QImage image(size, DtwImage::DTW_FORMAT);
    for (int i = 0; i < height; i++) {
        QRgb * line = reinterpret_cast<QRgb *>(image.scanLine(i));
        const int nextLineStart = cells[k].neighbours[DOWN];
        for (int j = 0; j < width; j++) {
            Q_ASSERT(k >= 0 && k < cells.size());
            line[j] = colors[k];
            k = cells[k].neighbours[RIGHT];
        }
        Q_ASSERT(k < 0);
        k = nextLineStart;
    }
    Q_ASSERT(k < 0);
    BENCHMARK_STOP();
    return image;
}

#ifdef QT_DEBUG
QImage DtwImage::dumpEnergy() const {
    Q_D(const DtwImage);
    QImage energyImage = QImage(d->size, QImage::Format_Grayscale8);
    const int width =  d->size.width() - 1;
    const int height = d->size.height() - 1;
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
    qInfo() << "Max energy:" << maxEnergy;
    return energyImage;
}

QImage DtwImage::dumpHighEnergy() const {
    Q_D(const DtwImage);
    return d->makeHighEnergyImage(DEF_CONTOUR_RATIO);
}

QImage DtwImage::dumpImage() const {
    Q_D(const DtwImage);
    return d->makeImage();
}


#endif

energy_t DtwImagePrivate::getThresholdEnergy(int ratio) const {
    //create a sorted array
    QList<energy_t> energies;
    energies.reserve(NM);
    foreach(const Cell& cell, cells)
        energies.append(cell.energy);
    qSort(energies);
    energy_t threshold = energies[NM - NM/ratio];

    qDebug() << "Threshold energy:" << threshold;
    return threshold;
}

energy_t DtwImagePrivate::energy(int x, int y) const {
    const int k = y*size.width() + x;
    Q_ASSERT(k>=0 && k < NM);
    return cells[k].energy;
}

void DtwImagePrivate::updateEnergy(index_t idx) {
    Q_ASSERT(idx >= 0 && idx < NM);
    index_t left =  cells[idx].neighbours[LEFT];
    index_t right = cells[idx].neighbours[RIGHT];
    index_t up = cells[idx].neighbours[UP];
    index_t down = cells[idx].neighbours[DOWN];

    if (left == INVALID_INDEX) {
        left = idx;
    }
    if (right == INVALID_INDEX) {
        right = idx;
    }
    if (up == INVALID_INDEX) {
        up = idx;
    }
    if (down == INVALID_INDEX) {
        down = idx;
    }
    cells[idx].energy = dualGradientEnergy(left,right,up,down);
}

/////////////////////////////Seam operations///////////////////////////////////

index_t DtwImagePrivate::SeamLayer::getMinPathEdge() const {
    const int size = this->size();
    index_t minEdge = INVALID_INDEX;
    energy_t minDist = std::numeric_limits<energy_t>::infinity();
    for (int i = 0; i < size; i++) {
        const energy_t d = dist(i);
        if (d < minDist) {
            minDist = d;
            minEdge = i;
        }
    }
    return minEdge;
}

Seam DtwImagePrivate::findSeamHelper(SeamLayer&& firstLayer,
                                     const Neighbour dir, int length) const {
    BENCHMARK_START();
    QList<SeamLayer*> layers;
    const int layerCapacity = firstLayer.size();
    Q_ASSERT(layerCapacity > 1);

    layers.reserve(length);

    SeamLayer* currentLayer = new SeamLayer(firstLayer);

    //relax all interlayer edges in loop
    while(--length) {
        int j = 0;
        SeamLayer* nextLayer = new SeamLayer(layerCapacity);

        {//Special handle for the first cell in row
            const index_t up = currentLayer->index(0);
            const index_t idx = cells[up].neighbours[dir];
            nextLayer->add(idx, 0, cells[idx].energy + currentLayer->dist(0));
            nextLayer->relax(1, cells[idx].energy + currentLayer->dist(1));
        }
        for (j = 1; j < (layerCapacity - 1); j++)
        {//Adding a new edge and relaxing diagonales
            const index_t up = currentLayer->index(j);
            const index_t idx = cells[up].neighbours[dir];
            nextLayer->add(idx, j, cells[idx].energy + currentLayer->dist(j));
            nextLayer->relax(j - 1, cells[idx].energy + currentLayer->dist(j - 1));
            nextLayer->relax(j + 1, cells[idx].energy + currentLayer->dist(j + 1));
        }
        {//Special handle for the last cell in row - relaxing 2 edges
            const index_t up = currentLayer->index(j);
            const index_t idx = cells[up].neighbours[dir];
            nextLayer->add(idx, j, cells[idx].energy + currentLayer->dist(j));
            nextLayer->relax(j - 1, cells[idx].energy + currentLayer->dist(j - 1));
        }
        Q_ASSERT(nextLayer->size() == layerCapacity);
        layers.append(currentLayer);
        currentLayer = nextLayer;
    }

    //Traverse minimal path
    index_t minPathEdge = currentLayer->getMinPathEdge();
    Seam seam;
    seam.reserve(length);
    seam.prepend(currentLayer->index(minPathEdge));
    do {
        SeamLayer * lastLayer = layers.takeLast();
        minPathEdge = lastLayer->edge(minPathEdge);
        seam.prepend(lastLayer->index(minPathEdge));
        delete lastLayer;
    } while (!layers.isEmpty());
    BENCHMARK_STOP();
    return seam;
}

Seam DtwImagePrivate::findVerticalSeam() const {
    SeamLayer firstLayer(size.width());
    index_t g = 0;
    for (index_t i = startingCell; i != INVALID_INDEX; i = cells[i].neighbours[RIGHT])
        firstLayer.add(i, g++, cells[i].energy);
    Q_ASSERT(firstLayer.size() == size.width());
    return findSeamHelper(std::move(firstLayer), DOWN, size.height());
}

Seam DtwImagePrivate::findHorizontalSeam() const {
    SeamLayer firstLayer(size.height());
    index_t g = 0;
    for (index_t i = startingCell; i != INVALID_INDEX; i = cells[i].neighbours[DOWN])
        firstLayer.add(i, g++, cells[i].energy);
    Q_ASSERT(firstLayer.size() == size.height());
    return findSeamHelper(std::move(firstLayer), RIGHT, size.width());
}

#ifdef QT_DEBUG
QImage DtwImage::dumpSeams() const {
    DtwImage tmp(*this);
    tmp.d_ptr->drawSeams();
    return tmp.dumpImage();
}

void DtwImagePrivate::drawSeams() {
    Seam vSeam = findVerticalSeam();
    Seam hSeam = findHorizontalSeam();
    foreach (index_t idx, vSeam) {
        colors[idx] = Qt::red;
    }
    foreach (index_t idx, hSeam) {
        colors[idx] = Qt::red;
    }

}

void DtwImagePrivate::drawTopContour() {
    QPair<Seam, energy_t> contour = findContour(startingCell);
    qInfo() << __PRETTY_FUNCTION__ << " : The top contour has energy " << contour.second;
    foreach (index_t idx, contour.first) {
        colors[idx] = Qt::green;
    }
}


QImage DtwImage::dumpTopContour() const {
    DtwImage tmp(*this);
    tmp.d_ptr->drawTopContour();
    return tmp.dumpImage();
}

#endif

void DtwImagePrivate::removeHorizontalSeam(const Seam& seam) {
#ifdef DIAGONAL_NEIGHBOURS
#error "removeSeam() unable to handle diagonal neighbours"
#endif
    BENCHMARK_START();
    //Update starting cell
    index_t idx = seam.front();
    if(idx == startingCell) startingCell = cells[idx].neighbours[DOWN];

    //Sew cells
    for(int i = 0;;) {
        const index_t up  = cells[idx].neighbours[UP];
        const index_t down = cells[idx].neighbours[DOWN];
        const index_t right = cells[idx].neighbours[RIGHT];
        if(up != INVALID_INDEX) cells[up].neighbours[DOWN] = down;
        if(down != INVALID_INDEX) cells[down].neighbours[UP] = up;
        if(right == INVALID_INDEX) break;
        idx = seam[++i];
        if(idx != right) {
            const index_t right_up = cells[right].neighbours[UP];
            if (idx == right_up) {
                cells[right].neighbours[LEFT] = up;
                cells[up].neighbours[RIGHT] = right;
            } else {
                Q_ASSERT (idx == cells[right].neighbours[DOWN]);
                cells[right].neighbours[LEFT] = down;
                cells[down].neighbours[RIGHT] = right;
            }
        }
    }
    size.rheight()--;
    BENCHMARK_STOP();
}

void DtwImagePrivate::removeVerticalSeam(const Seam& seam) {
#ifdef DIAGONAL_NEIGHBOURS
#error "removeSeam() unable to handle diagonal neighbours"
#endif
    BENCHMARK_START();
    //Update starting cell
    index_t idx = seam.front();
    if(idx == startingCell) startingCell = cells[idx].neighbours[RIGHT];

    //Sew cells and update energy
    for(int i = 0;;) {
#ifdef QT_DEBUG
        //Ensure the cell was not deleted yet and mark it as deleted
        Q_ASSERT(!std::isnan(cells[idx].energy));
        cells[idx].energy = DELETED_ENERGY;
#endif
        const index_t left = cells[idx].neighbours[LEFT];
        const index_t right = cells[idx].neighbours[RIGHT];
        const index_t down = cells[idx].neighbours[DOWN];
        if(left != INVALID_INDEX) {
            cells[left].neighbours[RIGHT] = right;
            updateEnergy(left);
        }
        if(right != INVALID_INDEX) {
            cells[right].neighbours[LEFT] = left;
            updateEnergy(right);
        }
        if(down == INVALID_INDEX) break;
        idx = seam[++i]; // Get next index from the seam
        if(idx != down) {
            const index_t down_left = cells[down].neighbours[LEFT];
            if (idx == down_left) {
                cells[down].neighbours[UP] = left;
                cells[left].neighbours[DOWN] = down;
                updateEnergy(left);
            } else {
                Q_ASSERT (idx == cells[down].neighbours[RIGHT]);
                cells[down].neighbours[UP] = right;
                cells[right].neighbours[DOWN] = down;
                updateEnergy(right);
            }
            updateEnergy(down);
        }
    }
    size.rwidth()--;
    BENCHMARK_STOP();
}

void DtwImagePrivate::resize(const QSize& newSize) {
    const QSize deltaSize = newSize - size;
    int dh = deltaSize.height();
    int dw = deltaSize.width();
    if ( dh > 0 || dw > 0) {
        qWarning() << "Scaling up is not implemented yet";
    }

    while(dw++ < 0) {
        removeVerticalSeam(findVerticalSeam());
    }

    while(dh++ < 0) {
        removeHorizontalSeam(findHorizontalSeam());
    }

}

////////////////////////////  Contour operations //////////////////////////////

QPair<Seam, energy_t> DtwImagePrivate::findContour(index_t start, int minLength) {
    Q_ASSERT(start > INVALID_INDEX && minLength > 1);
    BENCHMARK_START();

    //Fill in the first layer with BFS
    SeamLayer currentLayer(NM); //TODO: It's resonable to reserve less memory here.
    QQueue<index_t> queue;
    QVector<bool> visited(NM, false);
    queue.enqueue(start);
    visited[start] = true;
    while(!queue.empty()) {
        index_t idx = queue.dequeue();
        currentLayer.add(idx, INVALID_INDEX, -cells[idx].energy);
        std::for_each(cells[idx].neighbours.begin(), cells[idx].neighbours.end(),
                     [&](index_t i) mutable {
                        if(i != INVALID_INDEX && !visited[i]) {
                            queue.enqueue(i);
                            visited[i] = true;
                        }});
    }
    //TODO: perhaps need to do squeeze() here
    //Iterate throught the layers till the end
    QList <SeamLayer> layers;
    const int layerSize = currentLayer.size();
    Q_ASSERT(layerSize == NM);
    Seam candidateSeam;
    energy_t candidateEnergy = 0.0;

    for (int n = 0; n < layerSize; n++) {
        qInfo() << n << " level";
        SeamLayer nextLayer(currentLayer);
        for (int idx = 0; idx < layerSize; idx++) {
            const Cell& cell = cells[idx];
            std::for_each(cell.neighbours.begin(), cell.neighbours.end(),
                         [&](index_t i) mutable {
                            if (i != INVALID_INDEX) {
                                const index_t from = currentLayer.index(idx);
                                const energy_t energy = currentLayer.dist(idx) - cells[i].energy; //negative
                                if (i == idx) { //cycle found
                                    qInfo() << "Cycle found";
                                    if (n > minLength && energy < candidateEnergy) { //energy is stored as negative
                                        candidateEnergy = energy;
                                        //make candidate seam
                                        candidateSeam.clear();
                                        candidateSeam.reserve(n);
                                        candidateSeam.prepend(idx);
                                        index_t edgeTo = currentLayer.edge(idx);
                                        auto it = layers.end();
                                        while ( it-- != layers.begin()) {
                                            qInfo() << "Candidate found";
                                            candidateSeam.prepend(edgeTo);
                                            edgeTo = it->edge(edgeTo);
                                            Q_ASSERT(edgeTo != INVALID_INDEX);
                                        }
                                        Q_ASSERT(edgeTo == INVALID_INDEX);
                                    }
                                } else {
                                    nextLayer.relax(i, idx, energy, from);
                                }
                            }});
        }
        layers.append(currentLayer);
        currentLayer = nextLayer;
     }

    BENCHMARK_STOP();
    return QPair<Seam, energy_t>(candidateSeam, -candidateEnergy);
}

