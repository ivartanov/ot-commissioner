// TODO copyright

#include <catch2/catch.hpp>

#include "persistent_storage_json.hpp"

#include <fstream>
#include <vector>

#include <unistd.h>

#include "app/border_agent.hpp"

using namespace ot::commissioner::persistent_storage;
using namespace ot::commissioner;

TEST_CASE("Create default if not exists", "[ps_json]")
{
    persistent_storage_json psj("./test_ps.json");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);
    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Read empty file", "[ps_json]")
{
    std::ofstream test_tmp("./test.tmp");
    test_tmp.close();

    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);
    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Read non empty - default struct", "[ps_json]")
{
    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);
    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Add registrar", "[ps_json]")
{
    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    registrar_id new_id;

    REQUIRE(psj.add(registrar{EMPTY_ID, "0.0.0.1", 1, {"dom1"}}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 0);
    REQUIRE(psj.add(registrar{EMPTY_ID, "0.0.0.2", 2, {"dom2"}}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 1);
    REQUIRE(psj.add(registrar{EMPTY_ID, "0.0.0.3", 3, {"dom3"}}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 2);

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Add domain", "[ps_json]")
{
    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    domain_id new_id;

    REQUIRE(psj.add(domain{EMPTY_ID, "dom1"}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 0);
    REQUIRE(psj.add(domain{EMPTY_ID, "dom2"}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 1);
    REQUIRE(psj.add(domain{EMPTY_ID, "dom3"}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 2);

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Add network", "[ps_json]")
{
    // Make the test independent
    unlink("./test.tmp");

    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    network_id new_id;
    REQUIRE(psj.add(network{EMPTY_ID, EMPTY_ID, "nwk1", xpan_id{"FFFFFFFFFFFFFFF1"}, 11, "FFF1", "2000:aaa1::0/8", 1},
                    new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 0);
    REQUIRE(psj.add(network{EMPTY_ID, EMPTY_ID, "nwk2", xpan_id{"FFFFFFFFFFFFFFF2"}, 11, "FFF2", "2000:aaa2::0/8", 1},
                    new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 1);
    REQUIRE(psj.add(network{EMPTY_ID, EMPTY_ID, "nwk3", xpan_id{"FFFFFFFFFFFFFFF3"}, 11, "FFF3", "2000:aaa3::0/8", 1},
                    new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 2);

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Add br", "[ps_json]")
{
    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    border_router_id new_id;

    REQUIRE(psj.add(border_router{EMPTY_ID, EMPTY_ID,
                                  BorderAgent{"1.1.1.2", 11, "th1.x", BorderAgent::State{1, 0, 1, 0, 1}, "network_id",
                                              0x1011223344556677ll, "vendor_name", "model_name", Timestamp{0, 0, 0}, 1,
                                              "vendor_data", ByteArray{1, 2}, "domain_name", 0, 0, "", 0, 0xFFFF}},
                    new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 0);
    REQUIRE(psj.add(border_router{EMPTY_ID, EMPTY_ID,
                                  BorderAgent{"1.1.1.3", 12, "th1.x", BorderAgent::State{1, 0, 1, 0, 1}, "network_id",
                                              0x1011223344556677ll, "vendor_name", "model_name", Timestamp{0, 0, 0}, 1,
                                              "vendor_data", ByteArray{1, 2}, "domain_name", 0, 0, "", 0, 0xFFFF}},
                    new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 1);
    REQUIRE(psj.add(border_router{EMPTY_ID, EMPTY_ID,
                                  BorderAgent{"1.1.1.4", 13, "th1.x", BorderAgent::State{1, 0, 1, 0, 1}, "network_id",
                                              0x1011223344556677ll, "vendor_name", "model_name", Timestamp{0, 0, 0}, 1,
                                              "vendor_data", ByteArray{1, 2}, "domain_name", 0, 0, "", 0, 0xFFFF}},
                    new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 2);

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Del registrar", "[ps_json]")
{
    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    REQUIRE(psj.del(registrar_id(0)) == ps_status::PS_SUCCESS);
    REQUIRE(psj.del(registrar_id(1)) == ps_status::PS_SUCCESS);
    REQUIRE(psj.del(registrar_id(2)) == ps_status::PS_SUCCESS);
    REQUIRE(psj.del(registrar_id(50)) == ps_status::PS_SUCCESS);

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Del domain", "[ps_json]")
{
    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    REQUIRE(psj.del(domain_id(0)) == ps_status::PS_SUCCESS);
    REQUIRE(psj.del(domain_id(1)) == ps_status::PS_SUCCESS);
    REQUIRE(psj.del(domain_id(2)) == ps_status::PS_SUCCESS);
    REQUIRE(psj.del(domain_id(50)) == ps_status::PS_SUCCESS);

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Del network", "[ps_json]")
{
    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    REQUIRE(psj.del(network_id(0)) == ps_status::PS_SUCCESS);
    REQUIRE(psj.del(network_id(1)) == ps_status::PS_SUCCESS);
    REQUIRE(psj.del(network_id(2)) == ps_status::PS_SUCCESS);
    REQUIRE(psj.del(network_id(50)) == ps_status::PS_SUCCESS);

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Del br", "[ps_json]")
{
    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    REQUIRE(psj.del(border_router_id(0)) == ps_status::PS_SUCCESS);
    REQUIRE(psj.del(border_router_id(1)) == ps_status::PS_SUCCESS);
    REQUIRE(psj.del(border_router_id(2)) == ps_status::PS_SUCCESS);
    REQUIRE(psj.del(border_router_id(50)) == ps_status::PS_SUCCESS);

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Get registrar from empty", "[ps_json]")
{
    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    registrar ret_val;

    REQUIRE(psj.get(registrar_id(0), ret_val) == ps_status::PS_NOT_FOUND);

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Get domain from empty", "[ps_json]")
{
    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    domain ret_val;

    REQUIRE(psj.get(domain_id(0), ret_val) == ps_status::PS_NOT_FOUND);

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Get network from empty", "[ps_json]")
{
    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    network ret_val;

    REQUIRE(psj.get(network_id(0), ret_val) == ps_status::PS_NOT_FOUND);

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Get br from empty", "[ps_json]")
{
    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    border_router ret_val;

    REQUIRE(psj.get(border_router_id(0), ret_val) == ps_status::PS_NOT_FOUND);

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Get registrar, not empty", "[ps_json]")
{
    // Make test independent
    unlink("./test.tmp");

    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    registrar_id new_id;

    REQUIRE(psj.add(registrar{EMPTY_ID, "0.0.0.1", 1, {"dom1"}}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 0);
    REQUIRE(psj.add(registrar{EMPTY_ID, "0.0.0.2", 2, {"dom2"}}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 1);
    REQUIRE(psj.add(registrar{EMPTY_ID, "0.0.0.3", 3, {"dom3"}}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 2);

    registrar ret_val;

    REQUIRE(psj.get(registrar_id(3), ret_val) == ps_status::PS_NOT_FOUND);
    REQUIRE(psj.get(registrar_id(1), ret_val) == ps_status::PS_SUCCESS);
    REQUIRE(ret_val.id.id == 1);
    REQUIRE(ret_val.addr == "0.0.0.2");
    REQUIRE(ret_val.port == 2);
    REQUIRE(ret_val.domains == std::vector<std::string>{"dom2"});

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Get domain, not empty", "[ps_json]")
{
    // Make test independent
    unlink("./test.tmp");

    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    domain_id new_id;

    REQUIRE(psj.add(domain{EMPTY_ID, "dom1"}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 0);
    REQUIRE(psj.add(domain{EMPTY_ID, "dom2"}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 1);
    REQUIRE(psj.add(domain{EMPTY_ID, "dom3"}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 2);

    domain ret_val;

    REQUIRE(psj.get(domain_id(3), ret_val) == ps_status::PS_NOT_FOUND);
    REQUIRE(psj.get(domain_id(0), ret_val) == ps_status::PS_SUCCESS);
    REQUIRE(ret_val.id.id == 0);
    REQUIRE(ret_val.name == "dom1");

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Get network, not empty", "[ps_json]")
{
    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    network_id new_id;

// TODO define domain processing
#if 0
    REQUIRE(psj.add(network{EMPTY_ID, "nwk1", "dom1", "FFFFFFFFFFFFFFF1", 11, "FFF1", "2000:aaa1::0/8", 1}, new_id) ==
            ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 3);
    REQUIRE(psj.add(network{EMPTY_ID, "nwk2", "dom2", "FFFFFFFFFFFFFFF2", 12, "FFF2", "2000:aaa2::0/8", 1}, new_id) ==
            ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 4);
    REQUIRE(psj.add(network{EMPTY_ID, "nwk3", "dom3", "FFFFFFFFFFFFFFF3", 13, "FFF3", "2000:aaa3::0/8", 1}, new_id) ==
            ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 5);

    network ret_val;

    REQUIRE(psj.get(network_id(0), ret_val) == ps_status::PS_NOT_FOUND);
    REQUIRE(psj.get(network_id(5), ret_val) == ps_status::PS_SUCCESS);
    REQUIRE(ret_val.id.id == 5);
    REQUIRE(ret_val.name == "nwk3");
    REQUIRE(ret_val.domain_name == "dom3");
    REQUIRE(ret_val.channel == 13);
#endif

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Get br, not empty", "[ps_json]")
{
    // Make the test independent
    unlink("./test.tmp");

    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    border_router_id new_id;

    REQUIRE(psj.add(border_router{EMPTY_ID, EMPTY_ID,
                                  BorderAgent{"1.1.1.2", 11, "th1.x", BorderAgent::State{1, 0, 1, 0, 1}, "network_id",
                                              0x1011223344556677ll, "vendor_name", "model_name", Timestamp{0, 0, 0}, 1,
                                              "vendor_data", ByteArray{1, 2}, "domain_name", 0, 0, "", 0, 0xFFFF}},
                    new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 0);
    REQUIRE(psj.add(border_router{EMPTY_ID, EMPTY_ID,
                                  BorderAgent{"1.1.1.3", 12, "th1.x", BorderAgent::State{1, 0, 1, 0, 1}, "network_id",
                                              0x1011223344556677ll, "vendor_name", "model_name", Timestamp{0, 0, 0}, 1,
                                              "vendor_data", ByteArray{1, 2}, "domain_name", 0, 0, "", 0, 0xFFFF}},
                    new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 1);
    REQUIRE(psj.add(border_router{EMPTY_ID, EMPTY_ID,
                                  BorderAgent{"1.1.1.4", 13, "th1.x", BorderAgent::State{1, 0, 1, 0, 1}, "network_id",
                                              0x1011223344556677ll, "vendor_name", "model_name", Timestamp{0, 0, 0}, 1,
                                              "vendor_data", ByteArray{1, 2}, "domain_name", 0, 0, "", 0, 0xFFFF}},
                    new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 2);

    border_router ret_val;

    REQUIRE(psj.get(border_router_id(3), ret_val) == ps_status::PS_NOT_FOUND);
    REQUIRE(psj.get(border_router_id(1), ret_val) == ps_status::PS_SUCCESS);
    REQUIRE(ret_val.id.id == 1);
    REQUIRE(ret_val.agent.mPort == 12);
    REQUIRE(ret_val.agent.mAddr == "1.1.1.3");

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

// UPD
TEST_CASE("Upd registrar", "[ps_json]")
{
    // Make the test independent
    unlink("./test.tmp");

    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    // Add initial data
    registrar_id new_id;

    REQUIRE(psj.add(registrar{EMPTY_ID, "0.0.0.1", 1, {"dom1"}}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(psj.add(registrar{EMPTY_ID, "0.0.0.2", 2, {"dom2"}}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(psj.add(registrar{EMPTY_ID, "0.0.0.3", 3, {"dom3"}}, new_id) == ps_status::PS_SUCCESS);

    // Test actions
    registrar new_val{EMPTY_ID, "4.4.4.4", 1, {"dom4"}};

    REQUIRE(psj.update(new_val) == ps_status::PS_NOT_FOUND);
    new_val.id = 2;
    REQUIRE(psj.update(new_val) == ps_status::PS_SUCCESS);

    registrar ret_val;

    REQUIRE(psj.get(registrar_id(2), ret_val) == ps_status::PS_SUCCESS);
    REQUIRE(ret_val.id.id == 2);
    REQUIRE(ret_val.addr == "4.4.4.4");
    REQUIRE(ret_val.port == 1);
    REQUIRE(ret_val.domains == std::vector<std::string>{"dom4"});

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Upd domain", "[ps_json]")
{
    // Make the test independent
    unlink("./test.tmp");

    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    // Add initial data
    domain_id new_id;

    REQUIRE(psj.add(domain{EMPTY_ID, "dom1"}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(psj.add(domain{EMPTY_ID, "dom2"}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(psj.add(domain{EMPTY_ID, "dom3"}, new_id) == ps_status::PS_SUCCESS);

    // Test actions
    domain new_val{EMPTY_ID, "dom_upd"};

    REQUIRE(psj.update(new_val) == ps_status::PS_NOT_FOUND);
    new_val.id = 1;
    REQUIRE(psj.update(new_val) == ps_status::PS_SUCCESS);

    domain ret_val;

    REQUIRE(psj.get(domain_id(1), ret_val) == ps_status::PS_SUCCESS);
    REQUIRE(ret_val.id.id == 1);
    REQUIRE(ret_val.name == "dom_upd");

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Upd network", "[ps_json]")
{
    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

// Add domain processing
#if 0
    network new_val{EMPTY_ID, "nwk_upd", "dom_upd", "FFFFFFFFFFFFFFFA", 18, "FFFA", "2000:aaa1::0/64", 0};

    REQUIRE(psj.update(new_val) == ps_status::PS_NOT_FOUND);
    new_val.id = 5;
    REQUIRE(psj.update(new_val) == ps_status::PS_SUCCESS);

    network ret_val;

    REQUIRE(psj.get(network_id(5), ret_val) == ps_status::PS_SUCCESS);
    REQUIRE(ret_val.id.id == 5);
    REQUIRE(ret_val.name == "nwk_upd");
    REQUIRE(ret_val.domain_name == "dom_upd");
    REQUIRE(ret_val.channel == 18);
#endif

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Upd br", "[ps_json]")
{
    // Make test independent
    unlink("./test.tmp");

    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    // Add initial data
    border_router_id new_id;

    REQUIRE(psj.add(border_router{EMPTY_ID, EMPTY_ID,
                                  BorderAgent{"1.1.1.2", 11, "th1.x", BorderAgent::State{1, 0, 1, 0, 1}, "network_id",
                                              0x1011223344556677ll, "vendor_name", "model_name", Timestamp{0, 0, 0}, 1,
                                              "vendor_data", ByteArray{1, 2}, "domain_name", 0, 0, "", 0, 0xFFFF}},
                    new_id) == ps_status::PS_SUCCESS);
    REQUIRE(psj.add(border_router{EMPTY_ID, EMPTY_ID,
                                  BorderAgent{"1.1.1.3", 12, "th1.x", BorderAgent::State{1, 0, 1, 0, 1}, "network_id",
                                              0x1011223344556677ll, "vendor_name", "model_name", Timestamp{0, 0, 0}, 1,
                                              "vendor_data", ByteArray{1, 2}, "domain_name", 0, 0, "", 0, 0xFFFF}},
                    new_id) == ps_status::PS_SUCCESS);
    REQUIRE(psj.add(border_router{EMPTY_ID, EMPTY_ID,
                                  BorderAgent{"1.1.1.4", 13, "th1.x", BorderAgent::State{1, 0, 1, 0, 1}, "network_id",
                                              0x1011223344556677ll, "vendor_name", "model_name", Timestamp{0, 0, 0}, 1,
                                              "vendor_data", ByteArray{1, 2}, "domain_name", 0, 0, "", 0, 0xFFFF}},
                    new_id) == ps_status::PS_SUCCESS);

    // Test actions
    border_router new_val{EMPTY_ID, EMPTY_ID,
                          BorderAgent{"5.5.5.5", 18, "th1.x", BorderAgent::State{0, 0, 2, 0, 0}, "network_id",
                                      0x1011223344556677ll, "vendor_name", "model_name", Timestamp{0, 0, 0}, 1,
                                      "vendor_data", ByteArray{1, 2}, "domain_name", 0, 0, "", 0, 0xFFFF}};

    REQUIRE(psj.update(new_val) == ps_status::PS_NOT_FOUND);
    new_val.id = 2;
    REQUIRE(psj.update(new_val) == ps_status::PS_SUCCESS);

    border_router ret_val;

    REQUIRE(psj.get(border_router_id(2), ret_val) == ps_status::PS_SUCCESS);
    REQUIRE(ret_val.id.id == 2);
    REQUIRE(ret_val.agent.mPort == 18);
    REQUIRE(ret_val.agent.mAddr == "5.5.5.5");

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Lookup registrar", "[ps_json]")
{
    // Make test independent
    unlink("./test.tmp");

    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    //  Populate storage with test data
    registrar_id new_id;

    REQUIRE(psj.add(registrar{EMPTY_ID, "0.0.0.1", 1, {"dom1"}}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(psj.add(registrar{EMPTY_ID, "0.0.0.2", 1, {"dom2"}}, new_id) == ps_status::PS_SUCCESS);
    REQUIRE(psj.add(registrar{EMPTY_ID, "0.0.0.3", 3, {"dom3"}}, new_id) == ps_status::PS_SUCCESS);

    // Test actions
    std::vector<registrar> ret_lookup;

    registrar search_req{EMPTY_ID, "", 0, {}};
    REQUIRE(psj.lookup(search_req, ret_lookup) == ps_status::PS_SUCCESS);
    REQUIRE(ret_lookup.size() == 3);

    ret_lookup.clear();

    search_req.id.id = 0;
    REQUIRE(psj.lookup(search_req, ret_lookup) == ps_status::PS_SUCCESS);
    REQUIRE(ret_lookup.size() == 1);
    REQUIRE(ret_lookup[0].id.id == 0);

    ret_lookup.clear();

    search_req.id.id = 0;
    search_req.addr  = "0.0.0.2";
    REQUIRE(psj.lookup(search_req, ret_lookup) == ps_status::PS_NOT_FOUND);
    REQUIRE(ret_lookup.size() == 0);

    ret_lookup.clear();

    search_req.id.id = 0;
    search_req.addr  = "0.0.0.1";
    REQUIRE(psj.lookup(search_req, ret_lookup) == ps_status::PS_SUCCESS);
    REQUIRE(ret_lookup.size() == 1);
    REQUIRE(ret_lookup[0].id.id == 0);

    ret_lookup.clear();

    search_req.id.id = 0;
    search_req.addr  = "0.0.0.1";
    search_req.port  = 1;
    REQUIRE(psj.lookup(search_req, ret_lookup) == ps_status::PS_SUCCESS);
    REQUIRE(ret_lookup.size() == 1);
    REQUIRE(ret_lookup[0].id.id == 0);

    ret_lookup.clear();

    search_req.id.id   = 0;
    search_req.addr    = "0.0.0.1";
    search_req.port    = 1;
    search_req.domains = {"dom1"};
    REQUIRE(psj.lookup(search_req, ret_lookup) == ps_status::PS_SUCCESS);
    REQUIRE(ret_lookup.size() == 1);
    REQUIRE(ret_lookup[0].id.id == 0);

    ret_lookup.clear();

    search_req      = {};
    search_req.port = 1;
    REQUIRE(psj.lookup(search_req, ret_lookup) == ps_status::PS_SUCCESS);
    REQUIRE(ret_lookup.size() == 2);
    REQUIRE(ret_lookup[0].id.id == 0);
    REQUIRE(ret_lookup[1].id.id == 1);

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}

TEST_CASE("Lookup network", "[ps_json]")
{
    // Make test independent
    unlink("./test.tmp");

    persistent_storage_json psj("./test.tmp");

    REQUIRE(psj.open() == ps_status::PS_SUCCESS);

    // Populate storage with initial data
    network_id new_id;
    REQUIRE(psj.add(network{EMPTY_ID, EMPTY_ID, "nwk1", xpan_id{"FFFFFFFFFFFFFFF1"}, 11, "FFF1", "2000:aaa1::0/8", 1},
                    new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 0);
    REQUIRE(psj.add(network{EMPTY_ID, EMPTY_ID, "nwk2", xpan_id{"FFFFFFFFFFFFFFF2"}, 11, "FFF2", "2000:aaa2::0/8", 1},
                    new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 1);
    REQUIRE(psj.add(network{EMPTY_ID, EMPTY_ID, "nwk3", xpan_id{"FFFFFFFFFFFFFFF3"}, 11, "FFF3", "2000:aaa3::0/8", 1},
                    new_id) == ps_status::PS_SUCCESS);
    REQUIRE(new_id.id == 2);

    // The test
    std::vector<network> ret_lookup;

    REQUIRE(psj.lookup(network{}, ret_lookup) == ps_status::PS_SUCCESS);
    REQUIRE(ret_lookup.size() == 3);

    ret_lookup.clear();

    network net;
    net.name = "nwk1";
    net.ccm  = true;
    REQUIRE(psj.lookup(net, ret_lookup) == ps_status::PS_SUCCESS);
    REQUIRE(ret_lookup.size() == 1);

    REQUIRE(psj.close() == ps_status::PS_SUCCESS);
}
