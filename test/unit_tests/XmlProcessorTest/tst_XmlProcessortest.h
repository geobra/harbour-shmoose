#pragma once

#include <QObject>

class XmlProcessortest : public QObject
{
    Q_OBJECT

public:
    XmlProcessortest();
    ~XmlProcessortest();

private slots:
    void test_case1();

};
