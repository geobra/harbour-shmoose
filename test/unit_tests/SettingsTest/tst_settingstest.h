#pragma once

#include <QObject>

class SettingsTest : public QObject
{
    Q_OBJECT

public:
    SettingsTest();
    ~SettingsTest();

private slots:
    void test_case1();

};
