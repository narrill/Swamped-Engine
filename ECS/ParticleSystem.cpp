#include "ParticleSystem.h"
#include "Game.h"
#include "GlobalFunctions.h"

ParticleSystem::ParticleSystem() {
	m_bounds.m_max = XMFLOAT3(100, 100, 100);
	m_bounds.m_min = XMFLOAT3(-100, 0, -100);
}

ParticleSystem::~ParticleSystem() {

}

size_t ParticleSystem::GetParticleCount() {
	return m_particles.count();
}

vector<CollapsedNonComponent<Particle>> & ParticleSystem::GetCollapsedParticles() {
	return m_collapsedParticles;
}

void ParticleSystem::Collapse() {
	m_collapsedCount = 0;
	m_collapsedParticles.resize(m_particles.count());
	for (unsigned int c = 0; c < m_activeParticles.size(); c++) {
		if (m_activeParticles[c] == true) {
			m_collapsedParticles[m_collapsedCount] = { m_particles[c], c };
			m_collapsedCount++;
		}
	}
}

void ParticleSystem::Update(Game * g, float dt) {
	//generate new particles
	for (unsigned int c = 0; c < 200; c++) {
		unsigned int index = m_particles.add({ XMFLOAT3(fRand(-100, 100), fRand(0, 100), fRand(-100, 100)) });
		if (index == m_activeParticles.size())
			m_activeParticles.push_back(true);
		else
			m_activeParticles[index] = true;
	}

	Collapse();
	mutex freeMutex;
	//update positions
	parallel_for(size_t(0), m_collapsedCount, [&](unsigned int c) {
		XMVECTOR position;
		XMVECTOR velocity;
		auto& cp = m_collapsedParticles[c];
		auto& p = cp.m_component;

		//load stuff
		position = XMLoadFloat3(&cp.m_component.m_position);
		velocity = XMLoadFloat3(&cp.m_component.m_velocity);
		position += dt*velocity;

		//store stuff
		XMStoreFloat3(&m_particles[cp.m_handle].m_velocity, velocity);
		XMStoreFloat3(&m_particles[cp.m_handle].m_position, position);

		if (rand() % 500 == 0) {
			freeMutex.lock();
			m_activeParticles[cp.m_handle] = false;
			m_particles.free(cp.m_handle);
			freeMutex.unlock();
		}
	});
}