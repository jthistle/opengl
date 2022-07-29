add_library(ProjectLibs 
    glad.c stb_init.cpp global.h
    shader.cpp texture.cpp camera.cpp light.cpp pointLight.cpp directionalLight.cpp
    mesh.cpp model.cpp renderer.cpp gameObject.cpp cube.cpp bloomManager.cpp bloomRenderer.cpp
)
target_link_libraries(ProjectLibs -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lassimp)

add_executable(LearnOpenGL main.cpp)
target_link_libraries(LearnOpenGL ProjectLibs)
