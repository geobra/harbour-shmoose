#pragma once

#include <QObject>

class Shmoose;
class CapturePicture;

class CommandProcessor : public QObject
{
    Q_OBJECT
public:
    explicit CommandProcessor(Shmoose* shmoose, QObject *parent = nullptr);

signals:

public slots:
    void messageReceived(QString from, QString msg);
    void sendPicture(int id, const QString& filename);

private:
    Shmoose* shmoose_;
    CapturePicture* capturePicture_;
};
