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

#include <QString>
#include <QtTest>

#include "dtwimage.h"
#include "benchmark.h"

using namespace dtw;

class dtwImageTest : public QObject
{
    Q_OBJECT

public:
    dtwImageTest();

private:
    QImage originalImage;
    DtwImage * dtwImage;

    void resizeTest(const QSize& newSize);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

#ifdef QT_DEBUG
    void dumpEnergyTestCase();
#endif

    void resizeHalfWidthTestCase();
    void resizeHalfHeightTestCase();
    void resizeHalfSizeTestCase();
    void resizeDoubleWidthTestCase();
    void resizeDoubleHeightTestCase();
    void resizeDoubleSizeTestCase();
    void resizeTransposeTestCase();

    void makeColoringPageTestCase();
};

dtwImageTest::dtwImageTest()
    : originalImage(QImage(":/testImage.jpg").convertToFormat(DtwImage::DTW_FORMAT))
{
    originalImage.save("originalImage.jpg");
}

void dtwImageTest::initTestCase()
{
    BENCHMARK_START();
    dtwImage = new DtwImage(originalImage);
    BENCHMARK_STOP();
#ifdef QT_DEBUG
    QImage dumpedImage = dtwImage->dumpImage();
    dumpedImage.save("dumpedImage.jpg");
    QVERIFY(originalImage == dumpedImage);
#endif
}

void dtwImageTest::cleanupTestCase()
{
    delete(dtwImage);
}

#ifdef QT_DEBUG
void dtwImageTest::dumpEnergyTestCase() {
    QVERIFY(dtwImage->dumpEnergy().save("energy.jpg"));
}
#endif

void dtwImageTest::resizeTest(const QSize& newSize)
{
    QVERIFY(dtwImage->resize(newSize).size() == newSize);
}

void dtwImageTest::resizeHalfWidthTestCase()
{
    resizeTest(QSize(originalImage.size().width()/2, originalImage.size().height()));
}

void dtwImageTest::resizeHalfHeightTestCase() {
    resizeTest(QSize(originalImage.size().width(), originalImage.size().height()/2));
}

void dtwImageTest::resizeHalfSizeTestCase() {
    resizeTest(originalImage.size()/2);
}

void dtwImageTest::resizeDoubleWidthTestCase() {
    resizeTest(QSize(originalImage.size().width()*2, originalImage.size().height()));
}

void dtwImageTest::resizeDoubleHeightTestCase() {
    resizeTest(QSize(originalImage.size().width(), originalImage.size().height()*2));

}

void dtwImageTest::resizeDoubleSizeTestCase() {
    resizeTest(originalImage.size()*2);
}

void dtwImageTest::resizeTransposeTestCase() {
    resizeTest(originalImage.size().transposed());
}

void dtwImageTest::makeColoringPageTestCase()
{
    QVERIFY(dtwImage->makeColoringPage().save("coloringPage.jpg"));
}

QTEST_APPLESS_MAIN(dtwImageTest)

#include "tst_dtwimagetest.moc"
