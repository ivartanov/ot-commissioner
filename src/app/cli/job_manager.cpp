/*
 *    Copyright (c) 2020, The OpenThread Commissioner Authors.
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *    POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   The file implements command job manager.
 */

#include "app/cli/job_manager.hpp"
#include "app/cli/interpreter.hpp"
#include "app/cli/job.hpp"
#include "app/cli/security_materials.hpp"
#include "app/json.hpp"
#include "app/ps/registry.hpp"
#include "common/error_macros.hpp"
#include "common/utils.hpp"
#include "library/logging.hpp"

namespace ot {

namespace commissioner {

using Json   = nlohmann::json;
using XpanId = persistent_storage::xpan_id;

Error JobManager::Init(const Config &aConf)
{
    Error error;

    mDefaultConf = aConf;
    SuccessOrExit(error = sm::Init(aConf));
    SuccessOrExit(error = CommissionerApp::Create(mDefaultCommissioner, aConf));
exit:
    return error;
}

void JobManager::CleanupJobs()
{
    for (auto job : mJobPool)
    {
        ASSERT(job == NULL || job->IsStopped());
        delete job;
    }
    mJobPool.clear();
    mImportFile.clear();
}

void JobManager::SetImportFile(const std::string &importFile)
{
    mImportFile = importFile;
}

Error JobManager::CreateJob(CommissionerAppPtr &aCommissioner, const Interpreter::Expression &aExpr, uint64_t aXpanId)
{
    Interpreter::JobEvaluator eval;
    auto                      mapItem = Interpreter::mJobEvaluatorMap.find(aExpr[0]);

    if (mapItem != Interpreter::mJobEvaluatorMap.end())
    {
        return ERROR_INVALID_SYNTAX("{} not eligible for job", aExpr[0]);
    }
    eval     = mapItem->second;
    Job *job = new Job(mInterpreter, aCommissioner, aExpr, eval, aXpanId);
    mJobPool.push_back(job);

    return ERROR_NONE;
}

Error JobManager::PrepareJobs(const Interpreter::Expression &aExpr, const NidArray &aNids, bool aGroupAlias)
{
    if (aExpr[0] == "start")
        return PrepareStartJobs(aExpr, aNids, aGroupAlias);
    else if (aExpr[0] == "stop")
        return PrepareStopJobs(aExpr, aNids, aGroupAlias);

    Error error;

    for (auto nid : aNids)
    {
        auto entry = mCommissionerPool.find(nid);
        if (entry == mCommissionerPool.end())
        {
            if (!aGroupAlias)
            {
                WarningMsg(nid, "not started");
            }
            continue;
        }

        bool active = entry->second->IsActive();
        if (!active)
        {
            if (!aGroupAlias)
            {
                WarningMsg(nid, "not started");
            }
            continue;
        }

        Interpreter::Expression jobExpr = aExpr;

        if (!mImportFile.empty())
        {
            Error importError = AppendImport(nid, jobExpr);
            if (importError != ERROR_NONE)
            {
                ErrorMsg(nid, "import entry not found");
                continue;
            }
        }

        SuccessOrExit(error = CreateJob(entry->second, jobExpr, nid));
    }
exit:
    return error;
}

Error JobManager::PrepareStartJobs(const Interpreter::Expression &aExpr, const NidArray &aNids, bool aGroupAlias)
{
    Config conf  = mDefaultConf;
    Error  error = ERROR_NONE;

    ASSERT(aExpr[0] == "start");
    /*
     * Coming here is a result of using multi-network syntax.
     * Therefore, no extra arguments to be used, otherwise it
     * is multi-network syntax violation.
     */
    ASSERT(aExpr.size() == 1);

    for (auto nid : aNids)
    {
        BorderRouter br;

        auto entry = mCommissionerPool.find(nid);
        if (entry == mCommissionerPool.end())
        {
            CommissionerAppPtr commissioner;

            SuccessOrExit(error = PrepareDtlsConfig(nid, conf));
            SuccessOrExit(error = CommissionerApp::Create(commissioner, conf));
            mCommissionerPool[nid] = commissioner;
        }

        bool active = entry->second->IsActive();
        if (active)
        {
            if (!aGroupAlias)
            {
                InfoMsg(nid, "already started");
            }
            continue;
        }

        SuccessOrExit(error = PrepareDtlsConfig(nid, conf));
        SuccessOrExit(error = MakeBorderRouterChoice(nid, br));
        auto expr = aExpr;
        expr.push_back(br.agent.mAddr);
        expr.push_back(std::to_string(br.agent.mPort));
        ASSERT(expr.size() == 3); // 'start br_addr br_port'
        SuccessOrExit(error = CreateJob(entry->second, expr, nid));
    }
exit:
    return error;
}

Error JobManager::PrepareStopJobs(const Interpreter::Expression &aExpr, const NidArray &aNids, bool aGroupAlias)
{
    Error error = ERROR_NONE;

    ASSERT(aExpr[0] == "stop");

    for (auto nid : aNids)
    {
        auto entry = mCommissionerPool.find(nid);
        if (entry == mCommissionerPool.end())
        {
            if (!aGroupAlias)
            {
                WarningMsg(nid, "not known to be started");
            }
            continue;
        }

        bool active = entry->second->IsActive();
        if (!active)
        {
            if (!aGroupAlias)
            {
                InfoMsg(nid, "already stopped");
            }
            continue;
        }

        SuccessOrExit(error = CreateJob(entry->second, aExpr, nid));
    }
exit:
    return error;
}

Error JobManager::PrepareDtlsConfig(const uint64_t aNid, Config &aConfig)
{
    Error                       error;
    std::string                 domainName;
    bool                        isCCM = false;
    sm::SecurityMaterials       dtlsConfig;
    RegistryStatus              status;
    persistent_storage::network nwk;

    status = mInterpreter.mRegistry->get_network_by_xpan(aNid, nwk);
    VerifyOrExit(status == RegistryStatus::REG_SUCCESS, error = ERROR_IO_ERROR("network not found"));
    isCCM  = nwk.ccm > 0;
    status = mInterpreter.mRegistry->get_domain_name_by_xpan(aNid, domainName);
    if (status != RegistryStatus::REG_SUCCESS)
    {
        LOG_DEBUG(LOG_REGION_JOB_MANAGER, "{}: domain resolution failed with status={}", XpanId(aNid).str(), status);
    }

    aConfig.mEnableCcm = isCCM;
    if (!domainName.empty())
    {
        aConfig.mDomainName = domainName;
        if (domainName != "DefaultDomain")
        {
            error = sm::GetDomainSM(domainName, dtlsConfig);
            if (ERROR_NONE != error)
            {
                WarningMsg(aNid, error.GetMessage());
                error = ERROR_NONE;
            }
        }
        else
        {
            error = sm::GetDefaultDomainSM(nwk.xpan.str(), isCCM, dtlsConfig);
            if (ERROR_NONE != error)
            {
                WarningMsg(aNid, error.GetMessage());
                error = ERROR_NONE;
            }
            if (!dtlsConfig.IsEmpty())
            {
                goto update;
            }
            error = sm::GetDefaultDomainSM(nwk.name, isCCM, dtlsConfig);
            if (ERROR_NONE != error)
            {
                WarningMsg(aNid, error.GetMessage());
                error = ERROR_NONE;
            }
        }
    }
    if (!dtlsConfig.IsEmpty())
    {
        goto update;
    }
    error = sm::GetNetworkSM(nwk.xpan.str(), isCCM, dtlsConfig);
    if (ERROR_NONE != error)
    {
        WarningMsg(aNid, error.GetMessage());
        error = ERROR_NONE;
    }
    if (!dtlsConfig.IsEmpty())
    {
        goto update;
    }
    error = sm::GetNetworkSM(nwk.name, isCCM, dtlsConfig);
    if (ERROR_NONE != error)
    {
        WarningMsg(aNid, error.GetMessage());
        error = ERROR_NONE;
    }
update:

#define UPDATE_IF_SET(name)            \
    if (dtlsConfig.m##name.size() > 0) \
    aConfig.m##name = dtlsConfig.m##name

    UPDATE_IF_SET(Certificate);
    UPDATE_IF_SET(PrivateKey);
    UPDATE_IF_SET(TrustAnchor);
    UPDATE_IF_SET(PSKc);

#undef UPDATE_IF_SET

    if (dtlsConfig.IsEmpty())
    {
        InfoMsg(aNid, "no updates to DTLS configuration, default configuration will be used");
    }
exit:
    return error;
}

Error JobManager::MakeBorderRouterChoice(const uint64_t aNid, BorderRouter &br)
{
    using BRArray = std::vector<persistent_storage::border_router>;
    using Network = persistent_storage::network;

    Error          error;
    BRArray        brs;
    BRArray        choice;
    Network        nwk;
    RegistryStatus status = mInterpreter.mRegistry->get_border_routers_in_network(aNid, brs);

    VerifyOrExit(status == RegistryStatus::REG_SUCCESS,
                 error = ERROR_NOT_FOUND("br lookup failed with status={}", status));
    if (brs.size() == 1)
    {
        // looks like not much of a choice
        br = brs.front();
        ExitNow();
    }
    status = mInterpreter.mRegistry->get_network_by_xpan(aNid, nwk);
    VerifyOrExit(status == RegistryStatus::REG_SUCCESS, error = ERROR_NOT_FOUND("network lookup failed"));
    if (nwk.ccm > 0) // Dealing with domain network
    {
        // - try to find active and connectable Primary BBR
        for (auto item : brs)
        {
            if (item.agent.mState.mBbrIsPrimary && item.agent.mState.mConnectionMode > 0)
            {
                if (item.agent.mState.mBbrIsActive)
                {
                    br = item;
                    ExitNow();
                }
            }
        }
        // - go on with other active and connectable BBRs
        for (auto item : brs)
        {
            if (item.agent.mState.mBbrIsActive && item.agent.mState.mConnectionMode > 0)
            {
                choice.push_back(item);
            }
        }
    }
    else // Dealing with standalone networks
    {
        // go on with connectable BRs
        for (auto item : brs)
        {
            if (item.agent.mState.mConnectionMode > 0)
            {
                choice.push_back(item);
            }
        }
    }

    // Below a final triage is done

    // - prefer br with high-availability
    for (auto item : choice)
    {
        if (item.agent.mState.mThreadIfStatus > 1 && item.agent.mState.mAvailability > 0)
        {
            br = item;
            ExitNow();
        }
    }
    // - prefer br with Thread Interface actively participating in communication
    for (auto item : choice)
    {
        if (item.agent.mState.mThreadIfStatus > 1)
        {
            br = item;
            ExitNow();
        }
    }
    // - try to find br with Thread Interface at least enabled
    for (auto item : choice)
    {
        if (item.agent.mState.mThreadIfStatus > 0)
        {
            br = item;
            ExitNow();
        }
    }
    error = ERROR_NOT_FOUND("no active BR found");
exit:
    return error;
}

Error JobManager::AppendImport(const uint64_t aNid, Interpreter::Expression &aExpr)
{
    Error       error;
    XpanId      xpan(aNid);
    std::string jsonStr;
    Json        jsonSrc;
    Json        json;

    SuccessOrExit(error = JsonFromFile(jsonStr, mImportFile));
    jsonSrc = Json::parse(jsonStr);
    if (jsonSrc.count(xpan.str()) > 0)
    {
        json = jsonSrc[xpan.str()];
    }
    else if (aNid == 0) // must be single command
    {
        json = jsonSrc;
    }
    else
    {
        ExitNow(error = ERROR_NOT_FOUND("'{}' not found", xpan.str()));
    }
    jsonStr = json.dump(JSON_INDENT_DEFAULT);
    if (aExpr[0] == "opdataset")
    {
        VerifyOrExit(aExpr[1] == "set" && aExpr.size() == 3, error = ERROR_INVALID_ARGS("import usupported"));
        if (aExpr[2] == "active")
        {
            ActiveOperationalDataset dataset;
            SuccessOrExit(error = ActiveDatasetFromJson(dataset, jsonStr));
            // TODO: try importing wrong format
        }
        else if (aExpr[2] == "pending")
        {
            PendingOperationalDataset dataset;
            SuccessOrExit(error = PendingDatasetFromJson(dataset, jsonStr));
            // TODO: try importing wrong format
        }
        else
        {
            ExitNow(error = ERROR_INVALID_ARGS("import unsupported"));
        }
    }
    else if (aExpr[0] == "bbrdataset")
    {
        BbrDataset dataset;
        VerifyOrExit(aExpr.size() == 2 && aExpr[1] == "set", error = ERROR_INVALID_ARGS("import usupported"));
        SuccessOrExit(error = BbrDatasetFromJson(dataset, jsonStr));
        // TODO: try importing wrong format
    }
    else if (aExpr[0] == "commdataset")
    {
        CommissionerDataset dataset;
        VerifyOrExit(aExpr.size() == 2 && aExpr[1] == "set", error = ERROR_INVALID_ARGS("import usupported"));
        SuccessOrExit(error = CommissionerDatasetFromJson(dataset, jsonStr));
        // TODO: try importing wrong format
    }
    else
    {
        ASSERT(false); // never to reach here
    }
    aExpr.push_back(jsonStr);
exit:
    return error;
}

void JobManager::RunJobs()
{
    for (auto job : mJobPool)
    {
        ASSERT(job != NULL);
        job->Run();
    }
    WaitForJobs();
}

void JobManager::CancelCommand()
{
    for (auto job : mJobPool)
    {
        ASSERT(job != NULL);
        job->Cancel();
    }
    WaitForJobs();
}

void JobManager::WaitForJobs()
{
    for (auto job : mJobPool)
    {
        ASSERT(job != NULL);
        job->Wait();
    }
}

Interpreter::Value JobManager::CollectJobsValue()
{
    Interpreter::Value          value;
    nlohmann::json              json;
    persistent_storage::xpan_id xpan;

    for (auto job : mJobPool)
    {
        ASSERT(job->IsStopped());
        if (job->GetValue().HasNoError())
        {
            xpan = job->GetXpanId();
            try
            {
                std::string valueStr = job->GetValue().ToString();
                if (valueStr.empty())
                {
                    // this may occur with non-dataset commands
                    // like 'start', 'stop', etc.
                    valueStr = "true";
                    // please note, this is where job-based execution
                    // is different from single command run when
                    // nothing but [done] is printed; we need to see a
                    // distinguished result per network
                }
                json[xpan.str()] = nlohmann::json::parse(valueStr);
            } catch (std::exception &e)
            {
                ErrorMsg(xpan.value, e.what());
            }
        }
        else // produce error messages immediately before printing value
        {
            ErrorMsg(job->GetXpanId(), job->GetValue().ToString());
        }
    }
    value = json.dump(JSON_INDENT_DEFAULT);
    return value;
}

void JobManager::StopCommissionerPool()
{
    for (auto commissionerEntry : mCommissionerPool)
    {
        CommissionerAppPtr commissioner = commissionerEntry.second;

        if (commissioner->IsActive())
            commissioner->Stop();
    }
    if (mDefaultCommissioner->IsActive())
        mDefaultCommissioner->Stop();
}

Error JobManager::GetSelectedCommissioner(CommissionerAppPtr &aCommissioner)
{
    Error          error = ERROR_NONE;
    uint64_t       nid   = 0;
    RegistryStatus status;

    status = mInterpreter.mRegistry->get_current_network_xpan(nid);
    VerifyOrExit(RegistryStatus::REG_SUCCESS == status, error = ERROR_IO_ERROR("selected network not found"));

    if (nid != 0)
    {
        auto entry = mCommissionerPool.find(nid);
        if (entry != mCommissionerPool.end())
        {
            aCommissioner = entry->second;
        }
        else
        {
            Config             conf         = mDefaultConf;
            CommissionerAppPtr commissioner = nullptr;

            SuccessOrExit(error = PrepareDtlsConfig(nid, conf));
            SuccessOrExit(error = CommissionerApp::Create(commissioner, conf));
            mCommissionerPool[nid] = commissioner;
            aCommissioner          = mCommissionerPool[nid];
        }
    }
    else
    {
        aCommissioner = mDefaultCommissioner;
    }
exit:
    return error;
}

} // namespace commissioner

} // namespace ot
