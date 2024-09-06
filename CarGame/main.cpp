//change your default terminal application setting to console host 

#include <iostream>
#include <string>
using namespace std;


#include "olcConsoleGameEngine.h"

class OneLoneCoder_FormulaOLC : public olcConsoleGameEngine {

public:
	OneLoneCoder_FormulaOLC(){
		m_sAppName = L"Driving Game";
	}
private:
	float fCarPos = 0.0f;
	float fDistance = 0.0f;
	float fSpeed = 0.0f;


	float fCurvature = 0.0f;
	float fTrackCurvature = 0.0f;
	float fPlayerCurvature = 0.0f;
	float fTrackDistance = 0.0f;

	float fCurrentLapTime = 0.0f;

	vector<pair<float, float>> vecTrack; // curvature , distance
	list<float> listLapTimes; //list of lap times


protected:
	//called by olcConsoleGameEngine
	virtual bool OnUserCreate() {

		//create track
		vecTrack.push_back(make_pair(0.0f, 10.0f)); //start/finish line, curvature = 0, distance = 10
		vecTrack.push_back(make_pair(0.0f, 200.0f));
		vecTrack.push_back(make_pair(1.0f, 200.0f)); //curvature is 1
		vecTrack.push_back(make_pair(0.0f, 400.0f));
		vecTrack.push_back(make_pair(-1.0f, 100.0f));
		vecTrack.push_back(make_pair(0.0f, 200.0f));
		vecTrack.push_back(make_pair(-1.0f, 200.0f));
		vecTrack.push_back(make_pair(1.0f, 200.0f));
		vecTrack.push_back(make_pair(0.0f, 200.0f));
		vecTrack.push_back(make_pair(0.2f, 500.0f));
		vecTrack.push_back(make_pair(0.0f, 200.0f));

		// Calculate total track distance, so we can set lap times
		for (auto t : vecTrack) {
			fTrackDistance += t.second;
		}
		listLapTimes = { 0,0,0,0,0 }; //initalise list 
		fCurrentLapTime = 0.0f;

		return true;
	}

	//called by olcConsoleGameEngine
	virtual bool OnUserUpdate(float fElapsedTime) {

		if (m_keys[VK_UP].bHeld || m_keys['W'].bHeld) { //when key is pressed down
			fSpeed += 2.0f * fElapsedTime;
		}
		else { //when key is released
			fSpeed -= 1.0f * fElapsedTime;
		}

		//harder to turn at higher speeds (greater curvature = lower speed)
		if (m_keys[VK_LEFT].bHeld || m_keys['A'].bHeld) {
			fPlayerCurvature -= 0.7f * fElapsedTime * (1.0f - fSpeed / 2.0f);
		}

		if (m_keys[VK_RIGHT].bHeld || m_keys['D'].bHeld){
			fPlayerCurvature += 0.7f * fElapsedTime * (1.0f - fSpeed / 2.0f);
		}

		// If car curvature is too different to track curvature, car is off the track
		if (fabs(fPlayerCurvature - fTrackCurvature) >= 0.8f)
			fSpeed -= 5.0f * fElapsedTime; //slow down car


 
		// Clamp Speed
		if (fSpeed < 0.0f) {
			fSpeed = 0.0f;
		}
		if (fSpeed > 1.0f) {
			fSpeed = 1.0f;
		}

		// Move car along track according to car speed
		fDistance += (70.0f * fSpeed) * fElapsedTime;

		
		//get Point on track
		float fOffset = 0;
		int nTrackSection = 0;

		fCurrentLapTime += fElapsedTime;
		if (fDistance >= fTrackDistance) {
			fDistance -= fTrackDistance;
			listLapTimes.push_front(fCurrentLapTime);
			listLapTimes.pop_back(); //shows whatever time was there before
			fCurrentLapTime = 0.0f; //reset lap time when player reaches end
		}

		//Find position on track
		//iterate through the vector accumulating the distances
		while (nTrackSection < vecTrack.size() && fOffset <= fDistance) { 
			fOffset += vecTrack[nTrackSection].second;
			nTrackSection++;

		}

		//once index is found we can find out what section of the track we are in
		float fTargetCurvature = vecTrack[nTrackSection - 1].first;

		float ftrackCurveDiff = (fTargetCurvature - fCurvature) * fElapsedTime * fSpeed; //when speed = 0 , no change in curvature
		fCurvature += ftrackCurveDiff;

		// Accumulate track curvature
		fTrackCurvature += (fCurvature)*fElapsedTime * fSpeed;

		// Draw Sky - light blue and dark blue
		for (int y = 0; y < ScreenHeight() / 2; y++)
			for (int x = 0; x < ScreenWidth(); x++)
				Draw(x, y, y < ScreenHeight() / 4 ? PIXEL_HALF : PIXEL_SOLID, FG_DARK_BLUE);

		// Draw Scenery (hills are a sine wave where phase is adjusted by track curvature
		for (int x = 0; x < ScreenWidth(); x++)
		{
			int nHillHeight = (int)(fabs(sinf(x * 0.01f + fTrackCurvature) * 16.0f));
			for (int y = (ScreenHeight() / 2) - nHillHeight; y < ScreenHeight() / 2; y++)
				Draw(x, y, PIXEL_SOLID, FG_DARK_GREY);
		}


		//drawing the track (bottom half of screen)
		for (int y = 0; y < ScreenHeight() / 2; y++) {

			for (int x = 0; x < ScreenWidth(); x++) {

				// Perspective is used to modify the width of the track row segments
				float fPerspective = (float)y / (ScreenHeight() / 2.0f);


				float fMiddlePoint = 0.5f + fCurvature * powf((1.0f - fPerspective),3);
				float fRoadWidth = 0.1f + fPerspective * 0.8f; // Min 10% Max 90%
				float fClipWidth = fRoadWidth * 0.15f; //red and white lines on side of track

				fRoadWidth *= 0.5f; //road is symmetrical around middle point

				//check if car is on grass, cliping board or road
				int nLeftGrass = (fMiddlePoint - fRoadWidth - fClipWidth) * ScreenWidth();
				int nLeftClip = (fMiddlePoint - fRoadWidth) * ScreenWidth();
				int nRightClip = (fMiddlePoint + fRoadWidth) * ScreenWidth();
				int nRightGrass = (fMiddlePoint + fRoadWidth + fClipWidth) * ScreenWidth();

				int nRow = ScreenHeight() / 2 + y;

				//using sin wave where the phase is controlled by distance along the track
				//sin > 0 means grass is green
				//sin < 0 means grass is dark green
				int nGrassColour = sinf(20.0f * powf(1.0f - fPerspective, 3) + fDistance * 0.1f) > 0.0f ? FG_GREEN : FG_DARK_GREEN;
				int nClipColour = sinf(80.0f *  powf(1.0f - fPerspective, 2) + fDistance) > 0.0f ? FG_RED : FG_WHITE;

				// Start finish straight changes colour to show lap is finished
				int nRoadColour = (nTrackSection - 1) == 0 ? FG_WHITE : FG_GREY;

				//draw grass on left
				if (x >= 0 && x < nLeftGrass) {
					Draw(x, nRow, PIXEL_SOLID, nGrassColour);

				}
				//draw left clipping board
				if (x >= nLeftGrass && x < nLeftClip) {
					Draw(x, nRow, PIXEL_SOLID, nClipColour);
				}
				//draw road
				if (x >= nLeftClip && x < nRightClip) {
					Draw(x, nRow, PIXEL_SOLID, nRoadColour);
				}
				//draw right clipping board
				if (x >= nRightClip && x < nRightGrass) {
					Draw(x, nRow, PIXEL_SOLID, nClipColour);
				}
				//draw grass on right
				if (x >= nRightGrass && x < ScreenWidth()) {
					Draw(x, nRow, PIXEL_SOLID, nGrassColour);
				}

			}

		}

		//Draw car, if current accumulated track curvature is similar to accumulated player curvature
		//the car will remain in the middle
		fCarPos = fPlayerCurvature - fTrackCurvature;
		int nCarPos = ScreenWidth() / 2 + ((int)(ScreenWidth() * fCarPos) / 2.0) - 7; // Offset for sprite

		DrawStringAlpha(nCarPos, 80, L"   ||####||   " , FG_RED); //drawStringAlpha makes the spaces transparent
		DrawStringAlpha(nCarPos, 81, L"      ##      " , FG_RED);
		DrawStringAlpha(nCarPos, 82, L"     ####     " , FG_RED);
		DrawStringAlpha(nCarPos, 83, L"     ####     " , FG_RED);
		DrawStringAlpha(nCarPos, 84, L"|||  ####  |||", FG_RED);
		DrawStringAlpha(nCarPos, 85, L"|||########|||" , FG_RED);
		DrawStringAlpha(nCarPos, 86, L"|||  ####  |||" , FG_RED);

		// Draw Stats
		DrawString(0, 0, L"Distance: " + to_wstring(fDistance));
		DrawString(0, 1, L"Target Curvature: " + to_wstring(fCurvature));
		DrawString(0, 2, L"Player Curvature: " + to_wstring(fPlayerCurvature));
		DrawString(0, 3, L"Player Speed    : " + to_wstring(fSpeed));
		DrawString(0, 4, L"Track Curvature : " + to_wstring(fTrackCurvature));

		auto disp_time = [](float t) { //lamba function turning seconds into minutes

			int nMinutes = t / 60.0f;
			int nSeconds = t - (nMinutes * 60.0f);
			int nMilliSeconds = (t - (float)nSeconds) * 1000.0f;

			return to_wstring(nMinutes) + L"." + to_wstring(nSeconds) + L":" + to_wstring(nMilliSeconds);
		};

		DrawString(10, 8, disp_time(fCurrentLapTime));

		// Display last 5 lap times
		int j = 10;
		for (auto l : listLapTimes){
			DrawString(10, j, disp_time(l));
			j++;
		}


		return true;

	}



};


int main() {

	// Use olcConsoleGameEngine derived app
	OneLoneCoder_FormulaOLC game;
	game.ConstructConsole(160, 100, 8, 8);
	game.Start();

}