#include "dbConnection.h"
#include <QDebug>
#include <QMessageBox>

dbConnection db;

dbConnection::dbConnection()
{
    conn = NULL;
}

dbConnection::~dbConnection()
{
    if(conn)
        PQfinish(conn);
}

bool dbConnection::CloseConnection(){
    if(conn){
        PQfinish(conn);
        conn = NULL;
        return true;
    }
    return false;
}

/**
 * @brief Database::Connect
 * @author Andrea Dompe'
 * @param dbAddress
 * @param dbName
 */
bool dbConnection::Connect(QString dbAddress, QString dbName, QString port, QString password, QString user)
{
    this->dbAddress = dbAddress;
    this->dbName = dbName;
    this->dbPort = port;

    QString strConnection;
    for(int i = 0; conn == NULL && i < 60; i++)
    {
        strConnection = QString("host = \'%1\' port = \'%3\' dbname = \'%2\' user = \'%3\' password = \'%4\' connect_timeout = 50").arg(dbAddress).arg(dbName).arg(port).arg(user).arg(password);        
        conn = PQconnectdb(strConnection.toLatin1().data());
        QThread::msleep(1);
    }
    qDebug() << PQerrorMessage(conn);

    return (PQstatus(conn)==CONNECTION_OK);
}

/**
 * @brief Database::log
 * @param text
 */
void dbConnection::log(QString text){
    QString sQuery = QString("INSERT INTO LOG(tipo, testo, accounts_id) VALUES (0, '%1', %2)").arg(text.replace("'", "''"), sAccountID);
    execQuery(sQuery, false);
}

/**
 * @brief Database::execQuery
 * @param query
 * @param bLog
 * @return
 */
bool dbConnection::execQuery(QString sQuery, bool bLog, QStandardItemModel **pModel)
{
    bool ret = false;

    if(pModel != 0)
        *pModel = 0;

    if(bLog)
        log(sQuery);

    if(PQstatus(conn)!=CONNECTION_OK){
        //qDebug() << "PQstatus(conn)-->" << QString::number(PQstatus(conn));
        if(!Connect(dbAddress, dbName, dbPort)){
            PQreset(conn);
            if(PQstatus(conn)==CONNECTION_BAD){
                qFatal("Database error could not connect to PostgreSQL.");
                return false;
            }
        }
    }

    try {

        PGresult *result = PQexec(conn, sQuery.toUtf8().data());

        err.clear();
        errDescr.clear();
        if(result)
        {
            int retcode = PQresultStatus(result);
            ret = (retcode != PGRES_FATAL_ERROR);
            if(ret && retcode == PGRES_TUPLES_OK && pModel != 0)
            {
                int col, row;
                int nf = PQnfields(result);
                int nt = PQntuples(result);
                QStandardItemModel *model = new QStandardItemModel(0, nf, NULL);
                if(model)
                {
                    for(col = 0; col < nf; col++)
                        model->setHeaderData(col, Qt::Horizontal, PQfname(result, col));

                    for(row = 0; row < nt; row++)
                    {
                        model->insertRow(row);
                        for(col = 0; col < nf; col++)
                            model->setData(model->index(row, col), PQgetvalue(result, row, col));
                    }
                }
                *pModel = model;
            }
            else if(retcode == PGRES_BAD_RESPONSE || retcode == PGRES_NONFATAL_ERROR || retcode == PGRES_FATAL_ERROR)
            {
                err = PQresStatus(PQresultStatus(result));
                errDescr = PQresultErrorMessage(result);
            }
            PQclear(result);
        }

        if(!err.isEmpty())
            emit LogError(QString("%1: %2").arg(err).arg(errDescr)/*, 2*/);

    }catch(std::exception &err){
        qFatal(QString("Problems during query execution \n%1").arg(err.what()).toStdString().c_str());
    }

    CloseConnection();

    return ret;
}

/**
 * @brief Database::getFieldInt
 * @author Andrea Dompe'
 * @param sQuery
 * @return
 */
bool dbConnection::getFieldBool(QString field, QString sQuery){
    bool value = false;

    QStandardItemModel *rs;
    bool ret = execQuery(sQuery, false, &rs);
    if(ret){
        if(rs->rowCount() > 0)
            value = getFieldBool(field, rs);
        delete rs;
    }

    return value;
}

bool dbConnection::getFieldBool(QString field, QStandardItemModel *model, int iRow)
{
    bool valore = false;

    int index = getModelIndex(field, model);
    if(index >= 0){
        valore = (model->index(iRow, index).data().toString()=="t")?true:false;
    }

    return valore;
}

double dbConnection::getFieldDbl(QString field, QStandardItemModel *model, int iRow)
{
    double valore = 0.0;

    int index = getModelIndex(field, model);
    if(index >= 0)
        valore = model->index(iRow, index).data().toDouble();

    return valore;
}

/**
 * @brief Database::getFieldInt
 * @author Andrea Dompe'
 * @param sQuery
 * @return
 */
int dbConnection::getFieldInt(QString field, QString sQuery){
    int value = 0;

    QStandardItemModel *rs;
    bool ret = execQuery(sQuery, false, &rs);
    if(ret && rs){
        if(rs->rowCount() > 0)
            value = getFieldInt(field, rs);
        delete rs;
    }

    return value;
}

int dbConnection::getFieldInt(QString field, QStandardItemModel *model, int iRow){

    int valore = 0;

    int index = getModelIndex(field, model);
    if(index >= 0)
        valore = model->index(iRow, index).data().toInt();

    return valore;

}

/**
 * @brief Database::getFieldInt
 * @author Andrea Dompe'
 * @param sQuery
 * @return
 */
float dbConnection::getFieldFloat(QString field, QString sQuery){
    float value = 0.0;

    QStandardItemModel *rs;
    bool ret = execQuery(sQuery, false, &rs);
    if(ret){
        if(rs->rowCount() > 0)
            value = getFieldFloat(field, rs);
        delete rs;
    }

    return value;
}

float dbConnection::getFieldFloat(QString field, QStandardItemModel *model, int iRow)
{
    float valore = 0.0;

    int index = getModelIndex(field, model);
    if(index >= 0){
        valore = model->index(iRow, index).data().toFloat();
        QString fl = QString::number(valore, 'f', 2);
        valore = fl.toFloat();
    }

    return valore;
}

QString dbConnection::getFieldStr(QString field, QString sQuery){
    QString value = "";

    QStandardItemModel *rs;
    bool ret = execQuery(sQuery, false, &rs);
    if(ret){
        if(rs->rowCount() > 0)
            value = getFieldStr(field, rs);
        delete rs;
    }

    return value;
}

QString dbConnection::getFieldStr(QString field, QStandardItemModel *model, int iRow)
{
    QString valore = "";

    int index = getModelIndex(field, model);
    if(index >= 0)
        valore = model->index(iRow, index).data().toString();

    return valore;
}

/**
 * @brief Database::getModelIndex
  * @author Andrea Dompe'*
 * @param field
 * @param model
 * @return
 */
int dbConnection::getModelIndex(QString field, QStandardItemModel *model){

    int i = 0;
    bool trovato = false;
    QString s = "";

    //mutex.lock();
    if(!model)
        return -1;

    while(i <= model->columnCount() && !trovato){
        s = model->headerData(i, Qt::Horizontal).toString();
        if(model->headerData(i, Qt::Horizontal).toString().toLower().compare(field.toLower()) == 0)
            trovato = true;
        i++;
    }

    //mutex.unlock();

    return i-1;
}
