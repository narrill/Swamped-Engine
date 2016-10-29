#pragma once
#include "SystemBase.h"
#include "ComponentData.h"
#include "FreeVector.h"
#include <vector>
#include <map>

//A two component system that ensures the two component lists stay in sync
template <typename T, typename U>
class PairedSystem : public SystemBase {
public:
	void virtual Update(Game * g, float dT) {}

	//Creates a component of type T and adds it to m_components and m_handles
	void virtual Create(unsigned int entityId, T tc, U uc) {
		ComponentData comp = ComponentData(entityId);
		comp.m_active = true;
		unsigned int index = m_components1.add(tc);
		m_handles[entityId] = index;
		m_components2.add(uc);
		if (index == m_componentData.size())
			m_componentData.push_back(comp);
		else
			m_componentData[index] = comp;
	}

	//Deactivates the component with the given ID, frees the space in m_components, and erases the handle
	void virtual Remove(unsigned int entityId) {
		unsigned int handle = m_handles[entityId];
		m_componentData[handle].m_active = false;
		m_components1.free(handle);
		m_components2.free(handle);
		m_handles.erase(entityId);
	}

	//Returns a reference to the component of type T with the given ID
	T& GetComponent1(unsigned int entityId) {
		return m_components1[m_handles[entityId]];
	}

	//Returns a reference to the component of type U with the given ID
	U& GetComponent2(unsigned int entityId) {
		return m_components2[m_handles[entityId]];
	}

	//Returns a pointer to the component list for type T
	FreeVector<T> & GetComponentList1() {
		return m_components1;
	}

	//Returns a pointer to the component list for type U
	FreeVector<U> & GetComponentList2() {
		return m_components2;
	}

	//Returns a reference to the ComponentData vector
	vector<ComponentData> & GetComponentData() {
		return m_componentData;
	}

	//Gets the number of components
	size_t GetCount() {
		return m_components1.count();
	}

	//Initializes members
	PairedSystem() : SystemBase(){
		m_components1 = FreeVector<T>();
		m_components2 = FreeVector<U>();
		m_handles = map<unsigned int, unsigned int>();
	}
	~PairedSystem() {}
protected:
	//Holds components of type T
	FreeVector<T> m_components1;

	//Holds components of type U
	FreeVector<U> m_components2;
private:
	//Holds entityId - index pairs
	map<unsigned int, unsigned int> m_handles;
};