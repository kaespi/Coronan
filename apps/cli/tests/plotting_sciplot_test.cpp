#include "plotting_sciplot.hpp"

#include <catch2/catch.hpp>

namespace {

TEST_CASE("Converting a corona-api date to a Poco::DateTime", "[date]")
{
  SECTION("Converting a valid corona-api time string")
  {
    std::string const valid_time_string = "2021-10-24T04:21:22.000Z";
    auto const poco_date_from_string = coronan::cli::plotting::convert_date_str_to_poco_date(valid_time_string);
    Poco::DateTime poco_date_ref(2021, 10, 24, 4, 21, 22, 0);
    REQUIRE(poco_date_from_string==poco_date_ref);
  }
}

TEST_CASE("Parse a Gnuplot plotting command string", "[gnuplot]")
{
  SECTION("Parse a valid plot command, first curve")
  {
    auto const gp_cmd = "'plot0.dat' index 0 with lines linestyle 1 linewidth 2";
    auto const [what, with, linestyle] = coronan::cli::plotting::parse_gp_plot_string(gp_cmd);
    REQUIRE(what=="'plot0.dat' index 0");
    REQUIRE(with=="lines");
    REQUIRE(linestyle==1);
  }

  SECTION("Parse a valid plot command, N-th curve")
  {
    auto const gp_cmd = "'plot0.dat' index 28 with points linestyle 1333";
    auto const [what, with, linestyle] = coronan::cli::plotting::parse_gp_plot_string(gp_cmd);
    REQUIRE(what=="'plot0.dat' index 28");
    REQUIRE(with=="points");
    REQUIRE(linestyle==1333);
  }

  SECTION("Parse an invalid plot command, index missing")
  {
    auto const gp_cmd = "'plot0.dat' with lines linestyle 1";
    REQUIRE_THROWS(coronan::cli::plotting::parse_gp_plot_string(gp_cmd));
  }

  SECTION("Parse an invalid plot command, datafile missing")
  {
    auto const gp_cmd = "index 0 with lines linestyle 1";
    REQUIRE_THROWS(coronan::cli::plotting::parse_gp_plot_string(gp_cmd));
  }

  SECTION("Parse an invalid plot command, linestyle missing")
  {
    auto const gp_cmd = "'plot0.dat' index 0 with lines linewidth 2";
    REQUIRE_THROWS(coronan::cli::plotting::parse_gp_plot_string(gp_cmd));
  }
}

TEST_CASE("Plot xtics derivation test", "[gnuplot]")
{
  SECTION("Derive xtics value for a one year data set")
  {
    std::vector<Poco::DateTime> t{Poco::DateTime(2020,1,1), Poco::DateTime(2020,8,1), Poco::DateTime(2021,1,1)};
    auto const xt_sec = coronan::cli::plotting::get_suitable_xtics(t, 12);
    auto constexpr num_sec_per_day = 24*3600;
    auto constexpr avg_num_sec_per_month = 30*num_sec_per_day;
    REQUIRE(std::abs(xt_sec-avg_num_sec_per_month) < (3*num_sec_per_day));
  }

  SECTION("Derive xtics value for a one year data set, inverse order")
  {
    std::vector<Poco::DateTime> t{Poco::DateTime(2021,1,1), Poco::DateTime(2020,8,1), Poco::DateTime(2020,1,1)};
    auto const xt_sec = coronan::cli::plotting::get_suitable_xtics(t, 12);
    auto constexpr num_sec_per_day = 24*3600;
    auto constexpr avg_num_sec_per_month = 30*num_sec_per_day;
    REQUIRE(std::abs(xt_sec-avg_num_sec_per_month) < (3*num_sec_per_day));
  }
}

} // namespace
