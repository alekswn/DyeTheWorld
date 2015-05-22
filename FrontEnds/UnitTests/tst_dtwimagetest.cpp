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

using namespace dtw;

class dtwImageTest : public QObject
{
    Q_OBJECT

public:
    dtwImageTest();

private:
    QImage originalImage;
    DtwImage * dtwImage;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void getColoringPageTestCase();
};

dtwImageTest::dtwImageTest()
    : originalImage(":/testImage.jpg")
{
    originalImage.save("originalImage.jpg");
}

void dtwImageTest::initTestCase()
{
    dtwImage = new DtwImage(originalImage);
}

void dtwImageTest::cleanupTestCase()
{
    delete(dtwImage);
}

void dtwImageTest::getColoringPageTestCase()
{
    QVERIFY(dtwImage->getColoringPage().save("coloringPage.jpg"));
}

QTEST_APPLESS_MAIN(dtwImageTest)

#include "tst_dtwimagetest.moc"
