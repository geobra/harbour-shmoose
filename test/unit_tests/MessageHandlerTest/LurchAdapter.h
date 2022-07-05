#pragma once

#include <QObject>
#include <QString>

#include <Swiften/Swiften.h>

class LurchAdapter : public QObject
{
    Q_OBJECT
public:
    explicit LurchAdapter(QObject *parent = nullptr) {}
    ~LurchAdapter() {}

    int decryptMessageIfEncrypted(Swift::Message::ref message)
    {
        return 1; // was not encrypted
    }

    void callLurchCmd(const std::vector<std::string>& sl)
    {}

    bool isOmemoUser(const QString& bareJid)
    {
        return false;
    }

    bool exchangePlainBodyByOmemoStanzas(Swift::Message::ref msg)
    {
        return false;
    }

signals:
    void rawMessageStanzaForSending(QString);

};
