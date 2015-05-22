#include <QString>
#include <QtTest>

#include "dtwimage.h"

class dtwImageTest : public QObject
{
    Q_OBJECT

public:
    dtwImageTest();

private:
    DtwImage dtwImage;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testCase1();
};

dtwImageTest::dtwImageTest()
{
}

void dtwImageTest::initTestCase()
{
}

void dtwImageTest::cleanupTestCase()
{
}

void dtwImageTest::testCase1()
{
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(dtwImageTest)

#include "tst_dtwimagetest.moc"
