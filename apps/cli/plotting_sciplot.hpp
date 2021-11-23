
#pragma once

#include "coronan/corona-api_client.hpp"

#include <Poco/DateTime.h>
#include <string>

namespace coronan::cli::plotting {
void plot_data(std::string const& out_file, coronan::CountryData const& country_data);

Poco::DateTime convert_date_str_to_poco_date(std::string const& s);
std::tuple<std::string, std::string, int> parse_gp_plot_string(std::string const& s);
int get_suitable_xtics(std::vector<Poco::DateTime> const& t, int num_tics = 5);
} // namespace coronan::cli::plotting
