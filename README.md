# QT-database-manager
PostgreSQL manger class

example

    #include "dbConnection.h"

...

    QStandardItemModel *model;
    db.execQuery("SELECT name, surname FROM user", false, &model);
    if(model){
      QString name, surname;
      for(int i=0; i<model->rowCount(); i++ ){
          name = db.getFieldStr("name", model, i);
          surname = db.getFieldStr("surname", model, i);
          
          ...
      }
      delete model; // very important!        
    }
