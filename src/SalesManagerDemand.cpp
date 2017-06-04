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

#include "SalesManagerDemand.h"

#include "BusinessConnection.h"
#include "EconomicAgent.h"
#include "Model.h"
#include "OutputStorage.h"
#include "Region.h"
#include "Sector.h"

SalesManagerDemand::SalesManagerDemand() : SalesManager() {}

void SalesManagerDemand::distribute(Flow output_flow__chi) {
    Flow sum = 0;
    if (sum_demand_requests__D == 0) {
        for (int i = 0; i < business_connections.size(); i++) {
            business_connections[i]->push_flow__Z(0);
        }
    } else {
        for (int i = 0; i < business_connections.size(); i++) {
            Ratio zeta = business_connections[i]->last_demand_request__D / sum_demand_requests__D;
            business_connections[i]->push_flow__Z(MAKE_FLOW(zeta * output_flow__chi));
        }
    }
}
