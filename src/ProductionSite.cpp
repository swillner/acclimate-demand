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

#include "ProductionSite.h"

#include "CapacityManager.h"
#include "OutputStorage.h"
#include "PurchasingManager.h"
#include "Region.h"
#include "SalesManager.h"
#include "Sector.h"
#include "Storage.h"

ProductionSite::ProductionSite(
    Sector* sector, Region* region, Ratio possible_overcapacity_ratio__beta, SalesManager* sales_manager, PurchasingManager* purchasing_manager)
    : EconomicAgent(region, PRODUCTION_SITE),
      output_storage(new OutputStorage(sector, this, sales_manager, purchasing_manager)),
      capacity_manager(new CapacityManager(this, possible_overcapacity_ratio__beta)) {
    forcing__lambda = 1;
    production__X = 0;
    initial_production__X_star = 0;
    direct_damage__d = 0;
}

ProductionSite::~ProductionSite() {
    delete capacity_manager;
    delete output_storage;
}

void ProductionSite::iterate_consumption_and_production() {
    capacity_manager->calc_production_capacity__p();
    produce__X();
    damage__d();
    for (int i = 0; i < input_storages.size(); i++) {
        Flow used_flow__U_current = MAKE_FLOW(input_storages[i]->initial_input_flow__I_star
                                              * capacity_manager->production_capacity__p);  // initial_input_flow__I_star = initial_used_flow__U_star
        input_storages[i]->set_desired_used_flow__U_tilde(
            MAKE_FLOW(input_storages[i]->initial_input_flow__I_star * capacity_manager->desired_production_capacity__p_tilde * forcing__lambda));
        input_storages[i]->use_content__S(used_flow__U_current);
        input_storages[i]->iterate_consumption_and_production();
    }
    output_storage->calc_output_flow__chi();
    output_storage->sales_manager->iterate_supply();
}

void ProductionSite::produce__X() {
    production__X = MAKE_FLOW(capacity_manager->production_capacity__p * initial_production__X_star);
    output_storage->sector->add_production__X(production__X);
}

void ProductionSite::damage__d() { direct_damage__d = MAKE_FLOW((1 - forcing__lambda) * initial_production__X_star); }

void ProductionSite::add_initial_production__X_star(Flow initial_production_flow__X_star) {
    initial_production__X_star += initial_production_flow__X_star;
    production__X += initial_production_flow__X_star;
    output_storage->sales_manager->add_initial_demand_request__D_star(initial_production_flow__X_star);
}
