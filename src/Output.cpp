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

#include "Output.h"

#ifdef _OPENMP
#include <omp.h>
#else
#define omp_get_max_threads() 1
#endif

#include <iostream>
#include "BusinessConnection.h"
#include "CapacityManager.h"
#include "EconomicAgent.h"
#include "FinalDemand.h"
#include "Model.h"
#include "OutputStorage.h"
#include "ProductionSite.h"
#include "PurchasingManager.h"
#include "Region.h"
#include "SalesManager.h"
#include "Sector.h"
#include "TransportChainLink.h"
#include "types.h"

Output::Output(XMLInput* settings, Model* model, Scenario* scenario, pugi::xml_node output_node)
    : settings(settings), model(model), scenario(scenario), output_node(output_node) {
    start_time = 0;
    observables = output_node.child("observables");
}

void Output::start() {
    start_time = time(0);
    internal_write_header(localtime(&start_time), omp_get_max_threads(), VERSION);
    internal_write_xml_input(settings);
    internal_start();
}

#define MAP(name_, variable)                           \
    if (name == name_) {                               \
        internal_write_double(name.c_str(), variable); \
        continue;                                      \
    }

void Output::write_production_site_parameters(ProductionSite* p, pugi::xml_node parameters) {
    for (pugi::xml_node observable = parameters; observable; observable = observable.next_sibling("parameter")) {
        std::string name = observable.attribute("name").value();
        MAP("production", p->production__X);
        MAP("direct_damage", p->direct_damage__d);
        MAP("initial_production", p->initial_production__X_star);
        MAP("output_flow", p->output_storage->output_flow__chi);
        MAP("incoming_demand", p->output_storage->sales_manager->sum_demand_requests__D);
        MAP("production_capacity", p->capacity_manager->production_capacity__p);
        MAP("desired_production_capacity", p->capacity_manager->desired_production_capacity__p_tilde);
        MAP("possible_production_capacity", p->capacity_manager->possible_production_capacity__p_hat);
        MAP("forcing", p->forcing__lambda);
        if (name == "demand") {
            Demand demand = 0;
            for (int i = 0; i < p->input_storages.size(); i++) {
                demand += p->input_storages[i]->purchasing_manager->demand__D;
            }
            internal_write_double("demand", demand);
            continue;
        }
        if (name == "value_added") {
            Flow value_added = p->production__X;
            for (int i = 0; i < p->input_storages.size(); i++) {
                value_added -= p->input_storages[i]->used_flow__U;
            }
            internal_write_double("value_added", value_added);
            continue;
        }
        if (name == "used_flow") {
            Flow used_flow = 0;
            for (int i = 0; i < p->input_storages.size(); i++) {
                used_flow += p->input_storages[i]->used_flow__U;
            }
            internal_write_double("used_flow", used_flow);
            continue;
        }
        if (name == "efficiency") {
            Flow used_flow = 0;
            for (int i = 0; i < p->input_storages.size(); i++) {
                used_flow += p->input_storages[i]->used_flow__U;
            }
            internal_write_double("efficiency", p->production__X / used_flow);
            continue;
        }
    }
}

void Output::write_final_demand_parameters(FinalDemand* fd, pugi::xml_node parameters) {
    for (pugi::xml_node observable = parameters; observable; observable = observable.next_sibling("parameter")) {
        std::string name = observable.attribute("name").value();
        MAP("consumption_rate", fd->calc_consumption_rate__c());
    }
}

void Output::write_input_storage_parameters(Storage* s, pugi::xml_node parameters) {
    for (pugi::xml_node observable = parameters; observable; observable = observable.next_sibling("parameter")) {
        std::string name = observable.attribute("name").value();
        MAP("content", s->content__S);
        MAP("input_flow", s->last_input_flow__I());
        MAP("used_flow", s->used_flow__U);
        MAP("desired_used_flow", s->desired_used_flow__U_tilde);
        MAP("initial_input_flow", s->initial_input_flow__I_star);
        MAP("initial_used_flow", s->initial_input_flow__I_star);
        MAP("demand", s->purchasing_manager->demand__D);
        MAP("business_connections", s->purchasing_manager->business_connections.size());
    }
}

void Output::write_input_storages(EconomicAgent* ea, pugi::xml_node input_storages_node) {
    for (pugi::xml_node input_storage_node = input_storages_node; input_storage_node; input_storage_node = input_storage_node.next_sibling("input_storage")) {
        const char* storage_sector_name = 0;
        if (!input_storage_node.attribute("sector").empty()) {
            storage_sector_name = input_storage_node.attribute("sector").value();
        }
        for (int i = 0; i < ea->input_storages.size(); i++) {
            if (!storage_sector_name || ea->input_storages[i]->sector->name == storage_sector_name) {
                if (ea->type == FINAL_DEMAND) {
                    internal_start_target("final_demand_input_storage", ea->input_storages[i]->sector);
                } else {
                    internal_start_target("input_storage", ea->input_storages[i]->sector);
                }
                write_input_storage_parameters(ea->input_storages[i], input_storage_node.child("parameter"));
                internal_end_target();
            }
        }
    }
}

void Output::write_connection_parameters(BusinessConnection* b, pugi::xml_node parameters) {
    for (pugi::xml_node observable = parameters; observable; observable = observable.next_sibling("parameter")) {
        std::string name = observable.attribute("name").value();
        MAP("sent_flow", b->last_shipment__Z);
        MAP("demand_request", b->last_demand_request__D);
        MAP("transport_flow", b->calc_transport_flow());
        MAP("transport_flow_deficit", b->calc_transport_flow_deficit());
    }
}

void Output::write_outgoing_connections(ProductionSite* p, pugi::xml_node outgoing_connections_node) {
    for (pugi::xml_node outgoing_connection_node = outgoing_connections_node; outgoing_connection_node;
         outgoing_connection_node = outgoing_connection_node.next_sibling("outgoing_connection")) {
        const char* sector_to_name = 0;
        const char* region_to_name = 0;
        if (!outgoing_connection_node.attribute("sector").empty()) {
            sector_to_name = outgoing_connection_node.attribute("sector").value();
        }
        if (!outgoing_connection_node.attribute("region").empty()) {
            region_to_name = outgoing_connection_node.attribute("region").value();
        }
        for (int i = 0; i < p->output_storage->sales_manager->business_connections.size(); i++) {
            if (p->output_storage->sales_manager->business_connections[i]->buyer->storage->economic_agent->type == PRODUCTION_SITE) {
                ProductionSite* production_site_to = (ProductionSite*)p->output_storage->sales_manager->business_connections[i]->buyer->storage->economic_agent;
                if (!sector_to_name || production_site_to->output_storage->sector->name == sector_to_name) {
                    if (!region_to_name || production_site_to->region->name == region_to_name) {
                        internal_start_target("outgoing_connection", production_site_to->output_storage->sector, production_site_to->region);
                        write_connection_parameters(p->output_storage->sales_manager->business_connections[i], outgoing_connection_node.child("parameter"));
                        internal_end_target();
                    }
                }
            }
        }
    }
}

void Output::write_final_demand_connections(ProductionSite* p, pugi::xml_node outgoing_connections_node) {
    for (pugi::xml_node outgoing_connection_node = outgoing_connections_node; outgoing_connection_node;
         outgoing_connection_node = outgoing_connection_node.next_sibling("final_demand_connection")) {
        const char* region_to_name = 0;
        if (!outgoing_connection_node.attribute("region").empty()) {
            region_to_name = outgoing_connection_node.attribute("region").value();
        }
        for (int i = 0; i < p->output_storage->sales_manager->business_connections.size(); i++) {
            if (p->output_storage->sales_manager->business_connections[i]->buyer->storage->economic_agent->type == FINAL_DEMAND) {
                FinalDemand* final_demand_to = (FinalDemand*)p->output_storage->sales_manager->business_connections[i]->buyer->storage->economic_agent;
                if (!region_to_name || final_demand_to->region->name == region_to_name) {
                    internal_start_target("final_demand_connection", final_demand_to->region);
                    write_connection_parameters(p->output_storage->sales_manager->business_connections[i], outgoing_connection_node.child("parameter"));
                    internal_end_target();
                }
            }
        }
    }
}

void Output::iterate() {
    internal_iterate_begin();
    for (pugi::xml_node_iterator it = observables.begin(); it != observables.end(); ++it) {
        std::string name = it->name();
        if (name == "production_site") {
            if (it->attribute("sector").empty()) {
                if (it->attribute("region").empty()) {
                    for (int i_ = 0; i_ < model->sectors__C.size(); i_++) {
                        Sector* i = model->sectors__C[i_];
                        for (int p_ = 0; p_ < i->production_sites__N.size(); p_++) {
                            ProductionSite* p = i->production_sites__N[p_];
                            internal_start_target("production_site", i, p->region);
                            write_production_site_parameters(p, it->child("parameter"));
                            write_input_storages(p, it->child("input_storage"));
                            write_outgoing_connections(p, it->child("outgoing_connection"));
                            write_final_demand_connections(p, it->child("final_demand_connection"));
                            internal_end_target();
                        }
                    }
                } else {
                    Region* region = model->find_region(it->attribute("region").value());
                    if (region) {
                        for (std::vector<EconomicAgent*>::const_iterator ea = region->economic_agents.begin(); ea != region->economic_agents.end(); ea++) {
                            if ((*ea)->type == PRODUCTION_SITE) {
                                ProductionSite* p = (ProductionSite*)(*ea);
                                internal_start_target("production_site", p->output_storage->sector, p->region);
                                write_production_site_parameters(p, it->child("parameter"));
                                write_input_storages(p, it->child("input_storage"));
                                write_outgoing_connections(p, it->child("outgoing_connection"));
                                write_final_demand_connections(p, it->child("final_demand_connection"));
                                internal_end_target();
                            }
                        }
                    } else {
                        std::cerr << "WARNING: Region " << it->attribute("region").value() << " not found" << std::endl;
                    }
                }
            } else {
                ProductionSite* p = model->find_production_site(it->attribute("sector").value(), it->attribute("region").value());
                if (p) {
                    internal_start_target("production_site", p->output_storage->sector, p->region);
                    write_production_site_parameters(p, it->child("parameter"));
                    write_input_storages(p, it->child("input_storage"));
                    write_outgoing_connections(p, it->child("outgoing_connection"));
                    write_final_demand_connections(p, it->child("final_demand_connection"));
                    internal_end_target();
                } else {
                    std::cerr << "WARNING: Production site " << it->attribute("sector").value() << "," << it->attribute("region").value() << " not found"
                              << std::endl;
                }
            }
        } else if (name == "final_demand") {
            if (it->attribute("region").empty()) {
                for (std::vector<Region*>::const_iterator region = model->regions__R.begin(); region != model->regions__R.end(); region++) {
                    for (std::vector<EconomicAgent*>::const_iterator ea = (*region)->economic_agents.begin(); ea != (*region)->economic_agents.end(); ea++) {
                        if ((*ea)->type == FINAL_DEMAND) {
                            FinalDemand* fd = (FinalDemand*)(*ea);
                            internal_start_target("final_demand", *region);
                            write_final_demand_parameters(fd, it->child("parameter"));
                            write_input_storages(fd, it->child("input_storage"));
                            internal_end_target();
                        }
                    }
                }
            } else {
                FinalDemand* fd = model->find_final_demand(it->attribute("region").value());
                if (fd) {
                    internal_start_target("final_demand", fd->region);
                    write_final_demand_parameters(fd, it->child("parameter"));
                    write_input_storages(fd, it->child("input_storage"));
                    internal_end_target();
                } else {
                    std::cerr << "WARNING: Final demand " << it->attribute("region").value() << " not found" << std::endl;
                }
            }
        } else if (name == "region") {
            const char* region_name = 0;
            if (!it->attribute("region").empty()) {
                region_name = it->attribute("region").value();
            }
            for (std::vector<Region*>::const_iterator region = model->regions__R.begin(); region != model->regions__R.end(); region++) {
                if (!region_name || (*region)->name == region_name) {
                    internal_start_target("region", *region);
                    Ratio production_capacity = 0;
                    Flow production = 0;
                    Flow direct_damage = 0;
                    for (std::vector<EconomicAgent*>::const_iterator ea = (*region)->economic_agents.begin(); ea != (*region)->economic_agents.end(); ea++) {
                        if ((*ea)->type == PRODUCTION_SITE) {
                            production_capacity += ((ProductionSite*)(*ea))->capacity_manager->production_capacity__p;
                            production += ((ProductionSite*)(*ea))->production__X;
                            direct_damage += ((ProductionSite*)(*ea))->direct_damage__d;
                        }
                    }
                    for (pugi::xml_node observable = it->child("parameter"); observable; observable = observable.next_sibling("parameter")) {
                        std::string name = observable.attribute("name").value();
                        MAP("import", (*region)->current_import_flow__Z());
                        MAP("export", (*region)->current_export_flow__Z());
                        MAP("consumption", (*region)->current_consumption__C());
                        MAP("gdp", (*region)->current_gdp());
                        MAP("production_capacity", production_capacity);
                        MAP("production", production);
                        MAP("direct_damage", direct_damage);
                    }
                    internal_end_target();
                }
            }

        } else if (name == "world") {
            Quantity gdp = 0;
            Flow consumption = 0;
            Quantity storage = 0;
            Flow transport_flow = 0;
            Flow input_flow = 0;
            Flow production_used_flow = 0;
            Flow production = 0;
            Flow direct_damage = 0;
            Flow supply_deficit = 0;
            Ratio production_capacity = 0;
            int business_connections = 0;
            int new_connections = 0;
            int no_new_supplier_found = 0;

            for (std::vector<Region*>::const_iterator region = model->regions__R.begin(); region != model->regions__R.end(); region++) {
                gdp += (*region)->current_gdp();
                consumption += (*region)->current_consumption__C();
                for (std::vector<EconomicAgent*>::const_iterator ea = (*region)->economic_agents.begin(); ea != (*region)->economic_agents.end(); ea++) {
                    if ((*ea)->type == PRODUCTION_SITE) {
                        production += ((ProductionSite*)(*ea))->production__X;
                        direct_damage += ((ProductionSite*)(*ea))->direct_damage__d;
                        production_capacity += ((ProductionSite*)(*ea))->capacity_manager->production_capacity__p;
                        transport_flow += ((ProductionSite*)(*ea))->output_storage->sales_manager->calc_transport_flow();
                    }
                    for (std::vector<Storage*>::const_iterator s = (*ea)->input_storages.begin(); s != (*ea)->input_storages.end(); s++) {
                        storage += (*s)->content__S;
                        input_flow += (*s)->last_input_flow__I();
                        if ((*ea)->type == PRODUCTION_SITE) {
                            production_used_flow +=
                                MAKE_FLOW((*s)->initial_input_flow__I_star * ((ProductionSite*)(*ea))->capacity_manager->production_capacity__p);
                        }
                        business_connections += (*s)->purchasing_manager->business_connections.size();
                    }
                }
            }

            transport_flow += input_flow;
            internal_start_target("world");
            for (pugi::xml_node observable = it->child("parameter"); observable; observable = observable.next_sibling("parameter")) {
                std::string name = observable.attribute("name").value();
                MAP("gdp", gdp);
                MAP("consumption", consumption);
                MAP("storage", storage);
                MAP("transport_flow", transport_flow);
                MAP("production", production);
                MAP("direct_damage", direct_damage);
                MAP("production_used_flow", production_used_flow);
                MAP("input_flow", input_flow);
                MAP("production_capacity", production_capacity);
                MAP("business_connections", business_connections);
                MAP("efficiency", consumption / production);
            }
            internal_end_target();
        }
    }
    internal_iterate_end();
}

void Output::end() {
    time_t end_time = time(0);
    time_t duration = end_time - start_time;
    internal_write_footer(localtime(&duration));
    internal_end();
}

void Output::internal_write_int(const char* name, int v) { internal_write_double(name, v); }

Output::~Output() {}

void Output::internal_write_double(const char* name, double v) {}

void Output::internal_write_header(tm* timestamp, int max_threads, const char* version) {}

void Output::internal_write_footer(tm* duration) {}

void Output::internal_write_xml_input(XMLInput* input) {}

void Output::internal_start() {}

void Output::internal_iterate_begin() {}

void Output::internal_iterate_end() {}

void Output::internal_end() {}

void Output::internal_start_target(const char* name, Sector* sector, Region* region) {}

void Output::internal_start_target(const char* name, Sector* sector) {}

void Output::internal_start_target(const char* name, Region* region) {}

void Output::internal_start_target(const char* name) {}

void Output::internal_end_target() {}
