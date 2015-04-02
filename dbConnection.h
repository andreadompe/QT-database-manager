#ifndef DBCONNECTION_H
#define DBCONNECTION_H

#include <libpq-fe.h>
#include <QVariant>
#include <QObject>
#include <QThread>
#include <QStandardItemModel>

class dbConnection : public QObject
{
    Q_OBJECT
public:
    dbConnection();
    ~dbConnection();
    bool CloseConnection();
    bool Connect(QString dbAddress, QString dbName, QString port, QString password, QString user);
    bool execQuery(QString sQuery, bool bLog, QStandardItemModel **pModel = 0);
    QString errDescr;   // Error description
    QString err;

    /* getField methods */
    bool getFieldBool(QString field, QString sQuery);
    bool getFieldBool(QString field, QStandardItemModel *model, int iRow = 0);
    double getFieldDbl(QString field, QStandardItemModel *model, int iRow = 0);
    float getFieldFloat(QString field, QString sQuery);
    float getFieldFloat(QString field, QStandardItemModel *model, int iRow = 0);
    int getFieldInt(QString field, QString sQuery);
    int getFieldInt(QString field, QStandardItemModel *model, int iRow = 0);
    QString getFieldStr(QString field, QString sQuery);
    QString getFieldStr(QString field, QStandardItemModel *model, int iRow = 0);

private:
    PGconn *conn; 
    void log(QString text);
    QString dbAddress;
    QString dbName;
    QString dbPort;

    int getModelIndex(QString field, QStandardItemModel *model);
signals:
    void LogError(QString msg);
    void FromDatabase(const QString &msg, const QString &mitt, const QVariant &data);
};

extern dbConnection db;

#endif // DBCONNECTION_H
