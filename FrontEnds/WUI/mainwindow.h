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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QLabel>
#include <QString>

#include "dtwimage.h"


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void loadImage(const QImage& image);

public slots:
    void displayModeToggled(bool checked);
    void onDetailRatioChanged(int);
    void onZoomChanged(int);

private slots:
    void on_loadButton_clicked();

    void on_printButton_clicked();

    void on_saveButton_clicked();

private:
    Ui::MainWindow *ui;    
    QLabel * imageLabel;

    QPixmap originalPixmap;
    QPixmap displayedPixmap;

    dtw::DtwImage *dtwImage;

    double scaleFactor;

    void displayPixmap(const QPixmap& image);
    bool loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);

};

#endif // MAINWINDOW_H
