#include "qjson.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QStringList>

QJson::QJson()
{

}

/*
 * Json最多两层
 */
void QJson::json_add(QJsonObject &root,QString key,QString value)
{
        if(key.contains('.'))
        {
            QString frontkey=key.mid(0,key.indexOf('.'));
            QString backdkey=key.mid(key.indexOf('.')+1);
            QJsonObject tempobj;
            int i=0;
            for(QString k : root.keys())
            {
                if(k==frontkey)
                {
                   //qDebug()<<"found, insert backdkey:"<<backdkey<<endl;
                   tempobj=root[k].toObject();
                   break;
                }
            }
            tempobj.insert(backdkey,value);
            //qDebug()<<"not found, insert backdkey & frontkey:"<<backdkey<<endl;
            root.insert(frontkey,tempobj);
            return;
        }
        root.insert(key,value);
}
//find Json data
QJsonValue QJson::json_find(QJsonValue root,QString key)
{
    if(root.isArray() && root.toArray().size()==0)return NULL;
    if(root.isObject() && root.toObject().size()==0)return NULL;
    if(root.isObject())
    {
        QString fkey;
        if(key.contains('.'))
            fkey=key.mid(0,key.indexOf('.'));
        else
            fkey=key;
        for(QString k : root.toObject().keys())
        {
            if(k==fkey)
            {
                if(fkey==key)//查找到的是，参数key的最后子键
                {
                    //qDebug()<<root.toObject()[k].toString()<<endl;
                    return root.toObject()[k];
                }
                else
                {
                    return json_find(root.toObject()[k],key.mid(key.indexOf('.')+1));
                }
            }
        }
        return 0;
    }
    else if(root.isArray())
    {
        for(int i=0;i<root.toArray().size();i++)
        return json_find(root.toArray().at(i),key);
    }
    return NULL;
}

/*
void QJson::json_add_test(QJsonObject &root,char *type,int dataSize,...)
{
    const char *dname;
    const char *dvalue;
    QJsonObject jsontype;
    QJsonArray jsondataArray;

    va_list arg_ptr;
    va_start(arg_ptr, dataSize);
    while(dataSize)
    {
        dname = va_arg(arg_ptr, char *);
        dvalue = va_arg(arg_ptr, char *);
        jsondataArray.insert(0,QJsonObject({QPair<QString, QJsonValue>( QString(dname),dvalue)}));
        dataSize-=1;
    }
    va_end(arg_ptr);
    jsontype.insert("type", QString(type));
    // QJsonObject ()  std::initializer_list<QPair<QString, QJsonValue>>
    root.insert(0,jsontype);
    root.insert(1,QJsonObject({QPair<QString, QJsonValue>( QString("data"),jsondataArray)}));
}
*/
