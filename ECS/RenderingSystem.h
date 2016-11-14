#pragma once
#include "RenderingComponent.h"
#include "System.h"
#include "Vertex.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "FreeVector.h"
#include "EntityIdTypeDef.h"
#include <d3d11.h>
#include <unordered_map>

struct RenderingComponentData{
	FreeVector<unsigned int> * m_instanceCollection;
	unsigned int m_instanceHandle;
};

class RenderingSystem : public System<RenderingComponent> {
public:
	void Update(Game * g, float dt);
	void Init(IDXGISwapChain * swapChain, ID3D11Device * device, ID3D11DeviceContext * context, ID3D11RenderTargetView * renderTargetView, ID3D11DepthStencilView * depthStencilView);
	void CreateInstance(EntityId entityId, RenderingComponent * rc);
	RenderingSystem() {};
private:
	IDXGISwapChain*			m_swapChain;
	ID3D11Device*			m_device;
	ID3D11DeviceContext*	m_context;
	ID3D11RenderTargetView* m_backBufferRTV;
	ID3D11DepthStencilView* m_depthStencilView;
	Camera					m_camera;
	unordered_map<RenderingComponent*, FreeVector<EntityId>>	m_instancedComponents;
	vector<RenderingComponentData> m_instancedComponentData;
	DirectionalLight		m_dirLights[3];
};
