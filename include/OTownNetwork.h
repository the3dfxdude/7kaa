//
// Seven Kingdoms: Ancient Adversaries
//
// Copyright 2013 Richard Dijk <thor_madman@hotmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//

//Filename    : OTownNetwork.h
//Description : Header file for TownNetwork, an extension to linked towns
//              which is binary compatible with existing data by dynamically
//				generating from existing data

#ifndef __OTOWNNETWORK_H
#define __OTOWNNETWORK_H

#include <vector>
#include <ODYNARRB.h>



// TownNetwork, a collection of town_recno's, which are all in the same network
class TownNetwork
{
private:
	std::vector<int>	_Towns; // List of town recno's
	int					_Nation; // recno of the owning nation
	int					_MyRecno; // my recno (in TownNetworkArray)


public:
	int size() const {return _Towns.size();}
	int nation_recno() const {return _Nation;} // The nation_recno of all the towns in this list, or 0 if no towns are added (because independent villages do not form town networks)
	int recno() const {return _MyRecno;} // NOTE: recno can change by saving & loading; do not cache the recno of a town network in a way that persists beyond saving and loading.
	
	int	operator[](int index) {return _Towns[index];}


protected:
	// The following function are only called by TownNetworkArray to add and remove Towns from the network
	friend class TownNetworkArray;
	TownNetwork(int nationRecno);
	void set_recno(int recno);
	void add_town(int townRecno);
	void remove_town(int townRecno);
	void merge_in(TownNetwork const * pNetwork); // Merge the elements from network into this one
	TownNetwork* split_by_pulsed(TownNetwork *splitDestination, bool movePulsedTowns); // Splits the current network into two parts, based on pulsed status of towns
	void reset_pulsed();
};


// TownNetworkArray, a collection of TownNetworks. Handles updating of the TownNetwork, as a response
// to notifications by Town
class TownNetworkArray : private DynArrayB
{
public:
	TownNetworkArray();
	~TownNetworkArray();

	void init();
	void deinit();

	int size() const {return DynArrayB::size();}

	TownNetwork* operator[](int recNo) const {return network(recNo); }

	// All the changes that can happen to a Town that affect the Town Networks. Called by Town when the events occur.
	// Note that Independent Villages never form a town network.
	int town_created(int townRecno, int nationRecno, short const *linkedTowns, int linkedCount); // Returns the town network recno for the town
	void town_destroyed(int townRecno); // Call this when the town is about to be destroyed
	void town_pre_changing_nation(int townRecno); // Call this just before a town changes nations
	void town_post_changing_nation(int townRecno, int newNationRecno); // Call this just after a town has changed nations
	void recreate_after_load(); // Recreate the town network after the TownArray has finished loading from file

private:
	TownNetwork* network(int recNo) const {return (TownNetwork*) get_ptr(recNo);}
	int merge(int tn1, int tn2); // Merges the two town-network, and returns the new recno for the network
	TownNetwork* add_network(int nationRecno);
	void remove_network(int recno);
	int pulse(int townRecno, int nationRecno); // Pulses the towns that are linked, and returns the amount of pulses set
	void add_all_linked(int townRecno, TownNetwork *pTownNetwork);
};

extern TownNetworkArray town_network_array;

#endif