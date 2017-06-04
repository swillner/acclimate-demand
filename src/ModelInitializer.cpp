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

#include "ModelInitializer.h"

#include <cstring>
#include <iostream>
#include "BusinessConnection.h"
#include "EconomicAgent.h"
#include "FinalDemand.h"
#include "Model.h"
#include "OutputStorage.h"
#include "ProductionSite.h"
#include "PurchasingManager.h"
#include "PurchasingManagerDemand.h"
#include "PurchasingManagerNoDemand.h"
#include "Region.h"
#include "SalesManager.h"
#include "SalesManagerDemand.h"
#include "SalesManagerNoDemand.h"
#include "Sector.h"
#include "Storage.h"

ModelInitializer::ModelInitializer(Model* model, XMLInput* settings) : model(model), settings(settings) {
    flow_threshold = get_parameter("threshold", "network").attribute("value").as_double(-1);
    model->delta_t = get_parameter("delta_t").attribute("value").as_double(1);
    model->options.demand = get_parameter("demand").attribute("value").as_bool(false);
    model->options.no_self_supply = get_parameter("no_self_supply").attribute("value").as_bool(false);
    model->options.history_weight = get_parameter("history_weight").attribute("value").as_double(0);
}

ModelInitializer::~ModelInitializer() {}

pugi::xml_node ModelInitializer::get_named_property(std::string tag_name, std::string node_name, std::string property_name) {
    pugi::xml_node parameters_node = settings->child("parameters");
    struct property_given_type {
        std::string property_name;
        bool operator()(pugi::xml_node node) const { return strcmp(node.name(), "parameter") == 0 && property_name == node.attribute("name").value(); }
    } property_given;
    property_given.property_name = property_name;
    struct specific_node_type {
        property_given_type property_given;
        std::string node_name;
        bool operator()(pugi::xml_node node) const { return node_name == node.attribute("name").value() && !node.find_child(property_given).empty(); }
    } specific_node;
    specific_node.property_given = property_given;
    specific_node.node_name = node_name;
    struct unspecific_node_type {
        property_given_type property_given;
        bool operator()(pugi::xml_node node) const { return node.attribute("name").empty() && !node.find_child(property_given).empty(); }
    } unspecific_node;
    unspecific_node.property_given = property_given;
    pugi::xml_node node = parameters_node.find_child(specific_node);
    if (node.empty()) {
        node = parameters_node.find_child(unspecific_node);
        return node.find_child(property_given);
    } else {
        return node.find_child(property_given);
    }
}

pugi::xml_node ModelInitializer::get_parameter(std::string property_name, const char* parent_name) {
    pugi::xml_node parameters_node = settings->child(parent_name);
    struct property_given_type {
        std::string property_name;
        bool operator()(pugi::xml_node node) const { return strcmp(node.name(), "parameter") == 0 && property_name == node.attribute("name").value(); }
    } property_given;
    property_given.property_name = property_name;
    return parameters_node.find_child(property_given);
}

pugi::xml_node ModelInitializer::get_production_site_property(std::string sector_name, std::string region_name, std::string property_name) {
    pugi::xml_node parameters_node = settings->child("parameters");
    struct property_given_type {
        std::string property_name;
        bool operator()(pugi::xml_node node) const { return strcmp(node.name(), "parameter") == 0 && property_name == node.attribute("name").value(); }
    } property_given;
    property_given.property_name = property_name;
    struct specific_node_both_type {
        property_given_type property_given;
        std::string sector_name;
        std::string region_name;
        bool operator()(pugi::xml_node node) const {
            return sector_name == node.attribute("sector").value() && region_name == node.attribute("region").value()
                   && !node.find_child(property_given).empty();
        }
    } specific_node_both;
    specific_node_both.property_given = property_given;
    specific_node_both.sector_name = sector_name;
    specific_node_both.region_name = region_name;
    struct specific_node_sector_type {
        property_given_type property_given;
        std::string sector_name;
        bool operator()(pugi::xml_node node) const { return sector_name == node.attribute("sector").value() && !node.find_child(property_given).empty(); }
    } specific_node_sector;
    specific_node_sector.property_given = property_given;
    specific_node_sector.sector_name = sector_name;
    struct specific_node_region_type {
        property_given_type property_given;
        std::string region_name;
        bool operator()(pugi::xml_node node) const { return region_name == node.attribute("region").value() && !node.find_child(property_given).empty(); }
    } specific_node_region;
    specific_node_region.property_given = property_given;
    specific_node_region.region_name = region_name;
    struct unspecific_node_type {
        property_given_type property_given;
        bool operator()(pugi::xml_node node) const {
            return node.attribute("region").empty() && node.attribute("sector").empty() && !node.find_child(property_given).empty();
        }
    } unspecific_node;
    unspecific_node.property_given = property_given;
    pugi::xml_node node = parameters_node.find_child(specific_node_both);
    if (node.empty()) {
        node = parameters_node.find_child(specific_node_sector);
        if (node.empty()) {
            node = parameters_node.find_child(specific_node_region);
            if (node.empty()) {
                node = parameters_node.find_child(unspecific_node);
                return node.find_child(property_given);
            } else {
                return node.find_child(property_given);
            }
        } else {
            return node.find_child(property_given);
        }
    } else {
        return node.find_child(property_given);
    }
}

PurchasingManager* ModelInitializer::create_purchasing_manager(std::string sector_name, std::string region_name) {
    PurchasingManager* purchasing_manager;
    if (model->options.demand) {
        purchasing_manager = new PurchasingManagerDemand();
    } else {
        purchasing_manager = new PurchasingManagerNoDemand();
    }
    return purchasing_manager;
}

ProductionSite* ModelInitializer::add_production_site(Sector* sector, Region* region) {
    SalesManager* sales_manager;

    if (model->options.demand) {
        sales_manager = new SalesManagerDemand();
    } else {
        sales_manager = new SalesManagerNoDemand();
    }
    ProductionSite* production_site = new ProductionSite(
        sector, region, (Ratio)get_production_site_property(sector->name, region->name, "possible_overcapacity_ratio").attribute("value").as_double(1),
        sales_manager, create_purchasing_manager(sector->name, region->name));
    region->economic_agents.push_back(production_site);
    sector->production_sites__N.push_back(production_site);
    return production_site;
}

FinalDemand* ModelInitializer::add_final_demand(Region* region) {
    FinalDemand* final_demand = new FinalDemand(region);
    region->economic_agents.push_back(final_demand);
    return final_demand;
}

Sector* ModelInitializer::add_sector(std::string name) {
    Sector* sector = new Sector(model, name, (Time)get_named_property("sector", name, "storage_refill_enforcement").attribute("value").as_double(1),
                                (Ratio)get_named_property("sector", name, "upper_storage_limit").attribute("value").as_double(1.4),
                                (Time)get_named_property("sector", name, "initial_storage_fill_factor").attribute("value").as_double(1));
    model->sectors__C.push_back(sector);
    return sector;
}

int ModelInitializer::initialize_connection(Sector* sector_from, Region* region_from, Sector* sector_to, Region* region_to, Flow flow) {
    ProductionSite* production_site_from = model->find_production_site(sector_from, region_from->name);
    if (!production_site_from) {
        production_site_from = add_production_site(sector_from, region_from);
        if (!production_site_from) {
            return 0;
        }
    }
    ProductionSite* production_site_to = model->find_production_site(sector_to, region_to->name);
    if (!production_site_to) {
        production_site_to = add_production_site(sector_to, region_to);
        if (!production_site_to) {
            return 0;
        }
    }
    return initialize_connection(production_site_from, production_site_to, flow);
}

int ModelInitializer::initialize_connection(ProductionSite* production_site_from, EconomicAgent* economic_agent_to, Flow flow) {
    if (model->options.no_self_supply && ((void*)production_site_from == (void*)economic_agent_to)) {
        return 0;
    }
    Sector* sector_from = production_site_from->output_storage->sector;
    Storage* input_storage = economic_agent_to->find_input_storage(sector_from->name);
    if (!input_storage) {
        if (economic_agent_to->type == PRODUCTION_SITE) {
            Sector* sector_to = ((ProductionSite*)economic_agent_to)->output_storage->sector;
            if (sector_from == sector_to) {
                input_storage = ((ProductionSite*)economic_agent_to)->output_storage;
            } else {
                input_storage = new Storage(sector_from, economic_agent_to, create_purchasing_manager(sector_to->name, economic_agent_to->region->name));
            }
        } else {
            input_storage = new Storage(sector_from, economic_agent_to, create_purchasing_manager("FD", economic_agent_to->region->name));
        }
        economic_agent_to->input_storages.push_back(input_storage);
    }
    input_storage->add_initial_flow__Z_star(flow);
    production_site_from->add_initial_production__X_star(flow);

    BusinessConnection* business_connection =
        new BusinessConnection(input_storage->purchasing_manager, production_site_from->output_storage->sales_manager, flow);
    input_storage->purchasing_manager->business_connections.push_back(business_connection);

    if ((void*)production_site_from == (void*)economic_agent_to) {
        production_site_from->output_storage->self_supply_connection = business_connection;
    }
    return 0;
}

int ModelInitializer::clean_network() {
    // Removes production sites with input but without output (may occur due to flow threshold)
    int production_site_count;
    bool needs_cleaning = true;
    while (needs_cleaning) {
        production_site_count = 0;
        needs_cleaning = false;
        std::cerr << "(Re-)starting cleanup" << std::endl;
        for (std::vector<Sector*>::const_iterator sector = model->sectors__C.begin(); sector != model->sectors__C.end(); sector++) {
            for (std::vector<ProductionSite*>::iterator production_site = (*sector)->production_sites__N.begin();
                 production_site != (*sector)->production_sites__N.end();) {
                // Calculate value added
                Flow value_added = (*production_site)->initial_production__X_star;
                for (std::vector<Storage*>::const_iterator input_storage = (*production_site)->input_storages.begin();
                     input_storage != (*production_site)->input_storages.end(); input_storage++) {
                    value_added -= (*input_storage)->initial_input_flow__I_star;
                }

                if (value_added <= 0 || (*production_site)->output_storage->sales_manager->business_connections.size() == 0
                    || ((*production_site)->output_storage->sales_manager->business_connections.size() == 1
                        && (*production_site)->output_storage->self_supply_connection)
                    || (*production_site)->input_storages.size() == 0
                    || ((*production_site)->input_storages.size() == 1 && (*production_site)->output_storage->self_supply_connection)) {
                    needs_cleaning = true;
                    std::cerr << "WARNING: Removed production site " << (*sector)->name << "," << (*production_site)->region->name
                              << " during cleanup, reason: ";
                    if (value_added <= 0) {
                        std::cerr << "value added only " << value_added;
                    } else if ((*production_site)->output_storage->sales_manager->business_connections.size() == 0
                               || ((*production_site)->output_storage->sales_manager->business_connections.size() == 1
                                   && (*production_site)->output_storage->self_supply_connection)) {
                        std::cerr << "no outgoing connection";
                    } else {
                        std::cerr << "no incoming connection";
                    }
                    std::cerr << std::endl;

                    // Remove production_site from region
                    (*production_site)->region->remove_economic_agent(*production_site);

                    // Cleanup memory of production_site
                    delete *production_site;
                    production_site = (*sector)->production_sites__N.erase(production_site);
                } else {
                    production_site++;
                    production_site_count++;
                }
            }
        }
    }
    for (std::vector<Sector*>::const_iterator sector = model->sectors__C.begin(); sector != model->sectors__C.end(); sector++) {
        for (std::vector<ProductionSite*>::iterator production_site = (*sector)->production_sites__N.begin();
             production_site != (*sector)->production_sites__N.end(); production_site++) {
            Flow value_added = (*production_site)->initial_production__X_star;
            for (std::vector<Storage*>::const_iterator input_storage = (*production_site)->input_storages.begin();
                 input_storage != (*production_site)->input_storages.end(); input_storage++) {
                value_added -= (*input_storage)->initial_input_flow__I_star;
            }
            if (value_added <= 0) {
                std::cerr << "WARNING: Production site " << (*sector)->name << "," << (*production_site)->region->name << " has value added of only "
                          << value_added << std::endl;
            }
        }
    }
    std::cout << "Number of production sites: " << production_site_count << std::endl;
    return 0;
}
