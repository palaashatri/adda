#include "sim/SimRunner.h"
#include "sim/SimResult.h"
#include "db/DbCellLib.h"
#include "db/DbCell.h"
#include "db/DbView.h"

#include <cassert>
#include <cstdio>
#include <filesystem>
#include <string>

static int failures = 0;
#define CHECK(expr) do { \
  if (!(expr)) { \
    std::fprintf(stderr, "FAIL: %s (%s:%d)\n", #expr, __FILE__, __LINE__); \
    ++failures; \
  } \
} while(0)

static void test_write_netlist() {
  const auto workdir = std::filesystem::temp_directory_path() / "aurora_sim_test";

  aurora::db::DbCellLib lib("testlib");
  auto& cell = lib.createCell("inv");
  auto& view = cell.createView(aurora::db::DbViewType::Netlist);
  auto& vdd  = view.createNet("VDD");
  auto& gnd  = view.createNet("GND");
  auto& in   = view.createNet("IN");
  auto& out  = view.createNet("OUT");
  (void)view.createPin("VDD", aurora::db::DbPinDirection::Passive, vdd.id());
  (void)view.createPin("GND", aurora::db::DbPinDirection::Passive, gnd.id());
  (void)view.createPin("IN",  aurora::db::DbPinDirection::Input,   in.id());
  (void)view.createPin("OUT", aurora::db::DbPinDirection::Output,  out.id());

  aurora::sim::SimRunner runner(workdir);
  const bool ok = runner.writeSpiceNetlist(lib, cell, view);
  CHECK(ok);
  if (ok) {
    CHECK(std::filesystem::exists(runner.netlistPath()));
    CHECK(runner.netlistPath().filename() == "inv.spice");
  }
}

static void test_run_without_netlist_returns_error() {
  const auto workdir = std::filesystem::temp_directory_path() / "aurora_sim_empty";
  aurora::sim::SimRunner runner(workdir);
  const auto result = runner.run();
  CHECK(!result.success);
  CHECK(!result.errorMessage.empty());
}

static void test_workdir_accessor() {
  const auto workdir = std::filesystem::temp_directory_path() / "aurora_sim_acc";
  aurora::sim::SimRunner runner(workdir);
  CHECK(runner.workdir() == workdir);
}

int main() {
  test_write_netlist();
  test_run_without_netlist_returns_error();
  test_workdir_accessor();

  if (failures == 0) {
    std::puts("All SimRunner tests passed.");
    return 0;
  }
  std::fprintf(stderr, "%d test(s) failed.\n", failures);
  return 1;
}
