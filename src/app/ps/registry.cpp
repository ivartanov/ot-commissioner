#include "registry.hpp"
#include "persistent_storage_json.hpp"

#include <cassert>

namespace ot::commissioner::persistent_storage {

namespace {
registry_status success_status(ps_status ps_st)
{
    if (ps_st == ps_status::PS_SUCCESS)
    {
        return REG_SUCCESS;
    }
    return REG_ERROR;
}

registry_status map_status(ps_status ps_st)
{
    if (ps_st == ps_status::PS_ERROR)
    {
        return REG_ERROR;
    }
    if (ps_st == ps_status::PS_SUCCESS)
    {
        return REG_SUCCESS;
    }
    if (ps_st == ps_status::PS_NOT_FOUND)
    {
        return REG_NOT_FOUND;
    }
    return REG_ERROR;
}

} // namespace

registry::registry(persistent_storage *strg)
    : manage_storage(false)
    , storage(strg)
{
    assert(storage != nullptr);
}

registry::registry(std::string const &name)
    : manage_storage(true)
{
    storage = new persistent_storage_json(name);
}

registry::~registry()
{
    close();

    if (manage_storage)
    {
        delete storage;
    }
    storage = nullptr;
}

registry_status registry::open()
{
    if (storage == nullptr)
    {
        return REG_ERROR;
    }

    return success_status(storage->open());
}

registry_status registry::close()
{
    if (storage == nullptr)
    {
        return REG_ERROR;
    }
    return success_status(storage->close());
}

registry_status registry::lookup(registrar const *val, std::vector<registrar> &ret)
{
    assert(storage != nullptr);

    return map_status(storage->lookup(val, ret));
}

registry_status registry::lookup(domain const *val, std::vector<domain> &ret)
{
    assert(storage != nullptr);

    return map_status(storage->lookup(val, ret));
}

registry_status registry::lookup(network const *val, std::vector<network> &ret)
{
    assert(storage != nullptr);

    return map_status(storage->lookup(val, ret));
}

registry_status registry::lookup(border_router const *val, std::vector<border_router> &ret)
{
    assert(storage != nullptr);

    return map_status(storage->lookup(val, ret));
}

registry_status registry::get(registrar_id const &id, registrar &ret_val)
{
    assert(storage != nullptr);

    return map_status(storage->get(id, ret_val));
}

registry_status registry::get(domain_id const &id, domain &ret_val)
{
    assert(storage != nullptr);

    return map_status(storage->get(id, ret_val));
}

registry_status registry::get(network_id const &id, network &ret_val)
{
    assert(storage != nullptr);

    return map_status(storage->get(id, ret_val));
}

registry_status registry::get(border_router_id const &id, border_router &ret_val)
{
    assert(storage != nullptr);

    return map_status(storage->get(id, ret_val));
}

registry_status registry::add(registrar const &val, registrar_id &ret_id)
{
    assert(storage != nullptr);

    return map_status(storage->add(val, ret_id));
}

registry_status registry::add(domain const &val, domain_id &ret_id)
{
    assert(storage != nullptr);

    return map_status(storage->add(val, ret_id));
}

registry_status registry::add(network const &val, network_id &ret_id)
{
    assert(storage != nullptr);

    return map_status(storage->add(val, ret_id));
}

registry_status registry::add(border_router const &val, border_router_id &ret_id)
{
    assert(storage != nullptr);

    return map_status(storage->add(val, ret_id));
}

registry_status registry::del(registrar_id const &id)
{
    assert(storage != nullptr);

    return map_status(storage->del(id));
}

registry_status registry::del(domain_id const &id)
{
    assert(storage != nullptr);

    return map_status(storage->del(id));
}

registry_status registry::del(network_id const &id)
{
    assert(storage != nullptr);

    return map_status(storage->del(id));
}

registry_status registry::del(border_router_id const &id)
{
    assert(storage != nullptr);

    return map_status(storage->del(id));
}

registry_status registry::update(registrar const &val)
{
    assert(storage != nullptr);

    return map_status(storage->update(val));
}

registry_status registry::update(domain const &val)
{
    assert(storage != nullptr);

    return map_status(storage->update(val));
}

registry_status registry::update(network const &val)
{
    assert(storage != nullptr);

    return map_status(storage->update(val));
}

registry_status registry::update(border_router const &val)
{
    assert(storage != nullptr);

    return map_status(storage->update(val));
}

} // namespace ot::commissioner::persistent_storage