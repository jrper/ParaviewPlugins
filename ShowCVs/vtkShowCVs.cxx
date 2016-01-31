#include "vtkShowCVs.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkCell.h"
#include "vtkCellType.h"
#include "vtkCellData.h"
#include "vtkIdTypeArray.h"
#include "vtkCellArray.h"
#include "vtkPoints.h"
#include "vtkUnstructuredGrid.h"
#include "vtkSmartPointer.h"
#include "vtkDataObject.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkLine.h"
#include "vtkPolyLine.h"
#include "vtkTriangle.h"
#include "vtkQuadraticTriangle.h"
#include "vtkTetra.h"
#include "vtkQuad.h"
#include "vtkHexahedron.h"
#include "vtkPolygon.h"
#include "vtkDoubleArray.h"
#include "vtkGeometryFilter.h"
#include "vtkIntArray.h"
#include "vtkSortDataArray.h"
#include "vtkTrivialProducer.h"
#include "vtkMergePointFilter.h"
#include <vtkVersion.h>
#include <iostream>
#include <map>
#include <cmath>



vtkCxxRevisionMacro(vtkShowCVs, "$Revision: 0.5$");
vtkStandardNewMacro(vtkShowCVs);

double myAngle(double x[3]) {
return atan2(x[0],x[1]);
}

double myInternalAngle(vtkCell* cell,int id1,int id2,int id3) {
  double p1[3], p2[3], p3[3];
  
  cell->GetPoints()->GetPoint(id1,p1);
  cell->GetPoints()->GetPoint(id2,p2);
  cell->GetPoints()->GetPoint(id3,p3);

  double dot=0.0, n2=0.0,n3=0.0;

  for (int i=0; i<3; i++) {
    p2[i]=p2[i]-p1[i];
    p3[i]=p3[i]-p1[i];
    dot += p2[i]*p3[i];
    n2 += p2[i]*p2[i];
    n3 += p3[i]*p3[i];
  }

  return acos(dot/sqrt(n2*n3));
 
}

void Average(vtkCell* cell,int id1,int id2, double out[3]) {
  double p1[3], p2[3];
  
  cell->GetPoints()->GetPoint(id1,p1);
  cell->GetPoints()->GetPoint(id2,p2);

  for (int i=0; i<3; i++) {
    out[i]=(p1[i]+p2[i])/2.0;
  }
}   

void Average(vtkCell* cell,int id1,int id2,int id3, double out[3]) {
  double p1[3], p2[3], p3[3];
  
  cell->GetPoints()->GetPoint(id1,p1);
  cell->GetPoints()->GetPoint(id2,p2);
  cell->GetPoints()->GetPoint(id3,p3);

  for (int i=0; i<3; i++) {
    out[i]=(p1[i]+p2[i]+p3[i])/3.0;
  }
}

void Average(vtkCell* cell,int id1,int id2,int id3,int id4, double out[3]) {
  double p1[3], p2[3], p3[3], p4[3];
  
  cell->GetPoints()->GetPoint(id1,p1);
  cell->GetPoints()->GetPoint(id2,p2);
  cell->GetPoints()->GetPoint(id3,p3);
  cell->GetPoints()->GetPoint(id4,p4);

  for (int i=0; i<3; i++) {
    out[i]=(p1[i]+p2[i]+p3[i]+p4[i])/4.0;
  }
} 


void insertFace(vtkCellArray* faces,vtkCell* cell, vtkIdTypeArray** PointList,int pt1, int pt2,
		int id1,int id2,int id3,int id4) {

  vtkSmartPointer<vtkQuad> quad= vtkSmartPointer<vtkQuad>::New();

  quad->GetPointIds()->SetId(0,id1);
  quad->GetPointIds()->SetId(1,id2);
  quad->GetPointIds()->SetId(2,id3);
  quad->GetPointIds()->SetId(3,id4);


  vtkIdType face_id=faces->InsertNextCell(quad);

  PointList[cell->GetPointId(pt1)]->InsertNextValue(face_id);
  PointList[cell->GetPointId(pt2)]->InsertNextValue(face_id);
}

void insertFace(vtkCellArray* faces,int pid, vtkIdTypeArray** PointList,int pt1,
		int id1,int id2,int id3,int id4) {

  vtkSmartPointer<vtkQuad> quad= vtkSmartPointer<vtkQuad>::New();

  quad->GetPointIds()->SetId(0,id1);
  quad->GetPointIds()->SetId(1,id2);
  quad->GetPointIds()->SetId(2,id3);
  quad->GetPointIds()->SetId(3,id4);


  vtkIdType face_id=faces->InsertNextCell(quad);

  PointList[pid]->InsertNextValue(face_id);
}

void MakeQuadratic(vtkCell* incell, vtkCell* outcell) {
  double x[3];
  for (int i=0;i<3;i++) {
    incell->GetPoints()->GetPoint(i,x);
    outcell->GetPoints()->SetPoint(i,x);
  }
  Average(incell,0,1,x);
  outcell->GetPoints()->SetPoint(3,x);
  Average(incell,1,2,x);
  outcell->GetPoints()->SetPoint(4,x);
  Average(incell,2,0,x);
  outcell->GetPoints()->SetPoint(5,x);
}

vtkShowCVs::vtkShowCVs(){
#ifndef NDEBUG
  this->DebugOn();
#endif
  this->Degree=-1;
  this->Continuity=0;
}
vtkShowCVs::~vtkShowCVs(){};

int vtkShowCVs::RequestData(
		      vtkInformation* vtkNotUsed(request),
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{
  vtkInformation* outInfo=outputVector->GetInformationObject(0);
  vtkUnstructuredGrid* output= vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT() ) );
  
  //  vtkInformation *inInfo=inputVector[0]->GetInformationObject(0);
  vtkUnstructuredGrid* input= vtkUnstructuredGrid::GetData(inputVector[0]);


  vtkSmartPointer<vtkMergePointFilter> mergeFilter= vtkSmartPointer<vtkMergePointFilter>::New();

  
  mergeFilter->SetContinuity(this->Continuity);
  mergeFilter->SetDegree(this->Degree);
  vtkDebugMacro(<<"Setting input" );
#if VTK_MAJOR_VERSION <= 5
  mergeFilter->SetInput(input);
#else
  mergeFilter->SetInputData(input);
#endif
  vtkDebugMacro(<<"Updating merge point filter. " );
  mergeFilter->Update();
  vtkDebugMacro(<<"Getting output. " );
  input = mergeFilter->GetOutput();

  vtkDebugMacro(<<input->GetNumberOfCells() );


  vtkIdType NC=input->GetNumberOfCells();
  vtkIdType NP=input->GetNumberOfPoints();
  vtkIdType NF;

  vtkIdTypeArray* faces;

  vtkCell* cell=input->GetCell(0);
  int NPointsOut=0;
  int discontinuous=0;



  if (NC==0)
    {
      vtkDebugMacro(<<"NothingToExtract"<<NC<<NP);
      return 1;
    }  else {
    vtkDebugMacro(<<"Extracting Points" << NC);
  }


    switch (cell->GetCellType())
    {
    case  VTK_LINE:
      {
	vtkDebugMacro(<<"Linear Mesh" );
	if (2*NC==NP){
	  discontinuous=-1;
	  NPointsOut=2*NC;
	} else {
	  vtkDebugMacro(<<"Continuous " );
	  discontinuous=1;
	  NPointsOut=NP+NC;
	}
      }
      break;
      case  VTK_QUADRATIC_EDGE:
      {
	vtkDebugMacro(<<"Linear Mesh" );
	if (2*NC==NP){
	  discontinuous=-1;
	  NPointsOut=4*NC;
	} else {
	  vtkDebugMacro(<<"Continuous " );
	  discontinuous=1;
	  NPointsOut=NP+2*NC;
	}
      }
      break;
    case  VTK_TRIANGLE:
      {
	vtkDebugMacro(<<"Triangular Mesh " );
	if (3*NC==NP){
	  discontinuous=-1;
	  NPointsOut=3*NC;
	} else {
	  vtkDebugMacro(<<"Continuous " );
	  discontinuous=1;
	  NPointsOut=3*NC;
	}
      }
      break;
    case  VTK_TETRA:
      {
	if (4*NC==NP){
	  discontinuous=-1;
	  NPointsOut=4*NC;
	} else {
	  discontinuous=1;
	  input->InitializeFacesRepresentation(NC);
	  faces=input->GetFaces();
	  NPointsOut=4*NC;
	}
      }
      break;
    case  VTK_QUADRATIC_TRIANGLE:
      {
	vtkDebugMacro(<<"Triangular Quadratic Mesh " );
	if (6*NC==NP){
	  discontinuous=-1;
	  NPointsOut=6*NC;
	} else {
	  vtkDebugMacro(<<"Continuous " );
	  discontinuous=1;
	  NPointsOut=6*NC;
	}
      }
      break; 
    }

    vtkSmartPointer<vtkPoints> outpoints= vtkSmartPointer<vtkPoints>::New();
    outpoints->Allocate(NPointsOut);


    output->Allocate(NP);
  
    
    if (discontinuous < 0) {

      for (vtkIdType j=0;j<NP;j++)
      {
	outpoints->InsertNextPoint(input->GetPoint(j));
      };
      
      for(vtkIdType i = 0;i<NC;i++)
	{
	  vtkDebugMacro(<<"GetCell " << i);
	  vtkCell* cell=input->GetCell(i);
	  vtkDebugMacro(<<"Get Points ");
	  vtkPoints* pts=cell->GetPoints();
	  
	  vtkIdType NPP=pts->GetNumberOfPoints();
	  
	  vtkIdList* ptsIds=vtkIdList::New();

	  double center[3];
	  
	  switch (cell->GetCellType())
	    {

	    case  VTK_LINE:
	      {
		ptsIds->Allocate(3);
		ptsIds->InsertNextId(cell->GetPointIds()->GetId(0));
		ptsIds->InsertNextId(cell->GetPointIds()->GetId(1));

		vtkLine* myLine=vtkLine::New();

		ptsIds->InsertNextId(outpoints->InsertNextPoint(
		  0.5*pts->GetPoint(0)[0]+0.5*pts->GetPoint(1)[0],
		  0.5*pts->GetPoint(0)[1]+0.5*pts->GetPoint(1)[1],
		  0.5*pts->GetPoint(0)[2]+0.5*pts->GetPoint(1)[2]
							   ));
		myLine->GetPointIds()->SetId(0,ptsIds->GetId(0));
		myLine->GetPointIds()->SetId(1,ptsIds->GetId(2));

		output->InsertNextCell(myLine->GetCellType(),
				       myLine->GetPointIds());

		myLine->GetPointIds()->SetId(0,ptsIds->GetId(1));

		output->InsertNextCell(myLine->GetCellType(),
				       myLine->GetPointIds());

		myLine->Delete();
	      }	
	      break;
	    case  VTK_QUADRATIC_EDGE:
	      {
		ptsIds->Allocate(4);
		ptsIds->InsertNextId(cell->GetPointIds()->GetId(0));
		ptsIds->InsertNextId(cell->GetPointIds()->GetId(1));

		vtkLine* myLine=vtkLine::New();

		ptsIds->InsertNextId(outpoints->InsertNextPoint(
		  0.5*pts->GetPoint(0)[0]+0.5*pts->GetPoint(2)[0],
		  0.5*pts->GetPoint(0)[1]+0.5*pts->GetPoint(2)[1],
		  0.5*pts->GetPoint(0)[2]+0.5*pts->GetPoint(2)[2]
							   ));
		ptsIds->InsertNextId(outpoints->InsertNextPoint(
		  0.5*pts->GetPoint(1)[0]+0.5*pts->GetPoint(2)[0],
		  0.5*pts->GetPoint(1)[1]+0.5*pts->GetPoint(2)[1],
		  0.5*pts->GetPoint(1)[2]+0.5*pts->GetPoint(2)[2]
							   ));
		myLine->GetPointIds()->SetId(0,ptsIds->GetId(0));
		myLine->GetPointIds()->SetId(1,ptsIds->GetId(2));

		output->InsertNextCell(myLine->GetCellType(),
				       myLine->GetPointIds());

		myLine->GetPointIds()->SetId(0,ptsIds->GetId(3));

		output->InsertNextCell(myLine->GetCellType(),
				       myLine->GetPointIds());

		myLine->GetPointIds()->SetId(1,ptsIds->GetId(1));

		output->InsertNextCell(myLine->GetCellType(),
				       myLine->GetPointIds());

		myLine->Delete();
	      }	
	      break;
	    case  VTK_TRIANGLE:
	   {
	     ptsIds->Allocate(6);
	     vtkTriangle::TriangleCenter(pts->GetPoint(0),
					 pts->GetPoint(1),
					 pts->GetPoint(2),
					 center);
	     vtkQuad* myQuad=vtkQuad::New();
	     
	     ptsIds->InsertNextId(cell->GetPointIds()->GetId(0));
	     ptsIds->InsertNextId(cell->GetPointIds()->GetId(1));
	     ptsIds->InsertNextId(cell->GetPointIds()->GetId(2));
	     
	     ptsIds->InsertNextId(outpoints->InsertNextPoint(
		  0.5*pts->GetPoint(0)[0]+0.5*pts->GetPoint(1)[0],
		  0.5*pts->GetPoint(0)[1]+0.5*pts->GetPoint(1)[1],
		  0.5*pts->GetPoint(0)[2]+0.5*pts->GetPoint(1)[2]
							   ));

	     ptsIds->InsertNextId(outpoints->InsertNextPoint(
		  0.5*pts->GetPoint(1)[0]+0.5*pts->GetPoint(2)[0],
		  0.5*pts->GetPoint(1)[1]+0.5*pts->GetPoint(2)[1],
		  0.5*pts->GetPoint(1)[2]+0.5*pts->GetPoint(2)[2]
							   ));
	     ptsIds->InsertNextId(outpoints->InsertNextPoint(
			0.5*pts->GetPoint(2)[0]+0.5*pts->GetPoint(0)[0],
			0.5*pts->GetPoint(2)[1]+0.5*pts->GetPoint(0)[1],
			0.5*pts->GetPoint(2)[2]+0.5*pts->GetPoint(0)[2]
							   ));

	     ptsIds->InsertNextId(outpoints->InsertNextPoint(
	      (pts->GetPoint(0)[0]+pts->GetPoint(1)[0]+pts->GetPoint(2)[0])/3.0,
	      (pts->GetPoint(0)[1]+pts->GetPoint(1)[1]+pts->GetPoint(2)[1])/3.0,
	      (pts->GetPoint(0)[2]+pts->GetPoint(1)[2]+pts->GetPoint(2)[2])/3.0
							   ));

	  
	     myQuad->GetPointIds()->SetId(0,ptsIds->GetId(0));
	     myQuad->GetPointIds()->SetId(1,ptsIds->GetId(3));
	     myQuad->GetPointIds()->SetId(2,ptsIds->GetId(6));
	     myQuad->GetPointIds()->SetId(3,ptsIds->GetId(5));
	     output->InsertNextCell(myQuad->GetCellType(),
				    myQuad->GetPointIds());

	     myQuad->GetPointIds()->SetId(0,ptsIds->GetId(1));
	     myQuad->GetPointIds()->SetId(1,ptsIds->GetId(4));
	     myQuad->GetPointIds()->SetId(2,ptsIds->GetId(6));
	     myQuad->GetPointIds()->SetId(3,ptsIds->GetId(3));
	     output->InsertNextCell(myQuad->GetCellType(),
				    myQuad->GetPointIds());

	     myQuad->GetPointIds()->SetId(0,ptsIds->GetId(2));
	     myQuad->GetPointIds()->SetId(1,ptsIds->GetId(5));
	     myQuad->GetPointIds()->SetId(2,ptsIds->GetId(6));
	     myQuad->GetPointIds()->SetId(3,ptsIds->GetId(4));
	     output->InsertNextCell(myQuad->GetCellType(),
				    myQuad->GetPointIds());

	     myQuad->Delete();

	     break;
	   }
	    case  VTK_TETRA:
	      {
		ptsIds->Allocate(15);
		vtkTetra::TetraCenter(pts->GetPoint(0),
				      pts->GetPoint(1),
				      pts->GetPoint(2),
				      pts->GetPoint(3),
				      center);
		
		vtkHexahedron* myHex=vtkHexahedron::New();
		
		ptsIds->InsertNextId(cell->GetPointIds()->GetId(0));
		ptsIds->InsertNextId(cell->GetPointIds()->GetId(1));
		ptsIds->InsertNextId(cell->GetPointIds()->GetId(2));
		ptsIds->InsertNextId(cell->GetPointIds()->GetId(3));


	   // Points 4-9 are the line midpoints 
	   // 4: 0-1
	   // 5: 1-2
	   // 6: 2-3
	   // 7: 3-0
	   // 8: 1-3
	   // 9: 0-2


		ptsIds->InsertNextId(outpoints->InsertNextPoint(
			0.5*pts->GetPoint(0)[0]+0.5*pts->GetPoint(1)[0],
			0.5*pts->GetPoint(0)[1]+0.5*pts->GetPoint(1)[1],
			0.5*pts->GetPoint(0)[2]+0.5*pts->GetPoint(1)[2]
							   ));

		ptsIds->InsertNextId(outpoints->InsertNextPoint(
		    0.5*pts->GetPoint(1)[0]+0.5*pts->GetPoint(2)[0],
		    0.5*pts->GetPoint(1)[1]+0.5*pts->GetPoint(2)[1],
		    0.5*pts->GetPoint(1)[2]+0.5*pts->GetPoint(2)[2]
							   ));
		ptsIds->InsertNextId(outpoints->InsertNextPoint(
		    0.5*pts->GetPoint(2)[0]+0.5*pts->GetPoint(3)[0],
		    0.5*pts->GetPoint(2)[1]+0.5*pts->GetPoint(3)[1],
		    0.5*pts->GetPoint(2)[2]+0.5*pts->GetPoint(3)[2]
							   ));

		ptsIds->InsertNextId(outpoints->InsertNextPoint(
		    0.5*pts->GetPoint(3)[0]+0.5*pts->GetPoint(0)[0],
		    0.5*pts->GetPoint(3)[1]+0.5*pts->GetPoint(0)[1],
		    0.5*pts->GetPoint(3)[2]+0.5*pts->GetPoint(0)[2]
							   ));

		ptsIds->InsertNextId(outpoints->InsertNextPoint(
			0.5*pts->GetPoint(3)[0]+0.5*pts->GetPoint(1)[0],
			0.5*pts->GetPoint(3)[1]+0.5*pts->GetPoint(1)[1],
			0.5*pts->GetPoint(3)[2]+0.5*pts->GetPoint(1)[2]
							   ));

		ptsIds->InsertNextId(outpoints->InsertNextPoint(
			0.5*pts->GetPoint(2)[0]+0.5*pts->GetPoint(0)[0],
			0.5*pts->GetPoint(2)[1]+0.5*pts->GetPoint(0)[1],
			0.5*pts->GetPoint(2)[2]+0.5*pts->GetPoint(0)[2]
							   ));



	   // Points 10-13 are the Triangle midpoints
	   // 10 : 0-1-2
	   // 11 : 3-0-1
	   // 12 : 2-3-0
	   // 13 : 1-2-3

		ptsIds->InsertNextId(outpoints->InsertNextPoint(
	     (pts->GetPoint(0)[0]+pts->GetPoint(1)[0]+pts->GetPoint(2)[0])/3.0,
	     (pts->GetPoint(0)[1]+pts->GetPoint(1)[1]+pts->GetPoint(2)[1])/3.0,
	     (pts->GetPoint(0)[2]+pts->GetPoint(1)[2]+pts->GetPoint(2)[2])/3.0
							   ));

		ptsIds->InsertNextId(outpoints->InsertNextPoint(
	     (pts->GetPoint(0)[0]+pts->GetPoint(1)[0]+pts->GetPoint(3)[0])/3.0,
	     (pts->GetPoint(0)[1]+pts->GetPoint(1)[1]+pts->GetPoint(3)[1])/3.0,
	     (pts->GetPoint(0)[2]+pts->GetPoint(1)[2]+pts->GetPoint(3)[2])/3.0
							   ));

		ptsIds->InsertNextId(outpoints->InsertNextPoint(
	     (pts->GetPoint(0)[0]+pts->GetPoint(3)[0]+pts->GetPoint(2)[0])/3.0,
	     (pts->GetPoint(0)[1]+pts->GetPoint(3)[1]+pts->GetPoint(2)[1])/3.0,
	     (pts->GetPoint(0)[2]+pts->GetPoint(3)[2]+pts->GetPoint(2)[2])/3.0
							   ));


		ptsIds->InsertNextId(outpoints->InsertNextPoint(
	     (pts->GetPoint(3)[0]+pts->GetPoint(1)[0]+pts->GetPoint(2)[0])/3.0,
	     (pts->GetPoint(3)[1]+pts->GetPoint(1)[1]+pts->GetPoint(2)[1])/3.0,
	     (pts->GetPoint(3)[2]+pts->GetPoint(1)[2]+pts->GetPoint(2)[2])/3.0
							   ));

	   
	   // Point 14 is the tet midpoint

		ptsIds->InsertNextId(outpoints->InsertNextPoint(
	     (pts->GetPoint(0)[0]+pts->GetPoint(1)[0]+pts->GetPoint(2)[0]
	      +pts->GetPoint(3)[0])/4.0,
	     (pts->GetPoint(0)[1]+pts->GetPoint(1)[1]+pts->GetPoint(2)[1]
	               +pts->GetPoint(3)[1])/4.0,
	     (pts->GetPoint(0)[2]+pts->GetPoint(1)[2]+pts->GetPoint(2)[2]
	               +pts->GetPoint(3)[2])/4.0
							   ));

	  
		myHex->GetPointIds()->SetId(0,ptsIds->GetId(0));
		myHex->GetPointIds()->SetId(1,ptsIds->GetId(4));
		myHex->GetPointIds()->SetId(2,ptsIds->GetId(10));
		myHex->GetPointIds()->SetId(3,ptsIds->GetId(9));
		myHex->GetPointIds()->SetId(4,ptsIds->GetId(7));
		myHex->GetPointIds()->SetId(5,ptsIds->GetId(11));
		myHex->GetPointIds()->SetId(6,ptsIds->GetId(14));
		myHex->GetPointIds()->SetId(7,ptsIds->GetId(12));
		output->InsertNextCell(myHex->GetCellType(),
				  myHex->GetPointIds());


		myHex->GetPointIds()->SetId(0,ptsIds->GetId(1));
		myHex->GetPointIds()->SetId(1,ptsIds->GetId(5));
		myHex->GetPointIds()->SetId(2,ptsIds->GetId(10));
		myHex->GetPointIds()->SetId(3,ptsIds->GetId(4));
		myHex->GetPointIds()->SetId(4,ptsIds->GetId(8));
		myHex->GetPointIds()->SetId(5,ptsIds->GetId(13));
		myHex->GetPointIds()->SetId(6,ptsIds->GetId(14));
		myHex->GetPointIds()->SetId(7,ptsIds->GetId(11));
		output->InsertNextCell(myHex->GetCellType(),
				       myHex->GetPointIds());


		myHex->GetPointIds()->SetId(0,ptsIds->GetId(2));
		myHex->GetPointIds()->SetId(1,ptsIds->GetId(9));
		myHex->GetPointIds()->SetId(2,ptsIds->GetId(10));
		myHex->GetPointIds()->SetId(3,ptsIds->GetId(5));
		myHex->GetPointIds()->SetId(4,ptsIds->GetId(6));
		myHex->GetPointIds()->SetId(5,ptsIds->GetId(12));
		myHex->GetPointIds()->SetId(6,ptsIds->GetId(14));
		myHex->GetPointIds()->SetId(7,ptsIds->GetId(13));
		output->InsertNextCell(myHex->GetCellType(),
				       myHex->GetPointIds());

	   

		myHex->GetPointIds()->SetId(0,ptsIds->GetId(11));
		myHex->GetPointIds()->SetId(1,ptsIds->GetId(14));
		myHex->GetPointIds()->SetId(2,ptsIds->GetId(12));
		myHex->GetPointIds()->SetId(3,ptsIds->GetId(7));
		myHex->GetPointIds()->SetId(4,ptsIds->GetId(8));
		myHex->GetPointIds()->SetId(5,ptsIds->GetId(13));
		myHex->GetPointIds()->SetId(6,ptsIds->GetId(6));
		myHex->GetPointIds()->SetId(7,ptsIds->GetId(3));
		output->InsertNextCell(myHex->GetCellType(),
				       myHex->GetPointIds());


		myHex->Delete();
		break;
	      }
	    case  VTK_QUADRATIC_TRIANGLE:
	   {
	     ptsIds->Allocate(13);
	     vtkTriangle::TriangleCenter(pts->GetPoint(0),
					 pts->GetPoint(1),
					 pts->GetPoint(2),
					 center);
	     vtkQuad* myQuad=vtkQuad::New();

	     vtkPolygon* myPoly=vtkPolygon::New();

	     myPoly->GetPointIds()->SetNumberOfIds(5);
	     
	     ptsIds->InsertNextId(cell->GetPointIds()->GetId(0));
	     ptsIds->InsertNextId(outpoints->InsertNextPoint(
		  0.75*pts->GetPoint(0)[0]+0.25*pts->GetPoint(1)[0],
		  0.75*pts->GetPoint(0)[1]+0.25*pts->GetPoint(1)[1],
		  0.75*pts->GetPoint(0)[2]+0.25*pts->GetPoint(1)[2]
							   ));
	     ptsIds->InsertNextId(outpoints->InsertNextPoint(
		  0.25*pts->GetPoint(0)[0]+0.75*pts->GetPoint(1)[0],
		  0.25*pts->GetPoint(0)[1]+0.75*pts->GetPoint(1)[1],
		  0.25*pts->GetPoint(0)[2]+0.75*pts->GetPoint(1)[2]
							   ));
	     ptsIds->InsertNextId(cell->GetPointIds()->GetId(1));

	     ptsIds->InsertNextId(outpoints->InsertNextPoint(
2.0/3.0*pts->GetPoint(0)[0]+1.0/6.0*pts->GetPoint(1)[0]+1.0/6.0*pts->GetPoint(2)[0],
2.0/3.0*pts->GetPoint(0)[1]+1.0/6.0*pts->GetPoint(1)[1]+1.0/6.0*pts->GetPoint(2)[1],
2.0/3.0*pts->GetPoint(0)[2]+1.0/6.0*pts->GetPoint(1)[2]+1.0/6.0*pts->GetPoint(2)[2]));

	     ptsIds->InsertNextId(outpoints->InsertNextPoint(
1.0/6.0*pts->GetPoint(0)[0]+2.0/3.0*pts->GetPoint(1)[0]+1.0/6.0*pts->GetPoint(2)[0],
1.0/6.0*pts->GetPoint(0)[1]+2.0/3.0*pts->GetPoint(1)[1]+1.0/6.0*pts->GetPoint(2)[1],
1.0/6.0*pts->GetPoint(0)[2]+2.0/3.0*pts->GetPoint(1)[2]+1.0/6.0*pts->GetPoint(2)[2]));

             ptsIds->InsertNextId(outpoints->InsertNextPoint(
		  0.75*pts->GetPoint(0)[0]+0.25*pts->GetPoint(2)[0],
		  0.75*pts->GetPoint(0)[1]+0.25*pts->GetPoint(2)[1],
		  0.75*pts->GetPoint(0)[2]+0.25*pts->GetPoint(2)[2]
							   ));


	     ptsIds->InsertNextId(outpoints->InsertNextPoint(
		  0.75*pts->GetPoint(1)[0]+0.25*pts->GetPoint(2)[0],
		  0.75*pts->GetPoint(1)[1]+0.25*pts->GetPoint(2)[1],
		  0.75*pts->GetPoint(1)[2]+0.25*pts->GetPoint(2)[2]
							   ));

	     ptsIds->InsertNextId(outpoints->InsertNextPoint(
1.0/3.0*pts->GetPoint(0)[0]+1.0/3.0*pts->GetPoint(1)[0]+1.0/3.0*pts->GetPoint(2)[0],
1.0/3.0*pts->GetPoint(0)[1]+1.0/3.0*pts->GetPoint(1)[1]+1.0/3.0*pts->GetPoint(2)[1],
1.0/3.0*pts->GetPoint(0)[2]+1.0/3.0*pts->GetPoint(1)[2]+1.0/3.0*pts->GetPoint(2)[2]));

	     ptsIds->InsertNextId(outpoints->InsertNextPoint(
1.0/6.0*pts->GetPoint(0)[0]+1.0/6.0*pts->GetPoint(1)[0]+2.0/3.0*pts->GetPoint(2)[0],
1.0/6.0*pts->GetPoint(0)[1]+1.0/6.0*pts->GetPoint(1)[1]+2.0/3.0*pts->GetPoint(2)[1],
1.0/6.0*pts->GetPoint(0)[2]+1.0/6.0*pts->GetPoint(1)[2]+2.0/3.0*pts->GetPoint(2)[2]));

	     ptsIds->InsertNextId(outpoints->InsertNextPoint(
		  0.25*pts->GetPoint(0)[0]+0.75*pts->GetPoint(2)[0],
		  0.25*pts->GetPoint(0)[1]+0.75*pts->GetPoint(2)[1],
		  0.25*pts->GetPoint(0)[2]+0.75*pts->GetPoint(2)[2]
							   ));

	     ptsIds->InsertNextId(outpoints->InsertNextPoint(
		  0.25*pts->GetPoint(1)[0]+0.75*pts->GetPoint(2)[0],
		  0.25*pts->GetPoint(1)[1]+0.75*pts->GetPoint(2)[1],
		  0.25*pts->GetPoint(1)[2]+0.75*pts->GetPoint(2)[2]
							   ));

	     ptsIds->InsertNextId(cell->GetPointIds()->GetId(2));
	  
	     myQuad->GetPointIds()->SetId(0,ptsIds->GetId(0));
	     myQuad->GetPointIds()->SetId(1,ptsIds->GetId(1));
	     myQuad->GetPointIds()->SetId(2,ptsIds->GetId(4));
	     myQuad->GetPointIds()->SetId(3,ptsIds->GetId(6));
	     output->InsertNextCell(myQuad->GetCellType(),
				    myQuad->GetPointIds());

	     myPoly->GetPointIds()->SetId(0,ptsIds->GetId(1));
	     myPoly->GetPointIds()->SetId(1,ptsIds->GetId(2));
	     myPoly->GetPointIds()->SetId(2,ptsIds->GetId(5));
	     myPoly->GetPointIds()->SetId(3,ptsIds->GetId(8));
	     myPoly->GetPointIds()->SetId(4,ptsIds->GetId(4));
	     output->InsertNextCell(myPoly->GetCellType(),
				    myPoly->GetPointIds());

	     myQuad->GetPointIds()->SetId(0,ptsIds->GetId(2));
	     myQuad->GetPointIds()->SetId(1,ptsIds->GetId(3));
	     myQuad->GetPointIds()->SetId(2,ptsIds->GetId(7));
	     myQuad->GetPointIds()->SetId(3,ptsIds->GetId(5));
	     output->InsertNextCell(myQuad->GetCellType(),
				    myQuad->GetPointIds());

	     myPoly->GetPointIds()->SetId(0,ptsIds->GetId(10));
	     myPoly->GetPointIds()->SetId(1,ptsIds->GetId(6));
	     myPoly->GetPointIds()->SetId(2,ptsIds->GetId(4));
	     myPoly->GetPointIds()->SetId(3,ptsIds->GetId(8));
	     myPoly->GetPointIds()->SetId(4,ptsIds->GetId(9));
	     output->InsertNextCell(myPoly->GetCellType(),
				    myPoly->GetPointIds());

	     myPoly->GetPointIds()->SetId(0,ptsIds->GetId(7));
	     myPoly->GetPointIds()->SetId(1,ptsIds->GetId(11));
	     myPoly->GetPointIds()->SetId(2,ptsIds->GetId(9));
	     myPoly->GetPointIds()->SetId(3,ptsIds->GetId(8));
	     myPoly->GetPointIds()->SetId(4,ptsIds->GetId(5));
	     output->InsertNextCell(myPoly->GetCellType(),
				    myPoly->GetPointIds());

	     myQuad->GetPointIds()->SetId(0,ptsIds->GetId(9));
	     myQuad->GetPointIds()->SetId(1,ptsIds->GetId(11));
	     myQuad->GetPointIds()->SetId(2,ptsIds->GetId(12));
	     myQuad->GetPointIds()->SetId(3,ptsIds->GetId(10));
	     output->InsertNextCell(myQuad->GetCellType(),
				    myQuad->GetPointIds());


	     myQuad->Delete();
	     myPoly->Delete();
	     break;
	   }

	    }



	  ptsIds->Delete();

	}
      

    } else {

      // Continuous, this is more complicated.

      vtkSmartPointer<vtkMergePoints> mergePoints = vtkSmartPointer<vtkMergePoints>::New();
      vtkSmartPointer<vtkCellArray> faces = vtkSmartPointer<vtkCellArray>::New();
      vtkIdTypeArray *PointList[input->GetNumberOfPoints()];
      vtkIntArray *CellCentreList[input->GetNumberOfPoints()];
   
      mergePoints->SetTolerance(1.e-34);
      mergePoints->InitPointInsertion(outpoints, input->GetBounds());

      for(vtkIdType i=0;i<input->GetNumberOfPoints();i++){
	PointList[i] = vtkIdTypeArray::New();
	CellCentreList[i] = vtkIntArray::New();
	if (input->GetCell(0)->GetCellType()==VTK_LINE || input->GetCell(0)->GetCellType()==VTK_QUADRATIC_EDGE)
	  {
	    vtkIdType id;
	    mergePoints->InsertUniquePoint(input->GetPoints()->GetPoint(i),id);
	    PointList[i]->Allocate(3);
	    PointList[i]->InsertNextValue(-1);
	    PointList[i]->InsertNextValue(id);
	  }
      }

      vtkDebugMacro(<<"Cell Count" << NC);

      for(vtkIdType i=0;i<NC;i++)
	{
	  vtkCell* cell=input->GetCell(i);
	  vtkPoints* pts=cell->GetPoints();
	  vtkIdList* ptIds=cell->GetPointIds();
	  double x[3];
	  vtkIdType id;
	  switch (cell->GetCellType())
	    {
	    case  VTK_LINE:
	      {
		Average(cell,0,1,x);
		mergePoints->InsertUniquePoint(x,id);
		for (int j=0; j<2; j++)
		  {
		    if (PointList[cell->GetPointId(j)]->GetValue(0)==-1) {
		      PointList[cell->GetPointId(j)]->SetValue(0,id);
		    } else {
		      PointList[cell->GetPointId(j)]->InsertNextValue(id);
		    }
		  }
	      }
	      break;
	    case  VTK_QUADRATIC_EDGE:
	      {
		int el[4]={0,2,2,1};
		for (int k=0; k<2; k++) {
		  Average(cell,el[2*k],el[2*k+1],x);
		  mergePoints->InsertUniquePoint(x,id);
		  for (int j=0; j<2; j++)
		    {
		      if (PointList[cell->GetPointId(el[2*k+j])]->GetValue(0)==-1) {
			PointList[cell->GetPointId(el[2*k+j])]->SetValue(0,id);
		      } else {
			PointList[cell->GetPointId(el[2*k+j])]->InsertNextValue(id);
		      }
		    }
		}
	      }
	      break;
	    case  VTK_TRIANGLE:
	      {


		// Centre Point
		Average(cell,0,1,2,x);
		mergePoints->InsertUniquePoint(x,id);
		for(vtkIdType j=0;j<3;j++) {
		  PointList[ptIds->GetId(j)]->InsertNextValue(id);
		  CellCentreList[ptIds->GetId(j)]->InsertNextValue(1);
		}
		Average(cell,0,1,x);
		mergePoints->InsertUniquePoint(x,id);
		PointList[ptIds->GetId(0)]->InsertNextValue(id);
		PointList[ptIds->GetId(1)]->InsertNextValue(id);
		CellCentreList[ptIds->GetId(0)]->InsertNextValue(0);
		CellCentreList[ptIds->GetId(1)]->InsertNextValue(0);
		Average(cell,1,2,x);
		mergePoints->InsertUniquePoint(x,id);
		PointList[ptIds->GetId(1)]->InsertNextValue(id);
		PointList[ptIds->GetId(2)]->InsertNextValue(id);
		CellCentreList[ptIds->GetId(1)]->InsertNextValue(0);
		CellCentreList[ptIds->GetId(2)]->InsertNextValue(0);
		Average(cell,2,0,x);
		mergePoints->InsertUniquePoint(x,id);
		PointList[ptIds->GetId(2)]->InsertNextValue(id);
		PointList[ptIds->GetId(0)]->InsertNextValue(id);
		CellCentreList[ptIds->GetId(2)]->InsertNextValue(0);
		CellCentreList[ptIds->GetId(0)]->InsertNextValue(0);
	      }
	      break;
	      case  VTK_QUADRATIC_TRIANGLE:
	      {
		int M[12] = {0,3,5,1,3,4,2,5,4,3,4,5}, *l;
		for (int k=0;k<4;k++) {
		  l=&(M[3*k]);
		  Average(cell,l[0],l[1],l[2],x);
		  mergePoints->InsertUniquePoint(x,id);
		  for(vtkIdType j=0;j<3;j++) {
		    PointList[ptIds->GetId(l[j])]->InsertNextValue(id);
		    CellCentreList[ptIds->GetId(l[j])]->InsertNextValue(1);
		  }
		  Average(cell,l[0],l[1],x);
		  mergePoints->InsertUniquePoint(x,id);
		  PointList[ptIds->GetId(l[0])]->InsertNextValue(id);
		  PointList[ptIds->GetId(l[1])]->InsertNextValue(id);
		  CellCentreList[ptIds->GetId(l[0])]->InsertNextValue(0);
		  CellCentreList[ptIds->GetId(l[1])]->InsertNextValue(0);
		  Average(cell,l[1],l[2],x);
		  mergePoints->InsertUniquePoint(x,id);
		  PointList[ptIds->GetId(l[1])]->InsertNextValue(id);
		  PointList[ptIds->GetId(l[2])]->InsertNextValue(id);
		  CellCentreList[ptIds->GetId(l[1])]->InsertNextValue(0);
		  CellCentreList[ptIds->GetId(l[2])]->InsertNextValue(0);
		  Average(cell,l[2],l[0],x);
		  mergePoints->InsertUniquePoint(x,id);
		  PointList[ptIds->GetId(l[2])]->InsertNextValue(id);
		  PointList[ptIds->GetId(l[0])]->InsertNextValue(id);
		  CellCentreList[ptIds->GetId(l[2])]->InsertNextValue(0);
		  CellCentreList[ptIds->GetId(l[0])]->InsertNextValue(0);
		}
	      }
	      break;
	    case  VTK_TETRA:
	      {
		vtkIdType id[11];
		
		Average(cell,0,1,2,3,x);
		mergePoints->InsertUniquePoint(x,id[0]);
		Average(cell,0,1,2,x);
		mergePoints->InsertUniquePoint(x,id[1]);
		Average(cell,1,2,3,x);
		mergePoints->InsertUniquePoint(x,id[2]);
		Average(cell,2,3,0,x);
		mergePoints->InsertUniquePoint(x,id[3]);
		Average(cell,3,0,1,x);
		mergePoints->InsertUniquePoint(x,id[4]);
		Average(cell,0,1,x);
		mergePoints->InsertUniquePoint(x,id[5]);
		Average(cell,1,2,x);
		mergePoints->InsertUniquePoint(x,id[6]);
		Average(cell,2,0,x);
		mergePoints->InsertUniquePoint(x,id[7]);
		Average(cell,0,3,x);
		mergePoints->InsertUniquePoint(x,id[8]);
		Average(cell,1,3,x);
		mergePoints->InsertUniquePoint(x,id[9]);
		Average(cell,2,3,x);
		mergePoints->InsertUniquePoint(x,id[10]);
		
		insertFace(faces,cell,PointList,0,1,
			   id[5],id[1],id[0],id[4]);
		insertFace(faces,cell,PointList,1,2,
			   id[6],id[1],id[0],id[2]);
		insertFace(faces,cell,PointList,2,0,
			   id[7],id[1],id[0],id[3]);
		insertFace(faces,cell,PointList,0,3,
			   id[8],id[4],id[0],id[3]);
		insertFace(faces,cell,PointList,1,3,
			   id[9],id[2],id[0],id[4]);
		insertFace(faces,cell,PointList,2,3,
			   id[10],id[3],id[0],id[2]);
		
	      }
	    break;
	    case  VTK_QUADRATIC_TETRA:
	      {
		vtkIdType id[22];
		int M[16] = {0,4,6,7,
			     1,4,5,8,
			     2,5,6,9,
			     3,7,8,9}, *l;
		

		for(int k=0;k<4;k++) {
		  l=&(M[4*k]);

		  Average(cell,l[0],l[1],l[2],l[3],x);
		  mergePoints->InsertUniquePoint(x,id[0]);
		  Average(cell,l[0],l[1],l[2],x);
		  mergePoints->InsertUniquePoint(x,id[1]);
		  Average(cell,l[1],l[2],l[3],x);
		  mergePoints->InsertUniquePoint(x,id[2]);
		  Average(cell,l[2],l[3],l[0],x);
		  mergePoints->InsertUniquePoint(x,id[3]);
		  Average(cell,l[3],l[0],l[1],x);
		  mergePoints->InsertUniquePoint(x,id[4]);
		  Average(cell,l[0],l[1],x);
		  mergePoints->InsertUniquePoint(x,id[5]);
		  Average(cell,l[1],l[2],x);
		  mergePoints->InsertUniquePoint(x,id[6]);
		  Average(cell,l[2],l[0],x);
		  mergePoints->InsertUniquePoint(x,id[7]);
		  Average(cell,l[0],l[3],x);
		  mergePoints->InsertUniquePoint(x,id[8]);
		  Average(cell,l[1],l[3],x);
		  mergePoints->InsertUniquePoint(x,id[9]);
		  Average(cell,l[2],l[3],x);
		  mergePoints->InsertUniquePoint(x,id[10]);
		
		  insertFace(faces,cell,PointList,l[0],l[1],
			     id[5],id[1],id[0],id[4]);
		  insertFace(faces,cell,PointList,l[1],l[2],
			     id[6],id[1],id[0],id[2]);
		  insertFace(faces,cell,PointList,l[2],l[0],
			     id[7],id[1],id[0],id[3]);
		  insertFace(faces,cell,PointList,l[0],l[3],
			     id[8],id[4],id[0],id[3]);
		  insertFace(faces,cell,PointList,l[1],l[3],
			     id[9],id[2],id[0],id[4]);
		  insertFace(faces,cell,PointList,l[2],l[3],
			     id[10],id[3],id[0],id[2]);
		}
		

		Average(cell,0,1,2,3,x);
		mergePoints->InsertUniquePoint(x,id[0]);
		Average(cell,0,1,2,x);
		mergePoints->InsertUniquePoint(x,id[1]);
		Average(cell,1,2,3,x);
		mergePoints->InsertUniquePoint(x,id[2]);
		Average(cell,2,3,0,x);
		mergePoints->InsertUniquePoint(x,id[3]);
		Average(cell,3,0,1,x);
		mergePoints->InsertUniquePoint(x,id[4]);
		Average(cell,4,5,x);
		mergePoints->InsertUniquePoint(x,id[5]);
		Average(cell,5,6,x);
		mergePoints->InsertUniquePoint(x,id[6]);
		Average(cell,6,4,x);
		mergePoints->InsertUniquePoint(x,id[7]);
		Average(cell,4,7,x);
		mergePoints->InsertUniquePoint(x,id[9]);
		Average(cell,4,8,x);
		mergePoints->InsertUniquePoint(x,id[10]);
		Average(cell,5,8,x);
		mergePoints->InsertUniquePoint(x,id[11]);
		Average(cell,5,9,x);
		mergePoints->InsertUniquePoint(x,id[12]);
		Average(cell,6,7,x);
		mergePoints->InsertUniquePoint(x,id[13]);
		Average(cell,4,6,7,x);
		mergePoints->InsertUniquePoint(x,id[14]);
		Average(cell,4,5,8,x);
		mergePoints->InsertUniquePoint(x,id[15]);
		Average(cell,5,6,9,x);
		mergePoints->InsertUniquePoint(x,id[16]);
		Average(cell,7,8,9,x);
		mergePoints->InsertUniquePoint(x,id[17]);
		Average(cell,7,8,x);
		mergePoints->InsertUniquePoint(x,id[18]);
		Average(cell,8,9,x);
		mergePoints->InsertUniquePoint(x,id[19]);
		Average(cell,6,9,x);
		mergePoints->InsertUniquePoint(x,id[20]);
		Average(cell,7,9,x);
		mergePoints->InsertUniquePoint(x,id[21]);

		insertFace(faces,cell,PointList,4,5,
			     id[5],id[1],id[0],id[15]);
		insertFace(faces,cell,PointList,4,6,
			     id[7],id[1],id[0],id[14]);
		insertFace(faces,cell,PointList,5,6,
			     id[6],id[1],id[0],id[16]);
		insertFace(faces,cell,PointList,4,8,
			     id[10],id[4],id[0],id[15]);
		insertFace(faces,cell,PointList,4,7,
			     id[9],id[4],id[0],id[14]);
		insertFace(faces,cell,PointList,7,8,
			     id[18],id[4],id[0],id[17]);
		insertFace(faces,cell,PointList,5,8,
			     id[11],id[2],id[0],id[15]);
		insertFace(faces,cell,PointList,5,9,
			     id[12],id[2],id[0],id[16]);
		insertFace(faces,cell,PointList,8,9,
			     id[19],id[2],id[0],id[17]);
		insertFace(faces,cell,PointList,6,7,
			     id[13],id[3],id[0],id[14]);
		insertFace(faces,cell,PointList,6,9,
			     id[20],id[3],id[0],id[16]);
		insertFace(faces,cell,PointList,7,9,
			     id[21],id[3],id[0],id[17]);
		


	      }
	      break;
	      
	    
	    }
	}

      if (input->GetCellType(0)==VTK_TETRA ||
	  input->GetCellType(0)==VTK_QUADRATIC_TETRA) {

	vtkSmartPointer<vtkGeometryFilter> gf =
	      vtkSmartPointer<vtkGeometryFilter>::New();
	vtkSmartPointer<vtkPoints> locpoints =
	      vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkMergePoints> loc = vtkSmartPointer<vtkMergePoints>::New();

	loc->SetTolerance(1.e-34);
	loc->InitPointInsertion(locpoints, input->GetBounds());

	for (vtkIdType i=0; i<input->GetNumberOfPoints();i++) {
	  loc->InsertNextPoint(input->GetPoints()->GetPoint(i));
	} 
#if VTK_MAJOR_VERSION <= 5
        gf->SetInput(input);
#else
        gf->SetInputData(input);
#endif
	gf->Update();

	for (vtkIdType i=0; i<gf->GetOutput()->GetNumberOfCells();i++) {
	  vtkCell* cell=gf->GetOutput()->GetCell(i);
	  double x[3];
	  vtkIdType id[7], pid[3];

	  pid[0]=loc->IsInsertedPoint(cell->GetPoints()->GetPoint(0));
	  pid[1]=loc->IsInsertedPoint(cell->GetPoints()->GetPoint(1));
	  pid[2]=loc->IsInsertedPoint(cell->GetPoints()->GetPoint(2));

	  Average(cell,0,1,2,x);
	  mergePoints->InsertUniquePoint(x,id[0]);
	  Average(cell,0,1,x);
	  mergePoints->InsertUniquePoint(x,id[1]);
	  Average(cell,1,2,x);
	  mergePoints->InsertUniquePoint(x,id[2]);
	  Average(cell,2,0,x);
	  mergePoints->InsertUniquePoint(x,id[3]);

	  mergePoints->InsertUniquePoint(cell->GetPoints()->GetPoint(0),id[4]);
	  mergePoints->InsertUniquePoint(cell->GetPoints()->GetPoint(1),id[5]);
	  mergePoints->InsertUniquePoint(cell->GetPoints()->GetPoint(2),id[6]);

	  insertFace(faces,pid[0],PointList,0,
		     id[4],id[1],id[0],id[3]);
	  insertFace(faces,pid[1],PointList,1,
		     id[5],id[2],id[0],id[1]);
	  insertFace(faces,pid[2],PointList,2,
		     id[6],id[3],id[0],id[2]);
	}

      }

      for (vtkIdType i=0; i<NP;i++) {
	if (input->GetCellType(0)==VTK_LINE || input->GetCellType(0)==VTK_QUADRATIC_EDGE) {
	  vtkPolyLine *polyLine = vtkPolyLine::New();
	  
	  polyLine->GetPointIds()->SetNumberOfIds(PointList[i]->GetNumberOfTuples());
	  for (int j=0; j<PointList[i]->GetNumberOfTuples();j++) {
	    polyLine->GetPointIds()->SetId(j,PointList[i]->GetValue(i));
	  }

	  output->InsertNextCell(polyLine->GetCellType(),
				 polyLine->GetPointIds());
	  polyLine->Delete();
	}
	  if (input->GetCellType(0)==VTK_TRIANGLE ||
	      input->GetCellType(0)==VTK_QUADRATIC_TRIANGLE) {
	    vtkSmartPointer<vtkPolygon> polygon =
	      vtkSmartPointer<vtkPolygon>::New();
	    vtkSmartPointer<vtkDoubleArray> angleList =
	      vtkSmartPointer<vtkDoubleArray>::New();
	    vtkSmartPointer<vtkDoubleArray> angleCList =
	      vtkSmartPointer<vtkDoubleArray>::New();
	    vtkSmartPointer<vtkIdList> tempList =
	      vtkSmartPointer<vtkIdList>::New();
	    vtkSmartPointer<vtkIdList> tempCList =
	      vtkSmartPointer<vtkIdList>::New();

	    double x[3], p[3], v[3];
	    
	    input->GetPoints()->GetPoint(i,x);

	    for (int j=0;j<PointList[i]->GetNumberOfTuples();j++) {
	      vtkIdType id;
	      id=PointList[i]->GetValue(j);
	      int isCentrePoint=CellCentreList[i]->GetValue(j);
	      if (tempList->IsId(id)==-1) {
		tempList->InsertNextId(id);
		tempCList->InsertNextId(isCentrePoint);
		outpoints->GetPoint(id,p);
		vtkMath::Subtract(p,x,v);
		angleList->InsertNextValue(myAngle(v));
	      }
	    }
	  
	    angleCList->DeepCopy(angleList);
	    vtkSortDataArray::Sort(angleList,tempList);
	    vtkSortDataArray::Sort(angleCList,tempCList);

	    tempCList->InsertNextId(tempCList->GetId(0));
	    
	    for (int j=0;j<tempList->GetNumberOfIds();j++) {
	      polygon->GetPointIds()->InsertNextId(tempList->GetId(j));
	      if (tempCList->GetId(j)==tempCList->GetId(j+1)) {
		vtkIdType id;
		mergePoints->InsertUniquePoint(x,id);
		polygon->GetPointIds()->InsertNextId(id);
	      }
	    }							  

	    output->InsertNextCell(polygon->GetCellType(),
				   polygon->GetPointIds());
	  } else {
	    vtkSmartPointer<vtkIdList> fList =
	      vtkSmartPointer<vtkIdList>::New();
	    fList->InsertNextId(PointList[i]->GetNumberOfTuples());
	    for (int j=0;j<PointList[i]->GetNumberOfTuples();j++) {
	      vtkIdType npts;
	      vtkIdType *pts;
	      faces->GetCell(5*PointList[i]->GetValue(j),npts,pts);
	      fList->InsertNextId(4);
	      for (int k=0;k<4;k++) {
		fList->InsertNextId(pts[k]);
	      }
	    }
	    output->InsertNextCell(VTK_POLYHEDRON,
	    			   fList);

	  }
	  //	  output->SetCells(VTK_QUAD,faces);
	  PointList[i]->Delete();
	  CellCentreList[i]->Delete();
	}
    }
    output->SetPoints(outpoints);
    
    
    vtkCellData* cd=output->GetCellData();
    vtkFieldData* pd=(vtkFieldData *) input->GetPointData();

  cd->ShallowCopy(pd);
  

  //  outpoints->Delete();
  return 1;
}

int vtkShowCVs::RequestUpdateExtent(
			  vtkInformation* request,
			  vtkInformationVector** inputVector,
			  vtkInformationVector* outputVector)
 {

  vtkInformation* outInfo=outputVector->GetInformationObject(0);
  vtkInformation *inInfo=inputVector[0]->GetInformationObject(0);


  //  this->DebugOn();

  int piece, numPieces, ghostLevels;
  
  piece=outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER());
  numPieces=outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());
  ghostLevels=outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS());

  if (numPieces > 1)
  {
    ++ghostLevels;
  }

  vtkDebugMacro(<<"Running Update Extent"<<piece<<numPieces<<ghostLevels);

  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER(),piece);
  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES(),numPieces);
  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(),ghostLevels);
  inInfo->Set(vtkStreamingDemandDrivenPipeline::EXACT_EXTENT(),1);


  return 1;

};


int vtkShowCVs::FillInputPortInformation(int,vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkUnstructuredGrid");
  return 1;
}
