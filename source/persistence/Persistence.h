#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <QObject>

class Database;
class MessageController;
class SessionController;

class Persistence : public QObject
{
	Q_OBJECT
	Q_PROPERTY(MessageController* messageController READ getMessageController NOTIFY messageControllerChanged)
	Q_PROPERTY(SessionController* sessionController READ getSessionController NOTIFY sessionControllerChanged)

public:
	explicit Persistence(QObject *parent = 0);
	~Persistence();
	bool isValid();

signals:
	void messageControllerChanged();
	void sessionControllerChanged();

public slots:
	void addMessage(QString const &jid, QString const &message, unsigned int direction);
	void setCurrentChatPartner(QString const &jid);

private:
	MessageController* getMessageController();
	SessionController* getSessionController();

	Database *db_;
	MessageController *messageController_;
	SessionController *sessionController_;

	bool persistenceValid_;
};

#endif // PERSISTENCE_H
