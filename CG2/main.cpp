#include "RunaEngine/RunaEngine.h"

void MoveCamera();

enum class ModelList {
	NONE,
	PLANE,
	SPHERE,
	TEAPOT,
	BUNNY,
	SUZANNE,
	MULTIMESH
};

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	RunaEngine::Initialize(1280, 720, "TITLE");
	RunaEngine::Transform initTransform{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 3.1415f, 0.0f},
		{-2.0f, 0.0f, 0.0f}
	};
	static RunaEngine::Model* model[7];
	model[static_cast<int>(ModelList::NONE)]= CreateModel("resources/plane.obj"); // 使わないけど生成
	model[static_cast<int>(ModelList::PLANE)] = CreateModel("resources/plane.obj");
	model[static_cast<int>(ModelList::SPHERE)] = CreateModel("resources/Sphere.obj");
	model[static_cast<int>(ModelList::TEAPOT)] = CreateModel("resources/teapot.obj");
	model[static_cast<int>(ModelList::BUNNY)] = CreateModel("resources/bunny.obj");
	model[static_cast<int>(ModelList::SUZANNE)] = CreateModel("resources/suzanne.obj");
	model[static_cast<int>(ModelList::MULTIMESH)] = CreateModel("resources/multiMesh.obj");

	bool drawSprite = true;
	RunaEngine::Sprite* sprite = RunaEngine::CreateSprite("resources/uvChecker.png");
	RunaEngine::Transform spriteTransform = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	int currentLeftModel = static_cast<int>(ModelList::PLANE);

	DirectionalLight& directionalLight =
		RunaEngine::GetDirectionalLight();

	RunaEngine::Vector3 lightDirection =
		directionalLight.GetDirection();

	RunaEngine::Vector4 lightColor =
		directionalLight.GetColor();

	float lightIntensity =
		directionalLight.GetIntensity();
	int lightingMode =
		static_cast<int>(LightingMode::HALF_LAMBERT);

	RunaEngine::ModelDrawParameters leftParameters{};
	leftParameters.lightingMode = LightingMode::LAMBERT;
	leftParameters.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	RunaEngine::Transform leftTransform = initTransform;

	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();

		// 更新
		MoveCamera();
#ifdef USE_IMGUI
		ImGui::Begin("Settings");

		RunaEngine::Transform cameraTransform = RunaEngine::GetCameraTransform();
		ImGui::DragFloat3("cameraPos", &cameraTransform.translate.x);

		const char* modelItems[] = {
			"None",
			"Plane",
			"Sphere",
			"Utah Teapot",
			"Stanford Bunny",
			"Suzzane",
			"Multi Mesh Model",
		};

		if (ImGui::Combo(
			"Model1",
			&currentLeftModel,
			modelItems,
			IM_ARRAYSIZE(modelItems))) {

			leftTransform = initTransform;
		}

		ImGui::DragFloat3("scale", &leftTransform.scale.x, 0.01f);
		ImGui::DragFloat3("rotate", &leftTransform.rotate.x, 0.01f);
		ImGui::DragFloat3("translate", &leftTransform.translate.x, 0.01f);

		ImGui::DragFloat2("uvTransform", &leftParameters.uvTransform.m[3][0], 0.01f);
		
		const char* lightingItems[] = {
			"None",
			"Half Lambert",
			"Lambert"
		};

		ImGui::Combo(
			"Lighting Mode",
			&lightingMode,
			lightingItems,
			IM_ARRAYSIZE(lightingItems));


		if (ImGui::DragFloat3(
			"Light Direction",
			&lightDirection.x,
			0.01f)) {

			if (lightDirection.Length() > 0.0001f) {
				directionalLight.SetDirection(lightDirection);
			}
		}

		if (ImGui::ColorEdit4(
			"Light Color",
			&lightColor.x)) {

			directionalLight.SetColor(lightColor);
		}

		if (ImGui::SliderFloat(
			"Light Intensity",
			&lightIntensity,
			0.0f,
			10.0f)) {

			directionalLight.SetIntensity(lightIntensity);
		}

		
		leftParameters.lightingMode =
			static_cast<LightingMode>(lightingMode);
		ImGui::Text("Mesh Count: %zu", model[currentLeftModel]->GetMeshCount());
		ImGui::Text("SubMesh Count: %zu", model[currentLeftModel]->GetSubMeshCount());
		ImGui::Text("Material Count: %zu", model[currentLeftModel]->GetMaterialCount());

		ImGui::End();
#endif
		// 描画
		if (currentLeftModel != static_cast<int>(ModelList::NONE)) {
			RunaEngine::DrawModel(model[static_cast<int>(currentLeftModel)], leftTransform, leftParameters);
		}

		if (drawSprite) {
			RunaEngine::DrawSprite(sprite, spriteTransform);
		}
		
		RunaEngine::EndFrame();
	}

	RunaEngine::Shutdown();
	return 0;
}

void MoveCamera() {
	if (RunaEngine::IsPushKey(Key::A)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ -0.1f,0.0f,0.0f });
	} else if (RunaEngine::IsPushKey(Key::D)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ 0.1f,0.0f,0.0f });
	}

	if (RunaEngine::IsPushKey(Key::Space)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ 0.0f,0.1f,0.0f });
	} else if (RunaEngine::IsPushKey(Key::LeftShift)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ 0.0f,-0.1f,0.0f });
	}

	if (RunaEngine::IsPushKey(Key::W)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ 0.0f,0.0f,0.1f });
	} else if (RunaEngine::IsPushKey(Key::S)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ 0.0f,0.0f,-0.1f });
	}
}
