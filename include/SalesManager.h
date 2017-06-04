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

#ifndef _SALESMANAGER_H
#define _SALESMANAGER_H

#include "types.h"

class OutputStorage;
class PurchasingManager;
class BusinessConnection;

class SalesManager {
  public:
    OutputStorage* output_storage;
    std::vector<BusinessConnection*> business_connections;
    Demand sum_demand_requests__D;
    Demand demand_forcing;

  protected:
    virtual void distribute(Flow output_flow__chi) = 0;

  public:
    SalesManager();
    virtual ~SalesManager();
    void iterate_supply();
    void add_demand_request__D(Demand demand_request__D);
    void add_initial_demand_request__D_star(Demand initial_demand_request__D_star);
    Flow calc_transport_flow();
    void remove_business_connection(BusinessConnection* business_connection);
};

#endif
