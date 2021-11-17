
#include "plotting_sciplot.hpp"

#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>
#include <cmath>
#include <iostream>
#include <regex>
#include <sciplot/sciplot.hpp>
#include <tuple>
#include <vector>

namespace coronan::cli::plotting {
};

namespace {
// wrapper class to offer an easy interface for adding one curve to the plot
// (mainly handles the special case we're confronted with here: dates on the x-axis)
class CoronanPlot : public sciplot::Plot
{
public:
  template <typename Time_t, typename Data_t>
  void plot_curve(std::vector<Time_t> const& t, std::vector<Data_t> const& d, std::string const& label);
};

template <typename Time_t, typename Data_t>
void CoronanPlot::plot_curve(std::vector<Time_t> const& t, std::vector<Data_t> const& d, std::string const& label)
{
  auto& curve_specs = this->drawCurve(t, d);
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
struct curves_data
{
  curves_data() = default;
  curves_data(std::size_t size)
  {
    t.reserve(size);
    active.reserve(size);
    confirmed.reserve(size);
    deaths.reserve(size);
    recovered.reserve(size);
  }
  std::vector<Poco::DateTime> t;
  std::vector<decltype(coronan::CountryData::TimelineData::active)> active;
  std::vector<decltype(coronan::CountryData::TimelineData::confirmed)> confirmed;
  std::vector<decltype(coronan::CountryData::TimelineData::deaths)> deaths;
  std::vector<decltype(coronan::CountryData::TimelineData::recovered)> recovered;
};

// date formatting according to ISO 8601
std::string const gp_dateformat = "%Y-%m-%d";
// date formatting as provided by the corona-api.com service
std::string const covid_dateformat = "%Y-%m-%dT%H:%M:%S.%iZ";

void plot_data(std::string const& out_file, coronan::CountryData const& country_data)
{
  CoronanPlot plot;
  plot.xlabel("Time");
  plot.ylabel("Cases");

  // collect the data and store the data in separate vectors
  // (i.e. map the vector of structs to a struct of vectors)
  curves_data d(country_data.timeline.size());
  for (auto const& timeline_entry : country_data.timeline)
  {
    d.t.emplace_back(convert_date_str_to_poco_date(timeline_entry.date));
    d.active.emplace_back(timeline_entry.active);
    d.confirmed.emplace_back(timeline_entry.confirmed);
    d.deaths.emplace_back(timeline_entry.deaths);
    d.recovered.emplace_back(timeline_entry.recovered);
  }

  plot.plot_curve(d.t, d.active, "active");
  plot.plot_curve(d.t, d.confirmed, "confirmed");
  plot.plot_curve(d.t, d.deaths, "deaths");
  plot.plot_curve(d.t, d.recovered, "recovered");

  // special commands to let Gnuplot know that the data in 1st column
  // are of date/time format.
  plot.gnuplot("set xdata time");
  plot.gnuplot("set timefmt \"" + gp_dateformat + "\"");
  plot.gnuplot("set format x \"" + gp_dateformat + "\"");
  plot.gnuplot("set xtics " + std::to_string(get_suitable_xtics(d.t)));

  sciplot::Figure fig{{plot}};
  fig.title("COVID-19 data for " + country_data.info.name);
  fig.size(900, 600);
  fig.save(out_file);
}

Poco::DateTime convert_date_str_to_poco_date(std::string const& s)
{
  int tzd = 0;
  return Poco::DateTimeParser::parse(covid_dateformat, s, tzd);
}

std::tuple<std::string, std::string, int> parse_gp_plot_string(std::string const& s)
{
  std::regex re{"('[^']+' index \\d+) with ([a-z]+) .*linestyle (\\d+)"};

  if (std::smatch m; regex_search(s, m, re) && m.size() == 4)
    return std::make_tuple(m[1], m[2], std::stoi(m[3]));
  else
    throw std::runtime_error("regex doesn't match on \"" + s + "\"");
}

int get_suitable_xtics(std::vector<Poco::DateTime> const& t, int num_tics)
{
  auto dt = t.front() - t.back();
  return std::abs(dt.totalSeconds()) / num_tics;
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
  return Poco::DateTimeFormatter::format(val, coronan::cli::plotting::gp_dateformat);
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
