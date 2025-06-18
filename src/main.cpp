#include <cmath>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <ImGuiFileDialog.h>

#include "Camera/Camera.h"
#include "Render/ShaderProgram.h"
#include "Model/Model.h"
#include "Animator/Animation.h"
#include "Animator/Animator.h"
#include "Animator/AnimationMixer.h"
#include "Utils/ImageLoader/stb_image.h"
#include "Light/PointLight.h"
#include "Light/DirectionalLight.h"

int WINDOW_WIDTH = 1200;
int WINDOW_HEIGHT = 800;

auto *camera = new Camera(45.f, (float) WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f, glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = WINDOW_WIDTH / 2.0f;
float lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void processInput(GLFWwindow *window, float deltaTime);

void loadNewModel(const std::string &path, Model *&model, Animator *&animator,
                  AnimationMixer *&mixer);

void setParametersToDefault();

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    camera->recalculateProjectionMatrix(45.f, (float) width / height, 0.1f, 100.0f);
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

void renderLights(Model *lightSourceModel, ShaderProgram sp);

bool fullScreen = false;
float speed = 1.0f;
float scale = 1.f;
float angleY = 0.f;
bool loop = true;
bool play = true;

std::map<std::string, Animation *> animations = map<std::string, Animation *>();
std::vector<std::string> animationNames = std::vector<std::string>();
int selectedAnimation = -1;
int selectedShader = 0;
std::vector<std::string> shaderNames = std::vector<std::string>();
bool mixing = false;
glm::vec3 bgColor = glm::vec3(0.12f, 0.12f, 0.12f);
int selectedMixer1 = -1;
int selectedMixer2 = -1;
float weight = 0.f;
std::map<std::string, ShaderProgram *> shaders = map<std::string, ShaderProgram *>();
int boneIndex = 1;
int selectedLight = 0;
bool drawBones = false;
bool drawBonesInfluence = false;
std::vector<PointLight> lights = std::vector<PointLight>();
bool lmb = false;
bool rmb = false;
Model *model = nullptr;
const char* lightNums[] = {"1", "2", "3", "4"};

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWmonitor *monitor = fullScreen ? glfwGetPrimaryMonitor() : NULL;
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Animation", monitor, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    const char* vendor = (const char*)glGetString(GL_VENDOR);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    std::cout << "Vendor: " << vendor << "\nRenderer: " << renderer << std::endl;

    shaderNames.push_back("Diffuse shader");
    shaderNames.push_back("Normal shader");
    shaderNames.push_back("PBR shader");
    shaderNames.push_back("Toon shader");

    auto *shaderProgram = new ShaderProgram();

    shaderProgram->loadFragmentShader("../shaders/diffuseMapping/fragmentShader.glsl");
    shaderProgram->loadVertexShader("../shaders/diffuseMapping/vertexShader.glsl");
    shaderProgram->link();
    shaders.insert(std::make_pair(shaderNames[0], shaderProgram));
    shaderProgram = new ShaderProgram();
    shaderProgram->loadFragmentShader("../shaders/normalMapping/fragmentShader.glsl");
    shaderProgram->loadVertexShader("../shaders/normalMapping/vertexShader.glsl");
    shaderProgram->link();
    shaders.insert(std::make_pair(shaderNames[1], shaderProgram));
    shaderProgram = new ShaderProgram();
    shaderProgram->loadFragmentShader("../shaders/pbr/fragmentShader.glsl");
    shaderProgram->loadVertexShader("../shaders/pbr/vertexShader.glsl");
    shaderProgram->link();
    shaders.insert(std::make_pair(shaderNames[2], shaderProgram));
    shaderProgram = new ShaderProgram();
    shaderProgram->loadFragmentShader("../shaders/toon/fragmentShader.glsl");
    shaderProgram->loadVertexShader("../shaders/toon/vertexShader.glsl");
    shaderProgram->link();
    shaders.insert(std::make_pair(shaderNames[3], shaderProgram));

    float lastFrame = glfwGetTime();
    float deltaTime = 0.f;


    Animator *animator = nullptr;
    AnimationMixer *mixer = nullptr;

    Model bone = Model("../models/bone\\scene.glb");
    ShaderProgram boneShaderProgram = ShaderProgram();
    boneShaderProgram.loadFragmentShader("../shaders/circle/fragmentShader.glsl");
    boneShaderProgram.loadVertexShader("../shaders/circle/vertexShader.glsl");
    boneShaderProgram.link();

    ShaderProgram boneInfluenceSP = ShaderProgram();
    boneInfluenceSP.loadFragmentShader("../shaders/bonesInfluence/fragmentShader.glsl");
    boneInfluenceSP.loadVertexShader("../shaders/bonesInfluence/vertexShader.glsl");
    boneInfluenceSP.link();

    ShaderProgram lightShaderProgram = ShaderProgram();
    lightShaderProgram.loadFragmentShader("../shaders/light/fragmentShader.glsl");
    lightShaderProgram.loadVertexShader("../shaders/light/vertexShader.glsl");
    lightShaderProgram.link();

    lights.push_back({glm::vec3(5.f, 5.f, 5.f), glm::vec3(1.f, 1.f, 1.f)});
    lights.push_back({glm::vec3(-5.f, 5.f, 5.f), glm::vec3(1.f, 1.f, 1.f)});
    lights.push_back({glm::vec3(-5.f, 5.f, -5.f), glm::vec3(1.f, 1.f, 1.f)});
    lights.push_back({glm::vec3(5.f, 5.f, -5.f), glm::vec3(1.f, 1.f, 1.f)});

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window, deltaTime);

        if (animator && !mixing) {
            animator->UpdateAnimation(play ? deltaTime : 0, speed, loop);
        }
        if (mixer && mixing) {
            mixer->updateAnimations(play ? deltaTime : 0, speed);
        }

        glfwSetWindowTitle(window, std::string("fps: " + std::to_string(1 / deltaTime)).c_str());
        glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Animation Controls", nullptr, ImGuiWindowFlags_NoMove |
                                                    ImGuiWindowFlags_AlwaysAutoResize
        );
        if (ImGui::BeginTabBar("")) {
            if (ImGui::BeginTabItem("Preview")) {
                mixing = false;
                if (ImGui::Button("Load Model")) {
                    ImGuiFileDialog::Instance()->OpenDialog(
                            "ChooseModelDlg",
                            "Choose Model",
                            ".gltf,.fbx,.dae,.glb");
                }
                if (selectedAnimation != -1) {
                    std::vector<const char *> names;
                    for (const auto &name: animationNames) {
                        names.push_back(name.c_str());
                    }
                    if (ImGui::Combo("Animation", &selectedAnimation, names.data(), names.size())) {
                        animator->PlayAnimation(animations.at(animationNames[selectedAnimation]));
                    }
                }
                if (ImGui::Checkbox("Loop animation", &loop) && animator) {
                    animator->PlayAnimation(animations.at(animationNames[selectedAnimation]));
                }
                ImGui::EndTabItem();
            }
            ImGui::BeginDisabled(animations.size() < 2);
            if (ImGui::BeginTabItem("Mixer")) {
                mixing = true;
                std::vector<const char *> names;
                for (const auto &name: animationNames) {
                    names.push_back(name.c_str());
                }
                if (ImGui::Combo("1st animation", &selectedMixer1, names.data(), names.size())) {
                    mixer->setAnimations(animations.at(animationNames[selectedMixer1]),
                                         animations.at(animationNames[selectedMixer2]));
                }
                if (ImGui::Combo("2nd animation", &selectedMixer2, names.data(), names.size())) {
                    mixer->setAnimations(animations.at(animationNames[selectedMixer1]),
                                         animations.at(animationNames[selectedMixer2]));
                }
                ImGui::SliderFloat("Weight", &weight, 0.f, 1.f);
                ImGui::EndTabItem();
            }
            ImGui::EndDisabled();
            if (ImGui::BeginTabItem("Lights")) {
                std::vector<const char *> nums = std::vector<const char *>();
                for (int i = 0; i < lights.size(); i++) {
                    nums.push_back(lightNums[i]);
                }
                ImGui::Combo("Light source", &selectedLight, nums.data(), nums.size());
                ImGui::ColorPicker3("Light color", &lights[selectedLight].color[0]);
                ImGui::SliderFloat3("Position", &lights[selectedLight].position[0], -10.f, 10.f);
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::SliderFloat("Animation speed", &speed, 0.1f, 5.0f);
        ImGui::SliderFloat("Model scale", &scale, 0.01f, 1.f);
        ImGui::ColorPicker3("Background color", &bgColor[0]);
        ImGui::Checkbox("Show bones", &drawBones);
        ImGui::Checkbox("Show bones influence", &drawBonesInfluence);
        if (drawBonesInfluence && model) {
            ImGui::SliderInt("Bone id", &boneIndex, 1, model->GetBoneCount());
        }
        std::vector<const char *> names;
        for (const auto &name: shaderNames) {
            names.push_back(name.c_str());
        }
        ImGui::Combo("Shader", &selectedShader, names.data(), names.size());
        if (ImGui::Button("Reset to defaults")) {
            setParametersToDefault();
        }
        if (selectedAnimation > -1) {
            if (ImGui::Button(play ? "Stop" : "Play")) {
                play = !play;
            }

            ImGui::BeginChild("Timeline", ImVec2(0, 80), true, ImGuiWindowFlags_HorizontalScrollbar);

            float currentTime = animator->getCurrentTime() / 1000.f;
            const float duration = animations.at(animationNames[selectedAnimation])->GetDuration() / 1000.f;
            const float canvasWidth = ImGui::GetContentRegionAvail().x;
            const float timelineScale = canvasWidth / duration;

            ImDrawList *drawList = ImGui::GetWindowDrawList();
            ImVec2 canvasPos = ImGui::GetCursorScreenPos();
            ImVec2 canvasSize = ImGui::GetContentRegionAvail();

            drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                                    IM_COL32(30, 30, 30, 255));

            for (float t = 0; t <= duration; t += duration / 10.f) {
                float x = canvasPos.x + t * timelineScale;
                drawList->AddLine(ImVec2(x, canvasPos.y), ImVec2(x, canvasPos.y + 10),
                                  IM_COL32(100, 100, 100, 255), 1.0f);
                char label[32];
                sprintf(label, "%.1f", t);
                drawList->AddText(ImVec2(x - 5, canvasPos.y + 12), IM_COL32(200, 200, 200, 255), label);
            }


            float current_x = canvasPos.x + currentTime * timelineScale;
            drawList->AddLine(ImVec2(current_x, canvasPos.y), ImVec2(current_x, canvasPos.y + canvasSize.y),
                              IM_COL32(255, 0, 0, 255), 2.0f);

            if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(0)) {
                ImVec2 mousePos = ImGui::GetMousePos();

                currentTime = (mousePos.x - canvasPos.x) / timelineScale;
                currentTime = min(max(currentTime, 0.0f), duration - 0.01f);
                animator->setCurrentTime(currentTime * 1000.f);
            }

            ImGui::EndChild();
        }

        ImGui::End();

        if (ImGuiFileDialog::Instance()->Display("ChooseModelDlg")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                Logger::info("DIALOG", "path:" + filePath);
                try {
                    loadNewModel(filePath.c_str(), model, animator, mixer);
                } catch (const std::exception &e) {

                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        renderLights(&bone, lightShaderProgram);
        if (model && animator) {
            glm::mat4 modelMat = glm::mat4(1.f);
            modelMat = glm::rotate(modelMat, angleY, glm::vec3(0.f, 1.f, 0.f));
            modelMat = glm::scale(modelMat, glm::vec3(scale));
            ShaderProgram *shaderProgram = nullptr;
            if (!drawBonesInfluence) {
                shaderProgram = shaders.at(shaderNames[selectedShader]);
            } else {
                shaderProgram = &boneInfluenceSP;
            }

            shaderProgram->use();
            shaderProgram->setInt("boneId", boneIndex);

            auto transforms = mixing ? mixer->getFinalBoneMatrices(weight) : animator->getFinalBoneMeshMatrices();
            auto boneMatrices = animator->getFinalBoneMatrices();

            unsigned int id = shaderProgram->shaderProgram;
            glUniformMatrix4fv(glGetUniformLocation(id, "model"), 1, GL_FALSE, value_ptr(modelMat));
            glUniformMatrix4fv(glGetUniformLocation(id, "view"), 1, GL_FALSE, value_ptr(camera->getViewMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(id, "proj"), 1, GL_FALSE, value_ptr(camera->projection));
            glUniform3fv(glGetUniformLocation(id, "camera"), 1, value_ptr(camera->position));
            std::vector<glm::vec3> positions = std::vector<glm::vec3>();
            std::vector<glm::vec3> colors = std::vector<glm::vec3>();
            for (auto light: lights) {
                positions.push_back(light.position);
                colors.push_back(light.color);
            }
            glUniform3fv(glGetUniformLocation(id, "lPos"), lights.size(), value_ptr(positions[0]));
            glUniform3fv(glGetUniformLocation(id, "lColor"), lights.size(), value_ptr(colors[0]));
            glUniformMatrix4fv(
                    glGetUniformLocation(id, std::string("finalBonesMatrices").c_str()), transforms.size(),
                    GL_FALSE, &transforms[0][0][0]);

            model->Draw(*shaderProgram);

            ShaderProgram::unbind();

            if (drawBones) {
                boneShaderProgram.use();

                glDisable(GL_DEPTH_TEST);
                for (int i = 0; i < boneMatrices.size(); i++) {
                    unsigned int boneSPId = boneShaderProgram.shaderProgram;
                    glm::mat4 boneModel = glm::mat4(boneMatrices[i]);
                    glUniformMatrix4fv(glGetUniformLocation(boneSPId, "model"), 1, GL_FALSE, value_ptr(modelMat));
                    glUniformMatrix4fv(glGetUniformLocation(boneSPId, "view"), 1, GL_FALSE,
                                       value_ptr(camera->getViewMatrix()));
                    glUniformMatrix4fv(glGetUniformLocation(boneSPId, "proj"), 1, GL_FALSE,
                                       value_ptr(camera->projection));
                    glUniformMatrix4fv(glGetUniformLocation(boneSPId, "transform"), 1, GL_FALSE, value_ptr(boneModel));
                    boneShaderProgram.setInt("selected", boneIndex == i && drawBonesInfluence ? 1 : 0);
                    bone.Draw(boneShaderProgram);
                }
                glEnable(GL_DEPTH_TEST);
                ShaderProgram::unbind();
            }
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete shaderProgram;
    shaderProgram = nullptr;
    if (model) {
        delete model;
        model = nullptr;
    }
    if (animator) {
        delete animator;
        animator = nullptr;
    }

    glfwTerminate();
    return 0;
}

void setParametersToDefault() {
    speed = 1.f;
    scale = 1.f;
    angleY = 0.f;
    loop = true;
    delete camera;
    camera = nullptr;
    camera = new Camera(45.f, (float) WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f, glm::vec3(0.0f, 0.0f, 5.0f));
}

void renderLights(Model *lightSourceModel, ShaderProgram sp) {
    unsigned int spId = sp.shaderProgram;
    sp.use();
    for (auto light: lights) {
        glm::mat4 lightModel = glm::mat4(1.f);
        lightModel = glm::translate(lightModel, light.position);
        lightModel = glm::scale(lightModel, glm::vec3(5.f));
        glUniformMatrix4fv(glGetUniformLocation(spId, "view"), 1, GL_FALSE,
                           value_ptr(camera->getViewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(spId, "proj"), 1, GL_FALSE,
                           value_ptr(camera->projection));
        glUniformMatrix4fv(glGetUniformLocation(spId, "model"), 1, GL_FALSE, value_ptr(lightModel));
        glUniform3fv(glGetUniformLocation(spId, "color"), 1, value_ptr(light.color));
        lightSourceModel->Draw(sp);
    }
    ShaderProgram::unbind();
}

void loadNewModel(const std::string &path, Model *&model, Animator *&animator,
                  AnimationMixer *&mixer) {
    if (model) {
        delete model;
        model = nullptr;
    }
    if (animator) {
        delete animator;
        animator = nullptr;
    }
    if (mixer) {
        delete mixer;
        mixer = nullptr;
    }
    unsigned int numAnimations = 0;
    model = new Model(path.c_str(), numAnimations);
    for (auto pair: animations) {
        delete pair.second;
        pair.second = nullptr;
    }
    animations.clear();
    animationNames.clear();
    for (int i = 0; i < numAnimations; i++) {
        std::string name;
        Animation *anim = new Animation(path.c_str(), model, i, name);
        animations.insert(std::make_pair(name, anim));
    }
    for (auto pair: animations) {
        animationNames.push_back(pair.first);
    }
    animator = new Animator(animations.at(animationNames[0]));
    selectedAnimation = 0;
    if (animations.size() > 1) {
        selectedMixer1 = 0;
        selectedMixer2 = 1;
        mixer = new AnimationMixer(animations.at(animationNames[0]), animations.at(animationNames[1]));
    }

    setParametersToDefault();
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;

    lastX = xpos;
    lastY = ypos;
    if (lmb && !ImGui::GetIO().WantCaptureMouse) {
        camera->rotateRelatively(xoffset / 10.f);
    }
    if(rmb && !ImGui::GetIO().WantCaptureMouse) {
        angleY += xoffset / 20.f;
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            lmb = true;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            lastX = xpos;
            lastY = ypos;
        } else if (action == GLFW_RELEASE) {
            lmb = false;
        }
    }
    if(button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            rmb = true;
        } else if (action == GLFW_RELEASE) {
            rmb = false;
        }
    }
}

void processInput(GLFWwindow *window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->translate(-camera->lookAt() * deltaTime * 5.f);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->translate(camera->lookAt() * deltaTime * 5.f);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera->translate(glm::vec3(0, 1.f, 0) * deltaTime * 5.f);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera->translate(glm::vec3(0, -1.f, 0) * deltaTime * 5.f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE)
        camera->translate(glm::vec3(0.f));
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)
        camera->translate(glm::vec3(0.f));
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
        camera->translate(glm::vec3(0.f));
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
        camera->translate(glm::vec3(0.f));
}