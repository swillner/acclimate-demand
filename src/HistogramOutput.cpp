/*
  Copyright (C) 2013-2014 Sven Willner <sven.willner@pik-potsdam.de>

  This file is part of Acclimate (basic and demand version).

  Acclimate is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  Acclimate is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with Acclimate.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "HistogramOutput.h"

#include <fstream>

#include "Model.h"
#include "Region.h"
#include "Sector.h"

HistogramOutput::HistogramOutput(XMLInput* settings, Model* model, Scenario* scenario, pugi::xml_node output_node)
    : Output(settings, model, scenario, output_node) {
    windows = 0;
    count = 0;
    min = 0;
    max = 1;
    exclude_max = false;
}

int HistogramOutput::initialize() {
    if (output_node.attribute("file").empty()) {
        std::cerr << "ERROR: Output file name not given" << std::endl;
        return -1;
    }
    if (output_node.child("windows").attribute("count").empty()) {
        std::cerr << "ERROR: Number of windows not given" << std::endl;
        return -1;
    }
    min = output_node.child("windows").attribute("min").as_double(0);
    max = output_node.child("windows").attribute("max").as_double(1);
    exclude_max = output_node.child("windows").attribute("exclude_max").as_bool(false);
    windows = output_node.child("windows").attribute("count").as_int(1);
    std::string filename = output_node.attribute("file").value();
    file.open(filename.c_str(), std::ofstream::out);
    count = new unsigned int[windows];
    return 0;
}

void HistogramOutput::internal_write_header(tm* timestamp, int max_threads, const char* version) {
    file << "# Start time: " << asctime(timestamp) << "# Version: " << version << "\n"
         << "# Max number of threads: " << max_threads << "\n";
}

void HistogramOutput::internal_write_footer(tm* duration) { file << "# Duration: " << mktime(duration) << "s\n"; }

void HistogramOutput::internal_write_xml_input(XMLInput* input) {
    std::stringstream ss;
    input->print(ss, "  ", pugi::format_indent | pugi::format_no_declaration, pugi::encoding_utf8, 0);
    ss.flush();
    ss.seekg(0);
    std::string line;
    file << "# Settings:\n";
    while (std::getline(ss, line)) {
        file << "# " << line << "\n";
    }
    file << "#\n";
}

void HistogramOutput::internal_iterate_begin() {
    for (int i = 0; i < windows; i++) {
        count[i] = 0;
    }
}

void HistogramOutput::internal_iterate_end() {
    for (int i = 0; i < windows; i++) {
        file << model->time << " " << (min + i * (max - min) / (windows - 1)) << " " << count[i] << "\n";
    }
    file << "\n";
}

void HistogramOutput::internal_end() { file.close(); }

void HistogramOutput::internal_write_double(const char* name, double v) {
    if (v >= min && (v < max || (v == max && !exclude_max))) {
        count[(int)round((v - min) * (windows - 1) / (max - min))]++;
    }
}
