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

#include "Scenario.h"

#include <cstring>
#include <iostream>

#include "CapacityManager.h"
#include "EconomicAgent.h"
#include "FinalDemand.h"
#include "Model.h"
#include "OutputStorage.h"
#include "ProductionSite.h"
#include "Region.h"
#include "SalesManager.h"
#include "Sector.h"

Scenario::Scenario(XMLInput* settings, Model* model) : model(model) {
    scenario_node = settings->child("scenario");
    pugi::xml_node timespan_node = scenario_node.child("timespan");
    start_time = (Time)timespan_node.attribute("start").as_double(0);
    stop_time = (Time)timespan_node.attribute("stop").as_double(0);
    srand(settings->child("scenario").attribute("seed").as_int(0));
}

void Scenario::set_production_site_property(ProductionSite* production_site, pugi::xml_node node) {
    for (pugi::xml_node_iterator it = node.begin(); it != node.end(); it++) {
        if (strcmp(it->name(), "parameter") == 0) {
            std::string name = it->attribute("name").value();
            if (name == "remaining_capacity") {
                production_site->forcing__lambda =
                    (Forcing)(it->attribute("value").as_double(1) / production_site->capacity_manager->possible_overcapacity_ratio__beta);
            } else if (name == "incoming_demand") {
                production_site->output_storage->sales_manager->demand_forcing = (Demand)it->attribute("value").as_double(-1);
            }
        }
    }
}

void Scenario::set_final_demand_property(FinalDemand* final_demand, pugi::xml_node node) {
    for (pugi::xml_node_iterator it = node.begin(); it != node.end(); it++) {
        if (strcmp(it->name(), "parameter") == 0) {
            std::string name = it->attribute("name").value();
            if (name == "remaining_consumption_rate") {
                final_demand->forcing__kappa = (Forcing)it->attribute("value").as_double(1);
            }
        }
    }
}

bool Scenario::iterate() {
    if (model->time > stop_time) {
        return false;
    }

    // Get node from settings not describing an event for the current time step
    struct {
        Time time;
        Time delta_t;
        bool operator()(pugi::xml_node node) const {
            return strcmp(node.name(), "event") == 0
                   && (node.attribute("at").as_double(-2) + 1 == time || node.attribute("to").as_double(-2) + 1 == time
                       || (!node.attribute("start").empty() && !node.attribute("pulse_length").empty() && !node.attribute("pulse_count").empty()
                           && !node.attribute("break_length").empty() && node.attribute("start").as_double(-1) <= time
                           && (int)(time - (node.attribute("start").as_double(0) / delta_t))
                                      % (int)((node.attribute("pulse_length").as_double(0) + node.attribute("break_length").as_double(0)) / delta_t)
                                  == node.attribute("pulse_length").as_double(0) * delta_t));
        }
    } last_event;
    last_event.time = model->time;
    last_event.delta_t = model->delta_t;
    pugi::xml_node last_event_node = scenario_node.find_child(last_event);
    if (!last_event_node.empty()) {
        for (pugi::xml_node_iterator it = last_event_node.begin(); it != last_event_node.end(); it++) {
            std::string node_name = it->name();
            if (node_name == "production_site") {
                for (std::vector<Sector*>::const_iterator s = model->sectors__C.begin(); s != model->sectors__C.end(); s++) {
                    for (std::vector<ProductionSite*>::const_iterator p = (*s)->production_sites__N.begin(); p != (*s)->production_sites__N.end(); p++) {
                        (*p)->forcing__lambda = 1;
                    }
                }
            } else if (node_name == "final_demand") {
                for (std::vector<Region*>::const_iterator r = model->regions__R.begin(); r != model->regions__R.end(); r++) {
                    for (std::vector<EconomicAgent*>::const_iterator ea = (*r)->economic_agents.begin(); ea != (*r)->economic_agents.end(); ea++) {
                        if ((*ea)->type == FINAL_DEMAND) {
                            ((FinalDemand*)(*ea))->forcing__kappa = 1;
                        }
                    }
                }
            }
        }
    }

    // Get node from settings describing an event for the current time step
    struct {
        Time time;
        bool operator()(pugi::xml_node node) const { return strcmp(node.name(), "noise") == 0 && node.attribute("to").as_double(-2) + 1 == time; }
    } last_noise;
    last_noise.time = model->time;
    pugi::xml_node last_noise_node = scenario_node.find_child(last_noise);
    if (!last_noise_node.empty()) {
        for (std::vector<Sector*>::const_iterator s = model->sectors__C.begin(); s != model->sectors__C.end(); s++) {
            for (std::vector<ProductionSite*>::const_iterator p = (*s)->production_sites__N.begin(); p != (*s)->production_sites__N.end(); p++) {
                (*p)->forcing__lambda = 1;
            }
        }
    }

    struct {
        Time time;
        Time delta_t;
        bool operator()(pugi::xml_node node) const {
            return strcmp(node.name(), "event") == 0
                   && (node.attribute("at").as_double(-1) == time
                       || (node.attribute("from").as_double(-1) <= time && node.attribute("to").as_double(-1) >= time)
                       || (!node.attribute("start").empty() && !node.attribute("pulse_length").empty() && !node.attribute("pulse_count").empty()
                           && !node.attribute("break_length").empty() && node.attribute("start").as_double(-1) <= time
                           && node.attribute("start").as_double(0)
                                      + (node.attribute("pulse_length").as_double(0) + node.attribute("break_length").as_double(0))
                                            * node.attribute("pulse_count").as_double(0)
                                  > time
                           && (int)(time - (node.attribute("start").as_double(0) / delta_t))
                                      % (int)((node.attribute("pulse_length").as_double(0) + node.attribute("break_length").as_double(0)) / delta_t)
                                  < node.attribute("pulse_length").as_double(0) * delta_t));
        }
    } current_event;
    current_event.time = model->time;
    current_event.delta_t = model->delta_t;
    pugi::xml_node current_event_node = scenario_node.find_child(current_event);
    if (!current_event_node.empty()) {
        for (pugi::xml_node_iterator it = current_event_node.begin(); it != current_event_node.end(); it++) {
            std::string node_name = it->name();
            if (node_name == "production_site") {
                if (it->attribute("sector").empty()) {
                    if (it->attribute("region").empty()) {
                        for (std::vector<Sector*>::const_iterator s = model->sectors__C.begin(); s != model->sectors__C.end(); s++) {
                            for (std::vector<ProductionSite*>::const_iterator p = (*s)->production_sites__N.begin(); p != (*s)->production_sites__N.end();
                                 p++) {
                                set_production_site_property(*p, *it);
                            }
                        }
                    } else {
                        Region* region = model->find_region(it->attribute("region").value());
                        if (region) {
                            for (std::vector<EconomicAgent*>::const_iterator ea = region->economic_agents.begin(); ea != region->economic_agents.end(); ea++) {
                                if ((*ea)->type == PRODUCTION_SITE) {
                                    set_production_site_property((ProductionSite*)(*ea), *it);
                                }
                            }
                        } else {
                            std::cerr << "WARNING: Region " << it->attribute("region").value() << " not found" << std::endl;
                        }
                    }
                } else {
                    ProductionSite* production_site = model->find_production_site(it->attribute("sector").value(), it->attribute("region").value());
                    if (production_site) {
                        set_production_site_property(production_site, *it);
                    } else {
                        std::cerr << "WARNING: Production site " << it->attribute("sector").value() << "," << it->attribute("region").value() << " not found"
                                  << std::endl;
                    }
                }
            } else if (node_name == "final_demand") {
                if (it->attribute("region").empty()) {
                    for (std::vector<Region*>::const_iterator r = model->regions__R.begin(); r != model->regions__R.end(); r++) {
                        for (std::vector<EconomicAgent*>::const_iterator ea = (*r)->economic_agents.begin(); ea != (*r)->economic_agents.end(); ea++) {
                            if ((*ea)->type == FINAL_DEMAND) {
                                set_final_demand_property((FinalDemand*)(*ea), *it);
                            }
                        }
                    }
                } else {
                    FinalDemand* final_demand = model->find_final_demand(it->attribute("region").value());
                    if (final_demand) {
                        set_final_demand_property(final_demand, *it);
                    } else {
                        std::cerr << "WARNING: Final demand " << it->attribute("region").value() << " not found" << std::endl;
                    }
                }
            }
        }
    }

    struct {
        Time time;
        bool operator()(pugi::xml_node node) const {
            return strcmp(node.name(), "noise") == 0 && node.attribute("from").as_double(-1) <= time && node.attribute("to").as_double(-1) >= time;
        }
    } current_noise;
    current_noise.time = model->time;
    pugi::xml_node current_noise_node = scenario_node.find_child(current_noise);
    if (!current_noise_node.empty()) {
        int production_site_count = 0;
        for (std::vector<Sector*>::const_iterator s = model->sectors__C.begin(); s != model->sectors__C.end(); s++) {
            for (std::vector<ProductionSite*>::const_iterator p = (*s)->production_sites__N.begin(); p != (*s)->production_sites__N.end(); p++) {
                (*p)->forcing__lambda = 1;
                production_site_count++;
            }
        }
        int random_index = (int)(random() * production_site_count / RAND_MAX);
        int c = 0;
        bool random_damage = strcmp(current_noise_node.attribute("forcing").value(), "random") == 0;
        bool output = current_noise_node.attribute("output").as_bool(false);
        Forcing forcing = current_noise_node.attribute("forcing").as_double(0);
        for (std::vector<Sector*>::const_iterator s = model->sectors__C.begin(); s != model->sectors__C.end(); s++) {
            for (std::vector<ProductionSite*>::const_iterator p = (*s)->production_sites__N.begin(); p != (*s)->production_sites__N.end(); p++) {
                if (c == random_index) {
                    if (random_damage) {
                        (*p)->forcing__lambda = (Basetype)random() / (Basetype)RAND_MAX;
                    } else {
                        (*p)->forcing__lambda = forcing;
                    }
                    if (output) {
                        std::cout << std::endl
                                  << "Randomly chosen: " << (*p)->output_storage->sector->name << "->" << (*p)->region->name << " to " << (*p)->forcing__lambda;
                    }
                    break;
                }
                c++;
            }
            if (c == random_index) {
                break;
            }
        }
    }

    // Keep main loop running
    return true;
}
