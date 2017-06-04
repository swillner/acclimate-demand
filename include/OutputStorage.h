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

#ifndef _OUTPUTSTORAGE_H
#define _OUTPUTSTORAGE_H

#include "Storage.h"
#include "types.h"

class Sector;
class EconomicAgent;
class SalesManager;
class ProductionSite;
class BusinessConnection;

class OutputStorage : public Storage {
  public:
    Flow output_flow__chi;
    BusinessConnection* self_supply_connection;
    SalesManager* const sales_manager;

  public:
    OutputStorage(Sector* sector, ProductionSite* economic_agent, SalesManager* sales_manager, PurchasingManager* purchasing_manager);
    ~OutputStorage();
    void calc_output_flow__chi();
};

#endif
