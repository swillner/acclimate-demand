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

#include "OutputStorage.h"

#include "BusinessConnection.h"
#include "EconomicAgent.h"
#include "Model.h"
#include "ProductionSite.h"
#include "PurchasingManager.h"
#include "SalesManager.h"
#include "Sector.h"
#include "Storage.h"

OutputStorage::OutputStorage(Sector* sector, ProductionSite* economic_agent, SalesManager* sales_manager, PurchasingManager* purchasing_manager)
    : Storage(sector, economic_agent, purchasing_manager), sales_manager(sales_manager) {
    output_flow__chi = 0;
    self_supply_connection = 0;
    sales_manager->output_storage = this;
}

OutputStorage::~OutputStorage() { delete sales_manager; }

void OutputStorage::calc_output_flow__chi() { output_flow__chi = ((ProductionSite*)economic_agent)->production__X; }
