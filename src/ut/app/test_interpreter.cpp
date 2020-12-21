/**
 * TODO Put copyright here
 */
/**
 * @file Job manager unit tests
 */

#include "gmock/gmock.h"

#define private public

#include "commissioner_app_mock.hpp"
#include "app/cli/interpreter.hpp"
#include "app/file_util.hpp"
#include "app/ps/registry.hpp"

/**
 * The purpose of the object is to expect all calls to CommissionerApp::Create() calls.
 * If it is necessary, just expect its Create() method and take necessary actions
 */
static CommissionerAppStaticExpecter gCommissionerAppStaticExpecter;

namespace ot {
namespace commissioner {

Error CommissionerAppCreate(std::shared_ptr<CommissionerApp> &aCommApp, const Config &aConfig)
{
    return gCommissionerAppStaticExpecter.Create(aCommApp, aConfig);
}

} // namespace commissioner
} // namespace ot

using namespace ot::commissioner;

using ::testing::_;
using ::testing::DoAll;
using testing::Mock;
using ::testing::Return;
using ::testing::WithArg;

TEST(InterpreterTestSuite, TestInit)
{
    // Minimum test setup: create config file
    const std::string configFile = "./config";
    auto              error = WriteFile("{\"ThreadSMRoot\": \"/home/mike/dsr-commissioner/build/test/\"}", configFile);
    ASSERT_EQ(error.mCode, ErrorCode::kNone);

    std::shared_ptr<CommissionerApp> defaultCommissionerObject{new CommissionerAppMock()};
    ASSERT_NE(defaultCommissionerObject, nullptr);

    EXPECT_CALL(gCommissionerAppStaticExpecter, Create(_, _))
        .WillOnce(DoAll(WithArg<0>([&](std::shared_ptr<CommissionerApp> &a) { a = defaultCommissionerObject; }),
                        Return(Error{})));

    std::shared_ptr<Interpreter> pInterpreter(new Interpreter());
    ASSERT_NE(pInterpreter, nullptr);

    auto result = pInterpreter->Init("./config", "./persistent_storage");
    ASSERT_EQ(result.mCode, ErrorCode::kNone);
}
