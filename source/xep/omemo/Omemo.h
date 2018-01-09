#ifndef OMEMO_H
#define OMEMO_H

#include <QObject>

class Omemo : public QObject
{
    Q_OBJECT
public:
    explicit Omemo(QObject *parent = 0);

signals:

public slots:

private:
    void omemo_default_crypto_init(void);
};

#endif // OMEMO_H
