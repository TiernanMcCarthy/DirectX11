//GCamera.h
#pragma once
#include "GUtility.h"
#include <stdint.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
namespace GameData
{
	////////////////////////////////////////////////////////
	// Stores View and Projection matrices used by shaders
	// to translate 3D world into 2D screen surface
	// Camera can be moved and rotated. Also, user can change 
	// camera's target and position
	////////////////////////////////////////////////////////
	class Camera 	//https://www.gamedev.net/articles/programming/graphics/directx-11-c-game-camera-r2978/
	{

	public:
		//Construct at camera facing 0,0,0
		//place at 0,0,-1 so it can actuall see this vector
		Camera(void);
		//Create Camera from another one
		Camera(const Camera& camera);
		//copy all paramaters
		Camera& operator=(const Camera& camera);
		~Camera(void) {};

	private:
		//Initialise the view matrix of the camera from its start position
		void initViewMatrix();

	public:
		//Initialise the perspective matrix
							//FOV Angle??						//Window dimensions
		void InitProjMatrix(const float angle, const float width, const float height, const float nearest, const float farhest);

		//Initialise Orthogonal projection matrix
		void InitProjMatrix(const float width, const float height, const float near_plane, const float far_plane);

		void OnResize(uint32_t new_width, uint32_t new_height); //On Window change resize to the correct new

		///////////////////////////////////////////////
	/*** View matrix transformation interfaces ***/
	///////////////////////////////////////////////
		//Move Camera
		void Move(XMFLOAT3  DirectVec);

		//Rotate
		void Rotate(XMFLOAT3 Axis, float Deg);
		//Set camera position
		void SetPos(XMFLOAT3& new_position);
		//Get Camera position coordinates

		const XMFLOAT3& Target() const {return mTarget;}
		//Get Camera's up vector
		const XMFLOAT3 Up() {return GMathf()

	}