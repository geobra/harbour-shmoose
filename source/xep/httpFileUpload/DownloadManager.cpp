/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "DownloadManager.h"
#include "System.h"
#include "CryptoHelper.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

DownloadManager::DownloadManager(QObject *parent) : QObject(parent)
{
    connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(downloadFinished(QNetworkReply*)));
}

void DownloadManager::doDownload(const QUrl &requestedUrl, const QString &msgId)
{
    // check if file from that url is already local downloaded
    QUrl url(requestedUrl);
    QString hash = CryptoHelper::getHashOfString(requestedUrl.toString(), true);
    QString pathAndFile = System::getAttachmentPath() + QDir::separator() + hash;

    if (url.scheme() == "aesgcm") {
        url.setScheme("https");
        url.setFragment(QString::null);
    }

    if (QFile::exists(pathAndFile))
    {
        qDebug() << "file from " << requestedUrl.toString() << " already exist localy as " << pathAndFile;
    }
    else
    {
        QNetworkRequest request(url);
        QNetworkReply *reply = manager.get(request);
        FileWithCypher *file = new FileWithCypher(pathAndFile, this);

        if (!file->open(QIODevice::ReadWrite))
        {
            fprintf(stderr, "Could not open %s for writing: %s\n",
                    qPrintable(file->fileName()),
                    qPrintable(file->errorString()));
        }
        else
        {
            file->initDecryptionOnWrite(requestedUrl.fragment());

            connect(reply, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));

            currentDownloads.append(reply);
            downloadedFiles[reply] = file;
            relatedMsgIds[reply] = msgId;
        }
    }
}


void DownloadManager::sslErrors(const QList<QSslError> &sslErrors)
{
    foreach (const QSslError &error, sslErrors)
    {
        fprintf(stderr, "SSL error: %s\n", qPrintable(error.errorString()));
    }
}


void DownloadManager::downloadFinished(QNetworkReply *reply)
{
    QUrl url = reply->url();
    FileWithCypher *file = downloadedFiles[reply];

    if (reply->error())
    {
        fprintf(stderr, "Download of %s failed: %s\n",
                url.toEncoded().constData(),
                qPrintable(reply->errorString()));
        emit httpDownloadFailed(relatedMsgIds[reply]);
    }
    else
    {        
        file->setExpectedSize(reply->header(QNetworkRequest::ContentLengthHeader).toLongLong());
        file->write(reply->readAll());
        file->close();
        emit httpDownloadFinished(relatedMsgIds[reply]);
    }

    file->deleteLater();

    downloadedFiles.remove(reply);
    currentDownloads.removeAll(reply);
    reply->deleteLater();

    if (currentDownloads.isEmpty())
    {
        // all downloads finished
        qDebug() << "finished all downloads";
    }
}

