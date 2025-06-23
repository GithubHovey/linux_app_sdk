#include "module.h"
class Record{
public:
    Record(uint8_t max_size, uint8_t flag
    ):
    max_size(max_size),
    flag(flag),
    {

    }
    ~Record()override{

    };
    bool init() override ;
private:
    uint8_t max_size;
    uint8_t flag;
    void moduleThreadFunc() override ;
}