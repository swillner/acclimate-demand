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

#include "FinalDemand.h"

#include "PurchasingManager.h"
#include "Region.h"
#include "Storage.h"

FinalDemand::FinalDemand(Region* region) : EconomicAgent(region, FINAL_DEMAND) { forcing__kappa = 1; }

Ratio FinalDemand::calc_consumption_rate__c() {
    Ratio consumption_rate_c = 0;
    Flow numerator = 0;
    Flow denominator = 0;

    for (int i = 0; i < input_storages.size(); i++) {
        numerator += input_storages[i]->calc_possible_use__U_hat();
        denominator += input_storages[i]->initial_input_flow__I_star;  // initial_used_flow__U_star = initial_input_flow__I_star
    }

    consumption_rate_c = MIN(forcing__kappa, numerator / denominator);

    return consumption_rate_c;
}

void FinalDemand::iterate_consumption_and_production() {
    for (int i = 0; i < input_storages.size(); i++) {
        Flow used_flow = MIN(forcing__kappa * input_storages[i]->initial_input_flow__I_star,
                             input_storages[i]->calc_possible_use__U_hat());  // initial_used_flow__U_star = initial_input_flow__I_star
        input_storages[i]->set_desired_used_flow__U_tilde(forcing__kappa * input_storages[i]->initial_input_flow__I_star);
        input_storages[i]->use_content__S(used_flow);
        region->add_consumption_flow__Y(used_flow);

        input_storages[i]->iterate_consumption_and_production();
    }
}
