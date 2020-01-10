
#include "OrbConnect.h"
#include "Utils.h"
#include <map>
#include <sstream>
#include "SimulinkOrbiterInterface.h"
#include <algorithm>
#include <iostream>

using namespace std;

 
char* TARGET_VESSEL_NAME = "ISS";

VESSEL2* SO_Interface::getVesselReference() {
	return (VESSEL2*)oapiGetVesselInterface(oapiGetFocusObject());
}

string SO_Interface::subscribeData() {

	// Reference to controlled object
	VESSEL2* pVessel;

	// reference to target object
	OBJHANDLE targeObject = NULL;

	// Get controlled object and save reference
	pVessel = getVesselReference();

	// Get target object and save reference
	targeObject = oapiGetObjectByName(TARGET_VESSEL_NAME);

	if (targeObject == NULL) {
		return "ERR: Target Vessel not exist? Please check subscribedData()";
	}

	// Get data
	VESSEL_DATA vesselData = getVesselData(pVessel, targeObject);

	// Save data as string
	// Operator << is overloaded in VESSEL_DATA struct
	stringstream vesselDataAsString;
	vesselDataAsString << vesselData;

	return vesselDataAsString.str();
}

// Test log
void SO_Interface::vesselInfo(){

	VESSEL2* pVessel;
	pVessel = (VESSEL2*)oapiGetVesselInterface(oapiGetFocusObject());

	int countOfThrusters = pVessel->GetThrusterCount();
	int mainThrusters = pVessel->GetGroupThrusterCount(THGROUP_MAIN);

	char buffer[100];
	sprintf_s(buffer, "Number of thrusters: %d \n Main thrusters: %d", countOfThrusters, mainThrusters);

	oapiWriteLog("=============================");
	oapiWriteLog(buffer);
	oapiWriteLog("=============================");

}

// ============================================================================
// ================= Local Function which call orbiter API ====================
// ============================================================================

VESSEL_DATA SO_Interface::getVesselData(VESSEL2* pVessel, OBJHANDLE targeObject) {

	VESSEL_DATA data;

	data.relativeDistanceToTarget = getRelativeDistanceToTarget(pVessel, targeObject);
	data.relativeVelocityToTarget = getRelativeVelocityToTarget(pVessel, targeObject);
	data.angularVelocity = getAngularVelocity(pVessel);
	data.rotation = getRotation(pVessel);
	data.simTime = getSimTime();

	return data;
}

string SO_Interface::getRelativeDistanceToTarget(VESSEL2* pVessel, OBJHANDLE targeObject) {

	VECTOR3 position;

	// Orbiter API
	pVessel->GetRelativePos(targeObject, position);

	double vectorSize = vector3size(position);

	char buffer[100];
	sprintf_s(buffer, "%f", vectorSize);

	return buffer;
}

string SO_Interface::getRelativeVelocityToTarget(VESSEL2* pVessel, OBJHANDLE targeObject) {

	VECTOR3 velocity;

	// Orbiter API
	pVessel->GetRelativeVel(targeObject, velocity);

	double vectorSize = vector3size(velocity);

	char buffer[100];
	sprintf_s(buffer, "%f", vectorSize);

	return buffer;
}

string SO_Interface::getRotation(VESSEL2* pVessel) {

	double pitch = pVessel->GetPitch();
	double yaw = pVessel->GetYaw();
	double roll = pVessel->GetBank();

	char buffer[256];
	sprintf_s(buffer, "%f,%f,%f", pitch, roll, yaw);

	return buffer;
}

string SO_Interface::getAngularVelocity(VESSEL2* pVessel) {

	VECTOR3 velocity;

	// Orbiter API
	pVessel->GetAngularVel(velocity);

	char buffer[256];
	sprintf_s(buffer, "%f,%f,%f", velocity.x, velocity.y, velocity.z);

	return buffer;
}

string SO_Interface::getDockStatus(VESSEL2* pVessel) {

	// Orbiter API
	UINT status = pVessel->DockingStatus(0);

	char buffer[10];
	sprintf_s(buffer, "%d", status);

	return buffer;
}

string SO_Interface::getSimTime() {

	// Orbiter API
	double time = oapiGetSysTime();

	char buffer[100];
	sprintf_s(buffer, "%f", time);

	return buffer;
}

// =====================================================================
// ============================== Set Data =============================
// =====================================================================

void SO_Interface::setThrusterGroupLevel(VESSEL2* pVessel, vector<string> params) {

	int level = stoi(params[1]);
	THGROUP_TYPE thrusterGroup = (THGROUP_TYPE)stoi(params[2]);

	pVessel->SetThrusterGroupLevel(thrusterGroup, level);
}

void SO_Interface::processComands(string command) {


	try
	{		
		vector<string> commandParameters = stringSplit(command);
		vector<string> commands = {"THRUST"};

		if (commandParameters.size() < 1) {
			throw "Incorrect command pattern";
		}
		
		if (commandParameters.at(0) == commands.at(0)) {
			setThrusterGroupLevel(getVesselReference(), commandParameters);
		} else {
			throw "Command not exist";
		}

	}
	catch (string e)
	{
		char *err;
		sprintf(err, "%s", e);
		oapiWriteLog(err);
	}

}
