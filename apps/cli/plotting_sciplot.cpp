
#include "plotting_sciplot.hpp"

#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>
#include <cmath>
#include <iostream>
#include <regex>
#include <sciplot/sciplot.hpp>
#include <tuple>
#include <utility>
#include <vector>

namespace {
// wrapper class to offer an easy interface for adding one curve to the plot
// (mainly handles the special case we're confronted with here: dates on the x-axis)
class CoronanPlot : public sciplot::Plot
{
public:
  template <typename Time_t, typename Data_t>
  void plot_curve(std::pair<std::vector<Time_t> const&, std::vector<Data_t> const&> data, std::string const& label);
};

template <typename Time_t, typename Data_t>
void CoronanPlot::plot_curve(std::pair<std::vector<Time_t> const&, std::vector<Data_t> const&> data, std::string const& label)
{
  auto& curve_specs = this->drawCurve(data.first, data.second);
  try
  {
    // sciplot assumes that the data in the 1st column are numeric values. Therefore
    // it doesn't produce the "using ..." string which, however, is needed in Gnuplot
    // for the date/time input. Therefore the specifications for the string are re-
    // written here to produce a "using 1:2" in the plot command.
    auto [what, with, linestyle] = coronan::cli::plotting::parse_gp_plot_string(curve_specs.repr());
    curve_specs = sciplot::DrawSpecs(what, "1:2", with).lineStyle(linestyle).label(label);
  }
  catch (std::exception const& e)
  {
    std::cerr << "updating the gp string failed: " << e.what() << std::endl;
  }
}
} // namespace

namespace coronan::cli::plotting {
class curves_data
{
public:
  using data_time_t = Poco::DateTime;
  using data_active_t = decltype(coronan::CountryData::TimelineData::active);
  using data_confirmed_t = decltype(coronan::CountryData::TimelineData::confirmed);
  using data_deaths_t = decltype(coronan::CountryData::TimelineData::deaths);
  using data_recovered_t = decltype(coronan::CountryData::TimelineData::recovered);

  curves_data() = default;
  explicit curves_data(std::size_t size)
  {
    t.reserve(size);
    active.reserve(size);
    confirmed.reserve(size);
    deaths.reserve(size);
    recovered.reserve(size);
  }

  void append_data(data_time_t&& time, data_active_t active_cases, data_confirmed_t confirmed_cases,
                   data_deaths_t deaths_cases, data_recovered_t recovered_cases);
  [[nodiscard]] std::vector<data_time_t> const& get_time() const { return t; }
  [[nodiscard]] auto get_active_cases() const
  {
    return std::pair<std::vector<data_time_t> const&, std::vector<data_active_t> const&>(t, active);
  }
  [[nodiscard]] auto get_confirmed_cases() const
  {
    return std::pair<std::vector<data_time_t> const&, std::vector<data_confirmed_t> const&>(t, confirmed);
  }
  [[nodiscard]] auto get_deaths_cases() const
  {
    return std::pair<std::vector<data_time_t> const&, std::vector<data_deaths_t> const&>(t, deaths);
  }
  [[nodiscard]] auto get_recovered_cases() const
  {
    return std::pair<std::vector<data_time_t> const&, std::vector<data_recovered_t> const&>(t, recovered);
  }

private:
  std::vector<data_time_t> t;
  std::vector<data_active_t> active;
  std::vector<data_confirmed_t> confirmed;
  std::vector<data_deaths_t> deaths;
  std::vector<data_recovered_t> recovered;
};

// date formatting according to ISO 8601
constexpr std::string_view gp_dateformat = "%Y-%m-%d";
// date formatting as provided by the corona-api.com service
constexpr std::string_view covid_dateformat = "%Y-%m-%dT%H:%M:%S.%iZ";

// size of the plot output graphic WxH
constexpr std::pair<std::size_t, std::size_t> plot_size{900, 600};

void plot_data(std::string const& out_file, coronan::CountryData const& country_data)
{
  CoronanPlot plot;
  plot.xlabel("Time");
  plot.ylabel("Cases");

  // collect the data and store the data in separate vectors
  // (i.e. map the vector of structs to a struct of vectors)
  curves_data d(country_data.timeline.size());
  std::for_each(country_data.timeline.cbegin(), country_data.timeline.cend(), 
    [&d](auto const& te){d.append_data(convert_date_str_to_poco_date(te.date), te.active, te.confirmed, te.deaths, te.recovered);});

  plot.plot_curve(d.get_active_cases(), "active");
  plot.plot_curve(d.get_confirmed_cases(), "confirmed");
  plot.plot_curve(d.get_deaths_cases(), "deaths");
  plot.plot_curve(d.get_recovered_cases(), "recovered");

  // special commands to let Gnuplot know that the data in 1st column
  // are of date/time format.
  plot.gnuplot("set xdata time");
  plot.gnuplot("set timefmt \"" + std::string(gp_dateformat) + "\"");
  plot.gnuplot("set format x \"" + std::string(gp_dateformat) + "\"");
  plot.gnuplot("set xtics " + std::to_string(get_suitable_xtics(d.get_time())));

  sciplot::Figure fig{{plot}};
  fig.title("COVID-19 data for " + country_data.info.name);
  fig.size(plot_size.first, plot_size.second);
  fig.save(out_file);
}

Poco::DateTime convert_date_str_to_poco_date(std::string const& s)
{
  int tzd = 0;
  return Poco::DateTimeParser::parse(std::string(covid_dateformat), s, tzd);
}

std::tuple<std::string, std::string, int> parse_gp_plot_string(std::string const& s)
{
  std::regex re{"('[^']+' index \\d+) with ([a-z]+) .*linestyle (\\d+)"};

  std::smatch re_match;
  if (!std::regex_search(s, re_match, re) || (re_match.size() != 4)) {
    throw std::runtime_error("regex doesn't match on \"" + s + "\"");
  }

  return std::make_tuple(re_match[1], re_match[2], std::stoi(re_match[3]));
}

int get_suitable_xtics(std::vector<Poco::DateTime> const& t, int num_tics)
{
  auto dt = t.front() - t.back();
  return std::abs(dt.totalSeconds()) / num_tics;
}


void curves_data::append_data(data_time_t&& time, data_active_t active_cases, data_confirmed_t confirmed_cases,
                              data_deaths_t deaths_cases, data_recovered_t recovered_cases)
{
  t.emplace_back(time);
  active.push_back(active_cases);
  confirmed.push_back(confirmed_cases);
  deaths.push_back(deaths_cases);
  recovered.push_back(recovered_cases);
}
} // namespace coronan::cli::plotting

namespace sciplot::internal {
// Gnuplot time format in the data file is a special case. It's actually a string
// but the strings are not to be interpreted as labels. Therefore they need to be
// printed without quotes. To achieve that the (sciplot internal) function
// escapeIfNeeded() can be specialized.
template <>
auto escapeIfNeeded<Poco::DateTime>(Poco::DateTime const& val)
{
  return Poco::DateTimeFormatter::format(val, std::string(coronan::cli::plotting::gp_dateformat));
}

// sciplot assumes that the data in the vectors are either string or numbers. Since
// we want to handle missing data using the std::optional container, we need a
// template specialization to handle this case and print the missing symbol in case
// data is unavailable
template <>
auto escapeIfNeeded<decltype(coronan::CountryData::TimelineData::active)>(
    decltype(coronan::CountryData::TimelineData::active) const& val)
{
  return val ? std::to_string(val.value()) : sciplot::MISSING_INDICATOR;
}
} // namespace sciplot::internal
