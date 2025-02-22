#include "Globals.h"
#include "Application.h"
#include "ModuleEditorCamera.h"
#include "ModuleEditor.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ComponentCamera.h"
#include "ModuleRenderer3D.h"
#include "Viewport.h"
#include "SceneViewport.h"
#include "ModuleHints.h"
#include "DebugDraw.h"
#include "ModuleLevelManager.h"
#include "GameObject.h"
#include <vector>

#include "Leaks.h"

using namespace std;

ModuleEditorCamera::ModuleEditorCamera(bool start_enabled) : Module("Camera", start_enabled)
{
	dummy = new ComponentCamera(nullptr);
	picking = LineSegment(float3::zero, float3::unitY);
	last_hit = float3::zero;
}

ModuleEditorCamera::~ModuleEditorCamera()
{
	RELEASE(dummy);
}

// -----------------------------------------------------------------
bool ModuleEditorCamera::Init(Config* config)
{
	dummy->OnLoad(config);
	App->renderer3D->active_camera = dummy;
	App->renderer3D->culling_camera = dummy;

	return true;
}

// -----------------------------------------------------------------
bool ModuleEditorCamera::Start(Config* config)
{
	LOG("Setting up the camera");
	bool ret = true;

	Load(config);

	return ret;
}

// -----------------------------------------------------------------
bool ModuleEditorCamera::CleanUp()
{
	LOG("Cleaning camera");

	App->renderer3D->active_camera = nullptr;
	return true;
}

// -----------------------------------------------------------------
void ModuleEditorCamera::Save(Config * config) const
{
	config->AddFloat("Mov Speed", mov_speed);
	config->AddFloat("Rot Speed", rot_speed);
	config->AddFloat("Zoom Speed", zoom_speed);
	dummy->OnSave(*config);
}

// -----------------------------------------------------------------
void ModuleEditorCamera::Load(Config * config)
{
	// Beware, this method will be called again when loading a level!
	mov_speed  = config->GetFloat("Mov Speed", mov_speed); // global var, not level specific
	rot_speed  = config->GetFloat("Rot Speed", rot_speed); // global var, not level specific
	zoom_speed = config->GetFloat("Zoom Speed", zoom_speed); // global var, not level specific
	dummy->OnLoad(config);
}

// -----------------------------------------------------------------
void ModuleEditorCamera::DrawDebug()
{
	//dd::line(picking.a, picking.b, dd::colors::Blue);
	//dd::point(last_hit, dd::colors::Red);
}

// -----------------------------------------------------------------
update_status ModuleEditorCamera::Update(float dt)
{
	// Keyboard for WASD movement -------
	if (App->renderer3D->viewport->GetScene()->IsFocused()) 
    {
		Move(dt);
        App->renderer3D->active_camera->OnUpdateFrustum();
    }

	// Mouse ----------------------------
	if (App->renderer3D->viewport->GetScene()->IsFocused() && App->renderer3D->viewport->GetScene()->IsUsingGuizmo() == false)
	{
		// Check motion for lookat / Orbit cameras
		int motion_x, motion_y;
		App->input->GetMouseMotion(motion_x, motion_y);
		if (App->input->GetMouseButton(SDL_BUTTON_MIDDLE) == KEY_REPEAT && (motion_x != 0 || motion_y != 0))
		{
			float dx = (float)-motion_x * rot_speed * dt;
			float dy = (float)-motion_y * rot_speed * dt;

			if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT)
				Orbit(dx, dy);
			else
				LookAt(dx, dy);
		}

        float metric_proportion = App->hints->GetFloatValue(ModuleHints::METRIC_PROPORTION);

		// Mouse wheel for zoom
		int wheel = App->input->GetMouseWheel();
		if (wheel != 0)
			Zoom(wheel * zoom_speed * metric_proportion* dt);

		// Mouse Picking
		if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_DOWN)
		{
			//GameObject* pick = Pick();
			//if (pick != nullptr)
				//App->editor->SetSelected(pick, (App->editor->selection_type == ModuleEditor::SelectionGameObject && App->editor->selected.go == pick));
		}

        App->renderer3D->active_camera->OnUpdateFrustum();
    }


	return UPDATE_CONTINUE;
}

// -----------------------------------------------------------------
void ModuleEditorCamera::Look(const float3& position)
{
	App->renderer3D->active_camera->Look(position);
}

// -----------------------------------------------------------------
void ModuleEditorCamera::CenterOn(const float3& position, float distance)
{
	float3 v = App->renderer3D->active_camera->frustum.front.Neg();
	App->renderer3D->active_camera->frustum.pos = position + (v * distance);
	looking_at = position;
	looking = true;
}

// -----------------------------------------------------------------
ComponentCamera * ModuleEditorCamera::GetDummy() const
{
	return dummy;
}

// -----------------------------------------------------------------
void ModuleEditorCamera::Move(float dt)
{
	Frustum* frustum = &App->renderer3D->active_camera->frustum;

	float adjusted_speed = mov_speed;

	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) adjusted_speed *= 5.0f;
	if (App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT) adjusted_speed *= 0.5f;

	float3 right(frustum->WorldRight());
	float3 forward(frustum->front);

	float3 movement(float3::zero);

    float metric_proportion = App->hints->GetFloatValue(ModuleHints::METRIC_PROPORTION);

	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) movement += forward;
	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) movement -= forward;
	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) movement -= right;
	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) movement += right;
	if (App->input->GetKey(SDL_SCANCODE_R) == KEY_REPEAT) movement += float3::unitY;
	if (App->input->GetKey(SDL_SCANCODE_F) == KEY_REPEAT) movement -= float3::unitY;

	if (movement.Equals(float3::zero) == false)
	{
		frustum->Translate(movement * (metric_proportion*adjusted_speed * dt));
		looking = false;
	}
}

// -----------------------------------------------------------------
void ModuleEditorCamera::Orbit(float dx, float dy)
{
	float3 point = looking_at;

	// fake point should be a ray colliding with something
	if (looking == false)
	{
		LineSegment picking = App->renderer3D->active_camera->frustum.UnProjectLineSegment(0.f, 0.f);
		float distance;
		GameObject* hit = App->level->CastRay(picking, distance);

		if (hit != nullptr)
			point = picking.GetPoint(distance);
		else
			point = App->renderer3D->active_camera->frustum.pos + App->renderer3D->active_camera->frustum.front * 50.0f;

		looking = true;
		looking_at = point;
	}

	float3 focus = App->renderer3D->active_camera->frustum.pos - point;

	Quat qy(App->renderer3D->active_camera->frustum.up, dx);
	Quat qx(App->renderer3D->active_camera->frustum.WorldRight(), dy);

	focus = qx.Transform(focus);
	focus = qy.Transform(focus);

	App->renderer3D->active_camera->frustum.pos = focus + point;

	Look(point);
}

// -----------------------------------------------------------------
void ModuleEditorCamera::LookAt(float dx, float dy)
{
	looking = false;

	// x motion make the camera rotate in Y absolute axis (0,1,0) (not local)
	if (dx != 0.f)
	{
		Quat q = Quat::RotateY(dx);
		App->renderer3D->active_camera->frustum.front = q.Mul(App->renderer3D->active_camera->frustum.front).Normalized();
		// would not need this is we were rotating in the local Y, but that is too disorienting
		App->renderer3D->active_camera->frustum.up = q.Mul(App->renderer3D->active_camera->frustum.up).Normalized();
	}

	// y motion makes the camera rotate in X local axis, with tops
	if(dy != 0.f)
	{
		Quat q = Quat::RotateAxisAngle(App->renderer3D->active_camera->frustum.WorldRight(), dy);

		float3 new_up = q.Mul(App->renderer3D->active_camera->frustum.up).Normalized();

		if (new_up.y > 0.0f)
		{
			App->renderer3D->active_camera->frustum.up = new_up;
			App->renderer3D->active_camera->frustum.front = q.Mul(App->renderer3D->active_camera->frustum.front).Normalized();
		}
	}
}

// -----------------------------------------------------------------
void ModuleEditorCamera::Zoom(float zoom)
{
	if (looking == true)
	{
		float dist = looking_at.Distance(App->renderer3D->active_camera->frustum.pos);

		// Slower on closer distances
		if (dist < 15.0f)
			zoom *= 0.5f;
		if (dist < 7.5f)
			zoom *= 0.25f;
		if (dist < 1.0f && zoom > 0)
			zoom = 0;
	}

	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) 
		zoom *= 5.0f;

	float3 p = App->renderer3D->active_camera->frustum.front * zoom;
	App->renderer3D->active_camera->frustum.pos += p;
}

// -----------------------------------------------------------------
GameObject* ModuleEditorCamera::Pick(float3* hit_point) const
{
	// The point (1, 1) corresponds to the top-right corner of the near plane
	// (-1, -1) is bottom-left

	float width = (float) App->window->GetWidth();
	float height = (float) App->window->GetHeight();

	int mouse_x, mouse_y;
	App->input->GetMousePosition(mouse_x, mouse_y);

	float normalized_x = -(1.0f - (float(mouse_x) * 2.0f ) / width);
	float normalized_y = 1.0f - (float(mouse_y) * 2.0f ) / height;

	LineSegment picking = App->renderer3D->active_camera->frustum.UnProjectLineSegment(normalized_x, normalized_y);

	float distance;
	GameObject* hit = App->level->CastRay(picking, distance);

	if (hit != nullptr && hit_point != nullptr)
		*hit_point = picking.GetPoint(distance);

	return hit;
}
