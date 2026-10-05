# RunaEngine リファレンス

この資料は、エンジンを使う方法と、自分で中身を直すときの入口をまとめたものです。
コード中の名前はそのまま使い、説明はできるだけ普通の言葉にしています。

## 1. まず動かす

Visual Studioでプロジェクトを開き、`x64` の `Debug` を選んでビルドしてください。
実行時の作業フォルダーは `C:\Users\rukar\source\repos\CG2\CG2` です。
画像やシェーダーはこのフォルダーを基準に探します。exeだけ別の場所へ移動すると起動できません。

必要な環境は、このプロジェクトが指定しているVisual StudioのC++ツールセット `v145`、Windows SDK、DirectX 12対応のGPUです。
DebugだけImGui（数値を画面上で変更するためのUI）が有効になります。

最小の使用例です。`main.cpp` の `WinMain` 内で行います。

```cpp
RunaEngine::Initialize(1280, 720, "My Game");
auto* sprite = RunaEngine::CreateSprite("resources/sprite_test.png");
sprite->SetSize(128.0f, 128.0f);

while (RunaEngine::ProcessMessage()) {
    RunaEngine::BeginFrame();
    RunaEngine::DrawSprite(sprite, RunaEngine::Vector2{100.0f, 100.0f});
    RunaEngine::EndFrame();
}

RunaEngine::Shutdown();
```

`CreateSprite` は最初に1回、`DrawSprite` は表示したいフレームごとに呼びます。
「フレーム」は画面を1回更新することです。ループの中で毎回作り直す必要はありません。

### namespaceとAPIは、何のためにあるのか

`RunaEngine::` は「この関数や型はRunaEngineのもの」という名前の区別です。
これ自体は描画を遅くする処理ではありません。
APIとは、ここでは `CreateSprite` や `DrawSprite` のような「外から使う関数」のことです。

毎回長く書くのが面倒なら、ゲーム側の `.cpp` の中で次のように短くできます。

```cpp
namespace RE = RunaEngine;
auto* sprite = RE::CreateSprite("resources/sprite_test.png");
RE::DrawSprite(sprite, RE::Vector2{40.0f, 40.0f});
```

`using namespace RunaEngine;` も、ゲーム側の `.cpp` 内なら使えます。
自分で書く `.h` に広く置くと、そのヘッダーを読む別のファイルにも影響するので避けてください。
位置や色は `sprite->SetColor(...)` のように物体へ直接設定できます。
GPUの準備や削除のタイミングは、エンジンの関数に任せる構成です。

## 2. 1フレームの順序

```text
BeginFrame()
  ↓ 入力・時間を更新し、今回の描画を準備する
ゲームの更新（移動、アニメーションなど）
  ↓
DrawModel / DrawObject3D / DrawPrimitive3D
  ↓ 3Dの描画命令を記録する
DrawSprite
  ↓ 画像の位置・色などを、その呼び出しの状態で保存する
EndFrame()
  ↓ 画像を描画順に並べて描く → ImGuiを描く
  ↓ GPUへ命令を送る → 描画完了を待つ → 画面へ表示する
  ↓ 不要になった転送用メモリ・削除予約した物体を片付ける
次のBeginFrame()
```

`Draw...` は必ず `BeginFrame` と `EndFrame` の間で呼びます。
Spriteは、呼び出した場所にかかわらず、通常の3D描画の後にまとめて描かれます。
`SetDrawOrder` が比較するのはSprite同士の順番です。

## 3. 位置・大きさ・色の書き方

| 型 | 内容 | 例 |
|---|---|---|
| `Vector2` | 数値2つ | `{100.0f, 200.0f}`：画面の位置など |
| `Vector3` | 数値3つ | `{0.0f, 1.0f, 0.0f}`：3Dの位置など |
| `Vector4` | 数値4つ | `{1, 0, 0, 1}`：赤、緑、青、不透明度 |
| `Transform` | 大きさ、回転、位置のセット | 下の例 |

```cpp
RunaEngine::Transform transform{
    {1.0f, 1.0f, 1.0f}, // scale：大きさの倍率
    {0.0f, 0.0f, 0.0f}, // rotate：回転
    {0.0f, 0.0f, 0.0f}  // translate：位置
};
```

初期化を省いて `Transform transform{};` とすると倍率も0になります。
描画に使う倍率は、まず `{1, 1, 1}` にしてください。3Dの倍率0はエラーにします。
回転の単位はラジアンです。180度は約 `3.14159265f`、90度はその半分です。

Spriteの位置は画面左上が `(0, 0)`、右がXのプラス、下がYのプラス。単位はピクセルです。
3DはYが上で、初期カメラは `(0, 0, -10)` からZのプラス方向を見ています。
Spriteの表示サイズと3Dの大きさは、同じ単位ではありません。

色は通常0〜1で指定します。4つ目の値は1で不透明、0で透明です。
現在、半透明合成が有効なのはSpriteです。3Dの不透明度を変えるだけでは、背景と混ざって見えるようにはなりません。
画像の色と指定した色を掛け合わせるため、元の画像にない色が強くなるとは限りません。

## 4. Sprite：画像を表示する

```cpp
namespace RE = RunaEngine;
auto* sprite = RE::CreateSprite("resources/sprite_test.png");
sprite->SetSize(128.0f, 64.0f);
sprite->SetPivot(0.5f, 0.5f); // 指定位置を画像の中心にする
sprite->SetDrawOrder(10);
```

| 操作 | 意味 |
|---|---|
| `CreateSprite(path)` | 画像付きのSpriteを作る。最初の表示サイズは画像と同じ |
| `DrawSprite(sprite, Vector2{x,y})` | 指定位置に描く。倍率1・回転0で描く |
| `DrawSprite(sprite, Vector3{x,y,z})` | Zも指定できる。Spriteの前後順は描画順で決まる |
| `DrawSprite(sprite, transform)` | 倍率・回転・位置をまとめて指定して描く |
| `SetSize(width,height)` / `SetSize(Vector2)` | 表示サイズをピクセルで指定する |
| `SetPivot(x,y)` / `SetPivot(Vector2)` | 位置の基準。`0,0` は左上、`0.5,0.5` は中心 |
| `SetColor(Vector4)` | 色と不透明度を設定する |
| `SetDrawOrder(int)` | 小さい順に描く。大きい値が手前。同じ値なら呼んだ順。負の値も使用可 |
| `SetSpriteTexture(sprite,path)` | 画像を交換する。表示サイズと切り抜き範囲は維持する |
| `SetTextureRect(x,y,width,height)` | 元画像のピクセルで切り抜き範囲を指定する |
| `SetUVRect(left,top,right,bottom)` | 切り抜き範囲を0〜1の割合で指定する |
| `SetUVTransform(matrix)` | 画像の参照位置を行列で動かす。最初は使わなくてよい |
| `GetColor()` / `GetSize()` | 現在設定している色・サイズを取得する |
| `GetUVLeftTop()` / `GetUVRightBottom()` | 現在の切り抜き範囲を割合で取得する |
| `GetTextureHandle()` / `SetTextureHandle(handle)` | 読み込み済み画像の番号を取得・指定する |
| `DestroySprite(sprite)` | Spriteを削除する。渡した変数を `nullptr` にする |

「UV」は、画像上の場所を0〜1で表す座標です。
`SetTextureRect` は切り抜くだけです。表示の大きさも変えたい場合は `SetSize` を呼びます。
画像交換後の `SetTextureRect` は、新しい画像の幅・高さで計算します。

### 同じSpriteを複数回描く

```cpp
// BeginFrameとEndFrameの間
sprite->SetColor({1, 0, 0, 1});
RE::DrawSprite(sprite, RE::Vector2{40, 40});

sprite->SetColor({0, 0, 1, 1});
RE::DrawSprite(sprite, RE::Vector2{552, 40});
```

1回目は赤、2回目は青のまま描かれます。
色・サイズ・切り抜き・画像・基準位置・描画順を、呼び出した時点でそれぞれ保存します。
後でSpriteの設定を変えても、すでに予約した描画には影響しません。
`DrawSprite(sprite, position)` はSpriteの `GetTransform()` を書き換えません。
ゲーム側で保存したTransformを使いたい場合は、そのTransformを引数に渡してください。

### コマ送りのアニメーション

`SpriteAnimator` は、1枚の画像を区切って順に表示する補助クラスです。

```cpp
SpriteAnimationClip clip{};
clip.frameCount = 8;      // 画像中のコマ数
clip.columnCount = 4;     // 1行のコマ数
clip.frameWidth = 32;
clip.frameHeight = 32;
clip.secondsPerFrame = 0.1f;
SpriteAnimator animator;
animator.Initialize(sprite, clip);

// 毎フレーム、DrawSpriteより前で呼ぶ
animator.Update(RE::GetDeltaTime());
```

`Play`・`Pause`・`Reset` で再生を操作できます。
Spriteを削除した後は、そのSpriteを使うAnimatorも使わないでください。

## 5. ModelとObject3D：3Dモデルを表示する

`Model` は読み込んだ形と材質のデータです。
`Object3D` は、そのModelを使って置く1個の物体です。
同じ形の物体を100個置いても、Modelを100個読み込む必要はありません。

```cpp
auto* model = RE::CreateModel("resources/teapot.obj");
auto* left = RE::CreateObject3D(model);
auto* right = RE::CreateObject3D(model);
left->GetTransform().translate = {-2, 0, 0};
right->GetTransform().translate = {2, 0, 0};

// 毎フレーム
RE::DrawObject3D(left);
RE::DrawObject3D(right);
```

物体を作らず、その場で `DrawModel(model, transform)` として描くこともできます。
エンジン内部で、描画用の物体を使い回します。

| 操作 | 意味 |
|---|---|
| `CreateModel(path)` | OBJを読み込む。同じファイルなら同じModelを返す |
| `CreateObject3D(model)` | Modelを使う物体を作る |
| `DrawModel(model, position/transform)` | 指定位置またはTransformで描く |
| `DrawModel(model, transform, parameters)` | 色・照明・画像なども指定して描く |
| `DrawObject3D(object)` | 物体が持つTransformで描く |
| `DrawObject3D(object, position/transform)` | 物体のTransformを設定して描く |
| `DrawObject3D(object, parameters)` | 物体のTransformで、描画設定を指定して描く |
| `DrawObject3D(object, transform, parameters)` | Transformと描画設定を指定して描く |
| `object->GetTransform()` | 物体の大きさ・回転・位置を変更する |
| `SetObjectTexture(object,path)` | この物体だけの画像を変える |
| `object->ClearTextureOverride()` | 物体固有の画像設定を解除する |
| `SetModelTexture(model,path)` | そのModelを使う物体の共通画像を変える |
| `model->ClearTextureOverride()` | Model共通の画像設定を解除する |
| `model->GetMeshCount()` / `GetSubMeshCount()` / `GetMaterialCount()` | 形・材質ごとの区切りの数を調べる |
| `DestroyObject3D(object)` | 物体を削除し、変数を `nullptr` にする |
| `DestroyModel(model)` | Modelを削除する。使っているObject3Dを先に削除する |

同じModelを複数回 `CreateModel` すると、返されたポインターは共有されます。
`DestroyModel` で削除すると、別の変数に保存した同じModelのポインターも使えなくなります。
Model単位の画像変更は共有先にも効くので、1個だけ変えたい場合は `SetObjectTexture` を使います。

### 描画ごとの設定

```cpp
RE::ModelDrawParameters parameters{};
parameters.color = {1, 0.5f, 0.5f, 1};
parameters.lightingMode = LightingMode::LAMBERT;
parameters.textureOverride = RE::LoadTexture("resources/checkerBoard.png");

// 毎フレーム
RE::DrawObject3D(left, parameters);
```

画像指定の優先順は、`parameters.textureOverride` → Object3D固有 → Model共通 → MTLに書かれた画像 → 白画像です。
引数で指定する画像はその描画だけに使い、物体の設定を書き換えません。
最終的な色は、指定色 × MTLの色 × 画像の色を基に計算します。

`DrawObject3D` と `DrawPrimitive3D` も、同じ物体を1フレーム中に複数回描けます。
3Dの位置などは描画呼び出し時にコピーされ、後の変更で前の描画が書き換わりません。

### 現在の読み込み範囲

対応しているモデル形式はOBJです。FBX、glTFなどを読み込むAssimpはまだ導入していません。
OBJでは、正・負の頂点番号、通常の三角形、平面上の凹多角形、複数の形・材質の区切りを扱います。
頂点の法線（面が向く方向）がない場合は、面の向きから作ります。
法線のないモデルで角を滑らかに見せる処理はありません。滑らかな法線を持つOBJを書き出してください。

MTLからは画像、基本色 `Kd` を表示へ反映し、不透明度 `d`/`Tr` を出力色へ設定します。3Dの半透明合成は未実装です。
鏡面色 `Ks`・光沢値 `Ns` も保存しますが、現在の描画では使いません。
`map_Kd` の画像はMTLのあるフォルダーを基準に探します。
`map_Kd -s ...` などの追加指定は未対応なので、理由を表示して停止します。
自己交差する面や立体的にねじれた多角形は入力しないでください。三角形へ変換したOBJが確実です。

## 6. Primitive3D：三角形を表示する

```cpp
auto* triangle = RE::CreateTriangle3D({1, 0, 0, 1});
triangle->GetTransform().translate = {0, 0, 0};
triangle->SetLightingMode(LightingMode::NONE);
// 毎フレーム
RE::DrawPrimitive3D(triangle);
```

`CreateTriangle3D()` の初期色は `{1, 0.2f, 0.1f, 1}`、初期照明は `NONE` です。
`DrawPrimitive3D` は物体だけ、位置付き、Transform付きのいずれでも呼べます。
`SetColor`・`GetColor` で色を変更・取得し、`DestroyPrimitive3D` で削除します。

## 7. 光・カメラ・入力・時間・音

### 光

```cpp
auto& light = RE::GetDirectionalLight();
light.SetColor({1, 1, 1, 1});
light.SetDirection({0, -1, 0}); // 光が進む向き
light.SetIntensity(1.0f);
```

`SetDirection` には0以外の方向、`SetIntensity` には0以上の値を渡します。
光の不透明度で物体が透けることはありません。光は物体のRGBの明るさへ反映します。

| 照明設定 | 見え方 |
|---|---|
| `LightingMode::NONE` | 光の影響なし。画像と指定色をそのまま表示 |
| `LightingMode::HALF_LAMBERT` | 光の反対側も明るさを残す。Model描画の初期設定 |
| `LightingMode::LAMBERT` | 光へ向いた面が明るくなり、反対側は暗くなる |

現状は方向が決まった光1個です。鏡面反射・スポットライト・ポイントライトは今後の実装です。

### カメラ

`GetCameraTransform()` で位置・回転を変更できます。
`SetCameraTransform(transform)` は行列もその場で更新します。
`MoveCamera(Vector3)` は現在位置に移動量を足します。カメラ自身の向きとは独立した、世界の方向での移動です。
`GetCameraTransform()` を直接変更した結果は、次の `BeginFrame` で反映されます。
同じフレームの途中で反映したい場合は `SetCameraTransform` を呼んでください。
視野角は `0.45` ラジアン、表示範囲の手前は `0.1`、奥は `100` です。現在は固定です。

### 入力と時間

| 関数 | 内容 |
|---|---|
| `IsPushKey(Key::A)` | 押している間ずっとtrue |
| `IsTriggerKey(Key::A)` | 押した瞬間のフレームだけtrue |
| `GetMousePosition()` | ウィンドウ内のマウス位置 |
| `IsGamepadConnected()` | ゲームパッドが接続されているか |
| `IsButtonDown/Triggered/Released(GamepadButton::A)` | 押している間・押した瞬間・離した瞬間 |
| `GetLeftStick()` / `GetRightStick()` | スティックの横・縦。おおむね-1〜1 |
| `GetLeftTrigger()` / `GetRightTrigger()` | トリガーの押し込み。0〜1 |
| `GetDeltaTime()` | 前回から経過した秒数 |

入力は `BeginFrame` の中で更新します。移動速度は「1秒で動く量 × 経過秒数」にします。

```cpp
const float speed = 3.0f;
if (RE::IsPushKey(Key::D)) {
    left->GetTransform().translate.x += speed * RE::GetDeltaTime();
}
```

### 音

```cpp
auto sound = RE::LoadSound("resources/Alarm01.wav"); // 最初に読み込む
// 鳴らす必要があるときだけ呼ぶ
if (RE::IsTriggerKey(Key::Space)) {
    RE::PlaySound(sound, false, 0.5f); // 音番号、ループ、音量
}
```

`LoadSound` は `std::string` と `std::wstring` を受け付けます。
`PlaySound` を毎フレーム呼ぶと音が重なります。音単位の解放や停止は今回追加していません。
`ClearScene` は描画用の物体・モデル・画像を片付けます。音は `Shutdown` で片付けます。

## 8. 作成と削除の約束

`Create...` が返すポインターはエンジンが管理しています。ゲーム側で `delete` しないでください。

```cpp
RE::DestroyObject3D(left);
RE::DestroyObject3D(right);
RE::DestroyModel(model);
RE::DestroySprite(sprite);
RE::DestroyPrimitive3D(triangle);
```

削除関数は、渡した変数を `nullptr` にします。
別の変数へコピーしていたポインターまでは変更できません。コピーした方も使わないでください。
描画の途中で削除を呼んでも、そのフレームで予約済みの描画は完了してから実体を解放します。
削除後に新しい描画を予約するとエラーになります。

### 画像だけを解放する

```cpp
uint32_t handle = RE::LoadTexture("resources/checkerBoard.png");
// この画像を使うSprite・Model・Object3Dがなくなってから
RE::UnloadTexture(handle);
```

画像番号を `delete` してはいけません。
画像はSpriteやModelを削除しても共有のため保持します。不要な画像は `UnloadTexture`、場面全体は `ClearScene` で解放します。
使用中の画像の解放は拒否します。描画中に物体の削除も予約した場合は、`EndFrame` 後に画像を解放してください。
解放後の画像番号を再利用してはいけません。読み込み直すと新しい番号が返ります。
番号はGPU内の画像置き場の番号とは別なので、解放した置き場を再利用しても古い番号は無効のままです。
画像番号0はImGui専用です。

### 場面を丸ごと片付ける

```cpp
RE::ClearScene();
sprite = nullptr;
model = nullptr;
left = nullptr;
right = nullptr;
triangle = nullptr;
```

`ClearScene` はすべてのSprite・Object3D・Primitive3D・Model・画像を片付けます。
それ以前のポインターと画像番号は、以後使えません。ゲーム側の変数やAnimatorなども片付けます。
`BeginFrame` の後に呼んだ場合は `EndFrame` で実行します。新しい場面の読み込みはその後で行ってください。
フレームの外で呼ぶ場合は、必要な転送を完了させてから、その場で片付けます。
通常は「旧場面の `EndFrame` → `ClearScene` → 新場面を作成」の順がわかりやすいです。

同時に保持できる通常の画像は最大127枚です。128個の置き場のうち1個をImGuiに使います。
同じ画像を別の表記のパスで読み込んでも、同一のファイルなら1枚として扱います。
画像の読み込みに失敗した場合、画像番号や置き場は消費しません。

## 9. 内部を読むための地図

| ファイル | 読む目的 |
|---|---|
| [main.cpp](C:/Users/rukar/source/repos/CG2/CG2/main.cpp) | ゲームからどう呼ぶか。最初に読む |
| [RunaEngine.h](C:/Users/rukar/source/repos/CG2/CG2/RunaEngine/RunaEngine.h) | 外から使える関数一覧 |
| [RunaEngine.cpp](C:/Users/rukar/source/repos/CG2/CG2/RunaEngine/RunaEngine.cpp) | 1フレームの順番、物体の管理、描画順、削除時期 |
| [Sprite.cpp](C:/Users/rukar/source/repos/CG2/CG2/RunaEngine/Graphics/Sprite.cpp) | 画像を板の形へ配置して描く処理 |
| [Object3D.cpp](C:/Users/rukar/source/repos/CG2/CG2/RunaEngine/Graphics/Object3D.cpp) | 物体の位置・回転・倍率を描画へ渡す処理 |
| [Model.cpp](C:/Users/rukar/source/repos/CG2/CG2/RunaEngine/Graphics/Model.cpp) | 読み込んだ形を材質ごとに描く処理 |
| [ModelLoader.cpp](C:/Users/rukar/source/repos/CG2/CG2/RunaEngine/Graphics/ModelLoader.cpp) | OBJ/MTLの文字をエンジンのデータに変える処理 |
| [ResourceManager.cpp](C:/Users/rukar/source/repos/CG2/CG2/RunaEngine/Core/ResourceManager.cpp) | 同じモデルを重複して持たない管理 |
| [TextureManager.cpp](C:/Users/rukar/source/repos/CG2/CG2/RunaEngine/Core/TextureManager.cpp) | 画像の読み込み・転送・番号・解放 |
| [ConstantBuffer.h](C:/Users/rukar/source/repos/CG2/CG2/RunaEngine/Graphics/ConstantBuffer.h) | 描画ごとの数値をGPUへ渡す保存場所 |
| [TransformationMatrix.cpp](C:/Users/rukar/source/repos/CG2/CG2/RunaEngine/Graphics/TransformationMatrix.cpp) | 引き伸ばした物体の面の向きを正しく求める処理 |
| [Renderer.cpp](C:/Users/rukar/source/repos/CG2/CG2/RunaEngine/Graphics/Renderer.cpp) | 描く種類に合わせた設定の選択 |
| [DirectXCommon.cpp](C:/Users/rukar/source/repos/CG2/CG2/RunaEngine/Core/DirectXCommon.cpp) | 画面の準備、描画命令の送信、完了待ち |
| [ShaderCompiler.cpp](C:/Users/rukar/source/repos/CG2/CG2/RunaEngine/Core/ShaderCompiler.cpp) | GPUで動くプログラムを読み込む処理 |
| [EngineRegressionTests.cpp](C:/Users/rukar/source/repos/CG2/CG2/tests/EngineRegressionTests.cpp) | 今回の問題が再発していないか確認するコード |

全部を一度に理解しようとせず、`DrawSprite` 1回がどこを通るかを順に追ってください。
次に三角形1枚、次にModelの順で追うと、共通部分が見えます。

### C++からGPUへ渡すデータ

シェーダーは「GPU上で動く、頂点の位置や画素の色を計算するプログラム」です。
頂点用の `.VS.hlsl` と、色を決める `.PS.hlsl` に分かれています。

| 保存する内容 | C++側 | HLSL側 |
|---|---|---|
| 座標変換 | `TransformationMatrix`：WVP、World、WorldInverseTransposeの順、192バイト | Object3d/SpriteのVS、`b0` |
| 3Dの色・照明・UV | `Material`：色、照明番号、空き領域、UV行列、96バイト | Object3d/Primitive3DのPS、`b0` |
| Spriteの色・UV | `SpriteMaterial`：色、UV行列、80バイト | SpriteのPS、`b0` |
| 光 | `DirectionalLightData`：色、向き、強さ、32バイト | 3DのPS、`b1` |

`b0` はGPUが受け取るデータの番号です。頂点側と色側では別の番号の組を使います。
C++とHLSLで、項目の順番とメモリ上の位置を合わせる必要があります。
HLSLの項目配置は基本16バイトの区切り、保存場所の大きさ・開始位置は256バイトの区切りです。
この2つは違う決まりです。`ConstantBuffer` は後者に合わせて場所を確保しています。

`WVP` は物体から画面までの変換、`World` は物体から世界の位置への変換です。
`WorldInverseTranspose` は面の向き専用の変換です。
点の位置と面の向きでは、縦横の倍率が違うときの変換方法が違います。
このエンジンは行の順に行列を保存し、シェーダーでは `mul(座標, 行列)` と計算します。

## 10. 今回直した6つの問題

| 問題 | 修正した動き |
|---|---|
| 同じ物体を複数回描くと前の数値まで変わる | 描画1回ごとに別の場所へコピー。Spriteは呼んだ時点の状態を保存 |
| 縦横の倍率が違うと照明がずれる | 面の向き専用の変換を追加。倍率0など計算できない設定を検出 |
| 画像の置き場や番号の確認が足りない | 上限・存在・削除済み番号を確認。置き場の再利用と画像番号を分離 |
| エラーをDebugのassertに頼りすぎる | 描画の作成・転送・シェーダー・読み込みなどの失敗をReleaseでも検出。ログと停止理由を表示 |
| OBJ読み込みの扱える書き方が少ない | 負の番号、法線なし、凹多角形、MTLの色、MTL基準の画像パスなどへ対応 |
| 読み込み・削除の管理が足りない | 転送用メモリを完了後に解放。削除API、ClearScene、モデル共有、物体固有の画像を追加 |

加えて、Spriteの重複した関数宣言、存在しない関数への呼び出し、変更可能なTransform取得の不足を修正しました。
モデルや三角形の照明で、不透明度まで光の強さで変えていた処理も修正しました。
方向光も描画ごとにコピーするため、途中で光を変えても前の描画に影響しません。

`FrameBuffer` は描画ごとに保存場所を増やし、次のフレームから再利用します。
毎フレーム増え続ける構造ではありませんが、過去に一度使った最大の描画数まで保持します。
大量のパーティクルをこの方式で1個ずつ描くのは重いため、後述のまとめ描きを使う予定です。

## 11. エラーを見る・自動確認を実行する

通常実行で今回対象のエラーが起きると、理由を表示して停止します。
詳細は `C:\Users\rukar\source\repos\CG2\CG2\logs` のログと、Visual Studioの「出力」欄を見ます。
読み込み失敗ではファイル名、OBJ/MTLの内容のエラーでは行番号も確認してください。

Debugでは、DirectXの誤った使い方の検出機能を有効にし、重大なエラーで止まるようにしています。
Windowsの「グラフィックスツール」がない環境では、その検出機能が使えない旨をログに出して続行します。

ビルド後、PowerShellで次の確認を実行できます。

```powershell
Set-Location 'C:\Users\rukar\source\repos\CG2\CG2'
Start-Process '.\x64\Debug\CG2.exe' -ArgumentList '--self-test' -Wait -WindowStyle Hidden
Get-Content '.\logs\self-test.txt'

Start-Process '.\x64\Debug\CG2.exe' -ArgumentList '--lifecycle-test' -Wait -WindowStyle Hidden
Get-Content '.\logs\lifecycle-test.txt'
```

`--self-test` は画面を作らず、GPUで描いた色を読み戻して比較します。
WARPというCPUによるDirectX描画を使い、OBJ・画像管理・シェーダーも確認します。
`--lifecycle-test` は非表示ウィンドウで本体を起動し、削除・場面切り替え・再起動を確認します。
後者には通常実行と同じGPU・音の環境が必要です。
Releaseでも確認するときは、exeのパスの `Debug` を `Release` に変えます。
終了コード0が成功、1が失敗です。結果末尾の `ALL PASSED` または `FAILED` を見てください。
テストが作るファイルはlogs内です。ゲームの元画像・元モデルは変更しません。

今回の最終版では、Debugの描画・読み込み確認51項目と終了処理の確認17項目、Releaseの50項目と17項目がすべて通りました。
Releaseの1項目が少ないのは、Debug専用のDirectXエラー検出を使わないためです。
実際の描画確認は、赤・青の色、位置、サイズ、画像の切り抜き、3D物体の複数描画、光の強さまで比較しています。

## 12. 次の機能を自分で足すとき

現在まだない機能は、BlendModeの選択・2値抜き、パーティクルのまとめ描き、Phong/Blinn-Phong、スポットライト、ポイントライト、Assimp読み込みです。
今回の修正は、その前に既存の描画と管理を安定させる範囲です。

| 追加する機能 | 最初に作る小さな例 | 主に触る場所・注意 |
|---|---|---|
| BlendMode | 四角2枚を重ね、通常合成と加算を切り替える | 各GraphicsPipelineの合成設定。設定ごとの描画用オブジェクトを作って切り替える |
| 2値抜き | 葉の画像1枚で、透明度の低い部分を描かない | PSの`clip`や`discard`。抜いた部分が奥の物体を隠さないことを確認する |
| パーティクル | 板1枚→2枚→100枚へ増やす | 粒ごとの位置・色の配列をGPUへ渡し、1回の命令でまとめて描く。枚数の上限と完了前の上書きに注意 |
| Phong/Blinn-Phong | 球1個・光1個・カメラ固定で光沢を見る | Object3dのVS/PS。面の向き、世界での位置、カメラ位置を同じ座標系で計算する |
| ポイントライト | 球の周囲で光1個を動かす | 光の位置と距離による弱まりを追加。距離0でも割り算が壊れないようにする |
| スポットライト | 光の範囲を狭めて床を照らす | ポイントライトを理解してから、光の向きと角度を追加する |
| Assimp | 三角形のOBJ→FBX/glTF各1個の順に読む | 読み込み結果を既存のModelDataへ変換する。親子の位置変換、材質ごとの画像、面の表裏を確認する |

2値抜きは「半透明に混ぜる」処理とは別です。基準以上なら描き、それ未満なら描きません。
3D半透明を追加するときは、合成設定だけでなく、描く順序と奥行きの書き込みも考える必要があります。
現在の3Dには半透明物体を奥から並べる処理がありません。

新しい機能は、次の順で進めると原因を見つけやすくなります。

1. 既存の三角形1枚が描かれる状態を保存する。
2. 追加する機能を、まず1個の物体だけで動かす。
3. 自分の言葉で「追加したデータはどこからどこへ渡るか」を説明する。
4. 同じ物体を2回描く。値を途中で変えても混ざらないか確認する。
5. 削除・ClearScene・再読み込みを試す。
6. DebugとReleaseを確認してから、実際の場面へ組み込む。

CPUが命令を記録してからGPUが実行するまでには時間差があります。
その間は、渡した数値の保存場所・画像・モデルを上書きしたり削除したりできません。
現在は毎フレームGPUの完了を待つ単純な方式です。複数フレームを同時に進める高速化は、今の流れを理解してから行ってください。
その高速化をするときは、今回の保存場所の再利用と削除の時期も一緒に変える必要があります。
