//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

////////////////////////////////////////////////////////////////////////
//
// helixTool.cpp
//
// Description:
//     Interactive tool to draw a helix.
//     Uses OpenGL to draw a guideline for the helix.
//
//
// Steps in creating a tool command
//
// 1. Create a tool command class. 
//    Same process as an MPxCommand except define 2 methods
//    for interactive use: cancel, and finalize.
//    There is also an addition constructor MPxToolCommand(), which 
//    is called from your context when the command needs to be invoked.
////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <maya/MIOStream.h>
#include <math.h>

#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MEvent.h>
#include <maya/MGlobal.h>
#include <maya/M3dView.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MDagPath.h>

#include <maya/MPxContext.h>
#include <maya/MPxContextCommand.h>
#include <maya/MPxToolCommand.h> 
#include <maya/MToolsInfo.h>

#include <maya/MFnPlugin.h>
#include <maya/MFnNurbsCurve.h> 

#include <maya/MSyntax.h>
#include <maya/MArgParser.h>
#include <maya/MArgDatabase.h>
#include <maya/MCursor.h>

#include <maya/MGL.h>
#include <maya/MUIDrawManager.h>

#define PI 3.1415926

// TODO: Instructions 2. These are the flags we need to implement.
#define kPitchFlag			"-p"
#define kPitchFlagLong		"-pitch"
#define kRadiusFlag			"-r"
#define kRadiusFlagLong		"-radius"
#define kNumberCVsFlag		"-ncv"
#define kNumberCVsFlagLong	"-numCVs"
#define kUpsideDownFlag		"-ud"
#define kUpsideDownFlagLong	"-upsideDown"

/////////////////////////////////////////////////////////////
// The users tool command
/////////////////////////////////////////////////////////////

#define		NUMBER_OF_CVS		20

class helixTool : public MPxToolCommand
{
public:
	helixTool(); 
	virtual			~helixTool(); 
	static void*	creator();

	MStatus			doIt(const MArgList& args);
	MStatus			parseArgs(const MArgList& args);
	MStatus			redoIt();
	MStatus			undoIt();
	bool			isUndoable() const;
	MStatus			finalize();
	static MSyntax	newSyntax();

	void			setRadius(double newRadius);
	void			setPitch(double newPitch);
	void			setNumCVs(unsigned newNumCVs);
	void			setUpsideDown(bool newUpsideDown);

private:
	double			radius;     	// Helix radius
	double			pitch;      	// Helix pitch
	unsigned		numCV;			// Helix number of CVs
	bool			upDown;			// Helix upsideDown
	MDagPath		path;			// The dag path to the curve.
	// Don't save the pointer!
};


void* helixTool::creator()
{
	return new helixTool;
}

helixTool::~helixTool() {}

helixTool::helixTool()
{
	numCV = 20;
	upDown = false;
	setCommandString("helixToolCmd");
}

MSyntax helixTool::newSyntax()
{
	//TODO: Step 1. Create syntax here with MSyntax, make sure the parameters are all added with MSyntax::addFlag
}

MStatus helixTool::doIt(const MArgList &args)
	//
	// Description
	//     Sets up the helix parameters from arguments passed to the
	//     MEL command.
	//
{
	MStatus status;

	// TODO: Instructions 1. Call Argument parser before drawing
	status = parseArgs(args);

	if (MS::kSuccess != status)
		return status;

	return redoIt();
}

MStatus helixTool::parseArgs(const MArgList &args)
{
	//TODO: Step 2. Add parameter parser here. Use MArgDatabase::isFlagSet and MArgDatabase::getFlagArgument to update all parameters before drawing.
}	


MStatus helixTool::redoIt()
	//
	// Description
	//     This method creates the helix curve from the
	//     pitch and radius values
	//
{
	MStatus stat;

	const unsigned  deg     = 3;            // Curve Degree
	const unsigned  ncvs    = numCV;		// Number of CVs
	const unsigned  spans   = ncvs - deg;   // Number of spans
	const unsigned  nknots  = spans+2*deg-1;// Number of knots
	unsigned	    i;
	MPointArray		controlVertices;
	MDoubleArray	knotSequences;

	int upFactor;
	if (upDown) upFactor = -1;
	else upFactor = 1;

	// Set up cvs and knots for the helix
	//
	for (i = 0; i < ncvs; i++)
		controlVertices.append(MPoint(radius * cos((double) i),
		upFactor * pitch * (double) i, 
		radius * sin((double) i)));

	for (i = 0; i < nknots; i++)
		knotSequences.append((double) i);

	// Now create the curve
	//
	MFnNurbsCurve curveFn;

	curveFn.create(controlVertices, knotSequences, deg, 
		MFnNurbsCurve::kOpen, false, false, 
		MObject::kNullObj, &stat);

	if (!stat) {
		stat.perror("Error creating curve");
		return stat;
	}

	stat = curveFn.getPath( path );

	return stat;
}

MStatus helixTool::undoIt()
	//
	// Description
	//     Removes the helix curve from the model.
	//
{
	MStatus stat; 
	MObject transform = path.transform();
	stat = MGlobal::deleteNode( transform );
	return stat;
}

bool helixTool::isUndoable() const
	//
	// Description
	//     Set this command to be undoable.
	//
{
	return true;	
}

MStatus helixTool::finalize()
	//
	// Description
	//     Command is finished, construct a string for the command
	//     for journaling.
	//
{
	// TODO: Step 4. Finalize the command with MArgList, MArgList::AddArg (command, flag, parameter in order) and MPxToolCommand::doFinalize
}

void helixTool::setRadius(double newRadius)
{
	radius = newRadius;
}

void helixTool::setPitch(double newPitch)
{
	pitch = newPitch;
}

void helixTool::setNumCVs(unsigned newNumCVs)
{
	numCV = newNumCVs;
}

void helixTool::setUpsideDown(bool newUpsideDown)
{
	upDown = newUpsideDown;
}
///////////////////////////////////////////////////////////////////////
//
// The following routines are used to register/unregister
// the commands we are creating within Maya
//
///////////////////////////////////////////////////////////////////////
MStatus initializePlugin( MObject obj )
{
	MStatus status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "3.0", "Any");

	// Register the context creation command and the tool command 
	// that the helixContext will use.
	// 

	//TODO: Step 3. Register syntax function here
	status = plugin.registerCommand("helixToolCmd",
		helixTool::creator, // Syntax function here);
	if (!status) {
		status.perror("registerContextCommand");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus status;
	MFnPlugin plugin( obj );

	// Deregister the tool command
	//
	status = plugin.deregisterCommand("helixToolCmd");
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}

//TODO: I-------------------------------------Instructions------------------------------------------I
//TODO: S----------------------------------------Steps---------------------------------------------S
