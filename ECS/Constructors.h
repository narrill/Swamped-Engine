#pragma once
#include "Game.h"
#include "ContentManager.h"
#include "RenderingComponent.h"
#include <unordered_map>

using namespace DirectX;
//Contains static constructors for preformed entities
class Constructors {
	static unordered_map<std::string, RenderingComponent> m_renderingComponents;
public:
	static double fRand(double min, double max) {
		double f = (double)rand() / RAND_MAX;
		return min + f * (max - min);
	}
	static void Init(Game * g) {
		m_renderingComponents["testObj"] = {
			g->m_cm.GetMaterial("TestMaterial"),
			g->m_cm.GetMeshStore("cone.obj").m_m
		};
		m_renderingComponents["testObj2"] = {
			g->m_cm.GetMaterial("TestMaterial"),
			g->m_cm.GetMeshStore("cube.obj").m_m
		};
	}
	static void CreateTestObject(Game * g) {
		//get entityID and add system list to game
		EntityId eid = g->m_entities.add(vector<ISystem*>{
			&g->m_ts, 
			&g->m_cs,
			&g->m_rs
		});

		//get mesh and bounding box
		MeshStore ms = g->m_cm.GetMeshStore("cone.obj");
		PhysicsComponent pc;
		pc.m_velocity = XMFLOAT3(0, 0, 0);
		pc.m_acceleration = XMFLOAT3(0, 0, 0);
		
		//XMStoreFloat4(&pc.m_rotationalVelocity, XMQuaternionRotationRollPitchYaw(0, 15, 1));
		pc.m_rotationalVelocity = XMFLOAT3(fRand(-30, 30), fRand(-30, 30), fRand(-30, 30));
		//XMStoreFloat3(&pc.m_rotationalAcceleration, XMQuaternionRotationRollPitchYaw(0, 0, 0));
		TransformComponent tc;
		//XMStoreFloat3(&tc.m_rotation, XMQuaternionRotationRollPitchYaw(0, 0, 0));
		tc.m_position = XMFLOAT3(fRand(-100, 100), fRand(0, 100), fRand(-100, 100));
		//tc.m_position = XMFLOAT3(0, 10, -45);
		//vector<CollisionType> cTypes = { CollisionType::none };
		//copy collision mask into bounding box
		ms.m_bb.m_ct = CollisionType::none;

		//create components
		g->m_ts.Create(eid, tc, pc);
		g->m_cs.Create(eid, ms.m_bb);
		g->m_rs.Create(eid, &m_renderingComponents["testObj"]);
	}

	static void CreateTestObject2(Game * g) {
		//get entityID and add system list to game
		EntityId eid = g->m_entities.add(vector<ISystem*>{
			&g->m_ts,
			&g->m_cs,
			&g->m_rs
		});

		//get mesh and bounding box
		MeshStore ms = g->m_cm.GetMeshStore("cube.obj");
		PhysicsComponent pc;
		pc.m_velocity = XMFLOAT3(0, 0, 0);
		pc.m_acceleration = XMFLOAT3(0, 0, 0);

		//XMStoreFloat4(&pc.m_rotationalVelocity, XMQuaternionRotationRollPitchYaw(0, 15, 1));
		pc.m_rotationalVelocity = XMFLOAT3(fRand(-30, 30), fRand(-30, 30), fRand(-30, 30));
		//XMStoreFloat3(&pc.m_rotationalAcceleration, XMQuaternionRotationRollPitchYaw(0, 0, 0));
		TransformComponent tc;
		//XMStoreFloat3(&tc.m_rotation, XMQuaternionRotationRollPitchYaw(0, 0, 0));
		tc.m_position = XMFLOAT3(fRand(-100, 100), fRand(0, 100), fRand(-100, 100));
		//tc.m_position = XMFLOAT3(0, 10, -45);
		vector<CollisionType> cTypes = { CollisionType::none };
		//copy collision mask into bounding box
		ms.m_bb.m_ct = CollisionType::player;

		//create components
		g->m_ts.Create(eid, tc, pc);
		g->m_cs.Create(eid, ms.m_bb);
		g->m_rs.Create(eid, &m_renderingComponents["testObj2"]);
	}
};