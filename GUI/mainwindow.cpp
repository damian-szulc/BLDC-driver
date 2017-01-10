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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "commands.h"

#include <QMessageBox>
#include <QLabel>
#include <QtSerialPort/QSerialPort>
#include <QDebug>
#include <QFileDialog>
#include <QFile>

/*!
    \fn MainWindow::MainWindow(QWidget *parent)
    \brief Konstruktor klasy okna głównego

    Odpowiada za inicjalizację wszystkich niezbędnych narzędzi
*/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //inicjalizacja wybranych parametrów
    readBuffor = new QByteArray();
    ui->setupUi(this);
    //inicjalizacja obsługi portu szeregowego
    serial = new QSerialPort(this);
    //inicjalizacja okienka ustawień transmisji
    settings = new SettingsDialog;
    //inicjalizacja parametrów począktowych przycisków na menuBar
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionConfigure->setEnabled(true);
    ui->actionZbierajDane->setChecked(false);
    //zablokowanie wybranych kart - stan przy disconnected
    ui->tabWidget->widget(2)->setDisabled(true);
    ui->tabWidget->widget(1)->setDisabled(true);
    ui->tabWidget->widget(0)->setDisabled(true);
    //inicjalizacja paska statusu
    status = new QLabel;
    ui->statusBar->addWidget(status);
    //inicjalizacja połączeń
    initActionsConnections();
    //inicjalizacja wykresów
    initGraphs();
    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleError(QSerialPort::SerialPortError)));
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
}

/*!
    \fn MainWindow::~MainWindow()
    \brief Destruktor klasy okna głównego
*/
MainWindow::~MainWindow()
{
    delete settings;
    delete ui;
}

/*!
    \fn void MainWindow::initActionsConnections()
    \brief Inicjalizacja połączeń sygnałów ze slotami

*/
void MainWindow::initActionsConnections()
{
    connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(openSerialPort()));
    connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeSerialPort()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionConfigure, SIGNAL(triggered()), settings, SLOT(show()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

}

/*!
    \fn void MainWindow::openSerialPort()
    \brief Otwarcie portu szeregowego

    Inicjalizacja połączenia:
    -otwarcie portu
    -zmiany w GUI - uaktywnienie wybranych elementów
*/
void MainWindow::openSerialPort()
{
    //pobranie ustawień transmisji
    SettingsDialog::Settings p = settings->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    //próba otwarcia portu
    if (serial->open(QIODevice::ReadWrite)) {
        //włączenie wybranych ikon
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionConfigure->setEnabled(false);
        //włączenie wybranych elementów
        ui->tabWidget->widget(1)->setDisabled(false);
        ui->tabWidget->widget(0)->setDisabled(false);
        ui->tabWidget->widget(2)->setDisabled(false);

        //wysłanie komendy nawiązania połączenia do mikrokontrolera
        sendCommand(QString(CMD_HELLO));
        //aktualizacja pasku statusu
        showStatusMessage(tr("Port otwarty %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } else {
        //błąd w otwieraniu połączenia
        QMessageBox::critical(this, tr("Error"), serial->errorString());
        //aktualizacja statusu
        showStatusMessage(tr("Open error"));
    }
}

/*!
    \fn void MainWindow::closeSerialPort()
    \brief Zamknięcie portu szeregowego

    Zakończenie połączenia:
    -zamknięcie portu
    -wyłączenie wybranych ikon
*/
void MainWindow::closeSerialPort()
{
    //zamknięcie portu
    if (serial->isOpen())
        serial->close();

    //wyłączenie wybranych ikon
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionConfigure->setEnabled(true);
    //wyłączenie wybranych kart
    ui->tabWidget->widget(1)->setDisabled(true);
    ui->tabWidget->widget(0)->setDisabled(true);
    ui->tabWidget->widget(2)->setDisabled(true);
    //aktualizacja statusu
    showStatusMessage(tr("Rozłączony.."));
}

/*!
    \fn void MainWindow::handleError(QSerialPort::SerialPortError error)
    \brief Obsługa błędu w porcie szeregowym
*/
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Błąd krytyczny"), serial->errorString());
        closeSerialPort();
    }
}

/*!
    \fn void MainWindow::about()
    \brief Otwarcie okna informacji o aplikacji

    Wyświetlenie informacji o aplikacji BLDC.
*/
void MainWindow::about()
{
    QMessageBox::about(this, tr("Informacje BLDC"),
                       tr("<b>BLDC</b><br/><br/>Aplikacja BLDC została zrealizowana jako projekt inżynierski pt: \"Sterowanie bezczujnikowe silnikiem BLDC\". <br/><br/>Wersja z 1 grudnia 2016 <br /><br/> Autor: Szulc Damian"));
}

/*!
    \fn void MainWindow::writeData(const QByteArray &data)
    \brief Wysłanie danych i zapisanie w konsoli
*/
void MainWindow::writeData(const QByteArray &data)
{
    //wyślij informacje do uC
    serial->write(data);


}

/*!
    \fn void MainWindow::readData()
    \brief Odczytaj otrzymane dane
*/
void MainWindow::readData()
{
    //odczytanie otrzymanych danych
    QByteArray data = serial->readAll();
    //jeżeli otrzymano więcej niz rozmiar bufora, nic nie rób
    if(data.length() > 5)
        return;
    //dodaj dane do bufora
    readBuffor->append(data);
    //jeżeli nie zapełniono jeszcze bufora, nic nie rób
    if(readBuffor->length() < 5)
        return;
    //błąd jeżeli przepełniono bufor
    else if(readBuffor->length() > 5) {
        readBuffor->clear();
        return;
    }
    //obsłuż odebrany kod, jeżeli otrzymano dokładnie 5 znaków
    else {
        handleRecivedMessage(*readBuffor);
        readBuffor->clear();
    }

}

/*!
    \fn void MainWindow::showStatusMessage(const QString &message)
    \brief Wyświetl status na pasku statusu
*/
void MainWindow::showStatusMessage(const QString &message)
{
    status->setText(message);
}

/*!
    \fn void MainWindow::sendCommand(QString cmd, QString cmd2, QString cmd3,  QString cmd4, QString cmd5) {
    \brief Wyślij komendę do uC
*/
void MainWindow::sendCommand(QString cmd, QString cmd2, QString cmd3,  QString cmd4, QString cmd5) {
    //formuła komunikacji'
    if(cmd2 == QString("0"))
        cmd2 = QString(CMD_SPACER);
    if(cmd3 == QString("0"))
        cmd3 = QString(CMD_SPACER);
    if(cmd4 == QString("0"))
        cmd4 = QString(CMD_SPACER);
    if(cmd5 == QString("0"))
        cmd5 = QString(CMD_SPACER);
    QString msg = cmd + cmd2+cmd3+cmd4+cmd5 ;
    //wysyłanie
    serial->write(msg.toUtf8());
}

/*!
    \fn void MainWindow::sendValue(QString cmd, QString cmd2, QString cmd3, int value)
    \brief Wyślij wartość
*/
void MainWindow::sendValue(QString cmd, QString cmd2, QString cmd3, int value) {
    //formuła komunikacji'
    if(cmd2 == QString("0"))
        cmd2 = QString(CMD_SPACER);
    if(cmd3 == QString("0"))
        cmd3 = QString(CMD_SPACER);
    QString msg = cmd + cmd2 + cmd3;
    //konwersja komend do pożądanego formatu
    QByteArray data = msg.toUtf8();
    //konwersja liczby do odpowiedniego formatu
    QString hexadecimal;
    hexadecimal.setNum(value,16);
    if(hexadecimal.length() %2) hexadecimal.insert(0, QLatin1String("0") );
    if(hexadecimal.length() == 2)
        hexadecimal = "00" + hexadecimal;
    if(hexadecimal.length() == 3)
        hexadecimal = "0" + hexadecimal;
    QByteArray valueByte = QByteArray::fromHex( hexadecimal.toLatin1() );
    data.append(valueByte);
    //wysyłanie danych
    serial->write(data);

}

/*!
    \fn void MainWindow::sendValue2(QString cmd, int value, int value2)
    \brief Wyślij dwie wartości
*/
void MainWindow::sendValue2(QString cmd, int value, int value2) {
    //przygotowanie bufora
    QByteArray data = cmd.toUtf8();
    //konwersja wartości 1 do odp formatu
    QString hexadecimal;
    hexadecimal.setNum(value,16);
    if(hexadecimal.length() %2) hexadecimal.insert(0, QLatin1String("0") );
    if(hexadecimal.length() == 2)
        hexadecimal = "00" + hexadecimal;
    if(hexadecimal.length() == 3)
        hexadecimal = "0" + hexadecimal;
    QByteArray valueByte = QByteArray::fromHex( hexadecimal.toLatin1() );
    data.append(valueByte);
    //konwersja wartości 2 do odp formatu
    QString hexadecimal2;
    hexadecimal2.setNum(value2,16);
    if(hexadecimal2.length() %2) hexadecimal2.insert(0, QLatin1String("0") );
    if(hexadecimal2.length() == 2)
        hexadecimal2 = "00" + hexadecimal2;
    if(hexadecimal2.length() == 3)
        hexadecimal2 = "0" + hexadecimal2;
    QByteArray valueByte2 = QByteArray::fromHex( hexadecimal2.toLatin1() );
    data.append(valueByte2);
    //wysłanie danych
    serial->write(data);
}

/*!
    \fn void MainWindow::handleRecivedMessage(QByteArray data)
    \brief Obsłuż otrzymaną wiadomość
*/
void MainWindow::handleRecivedMessage(QByteArray data)
{
    //pobranie pierwszego znaku
    QString firstSign = QString(data.data()[0]);
    //jeżeli otrzymano dane o pomiarze prędkości i wypelnienia
    if(firstSign == QString(CMD2_POMIARY_PRED)) {

        QByteArray predHex = data.mid(1, 2);
        QString predHexString =  QString(QString(predHex.toHex()).toUtf8());
        QByteArray wypeHex = data.mid(3, 4);
        QString wypeHexString =  QString(QString(wypeHex.toHex()).toUtf8());
        //konwersja otrzymanych wartości prędkości
        bool ok = false;
        uint predkosc = predHexString.toUInt(&ok,16);
        double pred = predkosc;
        if(predkosc != 0)
            pred = 400000.0 * 60 / (6*predkosc * ui->liczbaParBiegun->value());
        //konwersja otrzymanych wartości prędkości
        bool ok2 = false;
        uint wypelnienie = wypeHexString.toUInt(&ok2,16);
        double wyp = wypelnienie;
        if(wypelnienie != 0)
            wyp = (wypelnienie + 1.0) /5;
        if(wyp <= 100) {
            //wyświetlenie otrzymanych danych
            ui->estPred->setText(QString::number( pred));
            ui->estD->setText(QString::number( wyp, 'f', 2));
            if(!ui->actionZbierajDane->isChecked()) {
                //wyświetl dane na wykresie

                updateGraph(&dutyVec,ui->rpmGraph, wyp, 1);
                updateGraph(&rpmVec,ui->rpmGraph, pred);

                updateGraph(&dutyVec2,ui->rpmGraph2, wyp, 1);
                updateGraph(&rpmVec2,ui->rpmGraph2, pred);
            }
        }
    }
}

/*!
    \fn void MainWindow::on_startSL_released()
    \brief Wysłanie danych o uruchomieniu silnika
*/
void MainWindow::on_startSL_released()
{
    //przeliczenia
    int val = (ui->dZad->value() - 0.2)*5;
    if(val == 500)
        val = 499;
    //wysłanie danych uC
    sendValue(QString(CMD_STER), QString(CMD_START), "0", val);
}

/*!
    \fn void MainWindow::on_stop_released()
    \brief Wysłanie danych o zatrzymaniu silnika
*/
void MainWindow::on_stop_released()
{
    sendCommand(QString(CMD_STER), QString(CMD_STOP));
}

/*!
    \fn void MainWindow::on_zerowanieT_valueChanged(int arg1)
    \brief Zmiana zadanej wartości czasu zerowania

    Po przeliczeniach, nowa wartość czasu zostaje wysłana do uC
*/
void MainWindow::on_zerowanieT_valueChanged(int arg1)
{
    sendValue(QString(CMD_PARAM), QString(CMD_ZER), QString(CMD_ZER_T), arg1);
}

/*!
    \fn void MainWindow::on_zerowanieD_valueChanged(int arg1)
    \brief Zmiana zadanej wartości wypełnienia podczas zerowania

    Po przeliczeniach, nowa wartość wypełnienia zostaje wysłana do uC
*/
void MainWindow::on_zerowanieD_valueChanged(int arg1)
{
    //przeliczanie 0 - 100
    arg1 -= 1;
    arg1 = arg1 * 499 / 99;
    sendValue(QString(CMD_PARAM), QString(CMD_ZER), QString(CMD_ZER_D), arg1);

}

/*!
    \fn void MainWindow::on_rolT_valueChanged(int arg1)
    \brief Zmiana zadanej wartości czasu rozruchu w pętli otwartej

    Po przeliczeniach, nowa wartość czasy zostaje wysłana do uC
*/
void MainWindow::on_rolT_valueChanged(int arg1)
{
    sendValue(QString(CMD_PARAM), QString(CMD_ROZR_OTW), QString(CMD_ROZR_OTW_T), arg1);
}

/*!
    \fn void MainWindow::on_rolFmin_valueChanged(int arg1)
    \brief Zmiana zadanej wartości minimalnej częstotliwości w rozruchu w pętli otwartej

    Po przeliczeniach, nowa wartość częstotliwości zostaje wysłana do uC
*/
void MainWindow::on_rolFmin_valueChanged(int arg1)
{
    sendValue(QString(CMD_PARAM), QString(CMD_ROZR_OTW), QString(CMD_ROZR_OTW_Fmin), arg1*6);
}

/*!
    \fn void MainWindow::on_rolFmax_valueChanged(int arg1)
    \brief Zmiana zadanej wartości maksymalnej częstotliwości w rozruchu w pętli otwartej

    Po przeliczeniach, nowa wartość częstotliwości zostaje wysłana do uC
*/
void MainWindow::on_rolFmax_valueChanged(int arg1)
{
    sendValue(QString(CMD_PARAM), QString(CMD_ROZR_OTW), QString(CMD_ROZR_OTW_Fmax), arg1*6);
}

/*!
    \fn MainWindow::on_rolDmin_valueChanged(int arg1)
    \brief Zmiana zadanej wartości minimalnej wypełnienia w rozruchu w pętli otwartej

    Po przeliczeniach, nowa wartość wypełnienia zostaje wysłana do uC
*/
void MainWindow::on_rolDmin_valueChanged(int arg1)
{
    //przeliczanie 1 - 100
    arg1 -= 1;
    arg1 = arg1 * 499 / 99;
    sendValue(QString(CMD_PARAM), QString(CMD_ROZR_OTW), QString(CMD_ROZR_OTW_Dmin), arg1);
}

/*!
    \fn MainWindow::on_rolDmax_valueChanged(int arg1)
    \brief Zmiana zadanej wartości max wypełnienia w rozruchu w pętli otwartej

    Po przeliczeniach, nowa wartość wypełnienia zostaje wysłana do uC
*/
void MainWindow::on_rolDmax_valueChanged(int arg1)
{
    arg1 -= 1;
    arg1 = arg1 * 499 / 99;
    sendValue(QString(CMD_PARAM), QString(CMD_ROZR_OTW), QString(CMD_ROZR_OTW_Dmax), arg1);
}

/*!
    \fn void MainWindow::on_horizontalSlider_valueChanged(int value)
    \brief Zmiana zadanej wartości rampy

    nowa wartość rampy zostaje wysłana do uC
*/
void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    sendValue(QString(CMD_PARAM), QString(CMD_RAMPA), QString(CMD_SPACER), value);
}

/*!
    \fn void MainWindow::on_dZad_valueChanged(double arg1)
    \brief Zmiana zadanej wartości wypełnienia

    Po przeliczeniach, nowa wartość wypełnienia zostaje wysłana do uC
*/
void MainWindow::on_dZad_valueChanged(double arg1)
{
    //przeliczenia
    int val = (arg1 - 0.2)*5;
    if(val == 500)
        val = 499;
    //wysłanie danych uC
    sendValue(QString(CMD_PARAM), QString(CMD_PRACA), QString(CMD_PRACA_WYP), val);
}

/*!
    \fn void MainWindow::on_pushButton_released()
    \brief Zapis do pliku CSV
*/
void MainWindow::on_pushButton_released()
{
    //wybierz plik
    QString fileName = QFileDialog::getSaveFileName();
    //otwórz plik
    QFile file(fileName);
    file.open( QIODevice::WriteOnly);
    QTextStream outStream(&file);
    //generowanie danych do zapisu

    //częstotliwość pomiarów
    float fsamp = 1000.0/KOMUNIKACJA_CO_MS;
    //zapisywanie danych
    for (int i = 0;i < rpmVec.size();i++)
        outStream << QString::number((double)i/fsamp, 'f', 4) + ";" + QString::number(rpmVec.at(i)) + ";" + QString::number(dutyVec.at(i)) + "\n";
    //zamknij plik
    file.close();
}


/*!
    \fn void MainWindow::initGraphs()
    \brief Inicjalizacja wykresów

    Inicjalizacja wykresów:
    -podpisanie osi
    -dobranie odpowiednich markerów
    -kolory
*/
void MainWindow::initGraphs()
{
    //czcionka na wykresach
    QFont legendFont = font();
    legendFont.setPixelSize(14);
    ui->rpmGraph->addGraph(ui->rpmGraph->xAxis, ui->rpmGraph->yAxis);
    ui->rpmGraph->graph(0)->setPen(QPen(Qt::blue));
    ui->rpmGraph->graph(0)->setName("Prędkość obrotowa2");
    ui->rpmGraph->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignBottom);
    ui->rpmGraph->legend->setFont(legendFont);
    ui->rpmGraph->legend->setBrush(QBrush(QColor(255,255,255,230)));
    //inicjalizacja osi X
    ui->rpmGraph->xAxis->setLabelFont(legendFont);
    ui->rpmGraph->xAxis->setTickLabelFont(legendFont);
    ui->rpmGraph->xAxis->setLabel("Sekundy (s)");
    //inicjalizacja lewej osi y
    ui->rpmGraph->yAxis->setLabel("Prędkość obrotowa, obr/min");
    ui->rpmGraph->yAxis->setLabelFont(legendFont);
    ui->rpmGraph->yAxis->setTickLabelFont(legendFont);
    ui->rpmGraph->yAxis->setLabelColor(QColor(Qt::blue));
    //inicjalizacja prawej osi y
    ui->rpmGraph->yAxis2->setLabel("Wypełnienie, %");
    ui->rpmGraph->yAxis2->setLabelFont(legendFont);
    ui->rpmGraph->yAxis2->setTickLabelFont(legendFont);
    ui->rpmGraph->yAxis2->setVisible(true);
    ui->rpmGraph->yAxis2->setLabelColor(QColor(Qt::red));
    ui->rpmGraph->addGraph(ui->rpmGraph->xAxis, ui->rpmGraph->yAxis2);
    ui->rpmGraph->graph(1)->setPen(QPen(Qt::red));
    ui->rpmGraph->graph(1)->setName("Wypełnienie");

    //czcionka na pozostałych wykresach
    legendFont.setPixelSize(10);

    //inicjalizacja drugiego wykresu
    ui->rpmGraph2->addGraph(ui->rpmGraph2->xAxis, ui->rpmGraph2->yAxis);
    ui->rpmGraph2->graph(0)->setPen(QPen(Qt::blue));
    ui->rpmGraph2->graph(0)->setName("Prędkość obrotowa2");
    ui->rpmGraph2->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignBottom);
    ui->rpmGraph2->legend->setFont(legendFont);
    ui->rpmGraph2->legend->setBrush(QBrush(QColor(255,255,255,230)));
    //inicjalizacja osi X
    ui->rpmGraph2->xAxis->setLabelFont(legendFont);
    ui->rpmGraph2->xAxis->setLabelPadding(0);
    ui->rpmGraph2->xAxis->setPadding(0);
    ui->rpmGraph2->xAxis->setTickLabelFont(legendFont);
    ui->rpmGraph2->xAxis->setLabel("Sekundy (s)");
    //inicjalizacja lewej osi y
    ui->rpmGraph2->yAxis->setLabel("Prędkość obrotowa, obr/min");
    ui->rpmGraph2->yAxis->setLabelFont(legendFont);
    ui->rpmGraph2->yAxis->setLabelPadding(0);
    ui->rpmGraph2->yAxis->setPadding(0);
    ui->rpmGraph2->yAxis->setTickLabelFont(legendFont);
    ui->rpmGraph2->yAxis->setLabelColor(QColor(Qt::blue));
    //inicjalizacja prawej osi y
    ui->rpmGraph2->yAxis2->setLabel("Wypełnienie, %");
    ui->rpmGraph2->yAxis2->setLabelFont(legendFont);
    ui->rpmGraph2->yAxis2->setLabelPadding(0);
    ui->rpmGraph2->yAxis2->setPadding(0);
    ui->rpmGraph2->yAxis2->setTickLabelFont(legendFont);
    ui->rpmGraph2->yAxis2->setVisible(true);
    ui->rpmGraph2->yAxis2->setLabelColor(QColor(Qt::red));
    ui->rpmGraph2->addGraph(ui->rpmGraph2->xAxis, ui->rpmGraph2->yAxis2);
    ui->rpmGraph2->graph(1)->setPen(QPen(Qt::red));
    ui->rpmGraph2->graph(1)->setName("Wypełnienie");

}

/*!
    \fn void MainWindow::updateGraph(QVector<double> *vec, QCustomPlot *plot, double value, int graph)
    \brief Aktualizacja wyświetlanych danych

*/
void MainWindow::updateGraph(QVector<double> *vec, QCustomPlot *plot, double value, int graph)
{
    //częstotliwość próbkowania
    float fsamp = 1000.0/KOMUNIKACJA_CO_MS;
    //dodanie danych do bazy
    vec->append(value);
    //usunięcie wybranych danych jeżeli przekroczono limit utrzymywanych danych
    if(vec->size() > ui->graphMaxSize->value()) {
        vec->remove(0, vec->size() - ui->graphMaxSize->value());
    }
    //rozmiar zgromadzonych danych
    int dataSize = vec->size();
    //generowanie wektora X
    QVector<double> xAxis(dataSize);
    //wartość maksymalna na danej osi
    double max = 0.1;
    for (int i = 0;i < vec->size();i++) {
        if(vec->at(i) > max)
            max = vec->at(i);

        xAxis[i] = (double)i / fsamp;
    }
    //wyświetlenia danych na wykresie
    plot->graph(graph)->setData(xAxis, *vec);
    //przeskalowanie osi Y
    max = max*1.1;
    if(graph == 0)
        plot->yAxis->setRange(0, max);
    else
        plot->yAxis2->setRange(0, max);
    //przeskalowanie osi x
    plot->xAxis->setRange(0, dataSize / fsamp);
    //wyświetl jeszcze raz
    plot->replot();
}

