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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    originalImage(QImage()),
    dtwImage(nullptr)
{
    ui->setupUi(this);

    ui->imageLabel->setScaledContents(true);

#ifdef QT_DEBUG
    loadImage(QImage(":/testImage.jpg"));
#else
    ui->energyButton->setHidden(true);
#endif


}

void MainWindow::loadImage(const QImage &image)
{
    originalImage = image;
    dtwImage = new dtw::DtwImage(image);
    displayModeToggled(true);
}

void MainWindow::displayImage(const QImage &image)
{
    displayedImage = image;
    ui->imageLabel->setPixmap(QPixmap::fromImage(image));
    ui->imageLabel->show();
}

void MainWindow::displayModeToggled(bool checked)
{
    if(!checked) return;

    if (ui->originalButton->isChecked())
        displayImage(originalImage);
    else if (ui->coloringPageButton->isChecked())
        displayImage(dtwImage->makeColoringPage(ui->detailsSpinBox->value()));
#ifdef QT_DEBUG
    else if (ui->energyButton->isChecked())
        displayImage(dtwImage->dumpEnergy());
#endif
}

void MainWindow::onDetailRatioChanged(int newPercent)
{
    if (ui->coloringPageButton->isChecked())
        displayImage(dtwImage->makeColoringPage(newPercent));
}


MainWindow::~MainWindow()
{
    delete dtwImage;
    delete ui;
}

