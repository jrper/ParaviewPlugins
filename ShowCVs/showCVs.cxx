#include "showCVs.h"

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
#include "vtkTriangle.h"
#include "vtkTetra.h"
#include "vtkQuad.h"
#include "vtkHexahedron.h"
#include "vtkPolygon.h"
#include <iostream>
#include <map>



vtkCxxRevisionMacro(showCVs, "$Revision: 0.0$");
vtkStandardNewMacro(showCVs);

showCVs::showCVs(){};
showCVs::~showCVs(){};

//vtkQuad* test::make_quad(
//			 vtkIdlist* pts,
//			 vtkIdType[4] a)
//{ vtkQuad* newQuad=vtkUnstructuredGrid::Quad::New();
//  vtkIdList* quadIds=newQuad->GetPointIds();
//  for (j=0,j<5,j++)
//    {
//      quadIds->SetId(j,pts.GetId(a[j]));
//   }
//  return quadIds
//}

int showCVs::RequestData(
		      vtkInformation* vtkNotUsed(request),
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{
  vtkInformation* outInfo=outputVector->GetInformationObject(0);
  vtkUnstructuredGrid* output= vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT() ) );
  
  vtkInformation *inInfo=inputVector[0]->GetInformationObject(0);
  vtkUnstructuredGrid* input= vtkUnstructuredGrid::GetData(inputVector[0]);

  vtkIdType NC=input->GetNumberOfCells();
  vtkIdType NP=input->GetNumberOfPoints();
  vtkIdType NF;

  vtkIdTypeArray* faces;

  this->DebugOff();
  vtkCell* cell=input->GetCell(0);
  int NPointsOut=0;
  int discontinuous=0;



  if (NC==0)
    {
      //      vtkDebugMacro(<<"NothingToExtract"<<NC<<NP);
      return 1;
    }  else {
    //    vtkDebugMacro(<<"Extracting Points" << NC);
  }


    switch (cell->GetCellType())
    {
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
	  //vtkDebugMacro(<<"GetCell " << i);
	  vtkCell* cell=input->GetCell(i);
	  //     vtkDebugMacro(<<"Get Points ");
	  vtkPoints* pts=cell->GetPoints();
	  
	  vtkIdType NPP=pts->GetNumberOfPoints();
	  
	  vtkIdList* ptsIds=vtkIdList::New();

	  double center[3];
	  
	  switch (cell->GetCellType())
	    {
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

	

	  vtkIdType NFF=cell->GetNumberOfFaces();

	  for (vtkIdType k=0;k<NFF;k++)
	    {
	      vtkCell* face=cell->GetFace(k);
	    }


	   //	   output->InsertNextCell(VTK_QUAD,
	   //				  ptsIds);
  //	   output->InsertNextCell(VTK_QUAD,
  //				  ptsIds);

	  ptsIds->Delete();

	}
      

    } else {

      //      this->DebugOn();

      // Continuous, so we build the face positions over cells and then loop over points in the edge mesh: 

      vtkSmartPointer<vtkIdList> EdgeList=
	vtkSmartPointer<vtkIdList>::New();
      vtkSmartPointer<vtkIdList> PointList=
	vtkSmartPointer<vtkIdList>::New();

      std::map<std::pair<vtkIdType,vtkIdType>,vtkIdType> mymap;
      typedef std::pair<vtkIdType,vtkIdType> vtkPair;

      //      vtkDebugMacro(<<"Cell Count" << NC);

      for(vtkIdType i=0;i<NC;i++)
	{
	  vtkCell* cell=input->GetCell(i);
	  vtkPoints* pts=cell->GetPoints();
	  vtkIdList* ptIds=cell->GetPointIds();
	  switch (input->GetCellType(i))
	    {
	    case  VTK_TRIANGLE:
	      {

	      PointList->InsertId(i,outpoints->InsertNextPoint(
				     (cell->GetPoints()->GetPoint(0)[0]
		      	      +cell->GetPoints()->GetPoint(1)[0]
				      +cell->GetPoints()->GetPoint(2)[0])/3.0,
				     (cell->GetPoints()->GetPoint(0)[1]
		      	      +cell->GetPoints()->GetPoint(1)[1]
				      +cell->GetPoints()->GetPoint(2)[1])/3.0,
				     (cell->GetPoints()->GetPoint(0)[2]
		      	      +cell->GetPoints()->GetPoint(1)[2]
				      +cell->GetPoints()->GetPoint(2)[2])/3.0
							       ));
	      }
	      break;
	      case  VTK_QUADRATIC_TRIANGLE:
	      {


	      PointList->InsertId(i,outpoints->InsertNextPoint(
				     0.75*cell->GetPoints()->GetPoint(0)[0]
				      +0.25*cell->GetPoints()->GetPoint(1)[0],
				     0.75*cell->GetPoints()->GetPoint(0)[1]
		      	      +0.25*cell->GetPoints()->GetPoint(1)[1],
				     0.75*cell->GetPoints()->GetPoint(0)[2]
		      	      +0.25*cell->GetPoints()->GetPoint(1)[2]
							       ));

	      PointList->InsertId(i,outpoints->InsertNextPoint(
				     (0.25*cell->GetPoints()->GetPoint(0)[0]
				      +0.75*cell->GetPoints()->GetPoint(1)[0]),
				     (0.25*cell->GetPoints()->GetPoint(0)[1]
				      +0.75*cell->GetPoints()->GetPoint(1)[1]),
				     (0.25*cell->GetPoints()->GetPoint(0)[2]
				      +0.75*cell->GetPoints()->GetPoint(1)[2])
							       ));



	      PointList->InsertId(i,outpoints->InsertNextPoint(
				     (4.0*cell->GetPoints()->GetPoint(0)[0]
		      	      +cell->GetPoints()->GetPoint(1)[0]
				      +cell->GetPoints()->GetPoint(2)[0])/6.0,
				     (4.0*cell->GetPoints()->GetPoint(0)[1]
		      	      +cell->GetPoints()->GetPoint(1)[1]
				      +cell->GetPoints()->GetPoint(2)[1])/6.0,
				     (4.0*cell->GetPoints()->GetPoint(0)[2]
		      	      +cell->GetPoints()->GetPoint(1)[2]
				      +cell->GetPoints()->GetPoint(2)[2])/6.0
							       ));


	       PointList->InsertId(i,outpoints->InsertNextPoint(
				     (cell->GetPoints()->GetPoint(0)[0]
		      	      +4.0*cell->GetPoints()->GetPoint(1)[0]
				      +cell->GetPoints()->GetPoint(2)[0])/6.0,
				     (cell->GetPoints()->GetPoint(0)[1]
		      	      +4.0*cell->GetPoints()->GetPoint(1)[1]
				      +cell->GetPoints()->GetPoint(2)[1])/6.0,
				     (cell->GetPoints()->GetPoint(0)[2]
		      	      +4.0*cell->GetPoints()->GetPoint(1)[2]
				      +cell->GetPoints()->GetPoint(2)[2])/6.0
							       ));

	       PointList->InsertId(i,outpoints->InsertNextPoint(
				     (0.75*cell->GetPoints()->GetPoint(0)[0]
		      	      +0.25*cell->GetPoints()->GetPoint(2)[0]),
				     (0.75*cell->GetPoints()->GetPoint(0)[1]
		      	      +0.25*cell->GetPoints()->GetPoint(2)[1]),
				     (0.75*cell->GetPoints()->GetPoint(0)[2]
		      	      +0.25*cell->GetPoints()->GetPoint(2)[2])
							       ));

	       PointList->InsertId(i,outpoints->InsertNextPoint(
				     (0.75*cell->GetPoints()->GetPoint(1)[0]
		      	      +0.25*cell->GetPoints()->GetPoint(2)[0]),
				     (0.75*cell->GetPoints()->GetPoint(1)[1]
		      	      +0.25*cell->GetPoints()->GetPoint(2)[1]),
				     (0.75*cell->GetPoints()->GetPoint(1)[2]
		      	      +0.25*cell->GetPoints()->GetPoint(2)[2])
							       ));


	       PointList->InsertId(i,outpoints->InsertNextPoint(
				     (cell->GetPoints()->GetPoint(0)[0]
		      	      +cell->GetPoints()->GetPoint(1)[0]
				      +cell->GetPoints()->GetPoint(2)[0])/3.0,
				     (cell->GetPoints()->GetPoint(0)[1]
		      	      +cell->GetPoints()->GetPoint(1)[1]
				      +cell->GetPoints()->GetPoint(2)[1])/3.0,
				     (cell->GetPoints()->GetPoint(0)[2]
		      	      +cell->GetPoints()->GetPoint(1)[2]
				      +cell->GetPoints()->GetPoint(2)[2])/3.0
							       ));

	       PointList->InsertId(i,outpoints->InsertNextPoint(
				     (cell->GetPoints()->GetPoint(0)[0]
		      	      +cell->GetPoints()->GetPoint(1)[0]
				    +4.0*cell->GetPoints()->GetPoint(2)[0])/6.0,
				     (cell->GetPoints()->GetPoint(0)[1]
		      	      +cell->GetPoints()->GetPoint(1)[1]
				    +4.0*cell->GetPoints()->GetPoint(2)[1])/6.0,
				     (cell->GetPoints()->GetPoint(0)[2]
		      	      +4.0*cell->GetPoints()->GetPoint(1)[2]
				    +4.0*cell->GetPoints()->GetPoint(2)[2])/6.0
							       ));


	       	      PointList->InsertId(i,outpoints->InsertNextPoint(
				     (0.25*cell->GetPoints()->GetPoint(0)[0]
		      	      +0.75*cell->GetPoints()->GetPoint(2)[0]),
				     (0.25*cell->GetPoints()->GetPoint(0)[1]
		      	      +0.75*cell->GetPoints()->GetPoint(2)[1]),
				     (0.25*cell->GetPoints()->GetPoint(0)[2]
		      	      +0.75*cell->GetPoints()->GetPoint(2)[2])
							       ));

		      PointList->InsertId(i,outpoints->InsertNextPoint(
				     (0.25*cell->GetPoints()->GetPoint(1)[0]
		      	      +0.75*cell->GetPoints()->GetPoint(2)[0]),
				     (0.25*cell->GetPoints()->GetPoint(1)[1]
		      	      +0.75*cell->GetPoints()->GetPoint(2)[1]),
				     (0.25*cell->GetPoints()->GetPoint(1)[2]
		      	      +0.75*cell->GetPoints()->GetPoint(2)[2])
							       ));



	      }
	      break;
	    }
	}


      for(vtkIdType i=0;i<NC;i++)
	{
	  switch (input->GetCellType(i))
	    {
	    case  VTK_TRIANGLE:
	      cell=input->GetCell(i);
	      for (vtkIdType j=0; j<3;j++)
		{
		  vtkCell* cellEdge=cell->GetEdge(j);
		  vtkSmartPointer<vtkIdList> cellNeighbours=
		    vtkSmartPointer<vtkIdList>::New();
		  vtkSmartPointer<vtkIdList> cellUniqueNeighbours=
		    vtkSmartPointer<vtkIdList>::New();
	     
		  if (cellEdge->GetPointIds()->GetNumberOfIds()>0)
		    { 
		      vtkIdList* edgePoints=cellEdge->GetPointIds();
		      input->GetCellNeighbors(i,edgePoints,cellNeighbours);
		      for (vtkIdType k=0;k<cellNeighbours->GetNumberOfIds();k++)
			{
			  
			  if (cellNeighbours->GetId(k)>=0)
			    if (input->GetCellType(cellNeighbours->GetId(k)) == VTK_TRIANGLE)
			      {
				cellUniqueNeighbours->InsertUniqueId(cellNeighbours->GetId(k));
			      }
			}

		      if (cellUniqueNeighbours->GetNumberOfIds()==0 ||
			  cellUniqueNeighbours->GetId(0)>i)
			{
		   vtkIdType newPointId=outpoints->InsertNextPoint(
		   (input->GetPoints()->GetPoint(edgePoints->GetId(0))[0]
		    +input->GetPoints()->GetPoint(edgePoints->GetId(1))[0])/2.0,
		   (input->GetPoints()->GetPoint(edgePoints->GetId(0))[1]
		    +input->GetPoints()->GetPoint(edgePoints->GetId(1))[1])/2.0,
		   (input->GetPoints()->GetPoint(edgePoints->GetId(0))[2]
		    +input->GetPoints()->GetPoint(edgePoints->GetId(1))[2])/2.0
								      );
		
		  EdgeList->InsertId(3*i+j,newPointId);
		  if (cellUniqueNeighbours->GetNumberOfIds()==0){
		    mymap.insert(std::pair<vtkPair,vtkIdType>
				 (vtkPair(i,i),
				  newPointId));
		      } else {
		    vtkIdType k=cellUniqueNeighbours->GetId(0);
		    mymap.insert(std::pair<vtkPair,vtkIdType>
				 (vtkPair(i,k),
				  newPointId));
		    mymap.insert(std::pair<vtkPair,vtkIdType>
				 (vtkPair(k,i),
				  newPointId));
		      }	  
			} else {
		      }
		    }
		}
	    case VTK_QUADRATIC_TRIANGLE:
for (vtkIdType j=0; j<3;j++)
		{
		  vtkCell* cellEdge=cell->GetEdge(j);
		  vtkSmartPointer<vtkIdList> cellNeighbours=
		    vtkSmartPointer<vtkIdList>::New();
		  vtkSmartPointer<vtkIdList> cellUniqueNeighbours=
		    vtkSmartPointer<vtkIdList>::New();
	     
		  if (cellEdge->GetPointIds()->GetNumberOfIds()>0)
		    { 
		      vtkIdList* edgePoints=cellEdge->GetPointIds();
		      input->GetCellNeighbors(i,edgePoints,cellNeighbours);
		      for (vtkIdType k=0;k<cellNeighbours->GetNumberOfIds();k++)
			{
			  
			  if (cellNeighbours->GetId(k)>=0)
			    if (input->GetCellType(cellNeighbours->GetId(k)) == VTK_TRIANGLE)
			      {
				cellUniqueNeighbours->InsertUniqueId(cellNeighbours->GetId(k));
			      }
			}

		      if (cellUniqueNeighbours->GetNumberOfIds()==0 ||
			  cellUniqueNeighbours->GetId(0)>i)
			{
		   vtkIdType newPointId=outpoints->InsertNextPoint(
		   (input->GetPoints()->GetPoint(edgePoints->GetId(0))[0]
		    +input->GetPoints()->GetPoint(edgePoints->GetId(1))[0])/2.0,
		   (input->GetPoints()->GetPoint(edgePoints->GetId(0))[1]
		    +input->GetPoints()->GetPoint(edgePoints->GetId(1))[1])/2.0,
		   (input->GetPoints()->GetPoint(edgePoints->GetId(0))[2]
		    +input->GetPoints()->GetPoint(edgePoints->GetId(1))[2])/2.0
								      );
		
		  EdgeList->InsertId(3*i+j,newPointId);
		  if (cellUniqueNeighbours->GetNumberOfIds()==0){
		    mymap.insert(std::pair<vtkPair,vtkIdType>
				 (vtkPair(i,i),
				  newPointId));
		      } else {
		    vtkIdType k=cellUniqueNeighbours->GetId(0);
		    mymap.insert(std::pair<vtkPair,vtkIdType>
				 (vtkPair(i,k),
				  newPointId));
		    mymap.insert(std::pair<vtkPair,vtkIdType>
				 (vtkPair(k,i),
				  newPointId));
		      }	  
			} else {
		      }
		    }
		}
	    }
	}

      for (vtkIdType i=0; i<NP;i++)
	{
	  vtkSmartPointer<vtkPolygon> polygon =
	    vtkSmartPointer<vtkPolygon>::New();
	  vtkSmartPointer<vtkIdList> cellIds=
	    vtkSmartPointer<vtkIdList>::New();
	   vtkSmartPointer<vtkIdList> visited=
	    vtkSmartPointer<vtkIdList>::New();
	  input->GetPointCells(i,cellIds);

	  vtkIdType m=cellIds->GetNumberOfIds();
	  vtkIdType M=cellIds->GetNumberOfIds();
	  vtkIdType n=0;
	  int flag=1;

	  for (vtkIdType j=0;(j<m)&&(flag>0);j++){
	    vtkIdType cell=cellIds->GetId(j);
	    vtkIdType ne=input->GetCell(cell)->GetNumberOfEdges();
	    for (vtkIdType k=0;k<ne;k++)
	      {
		vtkCell* edge=input->GetCell(cell)->GetEdge(k);
		vtkSmartPointer<vtkIdList> cellNeighbours=
		  vtkSmartPointer<vtkIdList>::New();
		if (edge->GetPointIds()->IsId(i)<0) continue; 
		input->GetCellNeighbors(cell,
					edge->GetPointIds(),
					cellNeighbours);
		if (cellNeighbours->GetNumberOfIds()>0) continue;
		vtkIdType PointId=EdgeList->GetId(3*cell+k);
		if (PointId>=0)
		  {
		    m-=1;
		    n=j;
		    polygon->GetPointIds()->InsertUniqueId(outpoints->InsertNextPoint(input->GetPoints()->GetPoint(i)));
		    polygon->GetPointIds()->InsertUniqueId(mymap[vtkPair(cell,cell)]);
		    flag=0;
		    break;
		  }
	      }
	  }

	  m+=1;

	  for (vtkIdType j=0;j<m;j++){
	    vtkIdType cell=cellIds->GetId(n);
	    visited->InsertNextId(n);
	    if(polygon->GetPointIds()->IsId(PointList->GetId(cell))<0)
	      polygon->GetPointIds()->InsertNextId(PointList->GetId(cell));
	    vtkIdType ne=input->GetCell(cell)->GetNumberOfEdges();
	    flag=1;
	    for (vtkIdType k=0;(k<ne)&&(flag>0);k++)
	      {
		vtkIdType ntest;
		vtkCell* edge=input->GetCell(cell)->GetEdge(k);
		vtkSmartPointer<vtkIdList> cellNeighbours=
		  vtkSmartPointer<vtkIdList>::New();
		if (edge->GetPointIds()->IsId(i)<0) continue;
		vtkIdType PointId=EdgeList->GetId(3*cell+k);
		input->GetCellNeighbors(cell,
					edge->GetPointIds(),
					cellNeighbours);
		if (cellNeighbours->GetNumberOfIds()==0 && j>0) 
		  polygon->GetPointIds()->InsertNextId(mymap[vtkPair(cell,cell)]);
		for (vtkIdType neigh=0;
		     neigh<cellNeighbours->GetNumberOfIds();neigh++)
		  {
		    ntest=cellIds->IsId(cellNeighbours->GetId(neigh));
		    //		    vtkDebugMacro( << i << " " << k << " " << visited->GetNumberOfIds() << " of " << m <<" "<< n << "->" << ntest);
		    // if (cellNeighbours->GetId(neigh)>cell)
		    //		    vtkDebugMacro(<< i <<" " << m-1 << " CellId " << cell << " NeighbourId " << ntest << " Sanity Check " 
		    //				  << input->GetCell(cellNeighbours->GetId(neigh))->GetPointIds()->GetId(0) << " "
		    //				  << input->GetCell(cellNeighbours->GetId(neigh))->GetPointIds()->GetId(1) << " "
		    //		  << input->GetCell(cellNeighbours->GetId(neigh))->GetPointIds()->GetId(2)  << " " 
		    //		  << edge->GetPointIds()->IsId(i) );
		    if ( (ntest>=0) &( (visited->IsId(ntest)<0||(visited->IsId(ntest)==0&j==M))))
		      {
			//	vtkDebugMacro(<< "Accepted");
			polygon->GetPointIds()->InsertNextId(mymap[vtkPair(cell,cellIds->GetId(ntest))]);	
			n=ntest;
			flag=0;
			break;
		    }
		  }
		cellNeighbours->Reset();
	      }
	  }
	  output->InsertNextCell(polygon->GetCellType(),
	  			 polygon->GetPointIds());
	}
    }

    output->SetPoints(outpoints);

    
    vtkCellData* cd=output->GetCellData();
    vtkFieldData* pd=(vtkFieldData *) input->GetPointData();

  cd->ShallowCopy(pd);
  

  //  outpoints->Delete();
  return 1;
}

int showCVs::RequestUpdateExtent(
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


int showCVs::FillInputPortInformation(int,vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkUnstructuredGrid");
  return 1;
}
