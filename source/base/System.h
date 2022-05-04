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
    Q_INVOKABLE static QString getOmemoPath();
    Q_INVOKABLE static QString getUniqueResourceId();
};

#endif // SYSTEM_H
