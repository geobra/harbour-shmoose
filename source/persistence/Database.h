#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>

class Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent = 0);
    bool open(QString const &jid);
    bool isValid();
    QSqlDatabase* getPointer();

    void dumpDataToStdOut() const;

    static const QString sqlMsgName_;
    static const QString sqlMsgMessage_;
    static const QString sqlMsgDirection_;
    static const QString sqlMsgType_;
    static const QString sqlMsgState_;

    static const QString sqlSessionName_;
    static const QString sqlSessionLastMsg_;
    static const QString sqlSessionUnreadMsg_;

    static const QString sqlId_;
    static const QString sqlJid_;
    static const QString sqlResource_;
    static const QString sqlTimestamp_;

signals:

public slots:

private:
    bool databaseValid_;
    QSqlDatabase database_;
};

#endif // DATABASE_H
