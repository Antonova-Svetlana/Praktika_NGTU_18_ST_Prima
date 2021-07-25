/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include <QtNetwork>
#include <QtCore>
#include <QDateTime>

#include "sender.h"

Sender::Sender(QWidget *parent)
    : QWidget(parent)
{
    statusLabel = new QLabel(tr("Ready to broadcast datagrams on port 45454"));
    statusIP = new QLabel(tr("IP:"));
    inputIP = new QLineEdit();
    inputIP->setInputMask("000.000.000.000;_");

    startButton = new QPushButton(tr("&Start"));
    textIP = new QPushButton(tr("Set IP"));
    auto quitButton = new QPushButton(tr("&Quit"));
    comboBox = new QComboBox();

    auto buttonBox = new QDialogButtonBox;
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    auto ipBox = new QHBoxLayout;
    ipBox->addWidget(statusIP);
    ipBox->addWidget(inputIP);

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(45452, QUdpSocket::ShareAddress);

    connect(startButton, &QPushButton::clicked, this, &Sender::startBroadcasting);
    connect(quitButton, &QPushButton::clicked, this, &Sender::close);
    connect(textIP, &QPushButton::clicked, this, &Sender::getIP);
    connect(&timer, &QTimer::timeout, this, &Sender::broadcastDatagram);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(comboBox);
    mainLayout->addLayout(ipBox);
    mainLayout->addWidget(textIP);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
        {
             comboBox->addItem(address.toString());
        }
    }

    setIP = comboBox->currentText();
    setWindowTitle(tr("Broadcast Sender"));
}

void Sender::getIP()
{
    setIP = inputIP->text();
}

void Sender::startBroadcasting()
{
    timer.start(10000);
}

void Sender::broadcastDatagram()
{
    QString ntpServer = "0.ru.pool.ntp.org";
    QByteArray dataNtpServer;
    QHostInfo info = QHostInfo::fromName(ntpServer);
    udpSocket->connectToHost(QHostAddress(info.addresses().at(0)), 123);
    char message[48]= {010, 0, 0, 0, 0, 0, 0, 0, 0};

    if(udpSocket->writeDatagram(message, sizeof(message), QHostAddress(info.addresses().at(0)), 123)){
        if (udpSocket->waitForReadyRead()){
            while (udpSocket->hasPendingDatagrams()){
                QByteArray QBABufferIn = udpSocket->readAll();
                //udpSocket->readDatagram(dataNtpServer.data(), dataNtpServer.size());
                if (QBABufferIn.size()== 48){
                    int count= 40;
                    long long timestamp = QDateTime::currentSecsSinceEpoch();
                    qDebug() << "GidGood";
                    //qDebug() << uchar(QBABufferIn.at(count))+ (uchar(QBABufferIn.at(count+ 1)) << 8)+ (uchar(QBABufferIn.at(count+ 2)) << 16)+ (uchar(QBABufferIn.at(count+ 3)) << 24);
                    long long DateTimeIn= uchar(QBABufferIn.at(count))+ (uchar(QBABufferIn.at(count+ 1)) << 8)+ (uchar(QBABufferIn.at(count+ 2)) << 16)+ (uchar(QBABufferIn.at(count+ 3)) << 24);
                    qDebug() << DateTimeIn;
                    //long tmit = ntohl((time_t)DateTimeIn);
                    //tmit-= 2208988800U;
                    //QDateTime dateTime= QDateTime::fromTime_t(tmit);
                }
            }
        }
    }
    statusLabel->setText(tr("Now broadcasting datagram %1").arg(messageNo));
    QByteArray datagram = "Broadcast message " + QByteArray::number(messageNo);
    udpSocket->writeDatagram(datagram, QHostAddress(setIP), 45454);
    ++messageNo;
}
