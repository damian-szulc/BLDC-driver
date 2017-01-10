/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QtGlobal>
#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include "qcustomplot.h"
QT_BEGIN_NAMESPACE

class QLabel;

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class Console;
class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    //Otwarcie portu szeregowego
    void openSerialPort();
    //Zamknięcie portu szeregowego
    void closeSerialPort();
    //Otwarcie okna informacji o aplikacji
    void about();
    //Wysłanie danych i zapisanie w konsoli
    void writeData(const QByteArray &data);
    //Odczytaj otrzymane dane
    void readData();
    //Obsługa błędu w porcie szeregowym
    void handleError(QSerialPort::SerialPortError error);
    //Wyślij komendę do uC
    void sendCommand(QString cmd, QString cmd2="0",QString cmd3="0",  QString cmd4="0", QString cmd5="0");
    //Wyślij wartość
    void sendValue(QString cmd, QString cmd2="0",QString cmd3="0", int value=0);
    //Wyślij dwie wartości
    void sendValue2(QString cmd, int value, int value2);
    //Wysłanie danych o uruchomieniu silnika
    void on_startSL_released();
    //wysłanie danych o zatrzymaniu silnika
    void on_stop_released();
    //Zmiana zadanej wartości czasu zerowania
    void on_zerowanieT_valueChanged(int arg1);
    //Zmiana zadanej wartości wypełnienia podczas zerowania
    void on_zerowanieD_valueChanged(int arg1);
    //Zmiana zadanej wartości czasu rozruchu w pętli otwartej
    void on_rolT_valueChanged(int arg1);
    //Zmiana zadanej wartości minimalnej częstotliwości w rozruchu w pętli otwartej
    void on_rolFmin_valueChanged(int arg1);
    //Zmiana zadanej wartości maksymalnej częstotliwości w rozruchu w pętli otwartej
    void on_rolFmax_valueChanged(int arg1);
    //Zmiana zadanej wartości minimalnej wypełnienia w rozruchu w pętli otwartej
    void on_rolDmin_valueChanged(int arg1);
    //Zmiana zadanej wartości max wypełnienia w rozruchu w pętli otwartej
    void on_rolDmax_valueChanged(int arg1);
    //Zmiana zadanej wartości wypełnienia
    void on_dZad_valueChanged(double arg1);
    //Zapis do pliku CSV
    void on_pushButton_released();



    void on_horizontalSlider_valueChanged(int value);

private:
    //Inicjalizacja połączeń sygnałów ze slotami
    void initActionsConnections();
    //Inicjalizacja wykresów
    void initGraphs();
    //Aktualizacja wyświetlanych danych
    void updateGraph(QVector<double> *vec, QCustomPlot *plot, double value, int graph =0);
    //Obsłuż otrzymaną wiadomość
    void handleRecivedMessage(QByteArray data);

private:
    //Wyświetl status na pasku statusu
    void showStatusMessage(const QString &message);

    Ui::MainWindow *ui;
    QLabel *status;
    Console *console;
    SettingsDialog *settings;
    QSerialPort *serial;

    QByteArray *readBuffor;
    QVector<double> dutyVec;
    QVector<double> rpmVec;
    QVector<double> dutyVec2;
    QVector<double> dutyVec3;
    QVector<double> rpmVec2;
    int graphMaxData;
};

#endif // MAINWINDOW_H
