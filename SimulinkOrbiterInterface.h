#ifndef _SIMULINK_ORBITER_H
#define _SIMULINK_ORBITER_H

#include "sdk/Orbitersdk.h"

using namespace std;

typedef struct VESSEL_DATA {

	string relativeDistanceToTarget;
	string relativeVelocityToTarget;
	string angularVelocity;
	string rotation;
	OBJHANDLE dockStatus;
	string simTime;

	friend ostream& operator <<(ostream& os, VESSEL_DATA const& s)
	{
		return os << s.relativeDistanceToTarget << '|'
			<< s.relativeVelocityToTarget << '|'
			<< s.angularVelocity << '|'
			<< s.rotation << '|'
			<< s.dockStatus << '|'
			<< s.simTime;
	}

} VESSEL_DATA;

class SO_Interface {
	public:	

		string subscribeData();

		// Parsing string from socket and call required function
		// napr. thrust|1|0.5 - call setThrusterGroupLevel(1,0.5)
		void processComands(vector<string> command);

		// Test log
		void vesselInfo();

	protected:

		VESSEL2* getVesselReference();

		// =====================================================================
		// ============================== GET Data =============================
		// =====================================================================

		// RETURN: Data o sledovanom objekte v strukture
		VESSEL_DATA getVesselData(VESSEL2* pVessel , OBJHANDLE targeObject);

		string getRelativeDistanceToTarget(VESSEL2* pVessel, OBJHANDLE targeObject);

		string getRelativeVelocityToTarget(VESSEL2* pVessel, OBJHANDLE targeObject);

		string getAngularVelocity(VESSEL2* pVessel);

		// RETURN: Pitch,Roll,Yaw rotation [radians]
		string getRotation(VESSEL2* pVessel);

		string getDockStatus(VESSEL2* pVessel);

		string getSimTime();

		// =====================================================================
		// ============================== Set Data =============================
		// =====================================================================

		// PARAMS: params[1] - engine thrust
		//		   params[2] - group of RCS engines			
		//	THGROUP_MAIN,             main thrusters - 0
		//	THGROUP_RETRO,            retro thrusters - 1
		//	THGROUP_HOVER,            hover thrusters - 2 
		//	THGROUP_ATT_PITCHUP,      rotation: pitch up
		//	THGROUP_ATT_PITCHDOWN,    rotation: pitch down
		//	THGROUP_ATT_YAWLEFT,      rotation: yaw left
		//	THGROUP_ATT_YAWRIGHT,     rotation: yaw right
		//	THGROUP_ATT_BANKLEFT,     rotation: bank left
		//	THGROUP_ATT_BANKRIGHT,    rotation: bank right
		//	THGROUP_ATT_RIGHT,        translation: move right
		//	THGROUP_ATT_LEFT,         translation: move left
		//	THGROUP_ATT_UP,           translation: move up
		//	THGROUP_ATT_DOWN,         translation: move down
		//	THGROUP_ATT_FORWARD,      translation: move forward
		//	THGROUP_ATT_BACK,         translation: move back
		void setThrusterGroupLevel(VESSEL2* pVessel, vector<string> params);

};

#endif