#ifndef manager_h_
#define manager_h_
#include "tools/database.h"

class Manager{
public:
    Manager():database_(NULL){}
    ~Manager():{}
    virtual Init(Database *database) {
        database_ = database;
        return true;
    }
private:
    Database *database_;
};

#endif /* define manager_h_ */