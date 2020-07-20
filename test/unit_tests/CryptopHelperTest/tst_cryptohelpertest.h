#ifndef TST_CRYPTOHELPERTEST_H
#define TST_CRYPTOHELPERTEST_H

#include <QObject>

class cryptohelpertest : public QObject
{
    Q_OBJECT

public:
    cryptohelpertest();
    ~cryptohelpertest();

private slots:
    void test_case1();

};

#endif // TST_CRYPTOHELPERTEST_H
