    Scene scene;
    Transformation sponzaTransf(
            vec3(0.0f), // translation
            quat(radians(90.0f), vec3(0.0f, 1.0f, 0.0f)), // rotation
            vec3(0.01f)); // scaling
    Animation* sponzaAnim = scene.take(new AnimationConstant(sponzaTransf));
    if (!importIntoScene(scene, "crytek-sponza/sponza.obj", sponzaAnim)) {
        return 1;
    }
    // Add an environment map light source
    Texture* map = scene.take(new TextureConstant(vec3(6.0f)));
    scene.take(new EnvMapEquiRect(map));

    // center view
    Transformation camTransf(vec3(0.0f, 1.7f, 0.0f), vec3(0.0f, 1.7f, -1.0f));
    Animation* camAnim = scene.take(new AnimationConstant(camTransf));
    Camera camera(radians(70.0f), float(width) / height, camAnim);

#if 0
    // plant closeup
    Transformation camTransf(vec3(-0.93f, 1.7f, -1.82f),
            quat(radians(34.79f), vec3(-0.0877701f, -0.995764f, -0.0273793f)));
    Animation* camAnim = scene.take(new AnimationConstant(camTransf));
    Camera camera(radians(30.0f), float(width) / height, camAnim);
#endif
