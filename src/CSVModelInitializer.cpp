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

#include "CSVModelInitializer.h"

#include <math.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "BusinessConnection.h"
#include "FinalDemand.h"
#include "Model.h"
#include "OutputStorage.h"
#include "ProductionSite.h"
#include "Region.h"
#include "Sector.h"

CSVModelInitializer::CSVModelInitializer(Model* model, XMLInput* settings) : ModelInitializer(model, settings) { transport_time = 0; }

int CSVModelInitializer::read_transport_times(std::string index_filename, std::string filename) {
    std::vector<Region*> regions;
    std::ifstream index_file(index_filename.c_str());
    if (!index_file) {
        std::cerr << "ERROR: Could not open index file '" << index_filename << "'" << std::endl;
        return -1;
    }
    unsigned short index = 0;
    while (true) {
        std::string line;
        if (!std::getline(index_file, line)) {
            break;
        }
        if (line.empty() || (line.length() == 1 && line[0] == '\r')) {
            continue;
        }
        std::istringstream ss(line);

        std::string region_name;
        if (!std::getline(ss, region_name, ',')) {
            std::cerr << "ERROR: Unexpected end in index file" << std::endl;
            return -1;
        }
        if (!region_name.empty() && region_name[region_name.size() - 1] == '\r') {
            region_name.erase(region_name.size() - 1);
        }

        Region* region = model->find_region(region_name);
        if (!region) {
            region = new Region(model, region_name);
            model->regions__R.push_back(region);
        }

        regions.push_back(region);
        index++;
    }

    std::ifstream transport_delays_file(filename.c_str());
    if (!transport_delays_file) {
        std::cerr << "ERROR: Could not open transport delays file '" << filename << "'" << std::endl;
        return -1;
    }

    TransportDelay transport_delay__tau = 1;
    std::string transport_line;

    for (int row = 0; row < regions.size(); row++) {
        if (!std::getline(transport_delays_file, transport_line)) {
            std::cerr << "ERROR: Index and transport_delays are not consistent: Not enough rows" << std::endl;
            return -1;
        }
        if (transport_line.empty() || (transport_line.length() == 1 && transport_line[0] == '\r')) {
            row--;
            continue;
        }

        std::istringstream transport_string_stream(transport_line);
        std::string transport_str;

        for (int col = 0; col < regions.size(); col++) {
            if (!std::getline(transport_string_stream, transport_str, ',')) {
                std::cerr << "ERROR: Index and transport_delays are not consistent: Not enough columns in row " << row << std::endl;
                return -1;
            }
            if (!transport_str.empty() && transport_str[transport_str.size() - 1] == '\r') {
                transport_str.erase(transport_str.size() - 1);
            }
            transport_delay__tau = atoi(transport_str.c_str());
            if (transport_delay__tau <= 0 || transport_delay__tau > 254) {
                std::cerr << "ERROR: Transport delay not valid: " << transport_delay__tau << std::endl;
                return -1;
            }
            Infrastructure* inf = new Infrastructure(transport_delay__tau);
            regions[row]->connections.push_back(inf);
            regions[col]->connections.push_back(inf);
            inf->connections.push_back(regions[row]);
            inf->connections.push_back(regions[col]);
            model->infrastructure__G.push_back(inf);
        }
    }

    return 0;
}

int CSVModelInitializer::read_index_file(std::string index_filename, std::vector<EconomicAgent*>* economic_agents) {
    std::ifstream index_file(index_filename.c_str());
    if (!index_file) {
        std::cerr << "ERROR: Could not open index file '" << index_filename << "'" << std::endl;
        return -1;
    }
    int index = 1;
    while (true) {
        std::string line;
        if (!std::getline(index_file, line)) {
            break;
        }
        if (line.empty() || (line.length() == 1 && line[0] == '\r')) {
            continue;
        }
        std::istringstream ss(line);

        std::string region_name;
        if (!std::getline(ss, region_name, ',')) {
            std::cerr << "ERROR: Unexpected end in index file" << std::endl;
            return -1;
        }

        std::string sector_name;
        if (!std::getline(ss, sector_name, ',')) {
            std::cerr << "ERROR: Unexpected end in index file" << std::endl;
            return -1;
        }
        if (!sector_name.empty() && sector_name[sector_name.size() - 1] == '\r') {
            sector_name.erase(sector_name.size() - 1);
        }

        Region* region = model->find_region(region_name);
        if (!region) {
            region = new Region(model, region_name);
            model->regions__R.push_back(region);
            if (transport_time != 0) {
                for (std::vector<Region*>::const_iterator it = model->regions__R.begin(); it != model->regions__R.end(); it++) {
                    Infrastructure* inf = new Infrastructure(transport_time);
                    region->connections.push_back(inf);
                    (*it)->connections.push_back(inf);
                    inf->connections.push_back(region);
                    inf->connections.push_back(*it);
                    model->infrastructure__G.push_back(inf);
                }
            }
        }

        if (sector_name == "FD") {
            FinalDemand* final_demand = model->find_final_demand(region);
            if (!final_demand) {
                final_demand = add_final_demand(region);
                economic_agents->push_back(final_demand);
            } else {
                std::cerr << "ERROR: Duplicate final demand for region " << region_name << std::endl;
                return -1;
            }
        } else {
            Sector* sector = model->find_sector(sector_name);
            if (!sector) {
                sector = add_sector(sector_name);
            }

            ProductionSite* production_site = model->find_production_site(sector, region->name);
            if (!production_site) {
                production_site = add_production_site(sector, region);
                if (!production_site) {
                    return -1;
                }
            }
            economic_agents->push_back(production_site);
        }
        index++;
    }
    return 0;
}

int CSVModelInitializer::read_flows(std::string filename, std::vector<EconomicAgent*>* economic_agents_row, std::vector<EconomicAgent*>* economic_agents_col) {
    std::ifstream flows_file(filename.c_str());
    if (!flows_file) {
        std::cerr << "ERROR: Could not open flows file '" << filename << "'" << std::endl;
        return -1;
    }

    Flow flow;
    std::string flows_line;

    for (int row = 0; row < economic_agents_row->size(); row++) {
        if (!std::getline(flows_file, flows_line)) {
            std::cerr << "ERROR: Index and flows are not consistent: Not enough rows" << std::endl;
            return -1;
        }
        if (flows_line.empty() || (flows_line.length() == 1 && flows_line[0] == '\r')) {
            row--;
            continue;
        }

        if (economic_agents_row->at(row)->type == PRODUCTION_SITE) {
            ProductionSite* production_site_from = (ProductionSite*)economic_agents_row->at(row);

            std::istringstream flows_string_stream(flows_line);
            std::string flows_str;

            for (int col = 0; col < economic_agents_col->size(); col++) {
                if (!std::getline(flows_string_stream, flows_str, ',')) {
                    std::cerr << "ERROR: Index and flows are not consistent: Not enough columns in row " << row << std::endl;
                    return -1;
                }
                if (!flows_str.empty() && flows_str[flows_str.size() - 1] == '\r') {
                    flows_str.erase(flows_str.size() - 1);
                }
                flow = atof(flows_str.c_str());

                EconomicAgent* economic_agent_to = economic_agents_col->at(col);
                if (flow > flow_threshold) {
                    int res = initialize_connection(production_site_from, economic_agent_to, MAKE_FLOW(flow * model->delta_t / 365.0));
                    if (res != 0) {
                        return res;
                    }
                }
            }
        }
    }
    return 0;
}

int CSVModelInitializer::read_flows(std::string index_filename, std::string filename) {
    std::vector<EconomicAgent*> economic_agents;
    int res;
    res = read_index_file(index_filename, &economic_agents);
    if (res) {
        return res;
    }
    res = read_flows(filename, &economic_agents, &economic_agents);
    return res;
}

int CSVModelInitializer::read_flows(std::string index_filename_row, std::string index_filename_col, std::string filename) {
    std::vector<EconomicAgent*> economic_agents_row;
    std::vector<EconomicAgent*> economic_agents_col;
    int res;
    res = read_index_file(index_filename_row, &economic_agents_row);
    if (res) {
        return res;
    }
    res = read_index_file(index_filename_col, &economic_agents_col);
    if (res) {
        return res;
    }
    res = read_flows(filename, &economic_agents_row, &economic_agents_col);
    return res;
}

int CSVModelInitializer::initialize() {
    if (flow_threshold < 0) {
        std::cerr << "ERROR: No valid value for 'threshold' given" << std::endl;
        return -1;
    }
    int res;

    // Set transport times
    pugi::xml_node node = settings->child("network").child("transport");
    if (node) {
        transport_time = node.attribute("value").as_int(0);
        if (!node.attribute("file").empty() && !node.attribute("index").empty()) {
            int res = read_transport_times(node.attribute("index").value(), node.attribute("file").value());
            if (res) {
                return res;
            }
        }
    }

    // Read all flows
    for (pugi::xml_node node = settings->child("network").child("flows"); node; node = node.next_sibling("flows")) {
        if (node.attribute("data").empty()) {
            std::cerr << "ERROR: Network flows file not given" << std::endl;
            return -1;
        }
        if (node.attribute("index").empty() && (node.attribute("row_index").empty() || node.attribute("col_index").empty())) {
            std::cerr << "ERROR: Network index file not given" << std::endl;
            return -1;
        }
        if (node.attribute("index").empty()) {
            res = read_flows(node.attribute("row_index").value(), node.attribute("col_index").value(), node.attribute("data").value());
            if (res) {
                return res;
            }
        } else {
            res = read_flows(node.attribute("index").value(), node.attribute("data").value());
            if (res) {
                return res;
            }
        }
    }

    res = clean_network();
    return res;
}
