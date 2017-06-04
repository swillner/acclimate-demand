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

#ifndef _BUSINESSCONNECTION_H
#define _BUSINESSCONNECTION_H

#include "types.h"

class Model;
class SalesManager;
class TransportChainLink;
class PurchasingManager;

class BusinessConnection {
  public:
    Flow last_shipment__Z;
    Demand last_demand_request__D;
    Ratio demand_fulfill_history;

  public:
    PurchasingManager* const buyer;
    SalesManager* seller;
    Flow initial_flow__Z_star;
    TransportChainLink* first_transport_link;
    bool domestic;

  protected:
    void establish_connection();

  public:
    BusinessConnection(PurchasingManager* buyer, SalesManager* seller, Flow initial_flow__Z_star);
    ~BusinessConnection();
    Flow calc_transport_flow_deficit();
    Flow calc_transport_flow();
    void calc_demand_fulfill_history();
    TransportDelay calc_transport_delay__tau();
    void push_flow__Z(Flow flow__Z);
    void deliver_flow__Z(Flow flow__Z);
    void send_demand_request__D(Demand demand_request__D);
    void disconnect_from_geography();
};

#endif
