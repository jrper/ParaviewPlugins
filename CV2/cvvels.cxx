#include "cvvels.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkCell.h"
#include "vtkCellType.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkUnstructuredGrid.h"
#include "vtkSmartPointer.h"
#include "vtkDataObject.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTriangle.h"
#include "vtkTetra.h"
#include "vtkVertex.h"
#include "vtkQuad.h"
#include "vtkPolygon.h"
#include "vtkHexahedron.h"

vtkCxxRevisionMacro(cvvels, "$Revision: 0.0$");
vtkStandardNewMacro(cvvels);

cvvels::cvvels(){};
cvvels::~cvvels(){};

//vtkQuad* cvvels::make_quad(
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

int cvvels::RequestData(
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


  vtkCellData* cd=output->GetCellData();
  vtkPointData* opd=output->GetPointData();
  output->Allocate(0);
  vtkPointData* ipd=input->GetPointData();
  vtkFieldData* pd=(vtkFieldData *) input->GetPointData();

  opd->InterpolateAllocate(ipd);




  //  this->DebugOn();

  if (NC==0)
    {
      //      vtkDebugMacro(<<"NothingToExtract"<<NC<<NP);
      return 1;
    }  else {
    //    vtkDebugMacro(<<"Extracting Points" << NC);
  }
    vtkSmartPointer<vtkPoints> outpoints= vtkSmartPointer<vtkPoints>::New();
    outpoints->Allocate(4*NC);
  

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
	     ptsIds->Allocate(8);
	   vtkTriangle::TriangleCenter(pts->GetPoint(0),
				       pts->GetPoint(1),
				       pts->GetPoint(2),
				       center);
	   vtkVertex* myVertex=vtkVertex::New();

	   ptsIds->InsertNextId(outpoints->InsertNextPoint(
			 5.0/12.0*pts->GetPoint(0)[0]
			+5.0/12.0*pts->GetPoint(1)[0]
			+1.0/6.0 *pts->GetPoint(2)[0],
			 5.0/12.0*pts->GetPoint(0)[1]
			+5.0/12.0*pts->GetPoint(1)[1]
			+1.0/6.0 *pts->GetPoint(2)[1],
			 5.0/12.0*pts->GetPoint(0)[2]
			+5.0/12.0*pts->GetPoint(1)[2]
			 +1.0/6.0 *pts->GetPoint(2)[2]							   ));


	   ptsIds->InsertNextId(outpoints->InsertNextPoint(
			 5.0/12.0*pts->GetPoint(1)[0]
			+5.0/12.0*pts->GetPoint(2)[0]
			+1.0/6.0 *pts->GetPoint(0)[0],
			 5.0/12.0*pts->GetPoint(1)[1]
			+5.0/12.0*pts->GetPoint(2)[1]
			+1.0/6.0 *pts->GetPoint(0)[1],
			 5.0/12.0*pts->GetPoint(1)[2]
			+5.0/12.0*pts->GetPoint(2)[2]
			 +1.0/6.0 *pts->GetPoint(0)[2]							   ));

	   ptsIds->InsertNextId(outpoints->InsertNextPoint(
			 5.0/12.0*pts->GetPoint(2)[0]
			+5.0/12.0*pts->GetPoint(0)[0]
			+1.0/6.0 *pts->GetPoint(1)[0],
			 5.0/12.0*pts->GetPoint(2)[1]
			+5.0/12.0*pts->GetPoint(0)[1]
			+1.0/6.0 *pts->GetPoint(1)[1],
			 5.0/12.0*pts->GetPoint(2)[2]
			+5.0/12.0*pts->GetPoint(0)[2]
			 +1.0/6.0 *pts->GetPoint(1)[2]							   ));

	   myVertex->GetPointIds()->SetId(0,ptsIds->GetId(0));
	   output->InsertNextCell(myVertex->GetCellType(),
				  myVertex->GetPointIds());

	   myVertex->GetPointIds()->SetId(0,ptsIds->GetId(1));
	   output->InsertNextCell(myVertex->GetCellType(),
				  myVertex->GetPointIds());
	   
	   myVertex->GetPointIds()->SetId(0,ptsIds->GetId(2));
	   output->InsertNextCell(myVertex->GetCellType(),
				  myVertex->GetPointIds());

	   myVertex->Delete();


	   
	   {
	     double w[3]={5.0/12,5.0/12,1.0/6.0};
	     opd->InterpolatePoint(ipd,3*i,cell->GetPointIds(),w);
	    }
	   {
	     double w[3]={2.0/12,5.0/12,5.0/12.0};
	     opd->InterpolatePoint(ipd,3*i+1,cell->GetPointIds(),w);
	       }
	   {
	     double w[3]={5.0/12,2.0/12,5.0/12.0};
	     opd->InterpolatePoint(ipd,3*i+2,cell->GetPointIds(),w);
	    }

	   break;
	 }
	 case  VTK_QUADRATIC_TRIANGLE:
	   {
	     ptsIds->Allocate(19);
	   vtkTriangle::TriangleCenter(pts->GetPoint(0),
				       pts->GetPoint(1),
				       pts->GetPoint(2),
				       center);
	   vtkQuad* myQuad=vtkQuad::New();
	   vtkPolygon* myPoly=vtkPolygon::New();
	   myPoly->GetPointIds()->SetNumberOfIds(8);


	   // Point 0 = Point 0
	   // Point 1 = Point 1
	   // Point 2 = Point 2
	   // Point 3 = Point 3
	   // Point 4 = Point 4
	   // Point 5 = Point 5

	   ptsIds->InsertNextId(cell->GetPointIds()->GetId(0));
	   ptsIds->InsertNextId(cell->GetPointIds()->GetId(1));
	   ptsIds->InsertNextId(cell->GetPointIds()->GetId(2));
	   ptsIds->InsertNextId(cell->GetPointIds()->GetId(3));
	   ptsIds->InsertNextId(cell->GetPointIds()->GetId(4));
	   ptsIds->InsertNextId(cell->GetPointIds()->GetId(5));

	   // Point 6 = midpoint (0,3)

	   ptsIds->InsertNextId(outpoints->InsertNextPoint(
			0.5*pts->GetPoint(0)[0]+0.5*pts->GetPoint(3)[0],
			0.5*pts->GetPoint(0)[1]+0.5*pts->GetPoint(3)[1],
			0.5*pts->GetPoint(0)[2]+0.5*pts->GetPoint(3)[2]
							   ));

	   // Point 7 = midpoint (3,1)

	   ptsIds->InsertNextId(outpoints->InsertNextPoint(
		    0.5*pts->GetPoint(1)[0]+0.5*pts->GetPoint(3)[0],
		    0.5*pts->GetPoint(1)[1]+0.5*pts->GetPoint(3)[1],
		    0.5*pts->GetPoint(1)[2]+0.5*pts->GetPoint(3)[2]
							   ));
	   // Point 8 = midpoint (1,4)

	   ptsIds->InsertNextId(outpoints->InsertNextPoint(
			0.5*pts->GetPoint(1)[0]+0.5*pts->GetPoint(4)[0],
			0.5*pts->GetPoint(1)[1]+0.5*pts->GetPoint(4)[1],
			0.5*pts->GetPoint(1)[2]+0.5*pts->GetPoint(4)[2]
							   ));

	   // Point 9 = midpoint (4,2)

	   ptsIds->InsertNextId(outpoints->InsertNextPoint(
			0.5*pts->GetPoint(4)[0]+0.5*pts->GetPoint(2)[0],
			0.5*pts->GetPoint(4)[1]+0.5*pts->GetPoint(2)[1],
			0.5*pts->GetPoint(4)[2]+0.5*pts->GetPoint(2)[2]
							   ));

	   // Point 10 = midpoint (2,5)

	   ptsIds->InsertNextId(outpoints->InsertNextPoint(
		    0.5*pts->GetPoint(2)[0]+0.5*pts->GetPoint(5)[0],
		    0.5*pts->GetPoint(2)[1]+0.5*pts->GetPoint(5)[1],
		    0.5*pts->GetPoint(2)[2]+0.5*pts->GetPoint(5)[2]
				

	   // Point 11 = midpoint (5,0)
		    
			   ));
	   ptsIds->InsertNextId(outpoints->InsertNextPoint(
			0.5*pts->GetPoint(5)[0]+0.5*pts->GetPoint(0)[0],
			0.5*pts->GetPoint(5)[1]+0.5*pts->GetPoint(0)[1],
			0.5*pts->GetPoint(5)[2]+0.5*pts->GetPoint(0)[2]
							   ));

	   // Point 12 = midpoint (3,4) Internal

	   ptsIds->InsertNextId(outpoints->InsertNextPoint(
			0.5*pts->GetPoint(3)[0]+0.5*pts->GetPoint(4)[0],
			0.5*pts->GetPoint(3)[1]+0.5*pts->GetPoint(4)[1],
			0.5*pts->GetPoint(3)[2]+0.5*pts->GetPoint(4)[2]
							   ));
	   // Point 13 = midpoint (4,5) Internal

	   ptsIds->InsertNextId(outpoints->InsertNextPoint(
		    0.5*pts->GetPoint(4)[0]+0.5*pts->GetPoint(5)[0],
		    0.5*pts->GetPoint(4)[1]+0.5*pts->GetPoint(5)[1],
		    0.5*pts->GetPoint(4)[2]+0.5*pts->GetPoint(5)[2]
							   ));

	   // Point 14 = midpoint (5,3) Internal

	   ptsIds->InsertNextId(outpoints->InsertNextPoint(
			0.5*pts->GetPoint(5)[0]+0.5*pts->GetPoint(3)[0],
			0.5*pts->GetPoint(5)[1]+0.5*pts->GetPoint(3)[1],
			0.5*pts->GetPoint(5)[2]+0.5*pts->GetPoint(3)[2]
							   ));

	   // Point 15 = centre (0,3,5)

	   ptsIds->InsertNextId(outpoints->InsertNextPoint(
	     (pts->GetPoint(0)[0]+pts->GetPoint(3)[0]+pts->GetPoint(5)[0])/3.0,
	     (pts->GetPoint(0)[1]+pts->GetPoint(3)[1]+pts->GetPoint(5)[1])/3.0,
	     (pts->GetPoint(0)[2]+pts->GetPoint(3)[2]+pts->GetPoint(5)[2])/3.0
							   ));

	   // Point 16 = centre (1,3,4)

	   ptsIds->InsertNextId(outpoints->InsertNextPoint(
	     (pts->GetPoint(1)[0]+pts->GetPoint(3)[0]+pts->GetPoint(4)[0])/3.0,
	     (pts->GetPoint(1)[1]+pts->GetPoint(3)[1]+pts->GetPoint(4)[1])/3.0,
	     (pts->GetPoint(1)[2]+pts->GetPoint(3)[2]+pts->GetPoint(4)[2])/3.0
							   ));

	   // Point 17 = centre (2,4,5)

	   ptsIds->InsertNextId(outpoints->InsertNextPoint(
	     (pts->GetPoint(2)[0]+pts->GetPoint(4)[0]+pts->GetPoint(5)[0])/3.0,
	     (pts->GetPoint(2)[1]+pts->GetPoint(4)[1]+pts->GetPoint(5)[1])/3.0,
	     (pts->GetPoint(2)[2]+pts->GetPoint(4)[2]+pts->GetPoint(5)[2])/3.0
							   ));

	   // Point 18 = centre (3,4,5)

	   ptsIds->InsertNextId(outpoints->InsertNextPoint(
	     (pts->GetPoint(3)[0]+pts->GetPoint(4)[0]+pts->GetPoint(5)[0])/3.0,
	     (pts->GetPoint(3)[1]+pts->GetPoint(4)[1]+pts->GetPoint(5)[1])/3.0,
	     (pts->GetPoint(3)[2]+pts->GetPoint(4)[2]+pts->GetPoint(5)[2])/3.0
							   ));

	   myQuad->GetPointIds()->SetId(0,ptsIds->GetId(0));
	   myQuad->GetPointIds()->SetId(1,ptsIds->GetId(6));
	   myQuad->GetPointIds()->SetId(2,ptsIds->GetId(15));
	   myQuad->GetPointIds()->SetId(3,ptsIds->GetId(11));
	   output->InsertNextCell(myQuad->GetCellType(),
				  myQuad->GetPointIds());

	   myPoly->GetPointIds()->SetId(0,ptsIds->GetId(3));
	   myPoly->GetPointIds()->SetId(1,ptsIds->GetId(7));
	   myPoly->GetPointIds()->SetId(2,ptsIds->GetId(16));
	   myPoly->GetPointIds()->SetId(3,ptsIds->GetId(12));
	   myPoly->GetPointIds()->SetId(4,ptsIds->GetId(18));
	   myPoly->GetPointIds()->SetId(5,ptsIds->GetId(14));
	   myPoly->GetPointIds()->SetId(6,ptsIds->GetId(15));
	   myPoly->GetPointIds()->SetId(7,ptsIds->GetId(6));
	   output->InsertNextCell(myPoly->GetCellType(),
				  myPoly->GetPointIds());

	   myQuad->GetPointIds()->SetId(0,ptsIds->GetId(1));
	   myQuad->GetPointIds()->SetId(1,ptsIds->GetId(8));
	   myQuad->GetPointIds()->SetId(2,ptsIds->GetId(16));
	   myQuad->GetPointIds()->SetId(3,ptsIds->GetId(7));
	   output->InsertNextCell(myQuad->GetCellType(),
				  myQuad->GetPointIds());



	   myPoly->GetPointIds()->SetId(0,ptsIds->GetId(5));
	   myPoly->GetPointIds()->SetId(1,ptsIds->GetId(11));
	   myPoly->GetPointIds()->SetId(2,ptsIds->GetId(15));
	   myPoly->GetPointIds()->SetId(3,ptsIds->GetId(14));
	   myPoly->GetPointIds()->SetId(4,ptsIds->GetId(18));
	   myPoly->GetPointIds()->SetId(5,ptsIds->GetId(13));
	   myPoly->GetPointIds()->SetId(6,ptsIds->GetId(17));
	   myPoly->GetPointIds()->SetId(7,ptsIds->GetId(10));
	   output->InsertNextCell(myPoly->GetCellType(),
				  myPoly->GetPointIds());

	   


	   myPoly->GetPointIds()->SetId(0,ptsIds->GetId(4));
	   myPoly->GetPointIds()->SetId(1,ptsIds->GetId(9));
	   myPoly->GetPointIds()->SetId(2,ptsIds->GetId(17));
	   myPoly->GetPointIds()->SetId(3,ptsIds->GetId(13));
	   myPoly->GetPointIds()->SetId(4,ptsIds->GetId(18));
	   myPoly->GetPointIds()->SetId(5,ptsIds->GetId(12));
	   myPoly->GetPointIds()->SetId(6,ptsIds->GetId(16));
	   myPoly->GetPointIds()->SetId(7,ptsIds->GetId(8));
	   output->InsertNextCell(myPoly->GetCellType(),
				  myPoly->GetPointIds());

	   myQuad->GetPointIds()->SetId(0,ptsIds->GetId(2));
	   myQuad->GetPointIds()->SetId(1,ptsIds->GetId(10));
	   myQuad->GetPointIds()->SetId(2,ptsIds->GetId(17));
	   myQuad->GetPointIds()->SetId(3,ptsIds->GetId(9));
	   output->InsertNextCell(myQuad->GetCellType(),
				  myQuad->GetPointIds());


	   



	   myQuad->Delete();
	   myPoly->Delete();

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
  
  output->SetPoints(outpoints);

  

  //  outpoints->Delete();
  return 1;
}

int cvvels::RequestUpdateExtent(
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

  if (numPieces > 1);
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


int cvvels::FillInputPortInformation(int,vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkUnstructuredGrid");
  return 1;
}
