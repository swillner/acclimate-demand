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

#include "SalesManagerNoDemand.h"

#include "BusinessConnection.h"
#include "Model.h"
#include "OutputStorage.h"
#include "ProductionSite.h"
#include "PurchasingManager.h"
#include "Sector.h"

SalesManagerNoDemand::SalesManagerNoDemand() : SalesManager() {}

void SalesManagerNoDemand::distribute(Flow output_flow__chi) {
    Flow initial_output_flow__chi = ((ProductionSite*)output_storage->economic_agent)->initial_production__X_star;

    for (int i = 0; i < business_connections.size(); i++) {
        business_connections[i]->push_flow__Z(MAKE_FLOW(output_flow__chi * business_connections[i]->initial_flow__Z_star / initial_output_flow__chi));
    }
}
