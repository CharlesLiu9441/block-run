#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <ciso646>
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;
auto ball_color = ORANGE;
const auto block_size = 20.0f;
const auto level_length = 50;
const auto level_count = 3;
//#define DEBUG
Mesh cube;
Material block_material, ball_material, hair_material;
Model ballModel;
Mesh* ballMesh;
bool died = 0;
string died_message;
Font font;
int level = 0;
vector<Matrix> trans[level_count];
struct ball {
	float x, y, z;//pos of center
	float radius;
	void draw() {
		DrawMesh(ballMesh[0], ball_material, MatrixMultiply(MatrixScale(1.5, 1.5, 1.5), MatrixMultiply(MatrixRotateY(-x / 25 * PI), MatrixMultiply(MatrixRotateX(PI / 2), MatrixTranslate(x, y, z)))));
		DrawMesh(ballMesh[1], hair_material, MatrixMultiply(MatrixScale(1.5, 1.5, 1.5), MatrixMultiply(MatrixRotateY(-x / 25 * PI), MatrixMultiply(MatrixRotateX(PI / 2), MatrixTranslate(x, y, z)))));
		DrawMesh(ballMesh[2], hair_material, MatrixMultiply(MatrixScale(1.5, 1.5, 1.5), MatrixMultiply(MatrixRotateY(-x / 25 * PI), MatrixMultiply(MatrixRotateX(PI / 2), MatrixTranslate(x, y, z)))));
#ifdef DEBUG
		DrawCubeWires(Vector3{ round(x / block_size) * block_size,round(y / block_size) * block_size,round(z / block_size) * block_size }, radius * 2, radius * 2, radius * 2, RED);
		DrawCube(Vector3{ round((x - radius) / block_size) * block_size,round((y - radius) / block_size) * block_size,round((z - radius) / block_size) * block_size }, radius * 2, radius * 2, radius * 2, GREEN);
		DrawCube(Vector3{ round((x - radius) / block_size) * block_size,round((y + radius) / block_size) * block_size,round((z - radius) / block_size) * block_size }, radius * 2, radius * 2, radius * 2, GREEN);
		DrawCube(Vector3{ round((x + radius) / block_size) * block_size,round((y - radius) / block_size) * block_size,round((z - radius) / block_size) * block_size }, radius * 2, radius * 2, radius * 2, GREEN);
		DrawCube(Vector3{ round((x + radius) / block_size) * block_size,round((y + radius) / block_size) * block_size,round((z - radius) / block_size) * block_size }, radius * 2, radius * 2, radius * 2, GREEN);
		DrawCube(Vector3{ round((x - radius) / block_size) * block_size,round((y - radius) / block_size) * block_size,round((z + radius) / block_size) * block_size }, radius * 2, radius * 2, radius * 2, GREEN);
		DrawCube(Vector3{ round((x - radius) / block_size) * block_size,round((y + radius) / block_size) * block_size,round((z + radius) / block_size) * block_size }, radius * 2, radius * 2, radius * 2, GREEN);
		DrawCube(Vector3{ round((x + radius) / block_size) * block_size,round((y - radius) / block_size) * block_size,round((z + radius) / block_size) * block_size }, radius * 2, radius * 2, radius * 2, GREEN);
		DrawCube(Vector3{ round((x + radius) / block_size) * block_size,round((y + radius) / block_size) * block_size,round((z + radius) / block_size) * block_size }, radius * 2, radius * 2, radius * 2, GREEN);
#endif
	}
};

struct block {
	float x, y, z;//pos of center
	float a;//Edge length
	void draw() {
		DrawMesh(cube, block_material, MatrixTranslate(x, y, z));
	}
};
int mp[level_count][level_length][50][50];
Camera3D camera;
ball player;
Vector3 pvel;
long long lst_gd;
bool is_pos_valid(int x, int y, int z) {
	return mp[level][x][y][z] xor ((x >= 0) && (x < level_length));
}
bool is_ball_pos_valid(float x, float y, float z) {
	return is_pos_valid(int(round((x - player.radius) / block_size) + level_length) % level_length, round((y - player.radius) / block_size), round((z - player.radius) / block_size)) &&
		is_pos_valid(int(round((x + player.radius) / block_size) + level_length) % level_length, round((y - player.radius) / block_size), round((z - player.radius) / block_size)) &&
		is_pos_valid(int(round((x - player.radius) / block_size) + level_length) % level_length, round((y + player.radius) / block_size), round((z - player.radius) / block_size)) &&
		is_pos_valid(int(round((x + player.radius) / block_size) + level_length) % level_length, round((y + player.radius) / block_size), round((z - player.radius) / block_size)) &&
		is_pos_valid(int(round((x - player.radius) / block_size) + level_length) % level_length, round((y - player.radius) / block_size), round((z + player.radius) / block_size)) &&
		is_pos_valid(int(round((x + player.radius) / block_size) + level_length) % level_length, round((y - player.radius) / block_size), round((z + player.radius) / block_size)) &&
		is_pos_valid(int(round((x - player.radius) / block_size) + level_length) % level_length, round((y + player.radius) / block_size), round((z + player.radius) / block_size)) &&
		is_pos_valid(int(round((x + player.radius) / block_size) + level_length) % level_length, round((y + player.radius) / block_size), round((z + player.radius) / block_size));
}
bool on_ground() {
	return !(is_pos_valid(int(round((player.x - player.radius) / block_size) + level_length) % level_length, round((player.y - player.radius) / block_size - 0.01), round((player.z - player.radius) / block_size)) &&
		is_pos_valid(int(round((player.x + player.radius) / block_size) + level_length) % level_length, round((player.y - player.radius) / block_size - 0.01), round((player.z - player.radius) / block_size)) &&
		is_pos_valid(int(round((player.x - player.radius) / block_size) + level_length) % level_length, round((player.y + player.radius) / block_size - 0.01), round((player.z - player.radius) / block_size)) &&
		is_pos_valid(int(round((player.x + player.radius) / block_size) + level_length) % level_length, round((player.y + player.radius) / block_size - 0.01), round((player.z - player.radius) / block_size)) &&
		is_pos_valid(int(round((player.x - player.radius) / block_size) + level_length) % level_length, round((player.y - player.radius) / block_size - 0.01), round((player.z + player.radius) / block_size)) &&
		is_pos_valid(int(round((player.x + player.radius) / block_size) + level_length) % level_length, round((player.y - player.radius) / block_size - 0.01), round((player.z + player.radius) / block_size)) &&
		is_pos_valid(int(round((player.x - player.radius) / block_size) + level_length) % level_length, round((player.y + player.radius) / block_size - 0.01), round((player.z + player.radius) / block_size)) &&
		is_pos_valid(int(round((player.x + player.radius) / block_size) + level_length) % level_length, round((player.y + player.radius) / block_size - 0.01), round((player.z + player.radius) / block_size)));
}
bool is_camera_pos_valid(float x, float y, float z) {
	return is_pos_valid(int(round((x - player.radius) / block_size) + level_length) % level_length, round((y - player.radius) / block_size), round((z - player.radius) / block_size));
}
void update_camera() {
	camera.position.x = player.x - 60;
	camera.position.z = player.z;
	camera.target.x = player.x;
	camera.target.z = player.z;
}
void die() {
	died = 1;
}
void restart() {
	camera = { 0 };
	camera.position = { -60.0f, 60.0f, 500.0f };
	camera.target = { 0.0f, block_size, 500.0f };
	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.fovy = 70.0f;
	camera.projection = CAMERA_PERSPECTIVE;
	player = ball{ 0,block_size,500,3};
	died = 0;
}
bool try_modify_pos(double dx, double dy, double dz) {
	float limit = 0.00001;
	float l = 0, r = 1, mid = (l + r) / 2.0;
	while (r - l > limit) {
		mid = (l + r) / 2.0;
		if (is_ball_pos_valid(player.x + dx * mid, player.y + dy * mid, player.z + dz * mid)) {
			l = mid;
		}
		else {
			r = mid - limit;
		}
	}
	player.x += dx * l;
	player.y += dy * l;
	player.z += dz * l;
	return l == 1;
}
void draw() {
	ClearBackground(WHITE);
	rlClearScreenBuffers(); 
	BeginMode3D(camera);
	player.draw();
	DrawMeshInstanced(cube, block_material, trans[level].data(), trans[level].size());
	EndMode3D();
	DrawFPS(0, 0);
	if (died) {
		DrawTextEx(font, "You Died", Vector2{ (float)GetRenderWidth()/2-64.0f*4,(float)GetRenderHeight()/2 - 64.0f }, 128, 0, RED);
	}
}
bool randcmp(int a, int b) {
	return rand() % 2 == 0;
}
void gen_level(int levl) {
	int last_jump = 0;
	int nw = 0;
	memset(mp[levl], 0, sizeof mp[levl]);
	switch (levl) {
	case 0:
		for (int i = 0; i < level_length; i++) {
			mp[0][i][1][22] = 1;
			mp[0][i][1][28] = 1;
			mp[0][i][2][22] = 1;
			mp[0][i][2][28] = 1;
			mp[0][i][3][22] = 1;
			mp[0][i][3][28] = 1;
			for (int k = 23; k <= 27; k++) {
				mp[0][i][0][k] = 1;
				mp[0][i][3][k] = mp[0][i][2][k] = mp[0][i][1][k] = (rand() % 3 == 0);
			}
		}
		nw = 25;
		for (int i = 0; i < 5; i++) {
			for (int k = 23; k <= 27; k++) {
				mp[0][i][1][k] = 0;
				mp[0][i][2][k] = 0;
				mp[0][i][3][k] = 0;
			}
		}
		for (int i = 5; i < level_length - 2; i++) {
			mp[0][i][1][nw] = 0;
			mp[0][i + 1][1][nw] = 0;
			mp[0][i + 2][1][nw] = 0;
			mp[0][i][2][nw] = 0;
			mp[0][i + 1][2][nw] = 0;
			mp[0][i + 2][2][nw] = 0;
			mp[0][i][3][nw] = 0;
			mp[0][i + 1][3][nw] = 0;
			mp[0][i + 2][3][nw] = 0;
			nw = max(23, min(27, nw + (rand() % 2) * 2 - 1));
		}
		for (int i = level_length - 2; i < level_length; i++) {
			for (int k = 23; k <= 27; k++) {
				mp[0][i][3][k] = mp[0][i][2][k] = mp[0][i][1][k] = 0;
			}
		}
		last_jump = 0;
		for (int i = 0; i < level_length; i++) {
			if (i > last_jump + 6 && i % 10 == 0) {
				last_jump = i;
				for (int k = 23; k <= 27; k++) {
					mp[0][i][1][k] = 1;
				}
			}
		}
		break;
	case 1:
		for (int i = 0; i < level_length; i++) {
			for (int k = 23; k <= 27; k++) {
				mp[1][i][0][k] = 1;
			}
		}
		last_jump = 0;
		for (int i = 0; i < level_length; i++) {
			if (i > last_jump + 6 && i % 10 == 0) {
				last_jump = i;
				for (int k = 23; k <= 27; k++) {
					mp[1][i][1][k] = 1;
					mp[1][i][2][k] = 1;
					mp[1][i][3][k] = 1;
					mp[1][i][4][k] = 1;
					mp[1][i][5][k] = 1;
				}
			}
		}
		break;
	case 2:
		int path[level_length + 1];
		for (int i = 0; i < level_length / 2; i++) {
			path[i] = 1;
		}
		for (int i = level_length / 2; i < level_length; i++) {
			path[i] = -1;
		}
		sort(path, path + level_length, randcmp);
		sort(path, path + level_length, randcmp);
		last_jump = 0;
		nw = 25;
		for (int i = 0; i < level_length - 2; i++) {
			mp[2][i][0][nw] = 1;
			mp[2][i + 1][0][nw] = 1;
			if (i > last_jump + 6 && (i +1)% 10 == 0) {
				last_jump = i;
				for (int k = 1; k <= 10; k++) {
					mp[2][i][k][nw] = 1;
				}
			}
			nw += path[i];
		}
		break;
	}
}
void regen_mesh() {
	for (int levl = 0; levl < level_count; levl++) {
		trans[levl].clear();
		for (int i = 0; i < level_length; i++) {
			for (int j = 0; j < 50; j++) {
				for (int k = 0; k < 50; k++) {
					if (mp[levl][i][j][k]) {
						trans[levl].push_back(MatrixTranslate(i * block_size, j * block_size, k * block_size));
					}
					if (mp[(levl + 1) % level_count][i][j][k]) {
						trans[levl].push_back(MatrixTranslate((i + level_length) * block_size, j * block_size, k * block_size));
					}
					if (mp[(levl - 1 + level_count) % level_count][i][j][k]) {
						trans[levl].push_back(MatrixTranslate((i - level_length) * block_size, j * block_size, k * block_size));
					}
				}
			}
		}
	}
}
void init() {
	srand(time(0));
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(1920, 1080, "Ball game");
	SetTargetFPS(60);
	rlEnableDepthTest();
	HideCursor();
	Shader shader = LoadShader("resources/lighting_instancing.vs",0);
	// Get shader locations
	shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvp");
	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
	Image image = LoadImage("resources/block.png");
	ballModel = LoadModel("resources/ball.obj");
	ballMesh = ballModel.meshes;
	Texture2D texture = LoadTextureFromImage(image);
	UnloadImage(image);
	cube = GenMeshCube(block_size, block_size, block_size);
	block_material = LoadMaterialDefault();
	block_material.shader = shader;
	block_material.maps[MATERIAL_MAP_DIFFUSE].texture = texture;
	ball_material = LoadMaterialDefault();
	ball_material.maps[MATERIAL_MAP_DIFFUSE].color = ORANGE;
	hair_material = LoadMaterialDefault();
	hair_material.maps[MATERIAL_MAP_DIFFUSE].color = BLACK;
	camera = { 0 };
	camera.position = { -60.0f, 60.0f, 500.0f };
	camera.target = { 0.0f, block_size, 500.0f };
	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.fovy = 70.0f;
	camera.projection = CAMERA_PERSPECTIVE;
	player = ball{ 0,block_size,500,3 };
	char text[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz01234567890`-=[]\\;',./~!@#$%^&*()_+{}|:\"<>?";
	int codepointsCount;
	int* codepoints = LoadCodepoints(text, &codepointsCount);
	font = LoadFontEx("resources/consola.ttf", 128, codepoints, codepointsCount);
	UnloadCodepoints(codepoints);
	rlEnableDepthTest();
	for (int lv = 0; lv < level_count; lv++) {
		gen_level(lv);
	}
	regen_mesh();
}
void update() {
	if (!died) {
		if (IsKeyDown(KEY_LEFT)||IsKeyDown(KEY_A)) {
			try_modify_pos(0, 0, -2);
		}
		if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
			try_modify_pos(0, 0, 2);
		}
		if (IsKeyDown(KEY_UP)) {
			camera.position.y++;
		}
		if (IsKeyDown(KEY_DOWN)) {
			camera.position.y--;
		}
		if (!on_ground()) {
			lst_gd++;
		}
		else {
			lst_gd = 0;
			pvel.y = 0;
		}
		if (lst_gd > 12) {
			pvel.y -= 0.1;
		}
		if (IsKeyDown(KEY_SPACE) && on_ground()) {
			pvel.y = 1.5;
		}
		try_modify_pos(pvel.x, pvel.y, pvel.z);
		player.x += 2+ IsKeyDown(KEY_W)- IsKeyDown(KEY_S);
		if (player.x >= (level_length - 1) * block_size) {
			player.x -= level_length * block_size;
			gen_level(level);
			regen_mesh();
			level++;
			if (level >= level_count) {
				level = 0;
			}
		}
		if (!is_ball_pos_valid(player.x, player.y, player.z)) {
			die();
		}
		if (player.y < -200) {
			die();
		}
		if (on_ground()) {
			ball_color = BLUE;
		}
		else {
			ball_color = ORANGE;
		}
	}
	else {
		if (IsKeyDown(KEY_ENTER)) {
			restart();
		}
	}
	update_camera();
	BeginDrawing();
	draw();
	EndDrawing();
}
int main(int argc, char** argv) {
	init();
	while (!WindowShouldClose()) {
		update();
	}
	CloseWindow();
	UnloadMaterial(block_material);
	UnloadMaterial(ball_material);
	UnloadMaterial(hair_material);
	UnloadMesh(cube);
	UnloadModel(ballModel);
	UnloadFont(font);
	return 0;
}