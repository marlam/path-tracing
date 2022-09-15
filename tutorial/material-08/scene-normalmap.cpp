    Scene scene;

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

    Texture* texKd = scene.take(new TextureImage("cglogo-texture.png"));
    Texture* texKs = scene.take(new TextureConstant(vec3(0.5f)));
    Texture* texS = scene.take(new TextureConstant(vec3(120.0f)));
    Texture* texNormal = scene.take(new TextureImage("cglogo-normalmap1.png", false));
    Material* boxMat = scene.take(new MaterialPhong(texKd, texKs, texS, nullptr, texNormal));
    Transformation boxT;
    boxT.translate(vec3(-1.1f, 0.2f, 1.0f));
    boxT.rotate(quat(radians(50.0f), normalize(vec3(1.0f, 1.0f, 0.0f))));
    boxT.scale(vec3(0.7f));
    Transformation boxFrontT;
    boxFrontT.translate(vec3(0.0f, 0.0f, 1.0f));
    Animation* boxFrontAnim = scene.take(new AnimationConstant(boxT * boxFrontT));
    scene.take(new Mesh(quadPos, quadNrm, quadTc, quadInd, boxMat, boxFrontAnim));
    Transformation boxLeftT;
    boxLeftT.translate(vec3(-1.0f, 0.0f, 0.0f));
    boxLeftT.rotate(quat(radians(-90.0f), normalize(vec3(0.0f, 1.0f, 0.0f))));
    Animation* boxLeftAnim = scene.take(new AnimationConstant(boxT * boxLeftT));
    scene.take(new Mesh(quadPos, quadNrm, quadTc, quadInd, boxMat, boxLeftAnim));
    Transformation boxTopT;
    boxTopT.translate(vec3(0.0f, 1.0f, 0.0f));
    boxTopT.rotate(quat(radians(-90.0f), normalize(vec3(1.0f, 0.0f, 0.0f))));
    Animation* boxTopAnim = scene.take(new AnimationConstant(boxT * boxTopT));
    scene.take(new Mesh(quadPos, quadNrm, quadTc, quadInd, boxMat, boxTopAnim));

    Texture* sphereTexKd = scene.take(new TextureTransformer(texKd, vec2(6.0f, 3.0f), vec2(0.0f)));
    Texture* sphereTexNormal = scene.take(new TextureTransformer(texNormal, vec2(6.0f, 3.0f), vec2(0.0f)));
    Material* sphereMat = scene.take(new MaterialPhong(sphereTexKd, texKs, texS, nullptr, sphereTexNormal));
    Transformation sphereT(vec3(+1.1f, 0.0f, 0.0f), quat(radians(90.0f), vec3(0.0f, 1.0f, 0.0f)));
    Animation* sphereAnim = scene.take(new AnimationConstant(sphereT));
    scene.take(new SurfaceSphere(vec3(0.0f), 1.0f, sphereMat, sphereAnim));

    Material* lightMat = scene.take(new MaterialLight(vec3(3.0f)));
    Transformation lightT;
    lightT.translate(vec3(-2.0f, 3.0f, 7.0f));
    lightT.scale(vec3(2.0f));
    Animation* lightAnim = scene.take(new AnimationConstant(lightT));
    scene.take(new SurfaceSphere(vec3(0.0f), 1.0f, lightMat, lightAnim), true);

    Transformation camT = Transformation(vec3(-0.2f, 0.0f, 5.0f));
    Animation* camAnim = scene.take(new AnimationConstant(camT));
    Camera camera(radians(42.0f), float(width) / height, camAnim);

