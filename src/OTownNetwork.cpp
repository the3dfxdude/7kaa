/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 2013 Richard Dijk <thor_madman@hotmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//Filename    : OTownNetwork.cpp
//Description : Source file for TownNetwork, an extension to linked towns
//              which is binary compatible with existing data by dynamically
//				generating from existing data


#include <OTownNetwork.h>
#include <OTOWN.h>
#include <stack>

#ifdef _DEBUG
#define DEBUG_CHECK	true
#define DEBUG_CHECK_WORK true
#else
#define DEBUG_CHECK	false
#define DEBUG_CHECK_WORK false
#endif


TownNetwork::TownNetwork(int nationRecno) : _Nation(nationRecno), _MyRecno(0)
{
	if (DEBUG_CHECK && nationRecno == 0) throw "nationRecno can not be 0 for a Town Network";
}


void TownNetwork::set_recno(int recno)
{
	if (DEBUG_CHECK && recno == 0) throw "My recno can not be 0";
	else if (DEBUG_CHECK && _MyRecno != 0) throw "My recno has already been set";

	_MyRecno = recno;
}


void TownNetwork::add_town(int townRecno)
{
	if (DEBUG_CHECK)
	{
		if (townRecno == 0) throw "townRecno is 0";
		else if (town_array[townRecno]->nation_recno != _Nation) throw "The town given by townRecno does not have the same nation as the network";
	}

	// If in heavy debugging phase, then ensure that townRecno is not already in the list
	if (DEBUG_CHECK_WORK)
	{
		for (std::vector<int>::iterator it = _Towns.begin(); it != _Towns.end(); ++it)
			if (*it == townRecno)
				throw "The specified town is already in the network";
	}

	_Towns.push_back(townRecno);
	town_array[townRecno]->town_network_recno = _MyRecno;
}


void TownNetwork::remove_town(int townRecno)
{
	if ( ! townRecno)
	{
		if (DEBUG_CHECK) throw "townRecno is 0";
		else return;
	}

	// Need to find the town with the given recno and remove it
	for (std::vector<int>::iterator it = _Towns.begin(); it != _Towns.end(); ++it)
	{
		if (*it == townRecno)
		{
			_Towns.erase(it);
			return;
		}
	}

	if (DEBUG_CHECK)
		throw "TownNetwork::remove_town did not find an item with that record number";
}


// ---- Function merge_in ----
// Merges the Towns of the other Town network into this one.
// Used by TownNetworkArray.
//
void TownNetwork::merge_in(TownNetwork const * pNetwork)
{
	std::vector<int> const *pTowns = &pNetwork->_Towns;

	if (DEBUG_CHECK && ((void*)pNetwork == (void*)this)) throw "Attempted to merge_in self";

	for (std::vector<int>::const_iterator it = pTowns->begin(); it != pTowns->end(); ++it)
	{
		if (DEBUG_CHECK_WORK)
			add_town(*it); // add_town does checks, such as uniqueness
		else
		{
			_Towns.push_back(*it);
			town_array[*it]->town_network_recno = _MyRecno;
		}
	}
}


// ---- Function split_by_pulsed ----
// Splits the current network into two parts, based on pulsed status of towns
// 
//  - splitDestination: the destination network to hold the towns that were split off
//  - movePulsedTowns: if this is true, then towns that have their pulsed true are moved into splitDestination.
//                     If this is false, then towns that have their pulsed false are moved into splitDestination.
//  - <returns>: The town network containing the non-pulsed Towns.
//
TownNetwork* TownNetwork::split_by_pulsed(TownNetwork *splitDestination, bool movePulsedTowns)
{
	// TODO: Change for greater efficiency: set _Towns[i] to 0 at first whilst splitting, then compact vector
	//       by performing a one-pass copy operation to move non-zero elements to the top, then call resize()

	for (std::vector<int>::iterator it = _Towns.begin(); it != _Towns.end(); /* 'it' updated in loop */)
	{
		Town *pTown = town_array[*it];

		if (pTown->town_network_pulsed == movePulsedTowns)
		{
			splitDestination->_Towns.push_back(*it);
			pTown->town_network_recno = splitDestination->recno();
			it = _Towns.erase(it);
		}
		else
			++it;
	}

	return (movePulsedTowns ? this : splitDestination);
}


// ---- Function reset_pulsed ----
// Resets pulsed value of each town in the town network. Called after pulsing event.
//
void TownNetwork::reset_pulsed()
{
	for (std::vector<int>::iterator it = _Towns.begin(); it != _Towns.end(); ++it)
	{
		if (DEBUG_CHECK && !town_array[*it]->town_network_pulsed)
			throw "Found unpulsed Town in the network ... Should have reached all in pulsing event";
		town_array[*it]->town_network_pulsed = false;
	}
}














TownNetworkArray::TownNetworkArray() : DynArrayB(sizeof(TownNetwork*), 10, DEFAULT_REUSE_INTERVAL_DAYS)
{
}

TownNetworkArray::~TownNetworkArray()
{
	deinit();
}

void TownNetworkArray::init()
{
}


void TownNetworkArray::deinit()
{
	//----- delete TownNetwork objects ------//

	if( size() > 0 )
	{
		TownNetwork* townNetworkPtr;

		for( int i=1 ; i<=size() ; i++ )
		{
			townNetworkPtr = (TownNetwork*) get_ptr(i);

			if( townNetworkPtr )
				delete townNetworkPtr;
		}

		zap();
	}
}


TownNetwork* TownNetworkArray::add_network(int nationRecno)
{
	TownNetwork *network = new TownNetwork(nationRecno);
	
	linkin(&network);
	network->set_recno(recno());

	return network;
}


void TownNetworkArray::remove_network(int recno)
{
	if (recno < 1 || recno > size())
	{
		if (DEBUG_CHECK) throw "Invalid recno passed to remove_network";
		else return;
	}

	TownNetwork *pNetwork = network(recno);
	if (DEBUG_CHECK && pNetwork == NULL) throw "Town Network has already been removed";
	delete pNetwork; // deleting null is safe
	linkout(recno);
}


// ---- Function town_created ----
// Updates the town networks when a town is created
// 
//  - townRecno: the record number of the newly created town
//  - nationRecno: the nation of the newly created town
//  - linkedTowns: the list of linked towns
//  - linkedCount: number of towns in the linked towns list
//  - <returns>: the town network recno for the newly created town
//
int TownNetworkArray::town_created(int townRecno, int nationRecno, short const *linkedTowns, int linkedCount)
{
	// Logic: iterate over all linked towns. First, find a town with the same
	// nation, and add it to that network. Then, for the remaining links, if any same-nation town
	// is in a different network then join these networks.

	if (DEBUG_CHECK)
	{
		if (townRecno == 0) throw "townRecno is 0";
		else if (linkedTowns == NULL) throw "linkedTowns is NULL";
		else if (linkedCount < 0) throw "linkedCount is not a nonnegative integer";
	}

	// Independent towns do not form town networks
	if (nationRecno == 0)
		return 0;

	int tnRecno = 0;
	for (int i = 0; i < linkedCount; ++i)
	{
		Town *pTown = town_array[linkedTowns[i]];
		
		if (pTown->nation_recno == nationRecno)
		{
			// Add some robustness for errors (in Release) by verifying and attempting to repair faults
			verify_town_network_before_merge(nationRecno, pTown->town_recno);

			// If it is the first own-nation town then add it to that network
			if (tnRecno == 0)
			{
				tnRecno = pTown->town_network_recno;
				network(tnRecno)->add_town(townRecno);
			}
			else if (pTown->town_network_recno != tnRecno)
			{
				// Found a different network that has now been connected to. Perform a merge.
				tnRecno = merge(tnRecno, pTown->town_network_recno);
			}
		}
	}

	// If there were no network to connect to, create a new network
	if (tnRecno == 0)
	{
		TownNetwork *pNetwork = add_network(nationRecno);
		pNetwork->add_town(townRecno);
		tnRecno = pNetwork->recno();
	}

	return tnRecno;
}


// ---- Function verify_town_before_merge ----
// Adds some robustness for errors by checking a town and fixing errors
// in the town network structure. Used only by town_create.
// 
//  - nationRecno: the nation recno of the town that is created
//  - townRecno: the recno of the town to check. This should be a town linked to the newly created town.
//
void TownNetworkArray::verify_town_network_before_merge(int nationRecno, int townRecno)
{
	Town *pTown = town_array[townRecno];
	int tnRecno = pTown->town_network_recno;

	bool shouldFix = false;

	if (tnRecno < 1 || tnRecno > size())
	{
		if (DEBUG_CHECK)
			throw "town_created: one of the same-nation linked towns has an invalid town network recno";
		else
			shouldFix = true;
	}
	else if (network(tnRecno) == NULL)
	{
		if (DEBUG_CHECK)
			throw "Town Network has been deleted, but towns still point to its recno";
		else
			shouldFix = true;
	}

	// Fixing errors in the town-network structure, by creating a new town network for the town
	if (shouldFix)
	{
		TownNetwork *pTN = add_network(nationRecno);
		pTN->add_town(townRecno);
	}
}


// ---- Function merge ----
// Merges the two town-network, and returns the new recno for the network
// 
//  - tn1, tn2: the record numbers of the the towns networks
//  - <returns>: the new town network recno for the combined network
//
int TownNetworkArray::merge(int tn1, int tn2)
{
	TownNetwork *pTN1 = network(tn1), *pTN2 = network(tn2), *pSwap;
	if (pTN2->size() > pTN1->size())
		pSwap = pTN1, pTN1 = pTN2, pTN2 = pSwap;

	// Merge 2 into 1
	pTN1->merge_in(pTN2);
	remove_network(pTN2->recno()); pTN2 = NULL;

	return pTN1->recno();
}


// ---- Function town_destroyed ----
// Updates the town networks when a town is destroyed
// 
//  - townRecno: the record number of the town.
//
void TownNetworkArray::town_destroyed(int townRecno)
{
	// Logic: Set Pulsing = false, and pulsed for the source Town to true. Remove source Town from network.
	// Iterate over all linked towns of same nation:
	//   If not Pulsing, then Pulse link, which will recursively pulse all towns that are linked via any number of hops. Set Pulsing = true.
	//   Else if Pulsing, if linked town has been pulsed, continue. If it has not been pulsed, then split off all pulsed towns so far into a
	//   new town network; set Pulsing = false, and redo iteration for current linked town.
	//   Finally, reset pulsed for all towns in the original network to false.
	// NOTE: Do not cache town-network numbers in this function, use pNetwork->town_network_recno instead.

	if (townRecno == 0) {if (DEBUG_CHECK) throw "townRecno is 0"; else return;}

	Town *pTown = town_array[townRecno];
	if (pTown == NULL) {if (DEBUG_CHECK) throw "Town no longer exists in TownArray"; else return;}

	// Independent towns do not form town networks
	if (pTown->nation_recno == 0)
	{
		if (DEBUG_CHECK && pTown->town_network_recno != 0) throw "Found independent town with a non-trivial town network";
		return;
	}

	if (DEBUG_CHECK && pTown->town_network_pulsed) throw "Town has town_network_pulsed to true outside pulsing session";


	TownNetwork *pNetwork = network(pTown->town_network_recno); // The currently 'active' network (the one being pulsed), may change during the loop
	
	// Remove the town from the town-network, before starting the rebuild. If this was the only town in it, then remove the network and return
	pNetwork->remove_town(townRecno);
	if (pNetwork->size() == 0)
	{
		remove_network(pNetwork->recno());
		return;
	}

	// Keep track of all town networks involved, so the pulsed can be reset
	TownNetwork *allNetworks[1 + MAX_LINKED_TOWN_TOWN], // Can only have as much disconnected parts as there can be links to the town
				**pNextAllNetwork = allNetworks; // Fill pointer
	*(pNextAllNetwork++) = NULL; // Set first element to 0, so it can be used as a end-of-list delimiter
	*(pNextAllNetwork++) = pNetwork;


	// Initialise pulsing-variables
	int nationRecno = pTown->nation_recno;
	bool Pulsing = false;
	pTown->town_network_pulsed = true;
	int pulsedCount = 0;
	
	// Iterate over the linked towns
	for (int i = 0; i < pTown->linked_town_count; ++i)
	{
		Town *pLinked = town_array[pTown->linked_town_array[i]];
		if (pLinked->nation_recno != nationRecno) continue;

		if (!Pulsing)
		{
			Pulsing = true;
			pulsedCount = pulse(pLinked->town_recno, pLinked->nation_recno);
		}
		else if ( ! pLinked->town_network_pulsed)
		{
			// Found disconnected parts, so need to split off all pulsed. Note: this may cause the town-network-recno's of any of the towns
			// in the original network to change, so the town network recno should not be cached.
			TownNetwork *pNewNetwork = add_network(nationRecno);
			*(pNextAllNetwork++) = pNewNetwork;
			// Decide which part gets to stay in the original network by checking which is larger
			bool movePulsed = (pulsedCount <= (pNetwork->size() / 2 + 1));
			pNetwork = pNetwork->split_by_pulsed(pNewNetwork, movePulsed); // pNetwork is set to the network of unpulsed towns
			// Redo iteration for current, now with pulsing false
			Pulsing = false;
			--i;
		}		
	}

	// Reset pulsed for all towns
	while( *(--pNextAllNetwork) != NULL )
		(*pNextAllNetwork)->reset_pulsed();
	pTown->town_network_pulsed = false;
}


// ---- Function pulse ----
// Pulses the towns that are linked, and returns the amount of pulses set
// 
//  - <returns>: number of pulses set (towns that were already pulsed are not counted)
//
int TownNetworkArray::pulse(int townRecno, int nationRecno)
{
	std::stack<int> towns;
	int pulsed = 0;

	// Use a stack to implement the algorithm of successively pulsing each linked town
	// without employing recursion

	towns.push(townRecno);
	while( towns.size() > 0 )
	{
		// Pop from the top
		int activeRecno = towns.top();
		towns.pop();
		Town *pTown = town_array[activeRecno];
		// If it has not yet been pulsed, do so now, and add linked towns to the stack
		if (!pTown->town_network_pulsed)
		{
			pTown->town_network_pulsed = true;
			++pulsed;
			for (int i = 0; i < pTown->linked_town_count; ++i)
			{
				// Only same-nation linked
				if (town_array[ pTown->linked_town_array[i] ]->nation_recno == nationRecno)
					towns.push(pTown->linked_town_array[i]);
			}
		}
	}

	return pulsed;
}

// ---- Function add_all_linked ----
// Adds all the linked towns (directly or indirectly) to the given town network.
// 
//  - townRecno: the town to start the link traversal for
//  - pTownNetwork: pointer to the town-network to add all towns to; should be an empty network
//
void TownNetworkArray::add_all_linked(int townRecno, TownNetwork *pTownNetwork)
{
	std::stack<int> towns;

	if (DEBUG_CHECK && pTownNetwork->size() > 0)
		throw "pTownNetwork does not point to a freshly created town network. Was this the intention?";
	else if (DEBUG_CHECK && town_array[townRecno]->nation_recno != pTownNetwork->nation_recno())
		throw "Start town nationality and Town Network nationality do not match";

	// Use a stack to implement the algorithm of successively pulsing each linked town
	// without employing recursion

	towns.push(townRecno);
	while( towns.size() > 0 )
	{
		// Pop from the top
		int activeRecno = towns.top();
		towns.pop();
		Town *pTown = town_array[activeRecno];
		// If it has not yet been pulsed, do so now, and add linked towns to the stack
		if (!pTown->town_network_pulsed && pTown->nation_recno == pTownNetwork->nation_recno())
		{
			// Add town, and set its pulsed value
			pTownNetwork->add_town(pTown->town_recno);
			pTown->town_network_pulsed = true;
			// Add linked towns
			for (int i = 0; i < pTown->linked_town_count; ++i)
			{
				// Only same-nation linked
				if (town_array[ pTown->linked_town_array[i] ]->nation_recno == pTownNetwork->nation_recno())
					towns.push(pTown->linked_town_array[i]);
			}
		}
	}

	// Pulse value was used for each town in the network - reset it now
	pTownNetwork->reset_pulsed();
}


// ---- Function town_pre_changing_nation ----
// Does the pre-update for the town networks when a town is about to change nation
// 
//  - townRecno: the record number of the town
//
void TownNetworkArray::town_pre_changing_nation(int townRecno)
{
	if (townRecno == 0) {if (DEBUG_CHECK) throw "townRecno is 0"; else return;}

	Town *pTown = town_array[townRecno];
	if (pTown == NULL) {if (DEBUG_CHECK) throw "Town no longer exists in TownArray"; else return;}

	// Independent towns do not form town networks
	if (pTown->nation_recno == 0)
		return;

	// Changing nation is equivalent to being destroyed (as old nation) and recreated as new nation.
	// Pre-step is destroying
	town_destroyed(townRecno);
}


// ---- Function town_post_changing_nation ----
// Does the post-update for the town networks when a town has just changed nation
// 
//  - townRecno: the record number of the town
//
void TownNetworkArray::town_post_changing_nation(int townRecno, int newNationRecno)
{
	if (townRecno == 0) {if (DEBUG_CHECK) throw "townRecno is 0"; else return;}

	Town *pTown = town_array[townRecno];
	if (pTown == NULL) {if (DEBUG_CHECK) throw "Town is no longer exists in TownArray"; else return;}

	if (DEBUG_CHECK && pTown->nation_recno != newNationRecno) throw "Town has not yet changed nations (newNationsRecno != nation_recno), but town_post_changing_nation was called already";

	// Independent towns do not form town networks
	if (newNationRecno == 0)
		return;

	// Changing nation is equivalent to being destroyed (as old nation) and recreated as new nation.
	// Post-step is creating
	town_created(townRecno, newNationRecno, pTown->linked_town_array, pTown->linked_town_count);
}


// ---- Function recreate_after_load ----
// Recreates the town network after TownArray has finished loading from file.
// Note: this function can only access the Towns and TownArray, as these are guaranteed to be loaded.
// 
void TownNetworkArray::recreate_after_load()
{
	for (int i = 1; i <= town_array.size(); ++i)
	{
		if (town_array.is_deleted(i)) continue; // Skip over empty elements
		Town *pTown = town_array[i];
		
		// Create town networks for each town without a town network.
		// Independent towns do not form town networks
		if (pTown->town_network_recno == 0 && pTown->nation_recno != 0)
		{
			TownNetwork *pNetwork = add_network(pTown->nation_recno);
			// Add all (indirectly) linked towns to the network
			add_all_linked(pTown->town_recno, pNetwork);
		}
	}
}