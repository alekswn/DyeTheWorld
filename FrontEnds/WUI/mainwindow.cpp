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
#include <QScrollBar>
#include <QStringList>
#include <QByteArray>
#include <QDialog>
#include <QImageReader>
#include <QImageWriter>
#include <QStandardPaths>
#include <QFileDialog>
#include <QPrintDialog>
#include <QPainter>
#include <QPrinter>
#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    imageLabel(nullptr),
    originalPixmap(),
    dtwImage(nullptr),
    scaleFactor(0.0)
{
    ui->setupUi(this);

    ui->scrollArea->setWidget(imageLabel = new QLabel());

#if defined(QT_NO_PRINTER) || defined(QT_NO_PRINTDIALOG)
    ui->printButton->setHidden(true);
#endif

#ifdef QT_DEBUG
    loadImage(QImage(":/testImage.jpg"));
#else
    ui->energyButton->setHidden(true);
#endif
}

MainWindow::~MainWindow()
{
    if (dtwImage != nullptr) delete dtwImage;
    delete ui;
}

void MainWindow::loadImage(const QImage &image)
{
    originalPixmap = QPixmap::fromImage(image);
    if (dtwImage != nullptr) delete dtwImage;
    dtwImage = new dtw::DtwImage(image);
    scaleFactor = 0.0;
    displayModeToggled(true);
}

void MainWindow::displayPixmap(const QPixmap &pixmap)
{
    if (scaleFactor == 0.0) {
        const int wScale = ui->scrollArea->size().width()*100 / pixmap.size().width();
        const int hScale = ui->scrollArea->size().height()*100 / pixmap.size().height();
        const int scale = qMin(wScale,hScale);
        ui->zoomSpinBox->setValue(scale);
        ui->zoomSpinBox->setMinimum(scale);
        ui->zoomSlider->setMinimum(scale);
    }
    QSize size = pixmap.size()*scaleFactor;
    imageLabel->setPixmap(pixmap.scaled(size));
    #ifdef QT_DEBUG
    ui->statusBar->showMessage("scaleFactor = " + QString::number(scaleFactor));
    #endif
    imageLabel->show();
    displayedPixmap = pixmap;
}

void MainWindow::displayModeToggled(bool checked)
{
    if(!checked || dtwImage == nullptr) return;

    if (ui->originalButton->isChecked())
        displayPixmap(originalPixmap);
    else if (ui->coloringPageButton->isChecked())
        displayPixmap(QPixmap::fromImage(dtwImage->makeColoringPage(ui->detailsSpinBox->value())));
#ifdef QT_DEBUG
    else if (ui->energyButton->isChecked())
        displayPixmap(QPixmap::fromImage(dtwImage->dumpEnergy()));
#endif
}

void MainWindow::onDetailRatioChanged(int newPercent)
{
    if (dtwImage != nullptr && ui->coloringPageButton->isChecked())
        displayPixmap(QPixmap::fromImage(dtwImage->makeColoringPage(newPercent)));
}

static void adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

void MainWindow::onZoomChanged(int newPercent)
{
    const double newScale = newPercent / 100.0;
    adjustScrollBar(ui->scrollArea->horizontalScrollBar(), newScale / scaleFactor);
    adjustScrollBar(ui->scrollArea->verticalScrollBar(), newScale / scaleFactor);
    scaleFactor = newScale;
    displayPixmap(displayedPixmap);
}

void MainWindow::on_loadButton_clicked()
{
    QStringList mimeTypeFilters;
    foreach (const QByteArray &mimeTypeName, QImageReader::supportedMimeTypes())
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    QFileDialog dialog(this, tr("Open File"),
                       picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
//    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

bool MainWindow::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage image = reader.read();
    if (image.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1.").arg(QDir::toNativeSeparators(fileName)));
        setWindowFilePath(QString());
        imageLabel->setPixmap(QPixmap());
        imageLabel->adjustSize();
        return false;
    }

    setWindowFilePath(fileName);
    loadImage(image);
    return true;
}

void MainWindow::on_printButton_clicked()
{
#if !defined(QT_NO_PRINTER) && !defined(QT_NO_PRINTDIALOG)
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec()) {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = displayedPixmap.size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(displayedPixmap.rect());
        painter.drawPixmap(0, 0, displayedPixmap);
    }
#endif
}

void MainWindow::on_saveButton_clicked()
{
    QStringList mimeTypeFilters;
    foreach (const QByteArray &mimeTypeName, QImageWriter::supportedMimeTypes())
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    QFileDialog dialog(this, tr("Save colouring page"),
                       picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");

    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first())) {}
}

bool MainWindow::saveFile(const QString &fileName)
{
    QImageWriter writer(fileName);
    if (!writer.canWrite()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot write file %1.").arg(QDir::toNativeSeparators(fileName)));
        return false;
    }
    writer.write(displayedPixmap.toImage());
    ui->statusBar->showMessage("Saved to file : " +  QDir::toNativeSeparators(fileName));
    return true;
}
