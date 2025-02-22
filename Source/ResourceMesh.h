#ifndef __RESOURCE_MESH_H__
#define __RESOURCE_MESH_H__

#include "Resource.h"
#include "Math.h"
#include "HashString.h"
#include "utils/SimpleBinStream.h"

#include <string>
#include <memory>

class Resource;
struct aiMesh;
struct par_shapes_mesh_s;

namespace Thekla { struct Atlas_Output_Mesh; }

enum VertexAttribs
{
    ATTRIB_TEX_COORDS_0 = 0, //1 << 0,
    ATTRIB_NORMALS      = 1, //1 << 1,
    ATTRIB_TANGENTS     = 2, //1 << 2,
    ATTRIB_BONES        = 3, //1 << 3,
    ATTRIB_TEX_COORDS_1 = 4, //1 << 4,
    ATTRIB_COUNT
};

class ResourceMesh : public Resource
{
	friend class ModuleMeshes;
public:
    
    struct MorphData
    {
        std::unique_ptr<float3[]> src_vertices;
        std::unique_ptr<float3[]> src_normals;
        std::unique_ptr<float3[]> src_tangents;
        std::unique_ptr<uint[]>   src_indices;
        uint                      num_indices;
    };

public:

	ResourceMesh(UID id);
	virtual ~ResourceMesh();


	void                  Save                (Config& config) const override;
	void                  Load                (const Config& config) override;

    void                  UpdateUniforms      (const float4x4* skin_palette, const float* morph_weights) const;
    void                  Draw                () const;

	bool                  LoadInMemory        () override;
    void                  ReleaseFromMemory   () override;
    bool                  Save                ();
    bool                  Save                (std::string& output) const;
    static UID            Import              (const aiMesh* mesh, const char* source_file);

    static UID            LoadSphere          (const char* sphere_name, float size, unsigned slices, unsigned stacks);
    static UID            LoadTorus           (const char* torus_name, float inner_r, float outer_r, unsigned slices, unsigned stacks);
    static UID            LoadCube            (const char* cube_name, float size);
    static UID            LoadCylinder        (const char* cylinder_name, float height, float radius, unsigned slices, unsigned stacks);
    static UID            LoadPlane           (const char* plane_name, float width, float height, unsigned slices, unsigned stacks); 

    void                  GenerateTexCoord1   ();


    uint                  GetNumVertices      () const { return num_vertices; }
    uint                  GetNumIndices       () const { return num_indices; }

    // morph targets
    uint                  GetNumMorphTargets  () const { return num_morph_targets; }
    const MorphData&      GetMorphTarget      (uint index) const { return morph_targets[index]; }

    // attribs

    bool                  HasAttrib           (VertexAttribs attrib) const { return (attrib_flags & (1 << uint(attrib))) != 0 ? true : false; }
    uint                  GetOffset           (VertexAttribs attrib) const { return offsets[attrib]; }
    uint                  GetAttribs          () const { return attrib_flags; }

    uint                  GetVBO              () const {return vbo; }
    uint                  GetVAO              () const {return vao; }
    uint                  GetIBO              () const {return ibo; }

    const float3*         GetVertices         () const {return src_vertices.get(); }
    const float2*         GetTexCoord0        () const {return src_texcoord0.get(); }
    const float2*         GetTexCoord1        () const {return src_texcoord1.get(); }
    const float3*         GetNormals          () const {return src_normals.get(); }
    const float3*         GetTangents         () const {return src_tangents.get(); }

    const HashString&     GetName             () const { return name; }

    static Resource::Type GetClassType      () {return Resource::mesh;}

private:

    uint                GetBoneWeightOffset () const { return sizeof(uint)*4*num_vertices; }
    static UID          Generate            (const char* shape_name, par_shapes_mesh_s* shape);
    void                GenerateAttribInfo  ();
    void                GenerateCPUBuffers  (const Thekla::Atlas_Output_Mesh* atlas);
    void                GenerateCPUBuffers  (const aiMesh* mesh);
	void                GenerateCPUBuffers  (par_shapes_mesh_s* shape);
    void                GenerateVBO         ();
    void                GenerateBoneData    (const aiMesh* mesh);
    void                GenerateVAO         ();
    void                GenerateTangentSpace();
    void                Clear               ();
    bool                Save                (const char* source, std::string& output);
	void                SaveToStream        (simple::mem_ostream<std::true_type>& write_stream) const;
    uint                GetMorphNumAttribs  () const;

public:

	struct Bone
	{
		HashString	name;
		float4x4	bind = float4x4::identity;
	};

    HashString                   name;

    uint                         vertex_size         = 0;
    uint                         attrib_flags        = 0;
    uint                         offsets[ATTRIB_COUNT] = { 0, 0, 0, 0, 0 };

    uint                         num_vertices = 0;

    std::unique_ptr<float3[]>    src_vertices;
    std::unique_ptr<float2[]>    src_texcoord0;
    std::unique_ptr<float2[]>    src_texcoord1;
    std::unique_ptr<float3[]>    src_normals;
    std::unique_ptr<float3[]>    src_tangents;

    std::unique_ptr<unsigned[]>  src_bone_indices;
    std::unique_ptr<float4[]>    src_bone_weights;

    uint                         num_indices = 0;
    std::unique_ptr<uint[]>      src_indices;

	uint                         num_bones	 = 0;
    std::unique_ptr<Bone[]>      bones;
    bool                         static_mesh = true;

    uint                         vao 	= 0;
    uint                         vbo 	= 0;
    uint                         ibo 	= 0;
    uint                         tbo    = 0; // texture buffer object for morphing
    uint                         morph_texture = 0;

    std::unique_ptr<MorphData[]> morph_targets;
    uint                         num_morph_targets = 0;
    uint                         morph_vertex_size = 0;

	AABB                         bbox;
};

#endif // __RESOURCE_MESH_H__
