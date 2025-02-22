#ifndef _SCENE_VIEWPORT_H_
#define _SCENE_VIEWPORT_H_

#include "Math.h"
#include "ImGuizmo.h"
#include "OGL.h"

class ComponentCamera;
class GameObject;
class PointLight;
class SpotLight;
class Config;

class SceneViewport
{
public:
    SceneViewport();
    ~SceneViewport();

    void                Draw                (ComponentCamera* camera, ComponentCamera* culling);

	void                Save                (Config* config) const;
	void                Load                (Config* config);


    unsigned            GetWidth            () const { return fb_width; }
    unsigned            GetHeight           () const { return fb_height; }
    unsigned            GetXPos             () const { return x_pos; }
    unsigned            GetYPos             () const { return y_pos; }

    bool                IsFocused           () const { return focused; }
	bool                IsUsingGuizmo       () const { return ImGuizmo::IsUsing(); }

    void                DrawGuizmoProperties(GameObject* go);
    void                DrawGuizmoProperties(PointLight* point);
    void                DrawGuizmoProperties(SpotLight* spot);


private:

    struct FramebufferInfo;

    void    GenerateFBOs    (unsigned w, unsigned h);
    void    GenerateFBO     (FramebufferInfo& buffer, unsigned w, unsigned h, bool depth, bool msaa, bool hdr);
    void    DrawQuickBar    (ComponentCamera* camera);
    void    DrawGuizmo      (ComponentCamera* camera);
    void    DrawGuizmo      (ComponentCamera* camera, GameObject* go);
    void    DrawGuizmo      (ComponentCamera* camera, PointLight* point);
    void    DrawGuizmo      (ComponentCamera* camera, SpotLight* spot);
    float   DistanceFromAtt (float constant, float linear, float quadric, float epsilon);
    void    PickSelection   (ComponentCamera* camera, int mouse_x, int mouse_y);
    void    DrawSelection   (ComponentCamera* camera, Framebuffer* framebuffer);
    void    ShowTexture     ();
    void    DrawGrid        (ComponentCamera* camera);

private:

    struct FramebufferInfo
    {
        std::unique_ptr<Framebuffer> framebuffer;
        std::unique_ptr<Texture2D>   texture_color;
        std::unique_ptr<Texture2D>   texture_depth;
    };

    enum FrameBuffers
    {
        FRAMEBUFFER_NO_MSAA = 0,
        FRAMEBUFFER_MSAA,
        FRAMEBUFFER_POSTPROCESS,
        FRAME_BUFFER_COUNT
    };

    FramebufferInfo framebuffers[FRAME_BUFFER_COUNT];
    FramebufferInfo selection_buffer;

    unsigned    fb_width    = 0;
    unsigned    fb_height   = 0;

    unsigned    x_pos       = 0;
    unsigned    y_pos       = 0;
    bool        active      = true;
    bool        focused     = false;
	bool        draw_plane  = true;
	bool        draw_axis   = true;
	bool        debug_draw  = true;

    ImGuizmo::OPERATION          guizmo_op      = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE               guizmo_mode    = ImGuizmo::LOCAL;
    bool                         guizmo_useSnap = false;
    float3                       guizmo_snap    = float3(1.0f);

    std::unique_ptr<Buffer>      grid_vbo;
    std::unique_ptr<Buffer>      grid_ibo;
    std::unique_ptr<VertexArray> grid_vao;

};

#endif /* _VIEWPORT_H_ */

