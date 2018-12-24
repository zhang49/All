#ifndef QJSON_H
#define QJSON_H

#include <QJsonObject>

class QJson
{
public:
    QJson();
    static QJsonValue json_find(QJsonValue root,QString key);
    static void json_add(QJsonObject &root,QString key,QString value);
    static void json_add_test(QJsonObject &root,char *type,int dataSize,...);
};

#endif // QJSON_H
