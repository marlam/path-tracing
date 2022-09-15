#pragma once

#include <cstdio>
#include <map>

#include "scene.hpp"
#include "material_light.hpp"
#include "material_lambertian.hpp"
#include "material_phong.hpp"
#include "mesh.hpp"

/* Helper definitions to portably deal with texture file names */
#ifdef _WIN32
# define IMPORT_DIR_SEP "\\"
# define IMPORT_NOT_DIR_SEP "/"
#else
# define IMPORT_DIR_SEP "/"
# define IMPORT_NOT_DIR_SEP "\\"
#endif

#include "tiny_obj_loader.h"

/* Helper function to get the base directory (for loading textures) */
inline std::string baseDir(const std::string& fileName)
{
    std::string basedir;
    size_t dirSepFound = fileName.find_last_of(IMPORT_DIR_SEP IMPORT_NOT_DIR_SEP);
    if (dirSepFound != std::string::npos)
        basedir = fileName.substr(0, dirSepFound);
    else
        basedir = ".";
    return basedir;
}

/* Helper function to fix texture file names depending on the OS */
inline std::string fixFileName(const std::string& fileName)
{
    std::string fixedName = fileName;
    for (size_t i = 0; i < fixedName.size(); i++) {
        if (fixedName[i] == IMPORT_NOT_DIR_SEP[0])
            fixedName[i] = IMPORT_DIR_SEP[0];
    }
    return fixedName;
}

/* Helper function to import a texture image */
inline Texture* importTexture(const std::string& basedir, const std::string& fileName, std::map<std::string, Texture*>& textureMap)
{
    auto it = textureMap.find(fileName);
    if (it != textureMap.end()) {
        fprintf(stderr, "    texture %s: found in cache\n", fileName.c_str());
        return it->second;
    } else {
        fprintf(stderr, "    texture %s: creating\n", fileName.c_str());
        std::string realFileName = basedir + IMPORT_DIR_SEP + fixFileName(fileName);
        Texture* tex = new TextureImage(realFileName);
        textureMap.insert(std::pair<std::string, Texture*>(fileName, tex));
        return tex;
    }
}

/* Helper function to split a multiline TinyObjLoader message */
inline std::vector<std::string> tinyObjMsgToLines(const std::string& s)
{
    std::vector<std::string> lines;
    size_t i = 0;
    for (;;) {
        size_t j = s.find_first_of('\n', i);
        if (j < std::string::npos) {
            lines.push_back(s.substr(i, j - i));
            i = j + 1;
        } else {
            break;
        }
    }
    return lines;
}

/* Import a triangle-based scene from an OBJ file. */
inline bool importIntoScene(Scene& scene, const std::string& fileName, const Animation* anim = nullptr)
{
    fprintf(stderr, "%s: importing...\n", fileName.c_str());
    tinyobj::ObjReaderConfig conf;
    conf.triangulate = true;
    conf.vertex_color = false;
    tinyobj::ObjReader reader;
    reader.ParseFromFile(fileName, conf);
    if (reader.Warning().size() > 0) {
        std::vector<std::string> lines = tinyObjMsgToLines(reader.Warning());
        for (size_t i = 0; i < lines.size(); i++)
            fprintf(stderr, "  warning: %s\n", lines[i].c_str());
    }
    if (!reader.Valid()) {
        if (reader.Error().size() > 0) {
            std::vector<std::string> lines = tinyObjMsgToLines(reader.Error());
            for (size_t i = 0; i < lines.size(); i++)
                fprintf(stderr, "  error: %s\n", lines[i].c_str());
        } else {
            fprintf(stderr, "  unknown error\n");
        }
        fprintf(stderr, "%s: import failure\n", fileName.c_str());
        return false;
    }
    const std::vector<tinyobj::material_t>& objMaterials = reader.GetMaterials();
    std::string basedir = baseDir(fileName);

    /* Create the materials */

    std::vector<Material*> materials;
    std::vector<bool> materialIsLight;
    std::map<std::string, Texture*> textureMap;
    for (size_t i = 0; i < objMaterials.size(); i++) {
        // get parameters
        const tinyobj::material_t& M = objMaterials[i];
        fprintf(stderr, "  material '%s'...\n", M.name.c_str());
        vec3 dif = vec3(M.diffuse);
        vec3 spc = vec3(M.specular);
        vec3 emi = vec3(M.emission);
        float shi = M.shininess;
        // check which material model to use
        if (dot(emi, emi) > 0.0f) {
            // this is a light source
            fprintf(stderr, "    using Light material model\n");
            materials.push_back(new MaterialLight(emi));
            materialIsLight.push_back(true);
        } else if (dot(spc, spc) <= 0.0f && M.specular_texname.size() == 0) {
            // no specular parts; assume MaterialLambertian
            fprintf(stderr, "    using Lambertian material model\n");
            Texture* tex;
            if (M.diffuse_texname.size() > 0) {
                tex = importTexture(basedir, M.diffuse_texname, textureMap);
            } else {
                tex = new TextureConstant(dif);
            }
            materials.push_back(new MaterialLambertian(tex));
            materialIsLight.push_back(false);
        } else {
            // MaterialPhong
            fprintf(stderr, "    using Phong material model\n");
            Texture* texKd;
            Texture* texKs;
            Texture* texS;
            if (M.diffuse_texname.size() > 0) {
                texKd = importTexture(basedir, M.diffuse_texname, textureMap);
            } else {
                texKd = new TextureConstant(dif);
            }
            if (M.specular_texname.size() > 0) {
                texKs = importTexture(basedir, M.specular_texname, textureMap);
            } else {
                texKs = new TextureConstant(spc);
            }
            if (M.specular_highlight_texname.size() > 0) {
                texS = importTexture(basedir, M.specular_highlight_texname, textureMap);
            } else {
                texS = new TextureConstant(vec3(shi));
            }
            materials.push_back(new MaterialPhong(texKd, texKs, texS));
            materialIsLight.push_back(false);
        }
    }
    for (auto it = textureMap.cbegin(); it != textureMap.cend(); it++)
        scene.take(it->second);
    for (size_t i = 0; i < materials.size(); i++)
        scene.take(materials[i]);

    /* Import the shapes */

    // Collect all geometry with the same material into one single Mesh.
    // So we loop over the materials, and for each material over all geometry.
    const tinyobj::attrib_t& attrib = reader.GetAttrib();
    const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
    fprintf(stderr, "  importing %zu shapes\n", shapes.size());
    Texture* nullTexture = scene.take(new TextureConstant(vec3(0.5f)));
    Material* nullMaterial = scene.take(new MaterialLambertian(nullTexture));
    for (int matId = -1; matId < static_cast<int>(materials.size()); matId++) {
        if (matId < 0) {
            fprintf(stderr, "  importing all geometry that uses no material\n");
        } else {
            fprintf(stderr, "  importing all geometry that uses material '%s'\n", objMaterials[matId].name.c_str());
        }
        for (size_t s = 0; s < shapes.size(); s++) {
            std::map<std::tuple<int, int, int>, unsigned int> indexTupleMap;
            std::vector<vec3> positions;
            std::vector<vec3> normals;
            std::vector<vec2> texcoords;
            std::vector<unsigned int> indices;
            bool haveNormals = true;
            bool haveTexCoords = true;
            const tinyobj::mesh_t& mesh = shapes[s].mesh;
            for (size_t i = 0; i < mesh.indices.size(); i++) {
                size_t triangleIndex = i / 3;
                if (mesh.material_ids[triangleIndex] == matId) {
                    const tinyobj::index_t& index = mesh.indices[i];
                    int vi = index.vertex_index;
                    int ni = index.normal_index;
                    int ti = index.texcoord_index;
                    std::tuple<int, int, int> indexTuple = std::make_tuple(vi, ni, ti);
                    auto it = indexTupleMap.find(indexTuple);
                    if (it == indexTupleMap.end()) {
                        unsigned int newIndex = indexTupleMap.size();
                        positions.push_back(vec3(attrib.vertices.data() + 3 * vi));
                        if (ni < 0)
                            haveNormals = false;
                        if (ti < 0)
                            haveTexCoords = false;
                        if (haveNormals) {
                            vec3 n = vec3(attrib.normals.data() + 3 * ni);
                            // do not trust normals to be of unit length: normalize!
                            normals.push_back(normalize(n));
                        }
                        if (haveTexCoords) {
                            texcoords.push_back(vec2(attrib.texcoords.data() + 2 * ti));
                        }
                        indices.push_back(newIndex);
                        indexTupleMap.insert(std::make_pair(indexTuple, newIndex));
                    } else {
                        indices.push_back(it->second);
                    }
                }
            }
            if (indices.size() == 0)
                continue;
            if (!haveNormals)
                normals.clear();
            if (!haveTexCoords)
                texcoords.clear();
            Material* mat = (matId < 0 ? nullMaterial : materials[matId]);
            bool isLight = (matId >= 0 && materialIsLight[matId]);
            scene.take(new Mesh(positions, normals, texcoords, indices, mat, anim), isLight);
        }
    }

    fprintf(stderr, "%s: import done\n", fileName.c_str());
    return true;
}
