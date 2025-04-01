#include "sdk.h"
#include <math.h>

Vector3 SDK::GetBoneWithRotation(uintptr_t mesh, int id)
{
	uint64_t boneArray = read<uint64_t>(mesh + 0x5C8);
	if (boneArray == 0) boneArray = read<uint64_t>(mesh + 0x5B8);

	//std::cout << boneArray << "\n"; //2232110166848
	//std::cout << 0x5C8 << "\n";

	FTransform bone = read<FTransform>(boneArray + (id * 0x60));
	//bone.Print();
	//Rotation (FQuat): (0.0533313, 0.703446, 0.01472, -0.708592)
	//Translation(Vector3) : (-8.89284, 11.0632, 150.903)
		//Scale(Vector3) : (1, 1, 1)
		//Rotation(FQuat) : (0, 0, 0, 1)
		//Translation(Vector3) : (0, 0, 0)
		//Scale(Vector3) : (1, 1, 1)
	FTransform componentToWorld = read<FTransform>(mesh + 0x1E0);
	D3DMATRIX matrix = MatrixMultiplication(bone.ToMatrixWithScale(), componentToWorld.ToMatrixWithScale());
	//std::cout << "GetBoneWithRotation x: " << matrix._41 << "\n";
	//std::cout << "GetBoneWithRotation y: " << matrix._42 << "\n";
	//std::cout << "GetBoneWithRotation z: " << matrix._43 << "\n";

	return Vector3(matrix._41, matrix._42, matrix._43);
}
struct CamewaDescwipsion
{
	Vector3 Location;
	Vector3 Rotation;
	float FieldOfView;
	char Useless[0x18];
};
Camera SDK::GetViewAngles() {
	CamewaDescwipsion camera;
	auto locationPointer = read<uintptr_t>(LocalPtrs::Gworld + 0x138);
	auto rotationPointer = read<uintptr_t>(LocalPtrs::Gworld + 0x148);

	struct Rotation
	{
		double a; //0x0000
		char pad_0008[24]; //0x0008
		double b; //0x0020
		char pad_0028[424]; //0x0028
		double c; //0x01D0
	};
	Rotation rotation;

	rotation = read<Rotation>(rotationPointer);

	camera.Location = read<Vector3>(locationPointer);
	camera.Rotation.x = asin(rotation.c) * (180.0 / M_PI);
	camera.Rotation.y = ((atan2(rotation.a * -1, rotation.b) * (180.0 / M_PI)) * -1) * -1;
	camera.FieldOfView = read<float>(LocalPtrs::PlayerController + 0x3AC) * 90.f;

	return { camera.Location, camera.Rotation, camera.FieldOfView };
}

Vector2 SDK::ProjectWorldToScreen(Vector3 WorldLocation)
{

	vCamera = SDK::GetViewAngles(); //get ur players newest view angles
	if (Debug::PrintLocations) {
		Util::Print3D("Location: ", vCamera.Location);
		Util::Print3D("Rotation: ", vCamera.Rotation);
		Util::Print2D("Fov: ", Vector2(vCamera.FieldOfView, 69));
	}

	D3DMATRIX tempMatrix = Matrix(vCamera.Rotation);

	Vector3 vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	Vector3 vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	Vector3 vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	Vector3 vDelta = WorldLocation - vCamera.Location;
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < 1.f)
		vTransformed.z = 1.f;

	return Vector2((Width / 2.0f) + vTransformed.x * (((Width / 2.0f) / tanf(vCamera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, (Height / 2.0f) - vTransformed.y * (((Width / 2.0f) / tanf(vCamera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z);
}
