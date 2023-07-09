#include "bike.pb.h"
#include "string"
#include "iostream"
#include "log.h"

using namespace bike;

int main()
{
    std::string data; //接受系列化的消息
    //模擬
    {
        mobile_request mr;
        mr.set_mobile("15923833401");
        mr.SerializeToString(&data);

    }
    DEBUG(data.c_str());
    {
        mobile_request mr;
        mr.ParseFromString(data); //解析
        std::string msg = mr.mobile();
        DEBUG(msg.c_str());    
    }
    return 0;
}