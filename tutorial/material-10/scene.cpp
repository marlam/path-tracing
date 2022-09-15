    // The scene and camera
    Scene scene;
    Prng scenePrng(1234);

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
                scene.take(new TextureConstant(vec3(0.4f))), 40, 40));
    Material* floorMaterial = scene.take(new MaterialLambertian(floorTexture));
    Transformation floorTransformation(vec3(0.0f), quat(radians(-90.0f), vec3(1.0f, 0.0f, 0.0f)), vec3(20.0f));
    Animation* floorAnimation = scene.take(new AnimationConstant(floorTransformation));
    scene.take(new Mesh(quadPos, quadNrm, quadTc, quadInd, floorMaterial, floorAnimation));
    // the objects
    for (int i = 0; i <= 21; i++) {
        for (int j = 0; j <= 23; j++) {
            Texture* kd = scene.take(new TextureConstant(vec3(
                            scenePrng.in01() * scenePrng.in01(),
                            scenePrng.in01() * scenePrng.in01(),
                            scenePrng.in01() * scenePrng.in01())));
            Material* mat = scene.take(new MaterialLambertian(kd));
            scene.take(new SurfaceSphere(vec3(i - 10.0f, 0.4f, j - 17.0f), 0.4f, mat));
        }
    }
    // the environment map
    Texture* map = scene.take(new TextureConstant(vec3(1.0f)));
    scene.take(new EnvMapEquiRect(map));
    // the camera
    Transformation camTrans(vec3(0.0f, 10.0f, 10.0f), vec3(0.0f, 0.4f, 0.0f));
    Animation* camAnim = scene.take(new AnimationConstant(camTrans));
    float focusDistance = 17.0f;
    float apertureDiameter = 0.8f;
    Camera camera(radians(50.0f), float(width) / height, focusDistance, apertureDiameter, camAnim);
