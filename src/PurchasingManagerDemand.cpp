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

#include "PurchasingManagerDemand.h"

#include "BusinessConnection.h"
#include "CapacityManager.h"
#include "Model.h"
#include "OutputStorage.h"
#include "ProductionSite.h"
#include "SalesManager.h"
#include "Sector.h"
#include "Storage.h"

PurchasingManagerDemand::PurchasingManagerDemand() : PurchasingManager() {}

void PurchasingManagerDemand::send_demand_requests__D() {
    if (demand__D == 0) {
        for (int i = 0; i < business_connections.size(); i++) {
            business_connections[i]->calc_demand_fulfill_history();
            business_connections[i]->send_demand_request__D(0);
        }
    } else {
        Demand denominator = 0;
        for (int i = 0; i < business_connections.size(); i++) {
            business_connections[i]->calc_demand_fulfill_history();
            denominator += business_connections[i]->demand_fulfill_history * business_connections[i]->initial_flow__Z_star;
        }

        if (denominator > 0) {
            for (int i = 0; i < business_connections.size(); i++) {
                Ratio eta = (Ratio)(business_connections[i]->demand_fulfill_history * business_connections[i]->initial_flow__Z_star / denominator);
                business_connections[i]->send_demand_request__D(MAKE_DEMAND(eta * demand__D));
            }
        } else {
            for (int i = 0; i < business_connections.size(); i++) {
                business_connections[i]->send_demand_request__D(0);
            }
        }
    }
}
