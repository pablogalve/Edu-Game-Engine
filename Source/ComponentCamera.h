#ifndef __COMPONENT_CAMERA_H__
#define __COMPONENT_CAMERA_H__

#include "Globals.h"
#include "Component.h"
#include "Math.h"
#include "Color.h"

class ComponentCamera : public Component
{
	friend class ModuleAudio;
public:
	ComponentCamera (GameObject* container);
	~ComponentCamera ();

	void OnSave(Config& config) const override;
	void OnLoad(Config* config) override;

	void OnStart() override;
	void OnUpdate(float dt) override;
	void OnDebugDraw(bool selected) const override;

	void OnUpdateTransform() override;
    void OnUpdateFrustum();

	float GetNearPlaneDist() const;
	float GetFarPlaneDist() const;
	float GetFOV() const;
	float GetAspectRatio() const;

	void SetNearPlaneDist(float dist);
	void SetFarPlaneDist(float dist);
	void SetFOV(float fov);
	void SetAspectRatio(float aspect_ratio);

	void Look(const float3& position);

	float4x4 GetOpenGLViewMatrix() const;
	float4x4 GetOpenGLProjectionMatrix() const;

    float4x4 GetCameraMatrix() const;
	float4x4 GetViewMatrix() const;
	float4x4 GetProjectionMatrix() const;
    float3   GetFront() const { return frustum.front; }
	float3   GetPos() const { return frustum.pos; }

public:
	Frustum frustum;
	Color background;
	bool projection_changed = false;
	const GameObject* looking_at = nullptr;
	uint looking_at_uid = 0;
};

#endif // __COMPONENT_CAMERA_H__
