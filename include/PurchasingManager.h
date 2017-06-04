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

#ifndef _PURCHASINGMANAGER_H
#define _PURCHASINGMANAGER_H

#include "types.h"

class BusinessConnection;
class Storage;
class SalesManager;

class PurchasingManager {
  public:
    Storage* storage;
    Demand demand__D;
    std::vector<BusinessConnection*> business_connections;

  protected:
    void calc_demand__D();
    virtual void send_demand_requests__D() = 0;

  public:
    PurchasingManager();
    virtual ~PurchasingManager();
    virtual Flow calc_transport_flow_deficit();
    virtual Flow calc_transport_flow();
    virtual Flow sum_last_shipments();
    virtual void iterate_consumption_and_production();
    virtual void iterate_demand();
    virtual void remove_business_connection(BusinessConnection* business_connection);
};

#endif
