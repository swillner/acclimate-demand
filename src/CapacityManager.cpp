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

#include "CapacityManager.h"

#include "EconomicAgent.h"
#include "OutputStorage.h"
#include "ProductionSite.h"
#include "PurchasingManager.h"
#include "Region.h"
#include "SalesManager.h"
#include "Sector.h"
#include "Storage.h"

CapacityManager::CapacityManager(ProductionSite* production_site, Ratio possible_overcapacity_ratio__beta)
    : production_site(production_site), possible_overcapacity_ratio__beta(possible_overcapacity_ratio__beta) {
    possible_production_capacity__p_hat = 1;
    desired_production_capacity__p_tilde = 0;
    production_capacity__p = 1;
}

void CapacityManager::calc_possible_production_capacity__p_hat() {
    possible_production_capacity__p_hat = production_site->forcing__lambda * possible_overcapacity_ratio__beta;

    for (int i = 0; i < production_site->input_storages.size(); i++) {
        Ratio tmp = production_site->input_storages[i]->calc_possible_use__U_hat()
                    / production_site->input_storages[i]->initial_input_flow__I_star;  // = initial_used_flow__U_star
        if (tmp < possible_production_capacity__p_hat) {
            possible_production_capacity__p_hat = tmp;
        }
    }
}

void CapacityManager::calc_desired_production_capacity__p_tilde() {
    desired_production_capacity__p_tilde = production_site->output_storage->sales_manager->sum_demand_requests__D / production_site->initial_production__X_star;
}

void CapacityManager::calc_production_capacity__p() {
    calc_possible_production_capacity__p_hat();
    calc_desired_production_capacity__p_tilde();

    production_capacity__p = MIN(possible_production_capacity__p_hat, desired_production_capacity__p_tilde);
}
