#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <mntent.h>
#include "L2Tests.h"
#include "L2TestsMock.h"
#include <fstream>
#include "UdevMock.h"
#include "WrapsMock.h"
#include "Wraps.h"
#include "Wraps.cpp"

#define JSON_TIMEOUT   (1000)
#define TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);
#define USBACCESS_CALLSIGN  _T("org.rdk.UsbAccess.1")
#define USBACCESSL2TEST_CALLSIGN _T("L2tests.1")

using ::testing::NiceMock;
using namespace WPEFramework;
using testing::StrictMock;
extern "C" int __real_system(const char* command);

/**
 * @brief Compare two request status objects
 *
 * @param[in] data Expected value
 * @return true if the argument and data match, false otherwise
 */
MATCHER_P(MatchRequestStatus, data, "")
{
    bool match = true;
    std::string expected;
    std::string actual;

    data.ToString(expected);
    arg.ToString(actual);
    TEST_LOG(" rec = %s, arg = %s",expected.c_str(),actual.c_str());
    EXPECT_STREQ(expected.c_str(),actual.c_str());

    return match;
}

class UsbAccess_L2test : public L2TestMocks {
    protected:
        Core::JSONRPC::Message message;
        string response;
        virtual ~UsbAccess_L2test() override;
    
    public:
        UsbAccess_L2test();
};

UsbAccess_L2test:: UsbAccess_L2test():L2TestMocks()
{
    TEST_LOG("UsbAccess_L2test constructor");
     uint32_t status = Core::ERROR_GENERAL;
     Core::JSONRPC::Message message;
    NiceMock<UdevImplMock> udevImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;
	NiceMock<IarmBusImplMock> iarmBusImplMock;
    IARM_EventHandler_t eventHandler;

    
     
     /*In order to ensure consistent behavior across all test APIs,
         the "/etc/device.properties" file is opened only once*/
        std::ofstream file("/etc/device.properties");
        file << "MODEL_NUM=HSTP11MWR";
        file.close();
        Udev::getInstance().impl = &udevImplMock;
        Wraps::getInstance().impl = &wrapsImplMock;
		IarmBus::getInstance().impl = &iarmBusImplMock;
    
    ON_CALL(iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    if ((string(IARM_BUS_SYSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_SYSMGR_EVENT_USB_MOUNT_CHANGED)) {
                        eventHandler = handler;
                    }
                    return IARM_RESULT_SUCCESS;
                }));

   
      status = ActivateService("org.rdk.UsbAccess");
      EXPECT_EQ(Core::ERROR_NONE, status);
}

UsbAccess_L2test:: ~UsbAccess_L2test(){

    TEST_LOG("UsbAccess_L2test Destructor");
    uint32_t status = Core::ERROR_GENERAL;
    Wraps::getInstance().impl = nullptr;
        Udev::getInstance().impl = nullptr;
		IarmBus::getInstance().impl = nullptr; 
    status = DeactivateService("org.rdk.UsbAccess");
    EXPECT_EQ(Core::ERROR_NONE, status);

}


TEST_F(UsbAccess_L2test, test1)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(USBACCESS_CALLSIGN, USBACCESSL2TEST_CALLSIGN);
    int32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    NiceMock<UdevImplMock> udevImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;
	NiceMock<IarmBusImplMock> iarmBusImplMock;
    Udev::getInstance().impl = &udevImplMock;
    Wraps::getInstance().impl = &wrapsImplMock;
    // EXPECT_CALL(wrapsImplMock, system(::testing::_))
    //     .Times(1)
    //     .WillOnce(::testing::Invoke(
    //         [&]( const char * command) {
    //             EXPECT_EQ(string(command), string(_T("/lib/rdk/userInitiatedFWDnld.sh usb '/tmp;reboot;' 'my.bin' 0 >> /opt/logs/swupdate.log &")));
    //             //return 0;
    //             return __real_system(command);
                
    //         }));

    EXPECT_CALL(wrapsImplMock, system(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&]( const char * command) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/userInitiatedFWDnld.sh usb '/tmp/reboot/' 'HSTP11MWR.bin' 0 >> /opt/logs/swupdate.log &")));
                //return 0;
                return __real_system(command);
                
            }));
 
 params["fileName"] = "/tmp/reboot/HSTP11MWR.bin";
    status = InvokeServiceMethod("org.rdk.UsbAccess.1", "updateFirmware", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);

      // params["fileName"] = "/tmp/reboot/my.bin";
 //_T("{\"fileName\":\"/tmp;reboot;/my.bin\"}"), response));
 //_T("{\"fileName\":\"/tmp/reboot/HSTP11MWR.bin\"}"), response));
}
