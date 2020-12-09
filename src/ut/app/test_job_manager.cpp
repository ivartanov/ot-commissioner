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
#include "app/cli/job_manager.hpp"
#include "app/file_util.hpp"

using namespace ot::commissioner;

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

TEST(JobManagerTestSuite, TestInit)
{
    Config       conf;
    Interpreter *pInterpreter = new Interpreter();
    JobManager   jm(*pInterpreter);

    conf.mThreadSMRoot = ".";

    EXPECT_CALL(gCommissionerAppStaticExpecter, Create(_, _)).Times(1).WillOnce(Return(Error{}));

    EXPECT_EQ(jm.Init(conf).mCode, ErrorCode::kNone);
}
