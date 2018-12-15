#ifndef message_manager_h_
#define message_manager_h_
#include "tools/database.h"

class MessageManager {
public:
    MessageManager(): database_(NULL) {}
    ~MessageManager() {}
    bool Init(Database *database);
private:
    Database *database_;
};

#endif /* define message_manager_h_ */

