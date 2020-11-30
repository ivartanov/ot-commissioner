#include "persistent_storage_json.hpp"
#include "file.hpp"
#include "utils.hpp"

namespace ot::commissioner::persistent_storage {

const std::string JSON_RGR     = "rgr";
const std::string JSON_RGR_SEQ = "rgr_seq";

const std::string JSON_DOM     = "dom";
const std::string JSON_DOM_SEQ = "dom_seq";

const std::string JSON_NWK     = "nwk";
const std::string JSON_NWK_SEQ = "nwk_seq";

const std::string JSON_BR     = "br";
const std::string JSON_BR_SEQ = "br_seq";

using nlohmann::json;

persistent_storage_json::persistent_storage_json(std::string const &fname)
    : file_name(fname)
    , cache()
    , storage_lock()
{
    semaphore_open("thrcomm_json_storage", storage_lock);
}

persistent_storage_json::~persistent_storage_json()
{
    close();
    cache.clear();
    semaphore_close(storage_lock);
}

json persistent_storage_json::json_default()
{
    return json{
        {JSON_RGR_SEQ, registrar_id(0)},      {JSON_DOM_SEQ, domain_id(0)},
        {JSON_NWK_SEQ, network_id(0)},        {JSON_BR_SEQ, border_router_id(0)},

        {JSON_RGR, std::vector<registrar>{}}, {JSON_DOM, std::vector<domain>{}},
        {JSON_NWK, std::vector<network>{}},   {JSON_BR, std::vector<border_router>{}},
    };
}

bool persistent_storage_json::cache_struct_validation()
{
    json base = json_default();

    for (auto &el : base.items())
    {
        auto to_check = cache.find(el.key());
        if (to_check == cache.end() || to_check->type_name() != el.value().type_name())
        {
            return false;
        }
    }

    return true;
}

ps_status persistent_storage_json::cache_from_file()
{
    if (semaphore_wait(storage_lock) != tg_os::sem::sem_status::SEM_SUCCESS)
    {
        return PS_ERROR;
    }

    std::string              fdata;
    tg_os::file::file_status fstat = tg_os::file::file_read(file_name, fdata);
    if (fstat != tg_os::file::file_status::FS_NOT_EXISTS && fstat != tg_os::file::file_status::FS_SUCCESS)
    {
        semaphore_post(storage_lock);
        return PS_ERROR;
    }

    if (fdata.size() == 0)
    {
        cache = json_default();

        semaphore_post(storage_lock);
        return PS_SUCCESS;
    }

    try
    {
        cache = json::parse(fdata);
    } catch (json::parse_error const &)
    {
        semaphore_post(storage_lock);
        return PS_ERROR;
    }

    if (cache.empty())
    {
        cache = json_default();
    }

    // base validation
    if (!cache_struct_validation())
    {
        semaphore_post(storage_lock);
        return PS_ERROR;
    }

    semaphore_post(storage_lock);
    return PS_SUCCESS;
}

ps_status persistent_storage_json::cache_to_file()
{
    if (semaphore_wait(storage_lock) != tg_os::sem::sem_status::SEM_SUCCESS)
    {
        return PS_ERROR;
    }

    tg_os::file::file_status fstat = tg_os::file::file_write(file_name, cache.dump(4));

    if (fstat != tg_os::file::file_status::FS_SUCCESS)
    {
        semaphore_post(storage_lock);
        return PS_ERROR;
    }

    semaphore_post(storage_lock);
    return PS_SUCCESS;
}

ps_status persistent_storage_json::open()
{
    cache_from_file();
    return cache_to_file();
}

ps_status persistent_storage_json::close()
{
    return cache_to_file();
}

ps_status persistent_storage_json::add(registrar const &val, registrar_id &ret_id)
{
    return add_one<registrar, registrar_id>(val, ret_id, JSON_RGR_SEQ, JSON_RGR);
}

ps_status persistent_storage_json::add(domain const &val, domain_id &ret_id)
{
    return add_one<domain, domain_id>(val, ret_id, JSON_DOM_SEQ, JSON_DOM);
}

ps_status persistent_storage_json::add(network const &val, network_id &ret_id)
{
    return add_one<network, network_id>(val, ret_id, JSON_NWK_SEQ, JSON_NWK);
}

ps_status persistent_storage_json::add(border_router const &val, border_router_id &ret_id)
{
// TODO migrate code part into the add(BorderAgent) wrapper method
#if 0
    ps_status status = PS_ERROR;
    domain dom;
	/* border agent -downwards, border_router upward
	   agent E network E domain
	 */
    // TODO I copy the data here. Possibly pass by value? Or even allow changing the argument here, but not in the
    // add_one()?
    border_router ins_val = val;

    // Check the domain exists for the domain name
    if ((ins_val.mPresentFlags & BorderAgent::kDomainNameBit) != 0)
    {
        std::vector<domain> domains;

        dom    = domain{EMPTY_ID, ins_val.mDomainName, std::vector<std::string>{}};
        status = lookup(&dom, domains);
        if (status == PS_ERROR || domains.size() > 1)
        {
            return PS_ERROR;
        }
        else if (status == PS_NOT_FOUND)
        {
            domain_id dom_id{EMPTY_ID};
            status = add(dom, dom_id);
            if (status != PS_SUCCESS)
            {
                return status;
            }
            dom.id = dom_id;
        }
        else
        {
            dom = domains[0];
        }
    }

    network nwk;
    // Check network exists for the network name and xpan
    if (ins_val.nwk_id.id == EMPTY_ID && ((ins_val.mPresentFlags & BorderAgent::kNetworkNameBit) != 0 ||
                                          (ins_val.mPresentFlags & BorderAgent::kExtendedPanIdBit) != 0))
    {
        // If xpan present lookup with it only
        // TODO discuss: is different name an error? Not yet.
        if ((ins_val.mPresentFlags & BorderAgent::kExtendedPanIdBit) != 0)
        {
            std::ostringstream tmp;
            // Assumption: no leading zeroes for the xpan
            tmp << std::hex << ins_val.mExtendedPanId;
            nwk.xpan = tmp.str();
        }
        else
        {
            nwk.name = ins_val.mNetworkName;
        }
        std::vector<network> nwks;
        status = lookup(&nwk, nwks);
        if (status == PS_ERROR || nwks.size() > 1)
        {
            // TODO remove domain if empty?
            return status;
        }
        else if (status == PS_NOT_FOUND)
        {
            network_id nwk_id{EMPTY_ID};
            status = add(nwk, nwk_id);
            if (status != PS_SUCCESS)
            {
                return status;
            }
            nwk.id = nwk_id;
        }
        else
        {
            nwk = nwks[0];
        }
        ins_val.nwk_id = nwk.id;
    }
#endif
    return add_one<border_router, border_router_id>(val, ret_id, JSON_BR_SEQ, JSON_BR);
}

    ps_status persistent_storage_json::del(registrar_id const &id)
    {
        return del_id<registrar, registrar_id>(id, JSON_RGR);
    }

    ps_status persistent_storage_json::del(domain_id const &id) { return del_id<domain, domain_id>(id, JSON_DOM); }

    ps_status persistent_storage_json::del(network_id const &id) { return del_id<network, network_id>(id, JSON_NWK); }

    ps_status persistent_storage_json::del(border_router_id const &id)
    {
        return del_id<border_router, border_router_id>(id, JSON_BR);
    }

    ps_status persistent_storage_json::get(registrar_id const &id, registrar &ret_val)
    {
        return get_id<registrar, registrar_id>(id, ret_val, JSON_RGR);
    }

    ps_status persistent_storage_json::get(domain_id const &id, domain &ret_val)
    {
        return get_id<domain, domain_id>(id, ret_val, JSON_DOM);
    }

    ps_status persistent_storage_json::get(network_id const &id, network &ret_val)
    {
        return get_id<network, network_id>(id, ret_val, JSON_NWK);
    }

    ps_status persistent_storage_json::get(border_router_id const &id, border_router &ret_val)
    {
        return get_id<border_router, border_router_id>(id, ret_val, JSON_BR);
    }

    ps_status persistent_storage_json::update(registrar const &val) { return upd_id<registrar>(val, JSON_RGR); }

    ps_status persistent_storage_json::update(domain const &val) { return upd_id<domain>(val, JSON_DOM); }

    ps_status persistent_storage_json::update(network const &val) { return upd_id<network>(val, JSON_NWK); }

    ps_status persistent_storage_json::update(border_router const &val) { return upd_id<border_router>(val, JSON_BR); }

    ps_status persistent_storage_json::lookup(registrar const *val, std::vector<registrar> &ret)
    {
        std::function<bool(registrar const &)> pred = [](registrar const &) { return true; };

        if (val)
        {
            pred = [val](registrar const &el) {
                bool ret = val->id.id != EMPTY_ID || !val->addr.empty() || val->port != 0 || !val->domains.empty();

                ret = ret && (val->id.id == EMPTY_ID || (el.id.id == val->id.id)) &&
                      (val->addr.empty() || str_cmp_icase(val->addr, el.addr)) &&
                      (val->port == 0 || (val->port == el.port));

                if (ret && !val->domains.empty())
                {
                    std::vector<std::string> el_tmp(el.domains);
                    std::vector<std::string> val_tmp(val->domains);
                    std::sort(std::begin(el_tmp), std::end(el_tmp));
                    std::sort(std::begin(val_tmp), std::end(val_tmp));
                    ret = std::includes(std::begin(el_tmp), std::end(el_tmp), std::begin(val_tmp), std::end(val_tmp));
                }
                return ret;
            };
        }

        return lookup_pred<registrar>(pred, ret, JSON_RGR);
    }

    ps_status persistent_storage_json::lookup(domain const *val, std::vector<domain> &ret)
    {
        std::function<bool(domain const &)> pred = [](domain const &) { return true; };

        if (val)
        {
            pred = [val](domain const &el) {
                bool ret = val->id.id != EMPTY_ID || !val->name.empty() || !val->networks.empty();

                ret = ret && (val->id.id == EMPTY_ID || (el.id.id == val->id.id)) &&
                      (val->name.empty() || (val->name == el.name));

                if (ret && !val->networks.empty())
                {
                    std::vector<std::string> el_tmp(el.networks);
                    std::vector<std::string> val_tmp(val->networks);
                    std::sort(std::begin(el_tmp), std::end(el_tmp));
                    std::sort(std::begin(val_tmp), std::end(val_tmp));
                    ret = std::includes(std::begin(el_tmp), std::end(el_tmp), std::begin(val_tmp), std::end(val_tmp));
                }
                return ret;
            };
        }

        return lookup_pred<domain>(pred, ret, JSON_DOM);
    }

    ps_status persistent_storage_json::lookup(network const *val, std::vector<network> &ret)
    {
        std::function<bool(network const &)> pred = [](network const &) { return true; };

        if (val)
        {
            pred = [val](network const &el) {
                bool ret = val->id.id != EMPTY_ID || !val->name.empty() || !val->domain_name.empty() ||
                           !val->xpan.empty() || !val->pan.empty() || !val->mlp.empty() || val->channel != 0 ||
                           val->ccm >= 0;

                ret = ret && (val->ccm < 0 || val->ccm == el.ccm) &&
                      (val->id.id == EMPTY_ID || (el.id.id == val->id.id)) &&
                      (val->name.empty() || (val->name == el.name)) &&
                      (val->domain_name.empty() || (val->domain_name == el.domain_name)) &&
                      (val->xpan.empty() || str_cmp_icase(val->xpan, el.xpan)) &&
                      (val->pan.empty() || str_cmp_icase(val->pan, el.pan)) &&
                      (val->mlp.empty() || str_cmp_icase(val->mlp, el.mlp)) &&
                      (val->channel == 0 || (val->channel == el.channel));

                return ret;
            };
        }

        return lookup_pred<network>(pred, ret, JSON_NWK);
    }

    ps_status persistent_storage_json::lookup(border_router const *val, std::vector<border_router> &ret)
    {
        std::function<bool(border_router const &)> pred = [](border_router const &) { return true; };

        if (val)
        {
            pred = [val](border_router const &el) {
                bool ret = val->id.id != EMPTY_ID || val->nwk_id.id != EMPTY_ID || val->agent.mPresentFlags != 0;

                ret = ret && (val->id.id == EMPTY_ID || (el.id.id == val->id.id)) &&
                      (val->nwk_id.id == EMPTY_ID || (el.nwk_id.id == val->nwk_id.id)) &&
                      ((val->agent.mPresentFlags & BorderAgent::kAddrBit) == 0 ||
                       ((el.agent.mPresentFlags & BorderAgent::kAddrBit) != 0 &&
                        str_cmp_icase(el.agent.mAddr, val->agent.mAddr))) &&
                      ((val->agent.mPresentFlags & BorderAgent::kPortBit) == 0 ||
                       ((el.agent.mPresentFlags & BorderAgent::kPortBit) != 0 && el.agent.mPort == val->agent.mPort)) &&
                      ((val->agent.mPresentFlags & BorderAgent::kThreadVersionBit) == 0 ||
                       ((el.agent.mPresentFlags & BorderAgent::kThreadVersionBit) != 0 &&
                        val->agent.mThreadVersion == el.agent.mThreadVersion)) &&
                      ((val->agent.mPresentFlags & BorderAgent::kStateBit) == 0 ||
                       ((el.agent.mPresentFlags & BorderAgent::kStateBit) != 0 &&
                        el.agent.mState == val->agent.mState)) &&
                      ((val->agent.mPresentFlags & BorderAgent::kVendorNameBit) == 0 ||
                       ((el.agent.mPresentFlags & BorderAgent::kVendorNameBit) != 0 &&
                        str_cmp_icase(el.agent.mVendorName, val->agent.mVendorName))) &&
                      ((val->agent.mPresentFlags & BorderAgent::kModelNameBit) == 0 ||
                       ((el.agent.mPresentFlags & BorderAgent::kModelNameBit) != 0 &&
                        str_cmp_icase(el.agent.mModelName, val->agent.mModelName))) &&
                      ((val->agent.mPresentFlags & BorderAgent::kActiveTimestampBit) == 0 ||
                       ((el.agent.mPresentFlags & BorderAgent::kActiveTimestampBit) != 0 &&
                        el.agent.mActiveTimestamp.Encode() == val->agent.mActiveTimestamp.Encode())) &&
                      ((val->agent.mPresentFlags & BorderAgent::kPartitionIdBit) == 0 ||
                       ((el.agent.mPresentFlags & BorderAgent::kPartitionIdBit) != 0 &&
                        el.agent.mPartitionId == val->agent.mPartitionId)) &&
                      ((val->agent.mPresentFlags & BorderAgent::kVendorDataBit) == 0 ||
                       ((el.agent.mPresentFlags & BorderAgent::kVendorDataBit) != 0 &&
                        el.agent.mVendorData == val->agent.mVendorData)) &&
                      ((val->agent.mPresentFlags & BorderAgent::kVendorOuiBit) == 0 ||
                       ((el.agent.mPresentFlags & BorderAgent::kVendorOuiBit) != 0 &&
                        el.agent.mVendorOui == val->agent.mVendorOui)) &&
                      ((val->agent.mPresentFlags & BorderAgent::kBbrSeqNumberBit) == 0 ||
                       ((el.agent.mPresentFlags & BorderAgent::kBbrSeqNumberBit) != 0 &&
                        el.agent.mBbrSeqNumber == val->agent.mBbrSeqNumber)) &&
                      ((val->agent.mPresentFlags & BorderAgent::kBbrPortBit) == 0 ||
                       ((el.agent.mPresentFlags & BorderAgent::kBbrPortBit) != 0 &&
                        el.agent.mBbrPort == val->agent.mBbrPort));
                return ret;
            };
        }

        return lookup_pred<border_router>(pred, ret, JSON_BR);
    }

} // namespace ot::commissioner::persistent_storage