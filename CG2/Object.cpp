#include "Object.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif

Object::Object(const std::string& name) : name_(name) {
	transform_ = initTransform;
	modelParameters_.lightingMode = LightingMode::LAMBERT;
	modelParameters_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void Object::SetDrawable(DrawableID id) {
	currentDrawableID_ = static_cast<int>(id);
	// マネージャーからポインタを受け取る
	object_ = DrawableManager::Get(id);
}

void Object::Update() {
#ifdef USE_IMGUI
	if (ImGui::TreeNode(name_.c_str())) {

		const char* drawableItems[] = {
			"None", "Plane", "Sphere", "Utah Teapot", "Stanford Bunny", "Suzzane", "Multi Mesh Model",
			"UV Checker (Sprite)"
		};

		// リソースの切り替え
		if (ImGui::Combo("Drawable Type", &currentDrawableID_, drawableItems, IM_ARRAYSIZE(drawableItems))) {
			SetDrawable(static_cast<DrawableID>(currentDrawableID_));
			transform_ = initTransform; // 切り替えたら位置をリセット
		}

		ImGui::DragFloat3("scale", &transform_.scale.x, 0.01f);
		ImGui::DragFloat3("rotate", &transform_.rotate.x, 0.01f);
		ImGui::DragFloat3("translate", &transform_.translate.x, 0.01f);

		// 中身がModelの時だけ表示する専用パラメータ
		if (std::holds_alternative<RunaEngine::Model*>(object_)) {
			ImGui::DragFloat2("uvTransform", &modelParameters_.uvTransform.m[3][0], 0.01f);

			auto* model = std::get<RunaEngine::Model*>(object_);
			if (model) {
				ImGui::Text("Mesh Count: %zu", model->GetMeshCount());
				ImGui::Text("SubMesh Count: %zu", model->GetSubMeshCount());
				ImGui::Text("Material Count: %zu", model->GetMaterialCount());
			}
		}

		ImGui::TreePop();
	}
#endif
}

void Object::Draw() {
	std::visit([this](auto* obj) {
		using T = std::decay_t<decltype(obj)>;

		// NONE の場合は obj が nullptr になるため、何も描画せずに終わる
		if (obj == nullptr) return;

		// 実行時の型に合わせてRunaEngineの描画関数を呼び分ける
		if constexpr (std::is_same_v<T, RunaEngine::Model*>) {
			RunaEngine::DrawModel(obj, transform_, modelParameters_);
		} else if constexpr (std::is_same_v<T, RunaEngine::Sprite*>) {
			RunaEngine::DrawSprite(obj, transform_);
		}
		}, object_);
}