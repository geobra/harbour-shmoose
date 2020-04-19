#ifndef SYSTEM_H
#define SYSTEM_H

#include <QObject>
#include <QString>

// https://forum.qt.io/topic/61543/qstandardpaths-not-available-in-qml/11
class System : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE static QString getAttachmentPath();
    Q_INVOKABLE static QString getAvatarPath();
};

#endif // SYSTEM_H
