
    // a basic quad
    std::vector<vec3> quadPos;
    quadPos.push_back(vec3(-1.0f, -1.0f, 0.0f));
    quadPos.push_back(vec3(+1.0f, -1.0f, 0.0f));
    quadPos.push_back(vec3(-1.0f, +1.0f, 0.0f));
    quadPos.push_back(vec3(+1.0f, +1.0f, 0.0f));
    std::vector<vec3> quadNrm;
    quadNrm.push_back(vec3(0.0f, 0.0f, +1.0f));
    quadNrm.push_back(vec3(0.0f, 0.0f, +1.0f));
    quadNrm.push_back(vec3(0.0f, 0.0f, +1.0f));
    quadNrm.push_back(vec3(0.0f, 0.0f, +1.0f));
    std::vector<vec2> quadTc;
    quadTc.push_back(vec2(0.0f, 0.0f));
    quadTc.push_back(vec2(1.0f, 0.0f));
    quadTc.push_back(vec2(0.0f, 1.0f));
    quadTc.push_back(vec2(1.0f, 1.0f));
    std::vector<unsigned int> quadInd;
    quadInd.push_back(0);
    quadInd.push_back(1);
    quadInd.push_back(2);
    quadInd.push_back(1);
    quadInd.push_back(3);
    quadInd.push_back(2);
    // the floor
    Texture* floorTexture = scene.take(new TextureChecker(
                scene.take(new TextureConstant(vec3(0.6f))),
                scene.take(new TextureConstant(vec3(0.4f))), 200, 200));
    Material* floorMaterial = scene.take(new MaterialLambertian(floorTexture));
    Transformation floorTransformation(vec3(0.0f), quat(radians(-90.0f), vec3(1.0f, 0.0f, 0.0f)), vec3(20.0f));
    Animation* floorAnimation = scene.take(new AnimationConstant(floorTransformation));
    scene.take(new Mesh(quadPos, quadNrm, quadTc, quadInd, floorMaterial, floorAnimation));
    // the first object
    Texture* firstObjectTexture = scene.take(new TextureConstant(vec3(1.0f, 1.0f, 1.0f)));
    Material* firstObjectMaterial = scene.take(new MaterialMirror(firstObjectTexture));
    scene.take(new SurfaceSphere(vec3(-1.5f, 1.0f, 0.0f), 1.0f, firstObjectMaterial));
    // the second object
    Texture* secondObjectTextureKd = scene.take(new TextureConstant(vec3(0.4f, 0.1f, 0.1f)));
    Texture* secondObjectTextureKs = scene.take(new TextureConstant(vec3(0.6f, 0.6f, 0.6f)));
    Texture* secondObjectTextureS = scene.take(new TextureConstant(vec3(500.0f)));
    Material* secondObjectMaterial = scene.take(new MaterialPhong(secondObjectTextureKd, secondObjectTextureKs, secondObjectTextureS));
    scene.take(new SurfaceSphere(vec3(+1.5f, 1.0f, 0.0f), 1.0f, secondObjectMaterial));
    // the environment map
# if 1
    Texture* map = scene.take(new TextureImage("kloofendal_48d_partly_cloudy_small.jpg"));
    scene.take(new EnvMapEquiRect(map));
# else
    Texture* mapPosX = scene.take(new TextureImage("kloofendal_48d_partly_cloudy_small_posx.jpg"));
    Texture* mapNegX = scene.take(new TextureImage("kloofendal_48d_partly_cloudy_small_negx.jpg"));
    Texture* mapPosY = scene.take(new TextureImage("kloofendal_48d_partly_cloudy_small_posy.jpg"));
    Texture* mapNegY = scene.take(new TextureImage("kloofendal_48d_partly_cloudy_small_negy.jpg"));
    Texture* mapPosZ = scene.take(new TextureImage("kloofendal_48d_partly_cloudy_small_posz.jpg"));
    Texture* mapNegZ = scene.take(new TextureImage("kloofendal_48d_partly_cloudy_small_negz.jpg"));
    scene.take(new EnvMapCube(mapPosX, mapNegX, mapPosY, mapNegY, mapPosZ, mapNegZ));
# endif

    // the camera
    Transformation camTrans(vec3(0.0f, 5.0f, 5.0f), vec3(0.0f, 1.0f, 0.0f));
    Animation* camAnim = scene.take(new AnimationConstant(camTrans));
    Camera camera(radians(35.0f), float(width) / height, camAnim);

